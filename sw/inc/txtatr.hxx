/*************************************************************************
 *
 *  $RCSfile: txtatr.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: ama $ $Date: 2000-09-25 12:00:00 $
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
#ifndef _TXTATR_HXX
#define _TXTATR_HXX

#ifndef _SV_GEN_HXX
#include <tools/gen.hxx>
#endif
#ifndef _TOOLS_COLOR_HXX
#include <tools/color.hxx>
#endif
#ifndef _LANG_HXX
#include <tools/lang.hxx>
#endif
#ifndef _SV_FONTTYPE_HXX //autogen
#include <vcl/fonttype.hxx>
#endif
#ifndef _SVX_SVXENUM_HXX //autogen
#include <svx/svxenum.hxx>
#endif

#ifndef _TXATBASE_HXX
#include <txatbase.hxx>     // SwTxtAttr/SwTxtAttrEnd
#endif

class SwTxtNode;    // fuer SwTxtFld
class SvxFont;
class SwCharSetCol;
class SwImplPrev;

// ATT_FONT ***********************************************************

class SwTxtFont: public SwTxtAttrEnd
{
    // Hier werden die alten Werte aus dem Font bei ChgFnt() gemerkt.
    String aPrevName;
    String aPrevStyleName;
    FontFamily ePrevFamily;
    FontPitch ePrevPitch;
    CharSet ePrevCharSet;
    BYTE nScript;
public:
    SwTxtFont( const SvxFontItem& rAttr, xub_StrLen nStart, xub_StrLen nEnd,
               const BYTE nScrpt );
    ~SwTxtFont( );

    virtual void ChgFnt(SwFont *);
    virtual void RstFnt(SwFont *);
    virtual void ChgTxtAttr( SwTxtAttr & );
    virtual void RstTxtAttr( SwTxtAttr & );
};

// ATT_POSTURE ***********************************************************

class SwTxtPosture : public SwTxtAttrEnd
{
    FontItalic ePrevPosture;
    BYTE nScript;
public:
    SwTxtPosture( const SvxPostureItem& rAttr, xub_StrLen nStart,
        xub_StrLen nEnd, const BYTE nScrpt );

    virtual void ChgFnt(SwFont *);
    virtual void RstFnt(SwFont *);
    virtual void ChgTxtAttr( SwTxtAttr & );
    virtual void RstTxtAttr( SwTxtAttr & );
};

// ATT_WEIGHT ************************************************************

class SwTxtWeight : public SwTxtAttrEnd
{
    // Hier merkt es sich das SV-Attribut Weight aus dem Font.
    FontWeight ePrevWeight;
    BYTE nScript;
public:
    SwTxtWeight( const SvxWeightItem& rAttr, xub_StrLen nStart, xub_StrLen nEnd,
        const BYTE nScrpt );

    virtual void ChgFnt(SwFont *);
    virtual void RstFnt(SwFont *);
    virtual void ChgTxtAttr( SwTxtAttr & );
    virtual void RstTxtAttr( SwTxtAttr & );
};

// ATT_SHADOWED **********************************************************

class SwTxtShadowed : public SwTxtAttrEnd
{
    BOOL bPrevShadow;
public:
    SwTxtShadowed( const SvxShadowedItem& rAttr, xub_StrLen nStart, xub_StrLen nEnd );

    virtual void ChgFnt(SwFont *);
    virtual void RstFnt(SwFont *);
    virtual void ChgTxtAttr( SwTxtAttr & );
    virtual void RstTxtAttr( SwTxtAttr & );
};

// ATT_AUTOKERN **********************************************************

class SwTxtAutoKern : public SwTxtAttrEnd
{
    BOOL bPrevAutoKern;
public:
    SwTxtAutoKern( const SvxAutoKernItem& rAttr, xub_StrLen nStart, xub_StrLen nEnd );

    virtual void ChgFnt(SwFont *);
    virtual void RstFnt(SwFont *);
};

// ATT_WORDLINEMODE **********************************************************

class SwTxtWordLineMode : public SwTxtAttrEnd
{
    BOOL bPrevWordLineMode;
public:
    SwTxtWordLineMode( const SvxWordLineModeItem& rAttr, xub_StrLen nStart, xub_StrLen nEnd );

    virtual void ChgFnt(SwFont *);
    virtual void RstFnt(SwFont *);
    virtual void ChgTxtAttr( SwTxtAttr & );
    virtual void RstTxtAttr( SwTxtAttr & );
};

// ATT_CONTOUR ***********************************************************

class SwTxtContour : public SwTxtAttrEnd
{
    BOOL bPrevContour;
public:
    SwTxtContour( const SvxContourItem& rAttr, xub_StrLen nStart, xub_StrLen nEnd );

    virtual void ChgFnt(SwFont *);
    virtual void RstFnt(SwFont *);
    virtual void ChgTxtAttr( SwTxtAttr & );
    virtual void RstTxtAttr( SwTxtAttr & );
};

// ATT_CROSSEDOUT ********************************************************

class SwTxtCrossedOut : public SwTxtAttrEnd
{
    FontStrikeout ePrevCrossedOut;
public:
    SwTxtCrossedOut( const SvxCrossedOutItem& rAttr, xub_StrLen nStart, xub_StrLen nEnd );

    virtual void ChgFnt(SwFont *);
    virtual void RstFnt(SwFont *);
    virtual void ChgTxtAttr( SwTxtAttr & );
    virtual void RstTxtAttr( SwTxtAttr & );
};

// ATT_UNDERLINE *********************************************************

class SwTxtUnderline : public SwTxtAttrEnd
{
    FontUnderline ePrevUnderline;
public:
    SwTxtUnderline( const SvxUnderlineItem& rAttr, xub_StrLen nStart, xub_StrLen nEnd );

    virtual void ChgFnt(SwFont *);
    virtual void RstFnt(SwFont *);
    virtual void ChgTxtAttr( SwTxtAttr & );
    virtual void RstTxtAttr( SwTxtAttr & );
};

// ATT_SIZE **************************************************************

class SwTxtSize : public SwTxtAttrEnd
{
    Size aPrevSize;
    BYTE nScript;
public:
    SwTxtSize( const SvxFontHeightItem& rAttr, xub_StrLen nStart,
        xub_StrLen nEnd, const BYTE nScrpt );

    virtual void ChgFnt(SwFont *);
    virtual void RstFnt(SwFont *);
    virtual void ChgTxtAttr( SwTxtAttr & );
    virtual void RstTxtAttr( SwTxtAttr & );
};

// ATT_COLOR *************************************************************

class SwTxtColor : public SwTxtAttrEnd
{
    friend class SwTxtCharSetColor;
    Color aPrevColor;
public:
    SwTxtColor( const SvxColorItem& rAttr, xub_StrLen nStart, xub_StrLen nEnd );

    virtual void ChgFnt(SwFont *);
    virtual void RstFnt(SwFont *);
    virtual void ChgTxtAttr( SwTxtAttr & );
    virtual void RstTxtAttr( SwTxtAttr & );
};

// ATT_CHARSETCOLOR ******************************************************

class SwTxtCharSetColor : public SwTxtAttrEnd
{
    SwCharSetCol *pPrevCharSetCol;
public:
    SwTxtCharSetColor( const SvxCharSetColorItem& rAttr, xub_StrLen nStart, xub_StrLen nEnd );
    ~SwTxtCharSetColor();
    virtual void ChgFnt(SwFont *);
    virtual void RstFnt(SwFont *);
    virtual void ChgTxtAttr( SwTxtAttr & );
    virtual void RstTxtAttr( SwTxtAttr & );
};

// ATT_CHARFMT *********************************************

class SwTxtCharFmt : public SwTxtAttrEnd
{
    SwImplPrev *pImpl;
    SwTxtNode* pMyTxtNd;
    BOOL bPrevNoHyph    : 1;
    BOOL bPrevBlink     : 1;
    BOOL bPrevURL       : 1;
    BOOL bColor         : 1;

public:
    SwTxtCharFmt( const SwFmtCharFmt& rAttr, xub_StrLen nStart, xub_StrLen nEnd );
    ~SwTxtCharFmt( );

    virtual void ChgFnt(SwFont *);
    virtual void RstFnt(SwFont *);

    // werden vom SwFmtCharFmt hierher weitergeleitet
    virtual void Modify( SfxPoolItem*, SfxPoolItem* );    // SwClient
    virtual BOOL GetInfo( SfxPoolItem& rInfo ) const;

    // erfrage und setze den TxtNode Pointer
    inline const SwTxtNode& GetTxtNode() const;
    void ChgTxtNode( const SwTxtNode* pNew ) { pMyTxtNd = (SwTxtNode*)pNew; }

};


// ATT_KERNING ***********************************************************

class SwTxtKerning : public SwTxtAttrEnd
{
    short nPrevKern;
public:
    SwTxtKerning( const SvxKerningItem& rAttr, xub_StrLen nStart, xub_StrLen nEnd );

    virtual void ChgFnt(SwFont *);
    virtual void RstFnt(SwFont *);
    virtual void ChgTxtAttr( SwTxtAttr & );
    virtual void RstTxtAttr( SwTxtAttr & );
};

// ATT_CASEMAP ***********************************************************

class SwTxtCaseMap : public SwTxtAttrEnd
{
    SvxCaseMap ePrevCaseMap;
public:
    SwTxtCaseMap( const SvxCaseMapItem& rAttr, xub_StrLen nStart, xub_StrLen nEnd );

    virtual void ChgFnt(SwFont *);
    virtual void RstFnt(SwFont *);
    virtual void ChgTxtAttr( SwTxtAttr & );
    virtual void RstTxtAttr( SwTxtAttr & );
};

// ATT_LANGUAGE **********************************************************

class SwTxtLanguage : public SwTxtAttrEnd
{
    LanguageType ePrevLang;
    BYTE nScript;
public:
    SwTxtLanguage( const SvxLanguageItem& rAttr, xub_StrLen nStart,
        xub_StrLen nEnd, const BYTE nScrpt );

    virtual void ChgFnt(SwFont *);
    virtual void RstFnt(SwFont *);
};


// ATT_ESCAPEMENT ********************************************************

class SwTxtEscapement : public SwTxtAttrEnd
{
    short   nPrevEsc;
    BYTE    nPrevPropr;

public:
    SwTxtEscapement( const SvxEscapementItem& rAttr, xub_StrLen nStart, xub_StrLen nEnd );

    virtual void ChgFnt(SwFont *);
    virtual void RstFnt(SwFont *);
    virtual void ChgTxtAttr( SwTxtAttr & );
    virtual void RstTxtAttr( SwTxtAttr & );
};

// ATT_BLINK ***********************

class SwTxtBlink : public SwTxtAttrEnd
{
    BOOL bPrev;
public:
    SwTxtBlink( const SvxBlinkItem& rAttr,
                        xub_StrLen nStart, xub_StrLen nEnd );

    virtual void ChgFnt(SwFont *);
    virtual void RstFnt(SwFont *);
    virtual void ChgTxtAttr( SwTxtAttr & );
    virtual void RstTxtAttr( SwTxtAttr & );
};

// ATT_BACKGROUND ***********************

class SwTxtBackground : public SwTxtAttrEnd
{
    Color *pPrevColor;
public:
    SwTxtBackground( const SvxBrushItem& rAttr,
                        xub_StrLen nStart, xub_StrLen nEnd );
    ~SwTxtBackground();
    virtual void ChgFnt(SwFont *);
    virtual void RstFnt(SwFont *);
    virtual void ChgTxtAttr( SwTxtAttr & );
    virtual void RstTxtAttr( SwTxtAttr & );
};

// ATT_NOHYPHENHERE **************************

class SwTxtNoHyphenHere : public SwTxtAttrEnd
{
    BOOL bPrev;
public:
    SwTxtNoHyphenHere( const SvxNoHyphenItem& rAttr,
                        xub_StrLen nStart, xub_StrLen nEnd );

    virtual void ChgFnt(SwFont *);
    virtual void RstFnt(SwFont *);
};

// ATT_HARDBLANK ******************************

class SwTxtHardBlank : public SwTxtAttr
{
    sal_Unicode cChar;
public:
    SwTxtHardBlank( const SwFmtHardBlank& rAttr, xub_StrLen nStart );
    inline sal_Unicode GetChar() const  { return cChar; }
};


// ATT_XNLCONTAINERITEM ******************************

class SwTxtXMLAttrContainer : public SwTxtAttrEnd
{
public:
    SwTxtXMLAttrContainer( const SvXMLAttrContainerItem& rAttr,
                        xub_StrLen nStart, xub_StrLen nEnd );
};



// --------------- Inline Implementierungen ------------------------


inline const SwTxtNode& SwTxtCharFmt::GetTxtNode() const
{
    ASSERT( pMyTxtNd, "SwTxtCharFmt:: wo ist mein TextNode?" );
    return *pMyTxtNd;
}



#endif
