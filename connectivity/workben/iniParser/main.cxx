/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: main.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: hr $ $Date: 2006-06-20 02:12:07 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/
#include <rtl/ustring.hxx>
#include <stdio.h>
#include <com/sun/star/io/IOException.hpp>
#ifndef _OSL_PROCESS_H_
#include <osl/process.h>
#endif
using namespace rtl;

#include <map>
#include <list>

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
            OUString sectionName = OUString::createFromAscii("no name section");
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
                            sectionName = OUString::createFromAscii("no name section");

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

    IniParser parser(OUString::createFromAscii("test.ini"));
    parser.Dump();
    return 0;
}
