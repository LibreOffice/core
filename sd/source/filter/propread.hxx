/*************************************************************************
 *
 *  $RCSfile: propread.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:48:45 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef _PROPREAD_HXX_
#define _PROPREAD_HXX_

#ifndef _SOLAR_H
#include <tools/solar.h>
#endif
#ifndef _SVSTOR_HXX
#include <so3/svstor.hxx>
#endif
#ifndef _SV_GEN_HXX
#include <tools/gen.hxx>
#endif
#ifndef _LIST_HXX
#include <tools/list.hxx>
#endif
#ifndef _STREAM_HXX
#include <tools/stream.hxx>
#endif
#ifndef _DATETIME_HXX
#include <tools/datetime.hxx>
#endif

#include <tools/string.hxx>

// SummaryInformation
#define PID_TITLE               0x02
#define PID_SUBJECT             0x03
#define PID_AUTHOR              0x04
#define PID_KEYWORDS            0x05
#define PID_COMMENTS            0x06
#define PID_TEMPLATE            0x07
#define PID_LASTAUTHOR          0x08
#define PID_REVNUMBER           0x09
#define PID_EDITTIME            0x0a
#define PID_LASTPRINTED_DTM     0x0b
#define PID_CREATE_DTM          0x0c
#define PID_LASTSAVED_DTM       0x0d

// DocumentSummaryInformation
#define PID_CATEGORY            0x02
#define PID_PRESFORMAT          0x03
#define PID_BYTECOUNT           0x04
#define PID_LINECOUNT           0x05
#define PID_PARACOUNT           0x06
#define PID_SLIDECOUNT          0x07
#define PID_NOTECOUNT           0x08
#define PID_HIDDENCOUNT         0x09
#define PID_MMCLIPCOUNT         0x0a
#define PID_SCALE               0x0b
#define PID_HEADINGPAIR         0x0c
#define PID_DOCPARTS            0x0d
#define PID_MANAGER             0x0e
#define PID_COMPANY             0x0f
#define PID_LINKSDIRTY          0x10

#define VT_EMPTY            0
#define VT_NULL             1
#define VT_I2               2
#define VT_I4               3
#define VT_R4               4
#define VT_R8               5
#define VT_CY               6
#define VT_DATE             7
#define VT_BSTR             8
#define VT_UI4              9
#define VT_ERROR            10
#define VT_BOOL             11
#define VT_VARIANT          12
#define VT_DECIMAL          14
#define VT_I1               16
#define VT_UI1              17
#define VT_UI2              18
#define VT_I8               20
#define VT_UI8              21
#define VT_INT              22
#define VT_UINT             23
#define VT_LPSTR            30
#define VT_LPWSTR           31
#define VT_FILETIME         64
#define VT_BLOB             65
#define VT_STREAM           66
#define VT_STORAGE          67
#define VT_STREAMED_OBJECT  68
#define VT_STORED_OBJECT    69
#define VT_BLOB_OBJECT      70
#define VT_CF               71
#define VT_CLSID            72
#define VT_VECTOR           0x1000
#define VT_ARRAY            0x2000
#define VT_BYREF            0x4000
#define VT_TYPEMASK         0xFFF

// ------------------------------------------------------------------------

class PropItem : public SvMemoryStream
{
    public :
                        PropItem(){};
        void            Clear();

        BOOL            Read( String& rString, UINT32 nType = VT_EMPTY, BOOL bDwordAlign = TRUE );
        PropItem&       operator=( PropItem& rPropItem );
};

// ------------------------------------------------------------------------

class Dictionary : protected List
{
    friend class Section;

        void        AddProperty( UINT32 nId, const String& rString );

    public :
                    Dictionary(){};
                    ~Dictionary();
        Dictionary& operator=( Dictionary& rDictionary );
        UINT32      GetProperty( const String& rPropName );
};

// ------------------------------------------------------------------------

class Section : private List
{

    protected:

        BYTE                    aFMTID[ 16 ];

        void                    AddProperty( UINT32 nId, const BYTE* pBuf, UINT32 nBufSize );

    public:
                                Section( const BYTE* pFMTID );
                                Section( Section& rSection );
                                ~Section();

        Section&                operator=( Section& rSection );
        BOOL                    GetProperty( UINT32 nId, PropItem& rPropItem );
        BOOL                    GetDictionary( Dictionary& rDict );
        const BYTE*             GetFMTID() const { return aFMTID; };
        void                    Read( SvStorageStream* pStrm );
};

// ------------------------------------------------------------------------

class PropRead : private List
{
        BOOL                    mbStatus;
        SvStorageStream*        mpSvStream;

        UINT16                  mnByteOrder;
        UINT16                  mnFormat;
        UINT16                  mnVersionLo;
        UINT16                  mnVersionHi;
        BYTE                    mApplicationCLSID[ 16 ];

        void                    AddSection( Section& rSection );

    public:
                                PropRead( SvStorage& rSvStorage, const String& rName );
                                ~PropRead();

        PropRead&               operator=( PropRead& rPropRead );
        const Section*          GetSection( const BYTE* pFMTID );
        BOOL                    IsValid() const { return mbStatus; };
        void                    Read();
};


#endif

