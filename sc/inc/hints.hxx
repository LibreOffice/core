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

#ifndef INCLUDED_SC_INC_HINTS_HXX
#define INCLUDED_SC_INC_HINTS_HXX

#include "global.hxx"
#include "address.hxx"
#include <svl/hint.hxx>

class ScPaintHint : public SfxHint
{
    ScRange     aRange;
    sal_uInt16  nParts;
    bool        bPrint;     //  flag indicating whether print/preview if affected

public:
                    ScPaintHint() = delete;
                    ScPaintHint( const ScRange& rRng, sal_uInt16 nPaint = PAINT_ALL );
                    virtual ~ScPaintHint();

    void            SetPrintFlag(bool bSet) { bPrint = bSet; }
    SCCOL           GetStartCol() const     { return aRange.aStart.Col(); }
    SCROW           GetStartRow() const     { return aRange.aStart.Row(); }
    SCTAB           GetStartTab() const     { return aRange.aStart.Tab(); }
    SCCOL           GetEndCol() const       { return aRange.aEnd.Col(); }
    SCROW           GetEndRow() const       { return aRange.aEnd.Row(); }
    SCTAB           GetEndTab() const       { return aRange.aEnd.Tab(); }
    sal_uInt16      GetParts() const        { return nParts; }
    bool            GetPrintFlag() const    { return bPrint; }
};

class ScUpdateRefHint : public SfxHint
{
    UpdateRefMode   eUpdateRefMode;
    ScRange         aRange;
    SCsCOL          nDx;
    SCsROW          nDy;
    SCsTAB          nDz;

public:
                    ScUpdateRefHint( UpdateRefMode eMode, const ScRange& rR,
                                        SCsCOL nX, SCsROW nY, SCsTAB nZ );
                    virtual ~ScUpdateRefHint();

    UpdateRefMode   GetMode() const         { return eUpdateRefMode; }
    const ScRange&  GetRange() const        { return aRange; }
    SCsCOL          GetDx() const           { return nDx; }
    SCsROW          GetDy() const           { return nDy; }
    SCsTAB          GetDz() const           { return nDz; }
};

//! move ScLinkRefreshedHint to a different file?
enum class ScLinkRefType {
    NONE, SHEET, AREA, DDE
};

class ScLinkRefreshedHint : public SfxHint
{
    ScLinkRefType nLinkType;
    OUString    aUrl;       // used for sheet links
    OUString    aDdeAppl;   // used for dde links:
    OUString    aDdeTopic;
    OUString    aDdeItem;
    sal_uInt8   nDdeMode;
    ScAddress   aDestPos;   // used to identify area links
                            //! also use source data for area links?

public:
                    ScLinkRefreshedHint();
                    virtual ~ScLinkRefreshedHint();

    void            SetSheetLink( const OUString& rSourceUrl );
    void            SetDdeLink( const OUString& rA, const OUString& rT, const OUString& rI, sal_uInt8 nM );
    void            SetAreaLink( const ScAddress& rPos );

    ScLinkRefType       GetLinkType() const { return nLinkType; }
    const OUString&     GetUrl() const      { return aUrl; }
    const OUString&     GetDdeAppl() const  { return aDdeAppl; }
    const OUString&     GetDdeTopic() const { return aDdeTopic; }
    const OUString&     GetDdeItem() const  { return aDdeItem; }
    const ScAddress&    GetDestPos() const  { return aDestPos; }
};

//! move ScAutoStyleHint to a different file?

class ScAutoStyleHint : public SfxHint
{
    ScRange     aRange;
    OUString    aStyle1;
    OUString    aStyle2;
    sal_uLong   nTimeout;

public:
                    ScAutoStyleHint( const ScRange& rR, const OUString& rSt1,
                                        sal_uLong nT, const OUString& rSt2 );
                    virtual ~ScAutoStyleHint();

    const ScRange&  GetRange() const    { return aRange; }
    const OUString& GetStyle1() const   { return aStyle1; }
    sal_uInt32      GetTimeout() const  { return nTimeout; }
    const OUString& GetStyle2() const   { return aStyle2; }
};

class ScDBRangeRefreshedHint : public SfxHint
{
    ScImportParam   aParam;

public:
                    ScDBRangeRefreshedHint( const ScImportParam& rP );
                    virtual ~ScDBRangeRefreshedHint();

    const ScImportParam&  GetImportParam() const    { return aParam; }
};

class ScDataPilotModifiedHint : public SfxHint
{
    OUString        maName;

public:
                    ScDataPilotModifiedHint( const OUString& rName );
                    virtual ~ScDataPilotModifiedHint();

    const OUString&   GetName() const { return maName; }
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
