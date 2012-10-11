/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <iostream>
#include <fstream>
#include <dirent.h>
#include <string>
#include <vector>

#include <osl/file.hxx>
#include <rtl/string.hxx>

#include "po.hxx"

using namespace std;

//Check wheather the two entry are the same but in different languages
bool IsSameEntry(const OString& rFirstEntry,const OString& rSecEntry)
{
    for(int i = PoEntry::PROJECT; i<=PoEntry::LOCALID;++i)
    {
        if ( rFirstEntry.getToken(i,'\t') != rSecEntry.getToken(i,'\t') &&
             i != PoEntry::DUMMY)
            return false;
    }
    return true;
}

//Get path of po file
OString GetPath(const OString& rPath, const OString& rLine)
{
    OString sSourceFile = rLine.getToken(PoEntry::SOURCEFILE,'\t');
    OString sSourcePath = rPath + "/" +
                          rLine.getToken(PoEntry::PROJECT,'\t') + "/" +
                          sSourceFile.copy(0,sSourceFile.lastIndexOf("\\")).
                                      replaceAll("\\","/");
    return sSourcePath;
}

OString DelLocalId(const OString& rLine)
{
    unsigned nTabIndex = 0;
    for(unsigned nComponent=0; nComponent<PoEntry::LOCALID; ++nComponent)
    {
        nTabIndex = rLine.indexOf('\t',nTabIndex);
        ++nTabIndex;
    }
    return rLine.replaceAt(nTabIndex,
                           rLine.indexOf('\t',nTabIndex)-nTabIndex,
                           "");
}

//Renew po files of the actual language
void HandleLanguage(struct dirent* pLangEntry, const OString& rPath,
                    const OString& rpo2loPath, const OString& rSDFPath)
{
    const OString LangEntryName = pLangEntry->d_name;

    //Generate and open sdf
    cout << "Process start with language: " <<  LangEntryName.getStr() << endl;
    OUString aTempUrl;
    if (osl::FileBase::createTempFile(0, 0, &aTempUrl)
        != osl::FileBase::E_None)
    {
        cerr << "osl::FileBase::createTempFile() failed\n";
        return;
    }
    OUString aTempPath;
    if (osl::FileBase::getSystemPathFromFileURL(aTempUrl, aTempPath)
        != osl::FileBase::E_None)
    {
        cerr
            << "osl::FileBase::getSystemPathFromFileURL(" << aTempUrl
            << ") failed\n";
        return;
    }
    const OString SDFFileName =
        OUStringToOString(aTempPath, RTL_TEXTENCODING_UTF8);
    system( (rpo2loPath +
            " -i " + rPath + "/" + LangEntryName +
            " -o " + SDFFileName +
            " -l " + LangEntryName +
            " -t " + rSDFPath).getStr());
    cout << "Language sdf is ready!" << endl;

    PoOfstream aNewPo;
    ifstream aSDFInput(SDFFileName.getStr());
    string s;
    getline(aSDFInput,s);
    OString sLine = OString(s.data(),s.length());
    while(!aSDFInput.eof())
    {
        OString sActUnTrans = sLine;
        const OString sPath = rPath + "/"+ LangEntryName;
        const OString sActSourcePath = GetPath(sPath,sActUnTrans);
        //Make new po file and add header
        if (!aNewPo.isOpen())
        {
            const OString sNewPoFileName = sActSourcePath + ".po_tmp";
            aNewPo.open(sNewPoFileName);
            if (!aNewPo.isOpen())
            {
                cerr
                    << "Cannot open temp file for new po: "
                    << sNewPoFileName.getStr() << endl;
                return;
            }
            const OString sOldPoFileName = sActSourcePath + ".po";
            ifstream aOldPo(sOldPoFileName.getStr());
            if (!aOldPo.is_open())
            {
                cerr
                    << "Cannot open old po file: "
                    << sOldPoFileName.getStr() << endl;
                return;
            }
            aNewPo.writeHeader(PoHeader(aOldPo));
            aOldPo.close();
        }

        //Set PoEntry and write out
        getline(aSDFInput,s);
        OString sActTrans;
        if (!aSDFInput.eof() &&
            IsSameEntry(sActUnTrans,sLine = OString(s.data(),s.length())))
        {
            sActTrans = sLine;
            getline(aSDFInput,s);
        }
        else
        {
            sActTrans ="";
        }
        const PoEntry::TYPE vInitializer[] =
            { PoEntry::TTEXT, PoEntry::TQUICKHELPTEXT, PoEntry::TTITLE };
        const vector<PoEntry::TYPE> vTypes( vInitializer,
            vInitializer + sizeof(vInitializer) / sizeof(vInitializer[0]) );
        unsigned short nDummyBit = 0;
        for( unsigned short nIndex=0; nIndex<vTypes.size(); ++nIndex )
        {
            if (!sActUnTrans.getToken(vTypes[nIndex],'\t').isEmpty())
            {
                /**Because of xrmex there are duplicated id's,
                   only use this if xrmex have already fixed*/
                const OString sSource =
                    sActUnTrans.getToken(PoEntry::SOURCEFILE,'\t');
                const OString sEnding =
                    sSource.copy(sSource.getLength()-4, 4);
                if (sActUnTrans.getToken(PoEntry::GROUPID,'\t')==
                    sActUnTrans.getToken(PoEntry::LOCALID,'\t') &&
                    ( sEnding == ".xrm" || sEnding == ".xml" ))
                {
                    sActUnTrans = DelLocalId(sActUnTrans);
                }
                PoEntry aPE(sActUnTrans, vTypes[nIndex]);
                const OString sActStr =
                    sActTrans.getToken(vTypes[nIndex],'\t');
                aPE.setMsgStr(sActStr);
                aPE.setFuzzy( sActStr.isEmpty() ? false :
                    static_cast<bool>(sActTrans.getToken(PoEntry::DUMMY,'\t').
                        copy(nDummyBit++,1).toBoolean()));
                aNewPo.writeEntry(aPE);
            }
        }
        //Check wheather next entry is in the same po file
        OString sNextSourcePath = aSDFInput.eof() ? "" :
            GetPath(sPath,sLine = OString(s.data(),s.length()));
        if (sNextSourcePath!=sActSourcePath)
        {
            aNewPo.close();
            system(("rm " + sActSourcePath +".po").getStr());
            system(("mv "+ sActSourcePath +".po_tmp " +
                    sActSourcePath +".po").getStr());
        }
    }

    //Close and remove sdf file
    aSDFInput.close();
    if (osl::File::remove(aTempUrl) != osl::FileBase::E_None)
    {
        cerr << "Warning: failure removing temporary " << aTempUrl << '\n';
    }
}


int main(int argc, char* argv[])
{
    //Usage
    if (argc < 4)
    {
        cout << "Use: renewpot translationsdir po2lo en-US.sdf" << endl;
        cout << "translationsdir: this directory contains the po" << endl;
        cout << "files of all languages. Every language has a" << endl;
        cout << "directory named with language id." << endl;
        return 1;
    }

    //Call processing function with all language directories
    DIR* pTranslations = opendir(argv[1]);
    while ( struct dirent* pActEntry = readdir(pTranslations) )
    {
        if ( OString(pActEntry->d_name).indexOf('.')==-1 )
            HandleLanguage(pActEntry,OString(argv[1]),
                           OString(argv[2]),OString(argv[3]));
    }
    closedir(pTranslations);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
