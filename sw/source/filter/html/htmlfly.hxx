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

#ifndef _HTMLFLY_HXX
#define _HTMLFLY_HXX

#include <tools/solar.h>
#include <o3tl/sorted_vector.hxx>

class SdrObject;
class SwFrmFmt;
class SwNodeIndex;
class SwPosFlyFrm;

// ACHTUNG: Die Werte dieses Enumgs gehen direkt in die
// Augabe Tabelle!!!
enum SwHTMLFrmType
{
    HTML_FRMTYPE_TABLE,
    HTML_FRMTYPE_TABLE_CAP,
    HTML_FRMTYPE_MULTICOL,
    HTML_FRMTYPE_EMPTY,
    HTML_FRMTYPE_TEXT,
    HTML_FRMTYPE_GRF,
    HTML_FRMTYPE_PLUGIN,
    HTML_FRMTYPE_APPLET,
    HTML_FRMTYPE_IFRAME,
    HTML_FRMTYPE_OLE,
    HTML_FRMTYPE_MARQUEE,
    HTML_FRMTYPE_CONTROL,
    HTML_FRMTYPE_DRAW,
    HTML_FRMTYPE_END
};

#define HTML_OUT_TBLNODE    0x00
#define HTML_OUT_GRFNODE    0x01
#define HTML_OUT_OLENODE    0x02
#define HTML_OUT_DIV        0x03
#define HTML_OUT_MULTICOL   0x04
#define HTML_OUT_SPACER     0x05
#define HTML_OUT_CONTROL    0x06
#define HTML_OUT_AMARQUEE   0x07
#define HTML_OUT_MARQUEE    0x08
#define HTML_OUT_GRFFRM     0x09
#define HTML_OUT_OLEGRF     0x0a
#define HTML_OUT_SPAN       0x0b
#define HTML_OUT_MASK       0x0f

#define HTML_POS_PREFIX     0x00
#define HTML_POS_BEFORE     0x10
#define HTML_POS_INSIDE     0x20
#define HTML_POS_ANY        0x30
#define HTML_POS_MASK       0x30

#define HTML_CNTNR_NONE     0x00
#define HTML_CNTNR_SPAN     0x40
#define HTML_CNTNR_DIV      0x80
#define HTML_CNTNR_MASK     0xc0


const sal_uInt16 MAX_FRMTYPES = HTML_FRMTYPE_END;
const sal_uInt16 MAX_BROWSERS = 4;

extern sal_uInt8 aHTMLOutFrmPageFlyTable[MAX_FRMTYPES][MAX_BROWSERS];
extern sal_uInt8 aHTMLOutFrmParaFrameTable[MAX_FRMTYPES][MAX_BROWSERS];
extern sal_uInt8 aHTMLOutFrmParaPrtAreaTable[MAX_FRMTYPES][MAX_BROWSERS];
extern sal_uInt8 aHTMLOutFrmParaOtherTable[MAX_FRMTYPES][MAX_BROWSERS];
extern sal_uInt8 aHTMLOutFrmAsCharTable[MAX_FRMTYPES][MAX_BROWSERS];

class SwHTMLPosFlyFrm
{
    const SwFrmFmt      *pFrmFmt;       // der Rahmen
    const SdrObject     *pSdrObject;    // ggf. Sdr-Objekt
    SwNodeIndex         *pNdIdx;        // Node-Index
    sal_uInt32              nOrdNum;        // Aus SwPosFlyFrm
    sal_Int32          nCntntIdx;      // seine Position im Content
    sal_uInt8               nOutputMode;    // Ausgabe-Infos

public:

    SwHTMLPosFlyFrm( const SwPosFlyFrm& rPosFly,
                     const SdrObject *pSdrObj, sal_uInt8 nOutMode );

    bool operator==( const SwHTMLPosFlyFrm& ) const { return false; }
    bool operator<( const SwHTMLPosFlyFrm& ) const;

    const SwFrmFmt& GetFmt() const { return *pFrmFmt; }
    const SdrObject *GetSdrObject() const { return pSdrObject; }

    const SwNodeIndex& GetNdIndex() const { return *pNdIdx; }

    sal_Int32 GetCntntIndex() const    { return nCntntIdx; }

    sal_uInt8 GetOutMode() const { return nOutputMode; }

    static sal_uInt8 GetOutFn( sal_uInt8 nMode ) { return nMode & HTML_OUT_MASK; }
    static sal_uInt8 GetOutPos( sal_uInt8 nMode ) { return nMode & HTML_POS_MASK; }
    static sal_uInt8 GetOutCntnr( sal_uInt8 nMode ) { return nMode & HTML_CNTNR_MASK; }

    sal_uInt8 GetOutFn() const { return nOutputMode & HTML_OUT_MASK; }
    sal_uInt8 GetOutPos() const { return nOutputMode & HTML_POS_MASK; }
    sal_uInt8 GetOutCntnr() const { return nOutputMode & HTML_CNTNR_MASK; }
};

class SwHTMLPosFlyFrms
    : public o3tl::sorted_vector<SwHTMLPosFlyFrm*,
                o3tl::less_ptr_to<SwHTMLPosFlyFrm>,
                o3tl::find_partialorder_ptrequals>
{};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
