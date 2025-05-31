#include <iostream>
#include <sstream>
#include <map>
#include <vector>
#include <unordered_map>
#include <queue>
#include <cmath>
#include<limits>
#include <iomanip>
#include<string>
#include<set>
#include "tinyxml/tinyxml.h"
#include "jsoncpp/json/json.h"
#include <emscripten/bind.h> // 这个include在ide里面有可能会报错但是可以正常编译

#define MAX 99999999.99

using namespace emscripten;

using namespace std;
using neighbor = pair<string, double>;

// 改为了类似json的格式方便数据转换
Json::Value nodes;
Json::Value ways;
Json::Value path;
Json::Value names;
vector<string> ans;

struct Node
{
    string id;
    double lon, lat;
    map<string, string> tag;
    vector<neighbor> nei; 


    void print()
    {
        cout << "Node id " << id << endl;
        cout << "    lon " << lon << ", lat " << lat << endl;
    }
};

map<string, Node> cnodes;
map<string,string> namesave;

struct Way
{
    string id;
    vector<Node> nodes;
    map<string, string> tags;

    void print()
    {
        cout << "Way id " << id << endl;
        cout << "    tags: ";
        for (pair<string, string> tag : tags)
            cout << tag.first << ":" << tag.second << ", ";
        cout << endl;
    }
};
map<string, Way> cways;
double cal(Node &a, Node &b){
    double lat1=a.lat;
    double lat2=b.lat;
    double lon1=a.lon;
    double lon2=b.lon;

    const double R = 6371000.0; // Earth radius in kilometers

    double dlat = (lat2 - lat1) * (M_PI / 180.0);
    double dlon = (lon2 - lon1) * (M_PI / 180.0);

    double d = sin(dlat / 2.0) * sin(dlat / 2.0) +
               cos(lat1 * (M_PI / 180.0)) * cos(lat2 * (M_PI / 180.0)) *
               sin(dlon / 2.0) * sin(dlon / 2.0);

    double c = 2.0 * atan2(sqrt(d), sqrt(1.0 - d));

    return R * c;
}
// 初始化，和上一版内容基本相同
void load()
{
    vector<string> diji(string start, string desti);
    TiXmlDocument tinyXmlDoc("map1");
    tinyXmlDoc.LoadFile();
    TiXmlElement *root = tinyXmlDoc.RootElement();

    TiXmlElement *nodeElement = root->FirstChildElement("node");
    for (; nodeElement; nodeElement = nodeElement->NextSiblingElement("node"))
    {
        Json::Value node;
        node["id"] = nodeElement->Attribute("id");
        node["lon"] = nodeElement->Attribute("lon");
        node["lat"] = nodeElement->Attribute("lat");
        nodes[nodeElement->Attribute("id")] = node;

        Node cnode;
        cnode.id = nodeElement->Attribute("id");
        cnode.lon = stold(nodeElement->Attribute("lon"));
        cnode.lat = stold(nodeElement->Attribute("lat"));
        //cnode.print();
        cnodes[nodeElement->Attribute("id")] = cnode;
    }
    TiXmlElement *wayElement = root->FirstChildElement("way");
    for (; wayElement; wayElement = wayElement->NextSiblingElement("way"))
    {
        Json::Value way;
        way["id"] = wayElement->Attribute("id");
        Way cway;
        cway.id = wayElement->Attribute("id");

        vector<Node>::iterator it;
        double dis=0;
        Json::Value wayNodes;
        TiXmlElement *childNode = wayElement->FirstChildElement("nd");
        for (; childNode; childNode = childNode->NextSiblingElement("nd"))
        {
            string ref = childNode->Attribute("ref");
            wayNodes.append(nodes[ref]);
            if(cway.nodes.size()!=0){
                it=cway.nodes.end();
                it--;
                string tem=it->id;
                dis = cal(cnodes[ref],cnodes[tem]);
                cnodes[tem].nei.push_back({ref,dis});
                cnodes[ref].nei.push_back({tem,dis});
                it->nei.push_back({ref,dis});
            }
            cway.nodes.push_back(cnodes[ref]);
        }
        way["nodes"] = wayNodes;

        Json::Value wayTags;
        Json::Value namval;
        TiXmlElement *childTag = wayElement->FirstChildElement("tag");
        string name;
        string value;
        for (; childTag; childTag = childTag->NextSiblingElement("tag"))
        {
             name = childTag->Attribute("k");
            value = childTag->Attribute("v");
            wayTags[name] = value;
            cway.tags.insert({name,value});
        }
        way["tags"] = wayTags;
        if(!wayTags["name"].isNull()){
            names.append(wayTags["name"]);
            namesave[cway.tags["name"]]=wayElement->Attribute("id");
        }
        ways[wayElement->Attribute("id")] = way;
        cways[wayElement->Attribute("id")] = cway;
        //cway.print();
    }
    //diji("11074402281","11074402282");
    //diji("647265133","667339615");
}

