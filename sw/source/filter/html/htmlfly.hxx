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

#ifndef INCLUDED_SW_SOURCE_FILTER_HTML_HTMLFLY_HXX
#define INCLUDED_SW_SOURCE_FILTER_HTML_HTMLFLY_HXX

#include <o3tl/sorted_vector.hxx>
#include <o3tl/typed_flags_set.hxx>
#include <sal/types.h>

class SdrObject;
class SwFrameFormat;
class SwNodeIndex;
class SwPosFlyFrame;

// ATTENTION: The values of this enum are used directly in the output table!!!
enum SwHTMLFrameType
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

enum class HtmlOut {
    TableNode,
    GraphicNode,
    OleNode,
    Div,
    MultiCol,
    Spacer,
    Control,
    AMarquee,
    Marquee,
    GraphicFrame,
    OleGraphic,
    Span
};

enum class HtmlPosition {
    Prefix,
    Before,
    Inside,
    Any
};

enum class HtmlContainerFlags {
    NONE     = 0x00,
    Span     = 0x01,
    Div      = 0x02,
};
namespace o3tl {
    template<> struct typed_flags<HtmlContainerFlags> : is_typed_flags<HtmlContainerFlags, 0x03> {};
}

const sal_uInt16 MAX_FRMTYPES = HTML_FRMTYPE_END;
const sal_uInt16 MAX_BROWSERS = 4;

struct AllHtmlFlags {
    HtmlOut            nOut;
    HtmlPosition       nPosition;
    HtmlContainerFlags nContainer;
};
extern AllHtmlFlags const aHTMLOutFramePageFlyTable[MAX_FRMTYPES][MAX_BROWSERS];
extern AllHtmlFlags const aHTMLOutFrameParaFrameTable[MAX_FRMTYPES][MAX_BROWSERS];
extern AllHtmlFlags const aHTMLOutFrameParaPrtAreaTable[MAX_FRMTYPES][MAX_BROWSERS];
extern AllHtmlFlags const aHTMLOutFrameParaOtherTable[MAX_FRMTYPES][MAX_BROWSERS];
extern AllHtmlFlags const aHTMLOutFrameAsCharTable[MAX_FRMTYPES][MAX_BROWSERS];

class SwHTMLPosFlyFrame
{
    const SwFrameFormat    *pFrameFormat;  // the frame
    const SdrObject        *pSdrObject;    // maybe Sdr-Object
    SwNodeIndex            *pNdIdx;        // Node-Index
    sal_uInt32              nOrdNum;       // from SwPosFlyFrame
    sal_Int32               nContentIdx;   // its position in content
    AllHtmlFlags            nAllFlags;

    SwHTMLPosFlyFrame(const SwHTMLPosFlyFrame&) = delete;
    SwHTMLPosFlyFrame& operator=(const SwHTMLPosFlyFrame&) = delete;

public:

    SwHTMLPosFlyFrame( const SwPosFlyFrame& rPosFly,
                     const SdrObject *pSdrObj, AllHtmlFlags nAllFlags );

    bool operator<( const SwHTMLPosFlyFrame& ) const;

    const SwFrameFormat& GetFormat() const       { return *pFrameFormat; }
    const SdrObject*     GetSdrObject() const    { return pSdrObject; }
    const SwNodeIndex&   GetNdIndex() const      { return *pNdIdx; }
    sal_Int32            GetContentIndex() const { return nContentIdx; }
    AllHtmlFlags const & GetOutMode() const      { return nAllFlags; }
    HtmlOut              GetOutFn() const        { return nAllFlags.nOut; }
    HtmlPosition         GetOutPos() const       { return nAllFlags.nPosition; }
};

class SwHTMLPosFlyFrames
    : public o3tl::sorted_vector<std::unique_ptr<SwHTMLPosFlyFrame>,
                o3tl::less_uniqueptr_to<SwHTMLPosFlyFrame>,
                o3tl::find_partialorder_ptrequals>
{};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
