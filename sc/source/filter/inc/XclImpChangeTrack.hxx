/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: XclImpChangeTrack.hxx,v $
 *
 *  $Revision: 1.20 $
 *
 *  last change: $Author: obo $ $Date: 2006-07-10 13:50:28 $
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

#ifndef SC_XCLIMPCHANGETRACK_HXX
#define SC_XCLIMPCHANGETRACK_HXX

#ifndef _DATETIME_HXX
#include <tools/datetime.hxx>
#endif

#ifndef SC_XIROOT_HXX
#include "xiroot.hxx"
#endif
#ifndef SC_XISTREAM_HXX
#include "xistream.hxx"
#endif

#ifndef _EXCFORM_HXX
#include "excform.hxx"
#endif
#ifndef _IMP_OP_HXX
#include "imp_op.hxx"
#endif


//___________________________________________________________________

class ScBaseCell;
class ScChangeAction;
class ScChangeTrack;
class XclImpChTrFmlConverter;

//___________________________________________________________________

struct XclImpChTrRecHeader
{
    sal_uInt32                  nSize;
    sal_uInt32                  nIndex;
    sal_uInt16                  nOpCode;
    sal_uInt16                  nAccept;
};

inline XclImpStream& operator>>( XclImpStream& rStrm, XclImpChTrRecHeader& rRecHeader )
{
    rStrm >> rRecHeader.nSize >> rRecHeader.nIndex >> rRecHeader.nOpCode >> rRecHeader.nAccept;
    return rStrm;
}

//___________________________________________________________________

class XclImpChangeTrack : protected XclImpRoot
{
private:
    XclImpChTrRecHeader         aRecHeader;
    String                      sOldUsername;

    ScChangeTrack*              pChangeTrack;
    SotStorageStreamRef          xInStrm;        // input stream
    XclImpStream*               pStrm;          // stream import class
    sal_uInt16                  nTabIdCount;
    sal_Bool                    bGlobExit;      // global exit loop

    enum { nmBase, nmFound, nmNested }
                                eNestedMode;    // action with nested content actions

    inline sal_Bool             FoundNestedMode() { return eNestedMode == nmFound; }

    void                        DoAcceptRejectAction( ScChangeAction* pAction );
    void                        DoAcceptRejectAction( sal_uInt32 nFirst, sal_uInt32 nLast );

    void                        DoInsertRange( const ScRange& rRange );
    void                        DoDeleteRange( const ScRange& rRange );

    inline sal_uInt8            LookAtuInt8();
    inline double               ReadRK();
    inline sal_Bool             ReadBool();
    inline void                 Read2DAddress( ScAddress& rAddress );
    inline void                 Read2DRange( ScRange& rRange );
    SCTAB                       ReadTabNum();
    void                        ReadDateTime( DateTime& rDateTime );

    inline void                 ReadString( String& rString );
    inline void                 IgnoreString();

    sal_Bool                    CheckRecord( sal_uInt16 nOpCode );

    void                        ReadFormula(
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
    void                        InitNestedMode();           // 0x014E, 0x0150
    void                        ReadNestedRecords();
    sal_Bool                    EndNestedMode();            // 0x014F, 0x0151

    void                        ReadRecords();

public:
                                XclImpChangeTrack( const XclImpRoot& rRoot, const XclImpStream& rBookStrm );
                                ~XclImpChangeTrack();

                                // reads extended 3D ref info following the formulas, returns sc tab nums
                                // ( called by XclImpChTrFmlConverter::Read3DTabReference() )
    sal_Bool                    Read3DTabRefInfo( SCTAB& rFirstTab, SCTAB& rLastTab );

    void                        Apply();
};

inline sal_uInt8 XclImpChangeTrack::LookAtuInt8()
{
    pStrm->PushPosition();
    sal_uInt8 nValue;
    *pStrm >> nValue;
    pStrm->PopPosition();
    return nValue;
}

inline double XclImpChangeTrack::ReadRK()
{
    return XclTools::GetDoubleFromRK( pStrm->ReadInt32() );
}

inline sal_Bool XclImpChangeTrack::ReadBool()
{
    return (pStrm->ReaduInt16() != 0);
}

inline void XclImpChangeTrack::Read2DAddress( ScAddress& rAddress )
{
    rAddress.SetRow( static_cast<SCROW>(pStrm->ReaduInt16()) );
    rAddress.SetCol( static_cast<SCCOL>(pStrm->ReaduInt16()) );
}

inline void XclImpChangeTrack::Read2DRange( ScRange& rRange )
{
    rRange.aStart.SetRow( static_cast<SCROW>(pStrm->ReaduInt16()) );
    rRange.aEnd.SetRow( static_cast<SCROW>(pStrm->ReaduInt16()) );
    rRange.aStart.SetCol( static_cast<SCCOL>(pStrm->ReaduInt16()) );
    rRange.aEnd.SetCol( static_cast<SCCOL>(pStrm->ReaduInt16()) );
}

inline void XclImpChangeTrack::ReadString( String& rString )
{
    rString = pStrm->ReadUniString();
}

inline void XclImpChangeTrack::IgnoreString()
{
    pStrm->IgnoreUniString();
}

//___________________________________________________________________
// derived class for special 3D ref handling

class XclImpChTrFmlConverter : public ExcelToSc8
{
private:
    XclImpChangeTrack&          rChangeTrack;

    virtual BOOL                Read3DTabReference( XclImpStream& rStrm, SCTAB& rFirstTab, SCTAB& rLastTab );

public:
    inline                      XclImpChTrFmlConverter(
                                    const XclImpRoot& rRoot,
                                    XclImpChangeTrack& rXclChTr );
    virtual                     ~XclImpChTrFmlConverter();
};

inline XclImpChTrFmlConverter::XclImpChTrFmlConverter(
        const XclImpRoot& rRoot,
        XclImpChangeTrack& rXclChTr ) :
    ExcelToSc8( rRoot ),
    rChangeTrack( rXclChTr )
{
}

//___________________________________________________________________

#endif

