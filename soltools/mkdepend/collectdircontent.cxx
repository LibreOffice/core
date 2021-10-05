/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
#include "collectdircontent.hxx"
#include <rtl/character.hxx>

PathFilePair IncludesCollection::split_path(const std::string& filePath) {
    std::string sepU = "/";
    std::string sepW = "\\";
    std::string::size_type pos = filePath.rfind (sepU);
    std::string::size_type posW = filePath.rfind (sepW);
    if ((posW != std::string::npos) && ((posW > pos) || (pos == std::string::npos))) pos = posW;
    if (pos != std::string::npos) {
        std::string dirName = filePath.substr(0, pos);
        return PathFilePair(dirName, filePath.substr(pos + 1, filePath.length()));
    } else
        return PathFilePair(".", filePath);
}

void IncludesCollection::add_to_collection(const std::string& dirPath) {
    DirContent dirContent;
#if defined(_WIN32)
    WIN32_FIND_DATA FindFileData;
    HANDLE hFind;
    hFind = FindFirstFile((dirPath + "\\*").c_str(), &FindFileData);
    if (hFind == INVALID_HANDLE_VALUE) {
        // Invalid File Handle - no need to try it anymore
        allIncludes.insert(EntriesPair(dirPath, DirContent()));
        return;
    }
    do {
        std::string winFileName(FindFileData.cFileName);
        transform(
            winFileName.begin(), winFileName.end(), winFileName.begin(),
            [](char c) {
                return rtl::toAsciiLowerCase(static_cast<unsigned char>(c));
            });
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
    }
    closedir(pdir);
#endif // defined( _WIN32 )
    allIncludes.insert(EntriesPair(dirPath, dirContent));
}

bool IncludesCollection::exists(std::string filePath) {
#if defined(_WIN32)
    transform(
        filePath.begin(), filePath.end(), filePath.begin(),
        [](char c) {
            return rtl::toAsciiLowerCase(static_cast<unsigned char>(c));
        });
#endif // defined( _WIN32 )
    PathFilePair dirFile = split_path(filePath);
    std::string dirPath = dirFile.first;
    std::string fileName = dirFile.second;
    DirMap::iterator mapIter = allIncludes.find(dirPath);
    if (mapIter == allIncludes.end()) {
        add_to_collection(dirPath);
        mapIter = allIncludes.find(dirPath);
    }
    DirContent dirContent = (*mapIter).second;
    DirContent::iterator dirIter = dirContent.find(fileName);
    return dirIter != dirContent.end();
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
