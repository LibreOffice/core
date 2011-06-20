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

#ifndef SC_XCLIMPCHANGETRACK_HXX
#define SC_XCLIMPCHANGETRACK_HXX

#include <tools/datetime.hxx>
#include "xiroot.hxx"
#include "xistream.hxx"
#include "excform.hxx"
#include "imp_op.hxx"


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
    sal_Bool                    Read3DTabRefInfo( SCTAB& rFirstTab, SCTAB& rLastTab, ExcelToSc8::ExternalTabInfo& rExtInfo );

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

//___________________________________________________________________
// derived class for special 3D ref handling

class XclImpChTrFmlConverter : public ExcelToSc8
{
private:
    XclImpChangeTrack&          rChangeTrack;

    virtual bool                Read3DTabReference( sal_uInt16 nIxti, SCTAB& rFirstTab, SCTAB& rLastTab, ExternalTabInfo& rExtInfo );

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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
