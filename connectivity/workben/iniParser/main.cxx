/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include <rtl/ustring.hxx>
#include <com/sun/star/io/IOException.hpp>
#include <osl/process.h>

#include <map>
#include <list>


struct ini_NameValue
{
    OUString sName;
    OUString sValue;

    inline ini_NameValue()
        {}
    inline ini_NameValue(
        OUString const & name, OUString const & value )
        : sName( name ),
          sValue( value )
        {}
};

typedef std::list<
    ini_NameValue
> NameValueList;

struct ini_Section
{
    OUString sName;
    NameValueList lList;
};
typedef std::map<OUString,
                ini_Section
                >IniSectionMap;


class IniParser
{
    IniSectionMap mAllSection;
public:
    IniSectionMap * getAllSection(){return &mAllSection;};
    ini_Section *  getSection(OUString const & secName)
    {
        if (mAllSection.find(secName) != mAllSection.end())
            return &mAllSection[secName];
        return NULL;
    }
    IniParser(OUString const & rIniName) throw(com::sun::star::io::IOException )
    {
        OUString curDirPth;
        OUString iniUrl;
        osl_getProcessWorkingDir( &curDirPth.pData );
        if (osl_getAbsoluteFileURL( curDirPth.pData,    rIniName.pData, &iniUrl.pData ))
            throw ::com::sun::star::io::IOException();


#if OSL_DEBUG_LEVEL > 1
        OString sFile = OUStringToOString(iniUrl, RTL_TEXTENCODING_ASCII_US);
        OSL_TRACE(__FILE__" -- parser() - %s\n", sFile.getStr());
#endif
        oslFileHandle handle=NULL;
        if (iniUrl.getLength() &&
            osl_File_E_None == osl_openFile(iniUrl.pData, &handle, osl_File_OpenFlag_Read))
        {
            rtl::ByteSequence seq;
            sal_uInt64 nSize = 0;

            osl_getFileSize(handle, &nSize);
            OUString sectionName( "no name section" );
            while (true)
            {
                sal_uInt64 nPos;
                if (osl_File_E_None != osl_getFilePos(handle, &nPos) || nPos >= nSize)
                    break;
                if (osl_File_E_None != osl_readLine(handle , (sal_Sequence **) &seq))
                    break;
                OString line( (const sal_Char *) seq.getConstArray(), seq.getLength() );
                sal_Int32 nIndex = line.indexOf('=');
                if (nIndex >= 1)
                {
                    ini_Section *aSection = &mAllSection[sectionName];
                    struct ini_NameValue nameValue;
                    nameValue.sName = OStringToOUString(
                        line.copy(0,nIndex).trim(), RTL_TEXTENCODING_ASCII_US );
                    nameValue.sValue = OStringToOUString(
                        line.copy(nIndex+1).trim(), RTL_TEXTENCODING_UTF8 );

                    aSection->lList.push_back(nameValue);

                }
                else
                {
                    sal_Int32 nIndexStart = line.indexOf('[');
                    sal_Int32 nIndexEnd = line.indexOf(']');
                    if ( nIndexEnd > nIndexStart && nIndexStart >=0)
                    {
                        sectionName =  OStringToOUString(
                            line.copy(nIndexStart + 1,nIndexEnd - nIndexStart -1).trim(), RTL_TEXTENCODING_ASCII_US );
                        if (!sectionName.getLength())
                            sectionName = "no name section";

                        ini_Section *aSection = &mAllSection[sectionName];
                        aSection->sName = sectionName;
                    }
                }
            }
            osl_closeFile(handle);
        }
#if OSL_DEBUG_LEVEL > 1
        else
        {
            OString file_tmp = OUStringToOString(iniUrl, RTL_TEXTENCODING_ASCII_US);
            OSL_TRACE( __FILE__" -- couldn't open file: %s", file_tmp.getStr() );
            throw ::com::sun::star::io::IOException();
        }
#endif
    }
#if OSL_DEBUG_LEVEL > 1
    void Dump()
    {
        IniSectionMap::iterator iBegin = mAllSection.begin();
        IniSectionMap::iterator iEnd = mAllSection.end();
        for(;iBegin != iEnd;iBegin++)
        {
            ini_Section *aSection = &(*iBegin).second;
            OString sec_name_tmp = OUStringToOString(aSection->sName, RTL_TEXTENCODING_ASCII_US);
            for(NameValueList::iterator itor=aSection->lList.begin();
                itor != aSection->lList.end();
                itor++)
            {
                    struct ini_NameValue * aValue = &(*itor);
                    OString name_tmp = OUStringToOString(aValue->sName, RTL_TEXTENCODING_ASCII_US);
                    OString value_tmp = OUStringToOString(aValue->sValue, RTL_TEXTENCODING_UTF8);
                    OSL_TRACE(
                        " section=%s name=%s value=%s\n",
                                        sec_name_tmp.getStr(),
                                        name_tmp.getStr(),
                                        value_tmp.getStr() );

            }
        }

    }
#endif

};

int main( int argc, char * argv[] )
{

    IniParser parser(OUString("test.ini"));
    parser.Dump();
    return 0;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
