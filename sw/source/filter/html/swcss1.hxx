/*************************************************************************
 *
 *  $RCSfile: swcss1.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:14:56 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef _SWCSS1_HXX
#define _SWCSS1_HXX

#ifndef _POOLFMT_HXX
#include "poolfmt.hxx"
#endif

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
public:
    SwCSS1Parser( SwDoc *pDoc, ULONG aFHeight[7], BOOL bNewDoc );
    virtual ~SwCSS1Parser();

    virtual BOOL ParseStyleSheet( const String& rIn );

    // Die Font-Hoehe fuer eine bestimmte Font-Groesse (0-6) ermitteln
    virtual ULONG GetFontHeight( USHORT nSize ) const;

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


