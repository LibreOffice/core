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
// local includes
#include "export.hxx"

DECLARE_LIST( LngLineList, ByteString * )

#define LNG_OK              0x0000
#define LNG_FILE_NOTFOUND   0x0001
#define LNG_COULD_NOT_OPEN  0x0002
#define SDF_OK              0x0003
#define SDF_FILE_NOTFOUND   0x0004
#define SDF_COULD_NOT_OPEN  0x0005

//
// class LngParser
//

class LngParser
{
private:
    sal_uInt16 nError;
    LngLineList *pLines;
    ByteString sSource;
    sal_Bool bDBIsUTF8;
    sal_Bool bULF;
    bool bQuiet;
    std::vector<ByteString> aLanguages;

    void FillInFallbacks( ByteStringHashMap Text );
    bool isNextGroup(  ByteString &sGroup_out , ByteString &sLine_in);
    void ReadLine( const ByteString &sLine_in , ByteStringHashMap &rText_inout );
    void WriteSDF( SvFileStream &aSDFStream , ByteStringHashMap &rText_inout ,
                    const ByteString &rPrj ,
                    const ByteString &rRoot , const ByteString &sActFileName , const ByteString &sID );
public:
    LngParser( const ByteString &rLngFile, sal_Bool bUTF8, sal_Bool bULFFormat );
    ~LngParser();

    sal_Bool CreateSDF( const ByteString &rSDFFile, const ByteString &rPrj, const ByteString &rRoot );
    sal_Bool Merge( const ByteString &rSDFFile, const ByteString &rDestinationFile , const ByteString &rPrj );
};
