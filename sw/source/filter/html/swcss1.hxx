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

#ifndef INCLUDED_SW_SOURCE_FILTER_HTML_SWCSS1_HXX
#define INCLUDED_SW_SOURCE_FILTER_HTML_SWCSS1_HXX

#include <sal/config.h>

#include <string_view>

#include <svtools/htmltokn.h>
#include <tools/solar.h>

#include <poolfmt.hxx>

#include "svxcss1.hxx"

class SwDoc;
class SwCharFormat;
class SwTextFormatColl;
class SvxBrushItem;
class SwFormatDrop;
class SwPageDesc;
class SwHTMLParser;

// This header looks harmless, but includes still quite
// inconspicuous one or the other! On the other hand this class
// is rarely needed. Therefore its own header.

class SwCSS1Parser : public SvxCSS1Parser
{
    SwDoc *m_pDoc;
    SwHTMLParser const& m_rHTMLParser;

    sal_uLong m_aFontHeights[7];

    sal_uInt16 m_nDropCapCnt;

    bool m_bIsNewDoc : 1;

    bool m_bBodyBGColorSet : 1;
    bool m_bBodyBackgroundSet : 1;
    bool m_bBodyTextSet : 1;
    bool m_bBodyLinkSet : 1;
    bool m_bBodyVLinkSet : 1;

    bool m_bSetFirstPageDesc : 1;
    bool m_bSetRightPageDesc : 1;

    bool m_bTableHeaderTextCollSet : 1;
    bool m_bTableTextCollSet : 1;

    bool m_bLinkCharFormatsSet : 1;

    const SwPageDesc* GetPageDesc( sal_uInt16 nPoolId, bool bCreate );

    void SetTableTextColl( bool bHeader );
    void SetLinkCharFormats();

protected:
    virtual void StyleParsed( const CSS1Selector *pSelector,
                              SfxItemSet& rItemSet,
                              SvxCSS1PropertyInfo& rPropInfo ) override;

    using CSS1Parser::ParseStyleSheet;

public:
    SwCSS1Parser( SwDoc *pDoc, SwHTMLParser const& rParser,
        sal_uInt32 const aFHeight[7], const OUString& rBaseURL, bool bNewDoc);
    virtual ~SwCSS1Parser() override;

    virtual bool ParseStyleSheet( const OUString& rIn ) override;

    // determine font height for a certain font size (0-6)
    virtual sal_uInt32 GetFontHeight( sal_uInt16 nSize ) const override;

    // fetch current font list (also zero is allowed)
    virtual const FontList *GetFontList() const override;

    // determine the character format of a token and a maybe empty class
    SwCharFormat* GetChrFormat( HtmlTokenId nToken, const OUString& rClass ) const;

    // determine a TextFormatColl of a Pool-Id
    SwTextFormatColl *GetTextFormatColl( sal_uInt16 nTextColl, const OUString& rClass );

    // This methods do the same as the one of SwDoc, but change the
    // encoding if required.
    SwTextFormatColl *GetTextCollFromPool( sal_uInt16 nPoolId ) const;
    SwCharFormat *GetCharFormatFromPool( sal_uInt16 nPoolId ) const;

    // Fetch the left or right page style. In documents with only
    // one style there is only a right page.
    // Otherwise the right page is the HTML pool style and the left
    // page a user style which is created on-demand if bCreate is set.
    SwPageDesc* GetMasterPageDesc();
    inline const SwPageDesc* GetFirstPageDesc( bool bCreate=false );
    inline const SwPageDesc* GetRightPageDesc( bool bCreate=false );
    inline const SwPageDesc* GetLeftPageDesc( bool bCreate=false );

    // Set attributes on the HTML page style (set attributes are
    // deleted from the Item-Set). Is called for the BODY tag.
    void SetPageDescAttrs( const SvxBrushItem *pBrush,
                           SfxItemSet *pItemSet=nullptr );

    void ChgPageDesc( const SwPageDesc *pPageDesc,
                      const SwPageDesc& rNewPageDesc );

    // Is called for @page
    void SetPageDescAttrs( const SwPageDesc *pPageDesc, SfxItemSet& rItemSet,
                           const SvxCSS1PropertyInfo& rPropInfo );

    // Fill a DropCap attribute
    void FillDropCap( SwFormatDrop& rDrop, SfxItemSet& rItemSet,
                      const OUString *pName=nullptr );

    bool SetFormatBreak( SfxItemSet& rItemSet,
                      const SvxCSS1PropertyInfo& rPropInfo );

    static void AddClassName( OUString& rFormatName, std::u16string_view rClass );

    static bool MayBePositioned( const SvxCSS1PropertyInfo& rPropInfo,
                                 bool bAutoWidth=false );

    static Css1ScriptFlags GetScriptFromClass( OUString& rClass,
                                               bool bSubClassOnly = true );

    bool IsBodyBGColorSet() const { return m_bBodyBGColorSet; }
    bool IsBodyBackgroundSet() const { return m_bBodyBackgroundSet; }
    bool IsBodyTextSet() const { return m_bBodyTextSet; }
    bool IsBodyLinkSet() const { return m_bBodyLinkSet; }
    bool IsBodyVLinkSet() const { return m_bBodyVLinkSet; }

    bool IsSetFirstPageDesc() const { return m_bSetFirstPageDesc; }
    bool IsSetRightPageDesc() const { return m_bSetRightPageDesc; }

    void SetBodyBGColorSet() { m_bBodyBGColorSet = true; }
    void SetBodyBackgroundSet() { m_bBodyBackgroundSet = true; }
    void SetBodyTextSet() { m_bBodyTextSet = true; }
    void SetBodyLinkSet() { m_bBodyLinkSet = true; }
    void SetBodyVLinkSet() { m_bBodyVLinkSet = true; }

    std::unique_ptr<SvxBrushItem> makePageDescBackground() const;

    inline void SetTHTagStyles();
    inline void SetTDTagStyles();
    inline void SetATagStyles();
    inline void SetDelayedStyles();

    virtual void SetDfltEncoding( rtl_TextEncoding eEnc ) override;
};

inline const SwPageDesc* SwCSS1Parser::GetFirstPageDesc( bool bCreate )
{
    return GetPageDesc( RES_POOLPAGE_FIRST, bCreate );
}

inline const SwPageDesc* SwCSS1Parser::GetRightPageDesc( bool bCreate )
{
    return GetPageDesc( RES_POOLPAGE_RIGHT, bCreate );
}

inline const SwPageDesc* SwCSS1Parser::GetLeftPageDesc( bool bCreate )
{
    return GetPageDesc( RES_POOLPAGE_LEFT, bCreate );
}

inline void SwCSS1Parser::SetTHTagStyles()
{
    if( !m_bTableHeaderTextCollSet )
        SetTableTextColl( true );
}

inline void SwCSS1Parser::SetTDTagStyles()
{
    if( !m_bTableTextCollSet )
        SetTableTextColl( false );
}

inline void SwCSS1Parser::SetATagStyles()
{
    if( !m_bLinkCharFormatsSet )
        SetLinkCharFormats();
}

inline void SwCSS1Parser::SetDelayedStyles()
{
    SetTHTagStyles();
    SetTDTagStyles();
    SetATagStyles();
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
