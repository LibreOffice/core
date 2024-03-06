/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#pragma once

#include "xiroot.hxx"
#include "xistream.hxx"
#include "excform.hxx"

struct ScCellValue;
class ScChangeAction;
class ScChangeTrack;
class DateTime;

struct XclImpChTrRecHeader
{
    sal_uInt32                  nSize;
    sal_uInt32                  nIndex;
    sal_uInt16                  nOpCode;
    sal_uInt16                  nAccept;
};

inline XclImpStream& operator>>( XclImpStream& rStrm, XclImpChTrRecHeader& rRecHeader )
{
    rRecHeader.nSize = rStrm.ReaduInt32();
    rRecHeader.nIndex = rStrm.ReaduInt32();
    rRecHeader.nOpCode = rStrm.ReaduInt16();
    rRecHeader.nAccept = rStrm.ReaduInt16();
    return rStrm;
}

class XclImpChangeTrack : protected XclImpRoot
{
private:
    XclImpChTrRecHeader         aRecHeader;
    OUString                    sOldUsername;

    std::unique_ptr<ScChangeTrack> pChangeTrack;
    rtl::Reference<SotStorageStream> xInStrm; // input stream
    std::unique_ptr<XclImpStream>  pStrm;          // stream import class
    sal_uInt16                  nTabIdCount;
    bool                        bGlobExit;      // global exit loop

    enum { nmBase, nmFound, nmNested }
                                eNestedMode;    // action with nested content actions

    bool                 FoundNestedMode() { return eNestedMode == nmFound; }

    void                        DoAcceptRejectAction( ScChangeAction* pAction );
    void                        DoAcceptRejectAction( sal_uInt32 nFirst, sal_uInt32 nLast );

    void DoInsertRange( const ScRange& rRange, bool bEndOfList );
    void                        DoDeleteRange( const ScRange& rRange );

    inline sal_uInt8            LookAtuInt8();
    inline void                 Read2DAddress( ScAddress& rAddress );
    inline void                 Read2DRange( ScRange& rRange );
    SCTAB                       ReadTabNum();
    void                        ReadDateTime( DateTime& rDateTime );

    bool                        CheckRecord( sal_uInt16 nOpCode );

    void                        ReadFormula(
                                    std::unique_ptr<ScTokenArray>& rpTokenArray,
                                    const ScAddress& rPosition );
    void ReadCell( ScCellValue& rCell, sal_uInt32& rFormat, sal_uInt16 nFlags, const ScAddress& rPosition );

    void                        ReadChTrInsert();           // 0x0137
    void                        ReadChTrInfo();             // 0x0138
    void                        ReadChTrCellContent();      // 0x013B
    void                        ReadChTrTabId();            // 0x013D
    void                        ReadChTrMoveRange();        // 0x0140
    void                        ReadChTrInsertTab();        // 0x014D
    void                        InitNestedMode();           // 0x014E, 0x0150
    void                        ReadNestedRecords();
    bool                        EndNestedMode();            // 0x014F, 0x0151

    void                        ReadRecords();

public:
                                XclImpChangeTrack( const XclImpRoot& rRoot, const XclImpStream& rBookStrm );
                                virtual ~XclImpChangeTrack() override;

                                // reads extended 3D ref info following the formulas, returns sc tab nums
                                // ( called by XclImpChTrFmlConverter::Read3DTabReference() )
    void                        Read3DTabRefInfo( SCTAB& rFirstTab, SCTAB& rLastTab, ExcelToSc8::ExternalTabInfo& rExtInfo );

    void                        Apply();
};

inline sal_uInt8 XclImpChangeTrack::LookAtuInt8()
{
    pStrm->PushPosition();
    sal_uInt8 nValue;
    nValue = pStrm->ReaduInt8();
    pStrm->PopPosition();
    return nValue;
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

// derived class for special 3D ref handling

class XclImpChTrFmlConverter : public ExcelToSc8
{
private:
    XclImpChangeTrack&          rChangeTrack;

    virtual bool                Read3DTabReference( sal_uInt16 nIxti, SCTAB& rFirstTab, SCTAB& rLastTab, ExternalTabInfo& rExtInfo ) override;

public:
    XclImpChTrFmlConverter( XclImpRoot& rRoot, XclImpChangeTrack& rXclChTr );
    virtual                     ~XclImpChTrFmlConverter() override;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