// 这两个函数返回的是json格式的数据
string getNodes()
{
    Json::StreamWriterBuilder builder;
    string s = Json::writeString(builder, nodes);
    return s;
}

string getWays()
{
    Json::StreamWriterBuilder builder;
    string s = Json::writeString(builder, ways);
    return s;
}

string getans(){
     Json::StreamWriterBuilder builder;
    string s = Json::writeString(builder, path);
    return s;
}

string getnames(){
     Json::StreamWriterBuilder builder;
    string s = Json::writeString(builder, names);
    return s;
}

string mysearchhandle(string a){
   auto it1=namesave.find(a);
   if(it1==namesave.end()){
    cout<<"search error! There isn't the place you search"<<endl;
    return "1.1";
   }
   string start=cways[namesave[a]].nodes[0].id;
   Json::Value tmp;
   
   tmp.append(cnodes[start].lon);
   tmp.append(cnodes[start].lat);
   
   Json::StreamWriterBuilder builder;
   string s = Json::writeString(builder, tmp);
    return s;
}


string astar(double ala, double alo, double bla, double blo, string jud){
    vector<string> myastar(string start, string desti, double ala, double alo, double bla, double blo,string jud);
    Json::Value node;
    vector<string> save;

    double tmp;
    double min=MAX;
    string tar;

    node["id"]="0";
    node["lon"]=to_string(alo);
    node["lat"]=to_string(ala);
    nodes["0"]=node;
    
    Node cnode;
    cnode.id="0";
    cnode.lon=alo;
    cnode.lat=ala;
    auto cmp=[](neighbor&a, neighbor&b){
        return a.second<b.second;
    };

    priority_queue<neighbor,vector<neighbor>, decltype(cmp)> pq(cmp);

    for(auto ele:cways){
        if(ele.second.tags.find("highway")!=ele.second.tags.end()||ele.second.tags.find("oneway")!=ele.second.tags.end()){
            for(auto t: ele.second.nodes){
                tmp=cal(cnode,t);
                if(pq.size()<7){
                    pq.push({t.id,tmp});
                }
                else{
                    if(tmp<pq.top().second){
                        pq.pop();
                        pq.push({t.id,tmp});
                    }
                }   
            } 
        }
    }
    int s=int(pq.size());
    for(int i=0; i<s; i++){
        auto ele=pq.top();
        pq.pop();
        cnode.nei.push_back(ele);
    }

    
    if(cnodes.find("0")==cnodes.end()){
       cnodes.insert({"0",cnode}); 
    }
    else{
        cnodes["0"]=cnode;
    }

    node["id"]="1";
    node["lon"]=to_string(blo);
    node["lat"]=to_string(bla);
    nodes["1"]=node;

    cnode.id="1";
    cnode.lon=blo;
    cnode.lat=bla;
    min=MAX;
   
    for(auto& ele:cways){
        if(ele.second.tags.find("highway")!=ele.second.tags.end()){
            for(auto&t: ele.second.nodes){
                tmp=cal(cnode,t);
                if(pq.size()<5){
                    pq.push({t.id,tmp});
                }
                else{
                    if(tmp<pq.top().second){
                        pq.pop();
                        pq.push({t.id,tmp});
                    }
                }  
            }
        }
    }
    s=pq.size();
    for(int i=0; i<s; i++){
        auto ele=pq.top();
        pq.pop();

       cnodes[ele.first].nei.push_back({cnode.id,ele.second});
       save.push_back(ele.first);
    }
    
    cnode.nei.clear();
    if(cnodes.find("1")==cnodes.end()){
       cnodes.insert({"1",cnode}); 
    }
    else{
        cnodes["1"]=cnode;
    }
    //cout<<"0: "<<cnodes["0"].nei.size()<<endl;
    //cout<<"1: "<<save.size()<<endl;

    if(save.size()==0||cnodes["0"].nei.size()==0){
        cout<<"error"<<endl;
        ans.push_back("error");
        ans.push_back("error");
    }
    else{
        myastar("0","1",ala,alo,bla,blo,jud);
    }

    int i=int(ans.size()-1);

    Json::Value temp;

    for(; i>=0; i--){
        temp.append(nodes[ans[i]]);
    } 

    path["nodes"]=temp;

    for(auto ele:save){
        auto it=cnodes[ele].nei.begin();
        while((*it).first!="1"){
            it++;
        }
        cnodes[ele].nei.erase(it);
    }
    return getans();
}
vector<string> myastar(string start, string desti, double ala, double alo, double bla, double blo,string jud){
     ans.clear();

    map<string, double> dist;
    set<string> v;
    map<string, string> pre;
    auto cmp=[](pair<double,string> &a, pair<double,string> &b){
        return a.first > b.first;
    };

    priority_queue<pair<double,string>,vector<pair<double,string>>, decltype(cmp)> pq(cmp);
    dist.insert({start,0});
    v.insert(start);
    string a=start;
    double con=0.0;

    for(;a!=desti;){
     for(auto ele:cnodes[a].nei){
         if(v.find(ele.first)==v.end()){
          con=dist[a]+ele.second+cal(cnodes["1"],cnodes[ele.first]);
          dist[ele.first]=dist[a]+ele.second;
          pre[ele.first]=a;
          pq.push({con,ele.first}); 
         }
     }
     auto tem=pq.top();
     pq.pop();
     while(v.find(tem.second)!=v.end()){
        tem=pq.top();
        pq.pop();
     }
     a=tem.second;
     v.insert(a);
    }
   // cout<<"the whole distance is "<<dist[a]<<"m"<<endl;
    while(a!=start){
        ans.push_back(a);
        //cout<<"way is "<<a<<endl;
        a=pre[a];
    }
    ans.push_back(a);
    cout<<"A*:"<<endl;
    cout<<"The shortest way's lenth is "<<dist[desti]<<" m"<<endl;
    if(jud=="a")
    cout<<"By car will take "<< dist[desti]/1166.6<<" minutes"<<endl;
    if(jud=="b")
    cout<<"By bicycle will take "<< dist[desti]/266.6<<" minutes"<<endl;
    if(jud=="c")
    cout<<"By foot will take "<< dist[desti]/90.0<<" minutes"<<endl;
    return ans;
    
    return ans;
}

