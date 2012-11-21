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
#include <string>
#include <vector>
#include <map>

#include <osl/file.hxx>
#include <rtl/string.hxx>

#include "po.hxx"

using namespace std;

bool isInSameFile( const OString& rFirstLine, const OString& rSecondLine)
{
    const OString rFirstSource =
        rFirstLine.getToken(PoEntry::SOURCEFILE,'\t');
    const OString rSecondSource =
        rSecondLine.getToken(PoEntry::SOURCEFILE,'\t');
    return
        rFirstSource.copy(0,rFirstSource.lastIndexOf("\\")) ==
        rSecondSource.copy(0,rSecondSource.lastIndexOf("\\"));
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
void HandleLanguage(const OString& LangEntryName, const OString& rOldPath,
    const OString& rNewPath, const OString& rpo2loPath,
    const OString& rSDFPath)
{
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
    const OString cmd = OString(rpo2loPath +
            " -i " + rOldPath + "/" + LangEntryName +
            " -o " + SDFFileName +
            " -l " + LangEntryName +
            " -t " + rSDFPath);
    if (system(cmd.getStr()) != 0)
    {
        std::cerr << "Error: Failed to execute " << cmd.getStr() << '\n';
        throw false;
    }
    cout << "Language sdf is ready!" << endl;

    //Store info for po entries
    ifstream aSDFInput(SDFFileName.getStr());
    map<sal_Int32,pair<OString,OString> > aPoInfos;
    string s;
    getline(aSDFInput,s);
    while(!aSDFInput.eof())
    {
        //Get strings belong to one po entry and store
        const OString sActUnTrans = OString(s.data(),s.length());
        if( sActUnTrans.getToken(PoEntry::LANGUAGEID,'\t')=="ast" ) throw;
        getline(aSDFInput,s);
        const OString sActTrans = OString(s.data(),s.length());

        if(!(aPoInfos.insert( pair<sal_Int32,pair<OString,OString> >(
            sActTrans.getToken(PoEntry::WIDTH,'\t').toInt32(),
            pair<OString,OString>(sActUnTrans,sActTrans))).second))
        {
            cerr << "Error: faild to insert into map!" << '\n';
            throw;
        }
        getline(aSDFInput,s);
    }

    //Close and remove sdf file
    aSDFInput.close();
    if (osl::File::remove(aTempUrl) != osl::FileBase::E_None)
    {
        cerr << "Warning: failure removing temporary " << aTempUrl << '\n';
    }

    //Construct and write out po entries
    PoOfstream aNewPo;
    for( map<sal_Int32,pair<OString,OString> >::iterator
        pActInfo=aPoInfos.begin(); pActInfo!=aPoInfos.end(); ++pActInfo )
    {
        //Make new po file and add header
        if ( pActInfo==aPoInfos.begin() ||
            !isInSameFile(((--pActInfo)++)->second.first,pActInfo->second.first) )
        {
            if( pActInfo!=aPoInfos.begin() )
                aNewPo.close();

            const OString sNewPoFileName =
                GetPath(rNewPath + "/" +LangEntryName,pActInfo->second.first) +
                ".po";
            const OString cmd2 = OString("mkdir -p " + sNewPoFileName.copy(0,sNewPoFileName.lastIndexOf("/")));
            if (system(cmd2.getStr()) != 0)
            {
                std::cerr << "Error: Failed to execute " << cmd2.getStr() << '\n';
                throw false;
            }

            aNewPo.open(sNewPoFileName);
            if (!aNewPo.isOpen())
            {
                cerr
                    << "Cannot open new po file: "
                    << sNewPoFileName.getStr() << endl;
                return;
            }
            const OString sOldPoFileName =
                GetPath(rOldPath + "/" +LangEntryName,pActInfo->second.first) +
                ".po";
            ifstream aOldPo(sOldPoFileName.getStr());
            if (!aOldPo.is_open())
            {
                cerr
                    << "Cannot open old po file: "
                    << sOldPoFileName.getStr() << endl;
                return;
            }

            PoHeader aTmp(aOldPo);
            aNewPo.writeHeader(aTmp);
            aOldPo.close();
        }

        //Write out po entries
        const PoEntry::TYPE vInitializer[] =
            { PoEntry::TTEXT, PoEntry::TQUICKHELPTEXT, PoEntry::TTITLE };
        const vector<PoEntry::TYPE> vTypes( vInitializer,
            vInitializer + sizeof(vInitializer) / sizeof(vInitializer[0]) );
        unsigned short nDummyBit = 0;
        for( unsigned short nIndex=0; nIndex<vTypes.size(); ++nIndex )
        {
            if (!pActInfo->second.first.getToken(vTypes[nIndex],'\t').isEmpty())
            {
                /**Because of xrmex there are duplicated id's,
                only use this if xrmex have already fixed*/
                const OString sSource =
                    pActInfo->second.first.getToken(PoEntry::SOURCEFILE,'\t');
                const OString sEnding =
                    sSource.copy(sSource.getLength()-4, 4);
                if (pActInfo->second.first.getToken(PoEntry::GROUPID,'\t')==
                    pActInfo->second.first.getToken(PoEntry::LOCALID,'\t') &&
                    ( sEnding == ".xrm" || sEnding == ".xml" ))
                {
                    pActInfo->second.first = DelLocalId(pActInfo->second.first);
                }
                try
                {
                    PoEntry aPE(pActInfo->second.first, vTypes[nIndex]);
                    const OString sActStr =
                        pActInfo->second.second.getToken(vTypes[nIndex],'\t');
                    aPE.setMsgStr(sActStr);
                    aPE.setFuzzy( sActStr.isEmpty() ? false :
                        static_cast<bool>(pActInfo->second.second.getToken(PoEntry::DUMMY,'\t').
                            copy(nDummyBit++,1).toBoolean()));
                    aNewPo.writeEntry(aPE);
                }
                catch( PoEntry::Exception& )
                {
                    cerr
                        << "Invalid sdf line "
                        << pActInfo->second.first.replaceAll("\t","\\t").getStr() << '\n';
                }
            }
        }
    }
    aNewPo.close();
    aPoInfos.clear();
}


int main(int argc, char* argv[])
{
    //Usage
    if (argc < 4)
    {
        cout << "Use: renewpot oldpots newpots po2lo en-US.sdf" << endl;
        return 1;
    }

    //Call processing function with all language directories
    OUString pathUrl;
    if( osl::Directory::getFileURLFromSystemPath(
        OStringToOUString( argv[ 1 ], RTL_TEXTENCODING_UTF8 ), pathUrl ) == osl::Directory::E_None )
    {
        osl::Directory dir( pathUrl );
        if( dir.reset() == osl::Directory::E_None )
        {
            for(;;)
            {
                osl::DirectoryItem item;
                if( dir.getNextItem( item ) != osl::Directory::E_None )
                    break;
                osl::FileStatus status( osl_FileStatus_Mask_FileName );
                if( item.getFileStatus( status ) == osl::File::E_None && status.getFileName().indexOf('.')==-1 )
                    HandleLanguage( OUStringToOString(status.getFileName(), RTL_TEXTENCODING_UTF8),
                           OString(argv[1]),
                           OString(argv[2]),OString(argv[3]),
                           OString(argv[4]));
            }
        }
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
