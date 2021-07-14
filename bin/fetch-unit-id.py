import os
import json
import requests
import time

mainLink = 'https://weblate-gsoc.documentfoundation.org/api/translations/'
txtfilepath = os.path.join(os.getcwd() , 'bin\ScreenshotJson.txt' )
apiKeyPath = 'C:\\Users\\saryn\\Desktop\\apikey.txt' #change it to your api key path 
apiKeyFile = open(apiKeyPath , 'r')
apiKey = apiKeyFile.read()
map = {}
keyidToUnitID = {}

# Convert Map < screenshotPath -> (uiPath , context)Pair > in json to dict
with open ('bin\ScreenshotJson.txt') as Json_File:
    map = json.load(Json_File)
""" 
 KeyIdtoUnitID contains the mapping of context keyid to corresponding units id
 it's used to avoid requesting all the links again to save time :

with open ('bin\KeyIdtoUnitID.txt') as Json_File:
    keyidToUnitID = json.load(Json_File)

 """


for screenshotLocation in map.keys():
    UIPath = map[screenshotLocation][0]
    componentSlug = UIPath.split('/')[0] + ('messages/')
    ProjectSlug = 'libo_ui-master/'
    language = 'en/'
    Location = ' location:' + UIPath
    query = {'q' : Location} 

    request = requests.get(mainLink + ProjectSlug +
    componentSlug + language + 'units/',params = query ,
    headers = {"Authorization" : "Bearer " + apiKey},
    timeout = 5)


    nextPageUrl = ''
    ContainNextPage = False
    time.sleep(1)
     
    while(True):
        time.sleep(1)
        if(ContainNextPage == True):
            response = (nextPageUrl).content
        else:
            response  = request.content

        #save response
        units = json.loads(response)
        UnitsResults = units['results']

        # Iterate over the response and map every keyid of specific context
        # to it's Unit id
        for unit in UnitsResults:
            keyidToUnitID[unit['note']] = unit['id']

        #if there is next page put the api key and request the next page
        if(units['next'] != None):
            ContainNextPage = True
            nextPageUrl = requests.get(units['next'] ,
            headers = {"Authorization" : "Bearer " + apiKey},
            timeout = 5)
        else:
            break


#Save the mapping of keyid of context to unit idd .
txtfilepath = os.path.join(os.getcwd() , 'bin\KeyIdtoUnitID.txt') 
with open( txtfilepath, 'w') as outfile :
    json.dump(keyidToUnitID , outfile)
outfile.close
 
#put the unit id of context in the map and save them .

for screenshotLocation in map.keys():
    try:
        for l in map[screenshotLocation][1]:
            l.append(keyidToUnitID[l[1]])
    except KeyError:
        continue

    
txtfilepath = os.path.join(os.getcwd() , 'bin\ScreenshotJsonWithUnitID.txt')
with open( txtfilepath, 'w') as outfile :
    json.dump(map , outfile)
outfile.close 