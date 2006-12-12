/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: minormk.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: kz $ $Date: 2006-12-12 16:34:22 $
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

#include <tools/stream.hxx>

class GenericInformationList;

//
// class MinorMk
//

class MinorMk : public SvFileStream
{
private:
    ByteString sRSCVERSION;
    ByteString sRSCREVISION;
    ByteString sBUILD;
    ByteString sLAST_MINOR;

    BOOL bExists;

    void ReadMinorMk( const String &rFileName );

public:
    MinorMk(
        GenericInformationList *pList,
        const ByteString &rVersion,
        const ByteString &rWorkStamp,
        const ByteString &rEnvironment,
        const ByteString &rMinor
    );
    MinorMk(
        const ByteString &rMinor,
        const ByteString &rEnvironment
    );
    MinorMk(
        GenericInformationList *pList,
        const ByteString rVersion
    );
    ~MinorMk();

    ByteString GetRscVersion()
        { return sRSCVERSION; }
    ByteString GetRscRevision()
        { return sRSCREVISION; }
    ByteString GetBuildNr()
        { return sBUILD; }
    ByteString GetLastMinor()
        { return sLAST_MINOR; }

    BOOL Exists()
        { return bExists; }
};
