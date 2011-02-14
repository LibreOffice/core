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

    sal_uLong aFontHeights[7];

    sal_uInt16 nDropCapCnt;

    sal_Bool bIsNewDoc : 1;

    sal_Bool bBodyBGColorSet : 1;
    sal_Bool bBodyBackgroundSet : 1;
    sal_Bool bBodyTextSet : 1;
    sal_Bool bBodyLinkSet : 1;
    sal_Bool bBodyVLinkSet : 1;

    sal_Bool bSetFirstPageDesc : 1;
    sal_Bool bSetRightPageDesc : 1;

    sal_Bool bTableHeaderTxtCollSet : 1;
    sal_Bool bTableTxtCollSet : 1;

    sal_Bool bLinkCharFmtsSet : 1;

    // die Vorlagen fuer DL anlegen
    SwTxtFmtColl* GetDefListTxtFmtColl( sal_uInt16 nCollId, sal_uInt16 nDeep );

    const SwPageDesc* GetPageDesc( sal_uInt16 nPoolId, sal_Bool bCreate );

    void SetTableTxtColl( sal_Bool bHeader );
    void SetLinkCharFmts();

protected:
    virtual sal_Bool StyleParsed( const CSS1Selector *pSelector,
                              SfxItemSet& rItemSet,
                              SvxCSS1PropertyInfo& rPropInfo );

    using CSS1Parser::ParseStyleSheet;

public:
    SwCSS1Parser( SwDoc *pDoc, sal_uInt32 aFHeight[7], const String& rBaseURL, sal_Bool bNewDoc );
    virtual ~SwCSS1Parser();

    virtual sal_Bool ParseStyleSheet( const String& rIn );

    // Die Font-Hoehe fuer eine bestimmte Font-Groesse (0-6) ermitteln
    virtual sal_uInt32 GetFontHeight( sal_uInt16 nSize ) const;

    // Die aktuelle Font-Liste holen (auch 0 ist erlaubt)
    virtual const FontList *GetFontList() const;

    // das Zeichen-Format zu einem Token und einer ggf leeren Klasse
    // ermitteln
    SwCharFmt* GetChrFmt( sal_uInt16 nToken, const String& rClass ) const;

    // eine TextFmtColl zu einer Pool-Id ermitteln
    SwTxtFmtColl *GetTxtFmtColl( sal_uInt16 nTxtColl, const String& rClass );

    // This methods do the same as the one of SwDoc, but change the
    // encoding if required.
    SwTxtFmtColl *GetTxtCollFromPool( sal_uInt16 nPoolId ) const;
    SwCharFmt *GetCharFmtFromPool( sal_uInt16 nPoolId ) const;

    // Die linke oder rechte Seiten-Vorlage holen. In Dokumenten mit nur
    // einer Vorlage gibt es nur eine rechtee Seite.
    // Ansonsten ist die rechte Seite die HTML-Poolvorlage und die linke
    // eine Benutzter-Vorlage, die on-demand angelegt wird, wenn
    // bCreate gesetzt ist.
    SwPageDesc* GetMasterPageDesc();
    inline const SwPageDesc* GetFirstPageDesc( sal_Bool bCreate=sal_False );
    inline const SwPageDesc* GetRightPageDesc( sal_Bool bCreate=sal_False );
    inline const SwPageDesc* GetLeftPageDesc( sal_Bool bCreate=sal_False );

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

    sal_Bool SetFmtBreak( SfxItemSet& rItemSet,
                      const SvxCSS1PropertyInfo& rPropInfo );


    static void AddClassName( String& rFmtName, const String& rClass );

    static inline void AddFirstLetterExt( String& rFmtName );

    static sal_Bool MayBePositioned( const SvxCSS1PropertyInfo& rPropInfo,
                                 sal_Bool bAutoWidth=sal_False );

    static sal_uInt16 GetScriptFromClass( String& rClass,
                                      sal_Bool bSubClassOnly = sal_True );

    sal_Bool IsBodyBGColorSet() const { return bBodyBGColorSet; }
    sal_Bool IsBodyBackgroundSet() const { return bBodyBackgroundSet; }
    sal_Bool IsBodyTextSet() const { return bBodyTextSet; }
    sal_Bool IsBodyLinkSet() const { return bBodyLinkSet; }
    sal_Bool IsBodyVLinkSet() const { return bBodyVLinkSet; }

    sal_Bool IsSetFirstPageDesc() const { return bSetFirstPageDesc; }
    sal_Bool IsSetRightPageDesc() const { return bSetRightPageDesc; }

    void SetBodyBGColorSet() { bBodyBGColorSet = sal_True; }
    void SetBodyBackgroundSet() { bBodyBackgroundSet = sal_True; }
    void SetBodyTextSet() { bBodyTextSet = sal_True; }
    void SetBodyLinkSet() { bBodyLinkSet = sal_True; }
    void SetBodyVLinkSet() { bBodyVLinkSet = sal_True; }

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

inline const SwPageDesc* SwCSS1Parser::GetFirstPageDesc( sal_Bool bCreate )
{
    return GetPageDesc( RES_POOLPAGE_FIRST, bCreate );
}

inline const SwPageDesc* SwCSS1Parser::GetRightPageDesc( sal_Bool bCreate )
{
    return GetPageDesc( RES_POOLPAGE_RIGHT, bCreate );
}

inline const SwPageDesc* SwCSS1Parser::GetLeftPageDesc( sal_Bool bCreate )
{
    return GetPageDesc( RES_POOLPAGE_LEFT, bCreate );
}

inline void SwCSS1Parser::SetTHTagStyles()
{
    if( !bTableHeaderTxtCollSet )
        SetTableTxtColl( sal_True );
}

inline void SwCSS1Parser::SetTDTagStyles()
{
    if( !bTableTxtCollSet )
        SetTableTxtColl( sal_False );
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


