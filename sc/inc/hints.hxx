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

#ifndef SC_HINTS_HXX
#define SC_HINTS_HXX

#include "global.hxx"
#include "address.hxx"
#include <svl/hint.hxx>

class ScPaintHint : public SfxHint
{
    ScRange     aRange;
    sal_uInt16      nParts;
    sal_Bool        bPrint;     //  flag indicating whether print/preview if affected

    ScPaintHint(); // disabled

public:
                    TYPEINFO();
                    ScPaintHint( const ScRange& rRng, sal_uInt16 nPaint = PAINT_ALL );
                    ~ScPaintHint();

    void            SetPrintFlag(sal_Bool bSet) { bPrint = bSet; }

    const ScRange&  GetRange() const        { return aRange; }
    SCCOL           GetStartCol() const     { return aRange.aStart.Col(); }
    SCROW           GetStartRow() const     { return aRange.aStart.Row(); }
    SCTAB           GetStartTab() const     { return aRange.aStart.Tab(); }
    SCCOL           GetEndCol() const       { return aRange.aEnd.Col(); }
    SCROW           GetEndRow() const       { return aRange.aEnd.Row(); }
    SCTAB           GetEndTab() const       { return aRange.aEnd.Tab(); }
    sal_uInt16          GetParts() const        { return nParts; }
    sal_Bool            GetPrintFlag() const    { return bPrint; }
};

class ScUpdateRefHint : public SfxHint
{
    UpdateRefMode   eUpdateRefMode;
    ScRange         aRange;
    SCsCOL          nDx;
    SCsROW          nDy;
    SCsTAB          nDz;

public:
                    TYPEINFO();

                    ScUpdateRefHint( UpdateRefMode eMode, const ScRange& rR,
                                        SCsCOL nX, SCsROW nY, SCsTAB nZ );
                    ~ScUpdateRefHint();

    UpdateRefMode   GetMode() const         { return eUpdateRefMode; }
    const ScRange&  GetRange() const        { return aRange; }
    SCsCOL          GetDx() const           { return nDx; }
    SCsROW          GetDy() const           { return nDy; }
    SCsTAB          GetDz() const           { return nDz; }
};

#define SC_POINTERCHANGED_NUMFMT    1

class ScPointerChangedHint : public SfxHint
{
    sal_uInt16          nFlags;

public:
                    TYPEINFO();

                    ~ScPointerChangedHint();

    sal_uInt16          GetFlags() const            { return nFlags; }
};

//! move ScLinkRefreshedHint to a different file?

#define SC_LINKREFTYPE_NONE     0
#define SC_LINKREFTYPE_SHEET    1
#define SC_LINKREFTYPE_AREA     2
#define SC_LINKREFTYPE_DDE      3

class ScLinkRefreshedHint : public SfxHint
{
    sal_uInt16      nLinkType;  // SC_LINKREFTYPE_...
    String      aUrl;       // used for sheet links
    String      aDdeAppl;   // used for dde links:
    String      aDdeTopic;
    String      aDdeItem;
    sal_uInt8       nDdeMode;
    ScAddress   aDestPos;   // used to identify area links
                            //! also use source data for area links?

public:
                    TYPEINFO();
                    ScLinkRefreshedHint();
                    ~ScLinkRefreshedHint();

    void            SetSheetLink( const String& rSourceUrl );
    void            SetDdeLink( const String& rA, const String& rT, const String& rI, sal_uInt8 nM );
    void            SetAreaLink( const ScAddress& rPos );

    sal_uInt16              GetLinkType() const { return nLinkType; }
    const String&       GetUrl() const      { return aUrl; }
    const String&       GetDdeAppl() const  { return aDdeAppl; }
    const String&       GetDdeTopic() const { return aDdeTopic; }
    const String&       GetDdeItem() const  { return aDdeItem; }
    sal_uInt8               GetDdeMode() const  { return nDdeMode; }
    const ScAddress&    GetDestPos() const  { return aDestPos; }
};

//! move ScAutoStyleHint to a different file?

class ScAutoStyleHint : public SfxHint
{
    ScRange     aRange;
    String      aStyle1;
    String      aStyle2;
    sal_uLong       nTimeout;

public:
                    TYPEINFO();
                    ScAutoStyleHint( const ScRange& rR, const String& rSt1,
                                        sal_uLong nT, const String& rSt2 );
                    ~ScAutoStyleHint();

    const ScRange&  GetRange() const    { return aRange; }
    const String&   GetStyle1() const   { return aStyle1; }
    sal_uInt32          GetTimeout() const  { return nTimeout; }
    const String&   GetStyle2() const   { return aStyle2; }
};

class ScDBRangeRefreshedHint : public SfxHint
{
    ScImportParam   aParam;

public:
                    TYPEINFO();
                    ScDBRangeRefreshedHint( const ScImportParam& rP );
                    ~ScDBRangeRefreshedHint();

    const ScImportParam&  GetImportParam() const    { return aParam; }
};

class ScDataPilotModifiedHint : public SfxHint
{
    String          maName;

public:
                    TYPEINFO();
                    ScDataPilotModifiedHint( const String& rName );
                    ~ScDataPilotModifiedHint();

    const String&   GetName() const { return maName; }
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
