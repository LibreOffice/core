/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
#include  "collectdircontent.hxx"

using namespace std;

PathFilePair IncludesCollection::split_path(const string& filePath) {
    string sepU = "/";
    string sepW = "\\";
    string::size_type pos = filePath.rfind (sepU);
    string::size_type posW = filePath.rfind (sepW);
    if ((posW != string::npos) && ((posW > pos) || (pos == string::npos))) pos = posW;
    if (pos != string::npos) {
        string dirName = filePath.substr(0, pos);
        return PathFilePair(dirName, filePath.substr(pos + 1, filePath.length()));
    } else
        return PathFilePair(".", filePath);
}

void IncludesCollection::add_to_collection(const string& dirPath) {
    DirContent dirContent;
#if defined( WNT )
    WIN32_FIND_DATA FindFileData;
    HANDLE hFind;
    hFind = FindFirstFile((dirPath + "\\*").c_str(), &FindFileData);
    if (hFind == INVALID_HANDLE_VALUE) {
        // Invalid File Handle - no need to try it anymore
        allIncludes.insert(EntriesPair(dirPath, DirContent()));
        return;
    };
    do {
        string winFileName(FindFileData.cFileName);
        transform(winFileName.begin(), winFileName.end(), winFileName.begin(), ::tolower);
        dirContent.insert(winFileName);
    } while (FindNextFile(hFind, &FindFileData));
#else
    DIR *pdir;
    dirent *pent;
    pdir = opendir(dirPath.c_str()); //"." refers to the current dir
    if (!pdir) {
        // Invalid File Handle - no need to try it anymore
        allIncludes.insert(EntriesPair(dirPath, DirContent()));
        return;
    }
    while ((pent = readdir(pdir))) {
        dirContent.insert(pent->d_name);
    };
    closedir(pdir);
#endif // defined( WNT )
    allIncludes.insert(EntriesPair(dirPath, dirContent));
}

bool IncludesCollection::exists(string filePath) {
#if defined( WNT )
    transform(filePath.begin(), filePath.end(), filePath.begin(), ::tolower);
#endif // defined( WNT )
    PathFilePair dirFile = split_path(filePath);
    string dirPath = dirFile.first;
    string fileName = dirFile.second;
    DirMap::iterator mapIter = allIncludes.find(dirPath);
    if (mapIter == allIncludes.end()) {
        add_to_collection(dirPath);
        mapIter = allIncludes.find(dirPath);
    };
    DirContent dirContent = (*mapIter).second;
    DirContent::iterator dirIter = dirContent.find(fileName);
    if (dirIter == dirContent.end()) {
        return false;
    } else {
        return true;
    };
}

extern "C" {

    IncludesCollection * create_IncludesCollection() {
            return new IncludesCollection;
    }

    void delete_IncludesCollection(IncludesCollection *m) {
            delete m;
    }

    int call_IncludesCollection_exists(IncludesCollection* m, const char * filePath) {
        return m->exists(filePath);
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
