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

#include <MNSINIParser.hxx>
#include <rtl/byteseq.hxx>

IniParser::IniParser(OUString const & rIniName) throw(com::sun::star::io::IOException )
{
    OUString iniUrl;
    if (osl_File_E_None != osl_getFileURLFromSystemPath(rIniName.pData, &iniUrl.pData))
        return;


#if OSL_DEBUG_LEVEL > 0
    OString sFile = OUStringToOString(iniUrl, RTL_TEXTENCODING_ASCII_US);
    OSL_TRACE(__FILE__" -- parser() - %s\n", sFile.getStr());
#endif
    oslFileHandle handle=NULL;
    oslFileError fileError = osl_File_E_INVAL;
    try{
        if (!iniUrl.isEmpty())
            fileError = osl_openFile(iniUrl.pData, &handle, osl_File_OpenFlag_Read);
    }
    catch(const ::com::sun::star::io::IOException&)
    {
#if OSL_DEBUG_LEVEL > 0
        OString file_tmp = OUStringToOString(iniUrl, RTL_TEXTENCODING_ASCII_US);
        OSL_TRACE( __FILE__" -- couldn't open file: %s", file_tmp.getStr() );
#endif
    }

    if (osl_File_E_None == fileError)
    {
        rtl::ByteSequence seq;
        sal_uInt64 nSize = 0;

        osl_getFileSize(handle, &nSize);
        OUString sectionName(  "no name section" );
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
                    if (sectionName.isEmpty())
                        sectionName = OUString("no name section");

                    ini_Section *aSection = &mAllSection[sectionName];
                    aSection->sName = sectionName;
                }
            }
        }
        osl_closeFile(handle);
    }
#if OSL_DEBUG_LEVEL > 0
    else
    {
        OString file_tmp = OUStringToOString(iniUrl, RTL_TEXTENCODING_ASCII_US);
        OSL_TRACE( __FILE__" -- couldn't open file: %s", file_tmp.getStr() );
    }
#endif
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