string dijistra_(double ala, double alo, double bla, double blo){
    string findNearestHighway(double lat1, double lon1);
    vector<string> diji(string start, string desti,double ala, double alo, double bla, double blo);
    string tmp1=findNearestHighway(ala,alo);
    string tmp2=findNearestHighway(bla, blo);
    diji(tmp1,tmp2,ala,alo,bla,blo);
    return getans();
}
vector<string> diji(string start, string desti, double ala, double alo, double bla, double blo){
    ans.clear();

    map<string, double> dist;
    set<string> v;
    map<string, string> pre;
    auto cmp=[](pair<double,string> &a, pair<double,string> &b){
        return a.first > b.first;
    };

    priority_queue<pair<double,string>,vector<pair<double,string>>, decltype(cmp)> pq(cmp);
    dist.insert({start,0});
    v.insert(start);
    string a=start;

    for(;a!=desti;){
     for(auto ele:cnodes[a].nei){
         if(v.find(ele.first)==v.end()){
          if(dist.find(ele.first)==dist.end()){
             dist[ele.first]=MAX;
          }
          if(ele.second+dist[a]<dist[ele.first]){
            dist[ele.first]=min(ele.second+dist[a],dist[ele.first]);
            pre[ele.first]=a;
          }
          pq.push({dist[ele.first],ele.first}); 
         }
     }
     auto tem=pq.top();
     pq.pop();
     while(v.find(tem.second)!=v.end()){
        tem=pq.top();
        pq.pop();
     }
     a=tem.second;
     v.insert(a);
    }
    
   

    while(a!=start){
        ans.push_back(a);
        //cout<<"way is "<<a<<endl;
        a=pre[a];
    }
    ans.push_back(a);
    int i=int(ans.size()-1);

    Json::Value temp;
    Json::Value t1,t2;
    t1["id"]="0";
    t1["lon"]=to_string(alo);
    t1["lat"]=to_string(ala);

    t2["id"]="1";
    t2["lon"]=to_string(blo);
    t2["lat"]=to_string(bla);

    temp.append(t1);
    for(; i>=0; i--){
        temp.append(nodes[ans[i]]);
    } 
    temp.append(t2);

    path["nodes"]=temp;

    cout<<"dijistra_basic:"<<endl;
    cout<<"The shortest way's lenth is "<<dist[desti]<<" m"<<endl;
    cout<<"By car will take "<< dist[desti]/1166.6<<" minutes"<<endl;
    return ans;
}

