/*************************************************************************
 *
 *  $RCSfile: XclImpChangeTrack.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: dr $ $Date: 2001-01-18 16:32:13 $
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

#ifndef _SC_XCLIMPCHANGETRACK_HXX
#define _SC_XCLIMPCHANGETRACK_HXX

#ifndef _DATETIME_HXX
#include <tools/datetime.hxx>
#endif

#ifndef _EXCFORM_HXX
#include "excform.hxx"
#endif
#ifndef _EXCIMP8_HXX
#include "excimp8.hxx"
#endif
#ifndef _FLTTOOLS_HXX
#include "flttools.hxx"
#endif
#ifndef _IMP_OP_HXX
#include "imp_op.hxx"
#endif
#ifndef _ROOT_HXX
#include "root.hxx"
#endif


//___________________________________________________________________

class ScBaseCell;
class ScChangeAction;
class ScChangeTrack;

//___________________________________________________________________

struct XclImpChTrRecHeader
{
    sal_uInt32                  nSize;
    sal_uInt32                  nIndex;
    sal_uInt16                  nOpCode;
    sal_uInt16                  nAccept;
};

//___________________________________________________________________

class XclImpChangeTrack : protected ExcRoot
{
private:
    XclImpChTrRecHeader         aRecHeader;
    String                      sOldUsername;

    ScChangeTrack*              pChangeTrack;
    SvStream*                   pInStrm;        // input stream
    SvMemoryStream*             pContStrm;      // memory stream for continue recs
    UINT32List                  aCutPosList;    // cut positions for string input
    SvStream*                   pStrm;          // either pInStrm or pContStrm
    sal_uInt32                  nStreamLen;
    sal_Int32                   nBytesLeft;
    sal_uInt16                  nTabIdCount;

    enum { nmBase, nmFound, nmNested }
                                eNestedMode;    // action with nested content actions

    inline sal_Bool             FoundNestedMode() { return eNestedMode == nmFound; }

    void                        DoAcceptRejectAction( ScChangeAction* pAction );
    void                        DoAcceptRejectAction( sal_uInt32 nFirst, sal_uInt32 nLast );

    void                        DoInsertRange( const ScRange& rRange );
    void                        DoDeleteRange( const ScRange& rRange );

    inline sal_Bool             CheckSize( sal_Int32 nBytes );
    inline void                 IgnoreBytes( sal_uInt32 nBytes );
    inline sal_uInt8            ReaduInt8();
    inline sal_uInt8            LookAtuInt8();
    inline sal_uInt16           ReaduInt16();
    inline sal_uInt32           ReaduInt32();
    inline double               ReadDouble();
    inline double               ReadRK();
    inline sal_Bool             ReadBool();
    inline void                 Read2DAddress( ScAddress& rAddress );
    inline void                 Read2DRange( ScRange& rRange );
    inline sal_uInt16           ReadTabNum();
    void                        ReadDateTime( DateTime& rDateTime );

    sal_Bool                    ReadStringHeader( sal_uInt16& rSize, sal_Bool& r16Bit );
    void                        ReadString( String& rString );
    void                        IgnoreString();
    void                        CopyFromStreamToStream( SvStream& rFromStrm, SvStream& rToStrm, sal_uInt16 nBytes );

    sal_Bool                    ReadRecordHeader();
    sal_Bool                    CheckRecord(
                                    sal_uInt16 nOpCode,
                                    sal_Int32 nMinSize = -1,
                                    sal_Int32 nMaxSize = -1 );

    sal_Bool                    ReadFormula(
                                    ScTokenArray*& rpTokenArray,
                                    const ScAddress& rPosition );
    void                        ReadCell(
                                    ScBaseCell*& rpCell,
                                    sal_uInt32& rFormat,
                                    sal_uInt16 nFlags,
                                    const ScAddress& rPosition );

    void                        ReadChTrInsert();           // 0x0137
    void                        ReadChTrInfo();             // 0x0138
    void                        ReadChTrCellContent();      // 0x013B
    void                        ReadChTrTabId();            // 0x013D
    void                        ReadChTrMoveRange();        // 0x0140
    void                        ReadChTrInsertTab();        // 0x014D
    void                        StartNestedMode();          // 0x014E, 0x0150
    sal_Bool                    EndNestedMode();            // 0x014F, 0x0151

                                // creates continue stream, if cont record present
                                // return: position of next regular record
    sal_uInt32                  PrepareReadRecord( sal_uInt16 nRecLen );
                                // destroys continue stream, sets input stream
                                // to next record
    void                        EndReadRecord( sal_uInt32 nNextPos );

    void                        ReadRecords();
    void                        ReadStream();

public:
                                XclImpChangeTrack( RootData* pRootData );
                                ~XclImpChangeTrack();

                                // reads extended 3D ref info following the formulas, returns sc tab nums
                                // ( called by XclImpChTrFmlConverter::Read3DTabReference() )
    sal_Bool                    Read3DTabRefInfo( sal_uInt16& rFirstTab, sal_uInt16& rLastTab );

    void                        Apply();
};

inline sal_Bool XclImpChangeTrack::CheckSize( sal_Int32 nBytes )
{
    DBG_ASSERT( nBytes <= nBytesLeft, "XclImpChangeTrack::CheckSize - unexpected end" );
    return nBytes <= nBytesLeft;
}

inline void XclImpChangeTrack::IgnoreBytes( sal_uInt32 nBytes )
{
    pStrm->SeekRel( nBytes );
    nBytesLeft -= nBytes;
}

inline sal_uInt8 XclImpChangeTrack::ReaduInt8()
{
    sal_uInt8 nValue;
    *pStrm >> nValue;
    nBytesLeft--;
    return nValue;
}

inline sal_uInt8 XclImpChangeTrack::LookAtuInt8()
{
    sal_uInt8 nValue;
    *pStrm >> nValue;
    pStrm->SeekRel( -1 );
    return nValue;
}

inline sal_uInt16 XclImpChangeTrack::ReaduInt16()
{
    sal_uInt16 nValue;
    *pStrm >> nValue;
    nBytesLeft -= 2;
    return nValue;
}

inline sal_uInt32 XclImpChangeTrack::ReaduInt32()
{
    sal_uInt32 nValue;
    *pStrm >> nValue;
    nBytesLeft -= 4;
    return nValue;
}

inline double XclImpChangeTrack::ReadDouble()
{
    double nDblValue;
    *pStrm >> nDblValue;
    nBytesLeft -= 8;
    return nDblValue;
}

inline double XclImpChangeTrack::ReadRK()
{
    sal_uInt32 nRKValue = ReaduInt32();
    return ImportExcel::RkToDouble( nRKValue );
}

inline sal_Bool XclImpChangeTrack::ReadBool()
{
    sal_uInt16 nBoolValue = ReaduInt16();
    return (nBoolValue != 0);
}

inline void XclImpChangeTrack::Read2DAddress( ScAddress& rAddress )
{
    rAddress.SetRow( ReaduInt16() );
    rAddress.SetCol( ReaduInt16() );
}

inline void XclImpChangeTrack::Read2DRange( ScRange& rRange )
{
    rRange.aStart.SetRow( ReaduInt16() );
    rRange.aEnd.SetRow( ReaduInt16() );
    rRange.aStart.SetCol( ReaduInt16() );
    rRange.aEnd.SetCol( ReaduInt16() );
}

inline sal_uInt16 XclImpChangeTrack::ReadTabNum()
{
    sal_uInt16 nTab = ReaduInt16();
    return pExcRoot->pImpTabIdBuffer->GetIndex( nTab, nTabIdCount );
}

//___________________________________________________________________
// derived class for special 3D ref handling

class XclImpChTrFmlConverter : public ExcelToSc8
{
private:
    XclImpChangeTrack&          rChangeTrack;
    UINT16                      nDummy;

    virtual BOOL                Read3DTabReference( UINT16& rFirstTab, UINT16& rLastTab );

public:
    inline                      XclImpChTrFmlConverter(
                                    RootData* pRootData,
                                    SvStream& rStrm,
                                    XclImpChangeTrack& rXclChTr );
    virtual                     ~XclImpChTrFmlConverter();
};

inline XclImpChTrFmlConverter::XclImpChTrFmlConverter(
        RootData* pRootData,
        SvStream& rStrm,
        XclImpChangeTrack& rXclChTr ) :
    ExcelToSc8( pRootData, rStrm, nDummy ),
    rChangeTrack( rXclChTr )
{
}

//___________________________________________________________________

#endif // _SC_XCLIMPCHANGETRACK_HXX

