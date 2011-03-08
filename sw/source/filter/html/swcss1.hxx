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

#ifndef _SWCSS1_HXX
#define _SWCSS1_HXX

#include "poolfmt.hxx"

#include "svxcss1.hxx"

class SwDoc;
class SwFmt;
class SwCharFmt;
class SwTxtFmtColl;
class SvxBrushItem;
class SwFmtDrop;
class SwPageDesc;

// Dieser Header seiht zwar harmlos aus, included aber eben doch
// ganz unauffaellig das ein oder andere! Andererseits wird diese
// Klasse recht selten benoetigt. Deshalb ein eigener Header.


class SwCSS1Parser : public SvxCSS1Parser
{
    SwDoc *pDoc;

    ULONG aFontHeights[7];

    USHORT nDropCapCnt;

    BOOL bIsNewDoc : 1;

    BOOL bBodyBGColorSet : 1;
    BOOL bBodyBackgroundSet : 1;
    BOOL bBodyTextSet : 1;
    BOOL bBodyLinkSet : 1;
    BOOL bBodyVLinkSet : 1;

    BOOL bSetFirstPageDesc : 1;
    BOOL bSetRightPageDesc : 1;

    BOOL bTableHeaderTxtCollSet : 1;
    BOOL bTableTxtCollSet : 1;

    BOOL bLinkCharFmtsSet : 1;

    // die Vorlagen fuer DL anlegen
    SwTxtFmtColl* GetDefListTxtFmtColl( USHORT nCollId, USHORT nDeep );

    const SwPageDesc* GetPageDesc( USHORT nPoolId, BOOL bCreate );

    void SetTableTxtColl( BOOL bHeader );
    void SetLinkCharFmts();

protected:
    virtual BOOL StyleParsed( const CSS1Selector *pSelector,
                              SfxItemSet& rItemSet,
                              SvxCSS1PropertyInfo& rPropInfo );

    using CSS1Parser::ParseStyleSheet;

public:
    SwCSS1Parser( SwDoc *pDoc, sal_uInt32 aFHeight[7], const String& rBaseURL, BOOL bNewDoc );
    virtual ~SwCSS1Parser();

    virtual BOOL ParseStyleSheet( const String& rIn );

    // Die Font-Hoehe fuer eine bestimmte Font-Groesse (0-6) ermitteln
    virtual sal_uInt32 GetFontHeight( USHORT nSize ) const;

    // Die aktuelle Font-Liste holen (auch 0 ist erlaubt)
    virtual const FontList *GetFontList() const;

    // das Zeichen-Format zu einem Token und einer ggf leeren Klasse
    // ermitteln
    SwCharFmt* GetChrFmt( USHORT nToken, const String& rClass ) const;

    // eine TextFmtColl zu einer Pool-Id ermitteln
    SwTxtFmtColl *GetTxtFmtColl( USHORT nTxtColl, const String& rClass );

    // This methods do the same as the one of SwDoc, but change the
    // encoding if required.
    SwTxtFmtColl *GetTxtCollFromPool( USHORT nPoolId ) const;
    SwCharFmt *GetCharFmtFromPool( USHORT nPoolId ) const;

    // Die linke oder rechte Seiten-Vorlage holen. In Dokumenten mit nur
    // einer Vorlage gibt es nur eine rechtee Seite.
    // Ansonsten ist die rechte Seite die HTML-Poolvorlage und die linke
    // eine Benutzter-Vorlage, die on-demand angelegt wird, wenn
    // bCreate gesetzt ist.
    SwPageDesc* GetMasterPageDesc();
    inline const SwPageDesc* GetFirstPageDesc( BOOL bCreate=FALSE );
    inline const SwPageDesc* GetRightPageDesc( BOOL bCreate=FALSE );
    inline const SwPageDesc* GetLeftPageDesc( BOOL bCreate=FALSE );

