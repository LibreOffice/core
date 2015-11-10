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

#include "poolfmt.hxx"

#include "svxcss1.hxx"

class SwDoc;
class SwCharFormat;
class SwTextFormatColl;
class SvxBrushItem;
class SwFormatDrop;
class SwPageDesc;

// Dieser Header seiht zwar harmlos aus, included aber eben doch
// ganz unauffaellig das ein oder andere! Andererseits wird diese
// Klasse recht selten benoetigt. Deshalb ein eigener Header.

class SwCSS1Parser : public SvxCSS1Parser
{
    SwDoc *pDoc;

    sal_uLong aFontHeights[7];

    sal_uInt16 nDropCapCnt;

    bool bIsNewDoc : 1;

    bool bBodyBGColorSet : 1;
    bool bBodyBackgroundSet : 1;
    bool bBodyTextSet : 1;
    bool bBodyLinkSet : 1;
    bool bBodyVLinkSet : 1;

    bool bSetFirstPageDesc : 1;
    bool bSetRightPageDesc : 1;

    bool bTableHeaderTextCollSet : 1;
    bool bTableTextCollSet : 1;

    bool bLinkCharFormatsSet : 1;

    const SwPageDesc* GetPageDesc( sal_uInt16 nPoolId, bool bCreate );

    void SetTableTextColl( bool bHeader );
    void SetLinkCharFormats();

protected:
    virtual bool StyleParsed( const CSS1Selector *pSelector,
                              SfxItemSet& rItemSet,
                              SvxCSS1PropertyInfo& rPropInfo ) override;

    using CSS1Parser::ParseStyleSheet;

public:
    SwCSS1Parser( SwDoc *pDoc, sal_uInt32 aFHeight[7], const OUString& rBaseURL, bool bNewDoc );
    virtual ~SwCSS1Parser();

    virtual bool ParseStyleSheet( const OUString& rIn ) override;

    // Die Font-Hoehe fuer eine bestimmte Font-Groesse (0-6) ermitteln
    virtual sal_uInt32 GetFontHeight( sal_uInt16 nSize ) const override;

    // Die aktuelle Font-Liste holen (auch 0 ist erlaubt)
    virtual const FontList *GetFontList() const override;

    // das Zeichen-Format zu einem Token und einer ggf leeren Klasse
    // ermitteln
    SwCharFormat* GetChrFormat( sal_uInt16 nToken, const OUString& rClass ) const;

    // eine TextFormatColl zu einer Pool-Id ermitteln
    SwTextFormatColl *GetTextFormatColl( sal_uInt16 nTextColl, const OUString& rClass );

    // This methods do the same as the one of SwDoc, but change the
    // encoding if required.
    SwTextFormatColl *GetTextCollFromPool( sal_uInt16 nPoolId ) const;
    SwCharFormat *GetCharFormatFromPool( sal_uInt16 nPoolId ) const;

    // Die linke oder rechte Seiten-Vorlage holen. In Dokumenten mit nur
    // einer Vorlage gibt es nur eine rechtee Seite.
    // Ansonsten ist die rechte Seite die HTML-Poolvorlage und die linke
    // eine Benutzter-Vorlage, die on-demand angelegt wird, wenn
    // bCreate gesetzt ist.
    SwPageDesc* GetMasterPageDesc();
    inline const SwPageDesc* GetFirstPageDesc( bool bCreate=false );
    inline const SwPageDesc* GetRightPageDesc( bool bCreate=false );
    inline const SwPageDesc* GetLeftPageDesc( bool bCreate=false );

    // Attribute an der HTML-Seitenvorlage setzen (gesetzte Attribute
    // werden aus dem Item-Set geloescht ). Wird fuer's BODY-Tag
    // aufgerufen.
    void SetPageDescAttrs( const SvxBrushItem *pBrush,
                           SfxItemSet *pItemSet=nullptr );

    void ChgPageDesc( const SwPageDesc *pPageDesc,
                      const SwPageDesc& rNewPageDesc );

    // Wird fuer @page aufgerufen.
    void SetPageDescAttrs( const SwPageDesc *pPageDesc, SfxItemSet& rItemSet,
                           const SvxCSS1PropertyInfo& rPropInfo );

    // Fuellen eines DropCap-Attributs
    void FillDropCap( SwFormatDrop& rDrop, SfxItemSet& rItemSet,
                      const OUString *pName=nullptr );

    bool SetFormatBreak( SfxItemSet& rItemSet,
                      const SvxCSS1PropertyInfo& rPropInfo );

    static void AddClassName( OUString& rFormatName, const OUString& rClass );

    static inline void AddFirstLetterExt( OUString& rFormatName );

    static bool MayBePositioned( const SvxCSS1PropertyInfo& rPropInfo,
                                 bool bAutoWidth=false );

    static sal_uInt16 GetScriptFromClass( OUString& rClass,
                                      bool bSubClassOnly = true );

    bool IsBodyBGColorSet() const { return bBodyBGColorSet; }
    bool IsBodyBackgroundSet() const { return bBodyBackgroundSet; }
    bool IsBodyTextSet() const { return bBodyTextSet; }
    bool IsBodyLinkSet() const { return bBodyLinkSet; }
    bool IsBodyVLinkSet() const { return bBodyVLinkSet; }

    bool IsSetFirstPageDesc() const { return bSetFirstPageDesc; }
    bool IsSetRightPageDesc() const { return bSetRightPageDesc; }

    void SetBodyBGColorSet() { bBodyBGColorSet = true; }
    void SetBodyBackgroundSet() { bBodyBackgroundSet = true; }
    void SetBodyTextSet() { bBodyTextSet = true; }
    void SetBodyLinkSet() { bBodyLinkSet = true; }
    void SetBodyVLinkSet() { bBodyVLinkSet = true; }

    SvxBrushItem makePageDescBackground() const;

    inline void SetTHTagStyles();
    inline void SetTDTagStyles();
    inline void SetATagStyles();
    inline void SetDelayedStyles();

    virtual void SetDfltEncoding( rtl_TextEncoding eEnc ) override;
};

inline void SwCSS1Parser::AddFirstLetterExt( OUString& rFormatName )
{
    rFormatName += ".FL";   // first letter
}

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
    if( !bTableHeaderTextCollSet )
        SetTableTextColl( true );
}

inline void SwCSS1Parser::SetTDTagStyles()
{
    if( !bTableTextCollSet )
        SetTableTextColl( false );
}

inline void SwCSS1Parser::SetATagStyles()
{
    if( !bLinkCharFormatsSet )
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