string dijistra_r(double ala, double alo, double bla, double blo,string jud){
    vector<string> dijir(string start, string desti, double ala, double alo, double bla, double blo,string jud);
    Json::Value node;
    vector<string> save;

    double tmp;
    double min=MAX;
    string tar;

    node["id"]="0";
    node["lon"]=to_string(alo);
    node["lat"]=to_string(ala);
    nodes["0"]=node;
    
    Node cnode;
    cnode.id="0";
    cnode.lon=alo;
    cnode.lat=ala;
    auto cmp=[](neighbor&a, neighbor&b){
        return a.second<b.second;
    };

    priority_queue<neighbor,vector<neighbor>, decltype(cmp)> pq(cmp);

    for(auto ele:cways){
        if(ele.second.tags.find("highway")!=ele.second.tags.end()||ele.second.tags.find("oneway")!=ele.second.tags.end()){
             //cout<<"nodes size is "<< ele.second.nodes.size()<<endl;
            for(auto t: ele.second.nodes){
                tmp=cal(cnode,t);
                if(pq.size()<10){
                    pq.push({t.id,tmp});
                }
                else{
                    if(tmp<pq.top().second){
                        pq.pop();
                        pq.push({t.id,tmp});
                    }
                }   
            } 
        }
    }
    int s=int(pq.size());
    for(int i=0; i<s; i++){
        auto ele=pq.top();
        pq.pop();
        cnode.nei.push_back(ele);
    }

    
    if(cnodes.find("0")==cnodes.end()){
       cnodes.insert({"0",cnode}); 
    }
    else{
        cnodes["0"]=cnode;
    }

    node["id"]="1";
    node["lon"]=to_string(blo);
    node["lat"]=to_string(bla);
    nodes["1"]=node;

    cnode.id="1";
    cnode.lon=blo;
    cnode.lat=bla;
    min=MAX;
    
    for(auto& ele:cways){
        if(ele.second.tags.find("highway")!=ele.second.tags.end()){
             //cout<<"nodes size is "<< ele.second.nodes.size()<<endl;
            for(auto&t: ele.second.nodes){
                tmp=cal(cnode,t);
                if(pq.size()<7){
                    pq.push({t.id,tmp});
                }
                else{
                    if(tmp<pq.top().second){
                        pq.pop();
                        pq.push({t.id,tmp});
                    }
                }  
            }
        }
    }
    s=pq.size();
    for(int i=0; i<s; i++){
        auto ele=pq.top();
        pq.pop();

       cnodes[ele.first].nei.push_back({cnode.id,ele.second});
       save.push_back(ele.first);
    }
    
    cnode.nei.clear();
    if(cnodes.find("1")==cnodes.end()){
       cnodes.insert({"1",cnode}); 
    }
    else{
        cnodes["1"]=cnode;
    }
   // cout<<"0: "<<cnodes["0"].nei.size()<<endl;
   // cout<<"1: "<<save.size()<<endl;

    if(save.size()==0||cnodes["0"].nei.size()==0){
        cout<<"error"<<endl;
        ans.push_back("error");
        ans.push_back("error");
    }
    else{
        vector<string> j=dijir("0","1",ala,alo,bla,blo,jud);
    }

    int i=int(ans.size()-1);

    Json::Value temp;

    for(; i>=0; i--){
        temp.append(nodes[ans[i]]);
    } 

    path["nodes"]=temp;
    
    //cout<<"way is "<<"0"<<endl;
    
    
    for(auto ele:save){
        auto it=cnodes[ele].nei.begin();
        while((*it).first!="1"){
            it++;
        }
        cnodes[ele].nei.erase(it);
    }
    return getans();
}
vector<string> dijir(string start, string desti, double ala, double alo, double bla, double blo,string jud){
    ans.clear();

    map<string, double> dist;
    set<string> v;
    map<string, string> pre;
    auto cmp=[](pair<double,string> &a, pair<double,string> &b){
        return a.first > b.first;
    };

    priority_queue<pair<double,string>,vector<pair<double,string>>, decltype(cmp)> pq(cmp);
    dist.insert({start,0});
    v.insert(start);
    string a=start;
    
    for(;a!=desti;){
     for(auto ele:cnodes[a].nei){
         if(v.find(ele.first)==v.end()){
          if(dist.find(ele.first)==dist.end()){
             dist[ele.first]=MAX;
          }
          if(ele.second+dist[a]<dist[ele.first]){
            dist[ele.first]=min(ele.second+dist[a],dist[ele.first]);
            pre[ele.first]=a;
          }
          pq.push({dist[ele.first],ele.first}); 
         }
     }
     auto tem=pq.top();
     pq.pop();
     while(v.find(tem.second)!=v.end()){
        tem=pq.top();
        pq.pop();
     }
     a=tem.second;
     v.insert(a);
    }
   

    while(a!=start){
        ans.push_back(a);
       // cout<<"way is "<<a<<endl;
        a=pre[a];
    }
    ans.push_back(a);
    cout<<"dijistra:"<<endl;
    cout<<"The shortest way's lenth is "<<dist[desti]<<" m"<<endl;
    if(jud=="a")
    cout<<"By car will take "<< dist[desti]/1166.6<<" minutes"<<endl;
    if(jud=="b")
    cout<<"By bicycle will take "<< dist[desti]/266.6<<" minutes"<<endl;
    if(jud=="c")
    cout<<"By foot will take "<< dist[desti]/90.0<<" minutes"<<endl;
    return ans;
}
// emscripten对于c++代码的处理，为了让前端能调用这些函数
EMSCRIPTEN_BINDINGS()
{
    emscripten::function("load", &load);
    emscripten::function("getNodes", &getNodes);
    emscripten::function("getWays", &getWays);
    emscripten::function("getans", &getans);
    emscripten::function("dijistra_",&dijistra_);
    emscripten::function( "dijistra_r",&dijistra_r);
    emscripten::function( "astar",&astar);
    emscripten::function( "getnames",&getnames);
    emscripten::function( "mysearchhandle",&mysearchhandle);
}

// Function to find the nearest highway between two points
string findNearestHighway(double lat1, double lon1)
{
    string nearestHighway="1111";

    double min=99999.0;
    double tmp;
    Node kita;
    kita.lat=lat1;
    kita.lon=lon1;

   // cout<<"size is "<<cways.size()<<endl;
    //cout<<"lat1 is "<<lat1<<" lon1 is "<<lon1<<endl;
    for(auto ele:cways){
        if(ele.second.tags.find("highway")!=ele.second.tags.end()){
             //cout<<"nodes size is "<< ele.second.nodes.size()<<endl;
            for(auto t: ele.second.nodes){
                tmp=cal(kita,t);
                //cout<<"lat is "<<t.lat<<" lon is "<<t.lon<<endl;
                if(tmp<min){
                    nearestHighway=t.id;
                    min=tmp;
                    //cout<<"min is"<<min<<endl;
                }
            }
        }
    }
    //cout<<"nearest is"<< nearestHighway<<endl;
    //cout<<" size is"<<cnodes[nearestHighway].nei.size()<<endl;
    //cout<<fixed<<setprecision(14)<<cnodes[nearestHighway].lat<<' '<<cnodes[nearestHighway].lon<<endl;
    return nearestHighway;
}