    // Attribute an der HTML-Seitenvorlage setzen (gesetzte Attribute
    // werden aus dem Item-Set geloescht ). Wird fuer's BODY-Tag
    // aufgerufen.
    void SetPageDescAttrs( const SvxBrushItem *pBrush,
                           SfxItemSet *pItemSet=0 );

    void ChgPageDesc( const SwPageDesc *pPageDesc,
                      const SwPageDesc& rNewPageDesc );

    // Wird fuer @page aufgerufen.
    void SetPageDescAttrs( const SwPageDesc *pPageDesc, SfxItemSet& rItemSet,
                           const SvxCSS1PropertyInfo& rPropInfo );

    // Fuellen eines DropCap-Attributs
    void FillDropCap( SwFmtDrop& rDrop, SfxItemSet& rItemSet,
                      const String *pName=0 );

    BOOL SetFmtBreak( SfxItemSet& rItemSet,
                      const SvxCSS1PropertyInfo& rPropInfo );


    static void AddClassName( String& rFmtName, const String& rClass );

    static inline void AddFirstLetterExt( String& rFmtName );

    static BOOL MayBePositioned( const SvxCSS1PropertyInfo& rPropInfo,
                                 BOOL bAutoWidth=FALSE );

    static sal_uInt16 GetScriptFromClass( String& rClass,
                                      sal_Bool bSubClassOnly = sal_True );

    BOOL IsBodyBGColorSet() const { return bBodyBGColorSet; }
    BOOL IsBodyBackgroundSet() const { return bBodyBackgroundSet; }
    BOOL IsBodyTextSet() const { return bBodyTextSet; }
    BOOL IsBodyLinkSet() const { return bBodyLinkSet; }
    BOOL IsBodyVLinkSet() const { return bBodyVLinkSet; }

    BOOL IsSetFirstPageDesc() const { return bSetFirstPageDesc; }
    BOOL IsSetRightPageDesc() const { return bSetRightPageDesc; }

    void SetBodyBGColorSet() { bBodyBGColorSet = TRUE; }
    void SetBodyBackgroundSet() { bBodyBackgroundSet = TRUE; }
    void SetBodyTextSet() { bBodyTextSet = TRUE; }
    void SetBodyLinkSet() { bBodyLinkSet = TRUE; }
    void SetBodyVLinkSet() { bBodyVLinkSet = TRUE; }

    const SvxBrushItem& GetPageDescBackground() const;

    inline void SetTHTagStyles();
    inline void SetTDTagStyles();
    inline void SetATagStyles();
    inline void SetDelayedStyles();

    virtual void SetDfltEncoding( rtl_TextEncoding eEnc );
};


inline void SwCSS1Parser::AddFirstLetterExt( String& rFmtName )
{
    rFmtName.AppendAscii( ".FL", 3 );   // first letter
}

inline const SwPageDesc* SwCSS1Parser::GetFirstPageDesc( BOOL bCreate )
{
    return GetPageDesc( RES_POOLPAGE_FIRST, bCreate );
}

inline const SwPageDesc* SwCSS1Parser::GetRightPageDesc( BOOL bCreate )
{
    return GetPageDesc( RES_POOLPAGE_RIGHT, bCreate );
}

inline const SwPageDesc* SwCSS1Parser::GetLeftPageDesc( BOOL bCreate )
{
    return GetPageDesc( RES_POOLPAGE_LEFT, bCreate );
}

inline void SwCSS1Parser::SetTHTagStyles()
{
    if( !bTableHeaderTxtCollSet )
        SetTableTxtColl( TRUE );
}

inline void SwCSS1Parser::SetTDTagStyles()
{
    if( !bTableTxtCollSet )
        SetTableTxtColl( FALSE );
}


inline void SwCSS1Parser::SetATagStyles()
{
    if( !bLinkCharFmtsSet )
        SetLinkCharFmts();
}

inline void SwCSS1Parser::SetDelayedStyles()
{
    SetTHTagStyles();
    SetTDTagStyles();
    SetATagStyles();
}


#endif


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
