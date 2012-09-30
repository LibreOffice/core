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
    const OString SDFFileName = LangEntryName + ".sdf";

    //Generate and open sdf
    cout << "Process start with language: " <<  LangEntryName.getStr() << endl;
    system( (rpo2loPath +
            " -i " + rPath + "/" + LangEntryName +
            " -o " + SDFFileName +
            " -l " + LangEntryName +
            " -t " + rSDFPath).getStr());
    cout << "Language sdf is ready!" << endl;

    ofstream aOutPut;
    ifstream aSDFInput(SDFFileName.getStr());
    string s;
    getline(aSDFInput,s);
    OString sLine = OString(s.data(),s.length());
    while(!aSDFInput.eof())
    {
        OString sActUnTrans = sLine;
        OString sPath = rPath + "/"+ LangEntryName;
        OString sActSourcePath = GetPath(sPath,sActUnTrans);
        //Make new po file, copy header with some changes
        if (!aOutPut.is_open())
        {
            aOutPut.open((sActSourcePath + ".po_tmp").getStr(),
                         std::ios_base::out | std::ios_base::trunc);
            ifstream aPOInput((sActSourcePath + ".po").getStr());
            getline(aPOInput,s);
            while(s!="")
            {
                if (s.find("#. extracted from")!=string::npos)
                    s = string(s,0,s.length()-3);
                if (s.find("Report-Msgid-Bugs-To")!=string::npos)
                    s = string("\"Report-Msgid-Bugs-To: ") +
                        "https://bugs.freedesktop.org/enter_bug.cgi?product=" +
                        "LibreOffice&bug_status=UNCONFIRMED&component=UI\\n\"";
                if (s.find("X-Generator")!=string::npos)
                    s = "\"X-Generator: LibreOffice\\n\"";
                aOutPut << s << endl;
                getline(aPOInput,s);
            };
            aPOInput.close();
        }

        //Set PoEntry and write out
        getline(aSDFInput,s);
        sLine = OString(s.data(),s.length());
        OString sActTrans;
        if (!aSDFInput.eof() && IsSameEntry(sActUnTrans,sLine))
        {
            sActTrans = sLine;
            getline(aSDFInput,s);
            sLine = OString(s.data(),s.length());
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
        for( unsigned nIndex=0; nIndex<vTypes.size(); ++nIndex)
        {
            if (!sActUnTrans.getToken(vTypes[nIndex],'\t').isEmpty())
            {
                /**Because of xrmex there are duplicated id's,
                   only use this if xrmex have already fixed
                if (sActUnTrans.getToken(PoEntry::GROUPID,'\t')==
                    sActUnTrans.getToken(PoEntry::LOCALID,'\t') &&
                    sActUnTrans.getToken(PoEntry::SOURCEFILE,'\t').
                                endsWith(".xrm"))
                {
                    sActUnTrans = DelLocalId(sActUnTrans);
                }*/
                PoEntry aPE(sActUnTrans, vTypes[nIndex]);
                const OString sActStr =
                    sActTrans.getToken(vTypes[nIndex],'\t');
                aPE.setTransStr(sActStr);
                aPE.setFuzzy( sActStr.isEmpty() ? false :
                    static_cast<bool>(sActTrans.getToken(PoEntry::DUMMY,'\t').
                        copy(nDummyBit++,1).toBoolean()));
                aPE.writeToFile(aOutPut);
            }
        }
        //Check wheather next entry is in the same po file
        OString sNextSourcePath =
            !aSDFInput.eof() ? GetPath(sPath,sLine) : "";
        if (sNextSourcePath!=sActSourcePath)
        {
            aOutPut.close();
            system(("rm " + sActSourcePath +".po").getStr());
            system(("mv "+ sActSourcePath +".po_tmp " +
                    sActSourcePath +".po").getStr());
        }
    }

    //Close and remove sdf file
    aSDFInput.close();
    system(("rm " + SDFFileName).getStr());
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
