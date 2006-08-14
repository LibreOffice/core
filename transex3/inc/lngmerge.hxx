/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: lngmerge.hxx,v $
 *
 *  $Revision: 1.8 $
 *
 *  last change: $Author: hr $ $Date: 2006-08-14 17:08:27 $
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
    USHORT nError;
    LngLineList *pLines;
    ByteString sSource;
    BOOL bDBIsUTF8;
    BOOL bULF;
    bool bQuiet;
    std::vector<ByteString> aLanguages;

    void FillInFallbacks( ByteStringHashMap Text );
    bool isNextGroup(  ByteString &sGroup_out , ByteString &sLine_in);
    void ReadLine( const ByteString &sLine_in , ByteStringHashMap &rText_inout );
    void WriteSDF( SvFileStream &aSDFStream , ByteStringHashMap &rText_inout ,
                    const ByteString &rPrj ,
                    const ByteString &rRoot , const ByteString &sActFileName , const ByteString &sID );
public:
    LngParser( const ByteString &rLngFile, BOOL bUTF8, BOOL bULFFormat, bool bQuiet_in );
    ~LngParser();

    BOOL CreateSDF( const ByteString &rSDFFile, const ByteString &rPrj, const ByteString &rRoot );
    BOOL Merge( const ByteString &rSDFFile, const ByteString &rDestinationFile , const ByteString &rPrj );
};
