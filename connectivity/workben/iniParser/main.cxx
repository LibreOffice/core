/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_connectivity.hxx"
#include <rtl/ustring.hxx>
#include <stdio.h>
#include <com/sun/star/io/IOException.hpp>
#include <osl/process.h>

#include <map>
#include <list>

using ::rtl::OUString;
using ::rtl::OString;
using ::rtl::OStringToOUString;
using ::rtl::OUStringToOString;

struct ini_NameValue
{
    rtl::OUString sName;
    rtl::OUString sValue;

    inline ini_NameValue() SAL_THROW( () )
        {}
    inline ini_NameValue(
        OUString const & name, OUString const & value ) SAL_THROW( () )
        : sName( name ),
          sValue( value )
        {}
};

typedef std::list<
    ini_NameValue
> NameValueList;

struct ini_Section
{
    rtl::OUString sName;
    NameValueList lList;
};
typedef std::map<rtl::OUString,
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
            OUString sectionName( RTL_CONSTASCII_USTRINGPARAM( "no name section" ));
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
                            sectionName = OUString(RTL_CONSTASCII_USTRINGPARAM("no name section"));

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

#if (defined UNX) || (defined OS2)
int main( int argc, char * argv[] )
#else
int _cdecl main( int argc, char * argv[] )
#endif

{

    IniParser parser(OUString(RTL_CONSTASCII_USTRINGPARAM("test.ini")));
    parser.Dump();
    return 0;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
