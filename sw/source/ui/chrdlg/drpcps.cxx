/*************************************************************************
 *
 *  $RCSfile: drpcps.cxx,v $
 *
 *  $Revision: 1.12 $
 *
 *  last change: $Author: kz $ $Date: 2003-10-15 10:00:09 $
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


#pragma hdrstop

#include <hintids.hxx>
#define _SVSTDARR_STRINGSDTOR
#define _SVSTDARR_STRINGSISORTDTOR
#include <svtools/svstdarr.hxx>

#include "cmdid.h"
#include "swmodule.hxx"
#include "view.hxx"
#include "wrtsh.hxx"
#include "globals.hrc"


#ifndef _METRIC_HXX //autogen
#include <vcl/metric.hxx>
#endif

#ifndef _SFXSTRITEM_HXX //autogen
#include <svtools/stritem.hxx>
#endif
#ifndef _SVX_FONTITEM_HXX //autogen
#include <svx/fontitem.hxx>
#endif
#ifndef _SVX_HTMLMODE_HXX //autogen
#include <svx/htmlmode.hxx>
#endif
#ifndef _SFX_OBJSH_HXX //autogen
#include <sfx2/objsh.hxx>
#endif
#ifndef _SVX_SVXFONT_HXX
#include <svx/svxfont.hxx>
#endif
#ifndef _SV_PRINT_HXX
#include <vcl/print.hxx>
#endif
#ifndef _SFX_PRINTER_HXX
#include <sfx2/printer.hxx>
#endif
#ifndef _COM_SUN_STAR_I18N_SCRIPTTYPE_HDL_
#include <com/sun/star/i18n/ScriptType.hdl>
#endif
#ifndef _SVX_SCRIPTTYPEITEM_HXX
#include <svx/scripttypeitem.hxx>
#endif
#ifndef _COM_SUN_STAR_UNO_REFERENCE_H_
#include <com/sun/star/uno/Reference.h>
#endif
#ifndef _COM_SUN_STAR_I18N_XBREAKITERATOR_HPP_
#include <com/sun/star/i18n/XBreakIterator.hpp>
#endif
#ifndef _COMPHELPER_PROCESSFACTORY_HXX_
#include <comphelper/processfactory.hxx>
#endif

#define _SVSTDARR_XUB_STRLEN
#define _SVSTDARR_USHORTS
#define _SVSTDARR_ULONGS
#include <svtools/svstdarr.hxx>

#include "charatr.hxx"
#include "viewopt.hxx"
#include "drpcps.hxx"
#include "paratr.hxx"
#include "uitool.hxx"
#include "charfmt.hxx"

#include "chrdlg.hrc"
#include "drpcps.hrc"


using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
//using namespace ::com::sun::star::i18n;   !using this namespace leads to mysterious conflicts with ScriptType::...!
//                                              so don't use this instead of the following defines!

#define I18N                ::com::sun::star::i18n
#define I18N_SCRIPTTYPE     ::com::sun::star::i18n::ScriptType

// Globals ******************************************************************

static USHORT __FAR_DATA aPageRg[] = {
    RES_PARATR_DROP, RES_PARATR_DROP,
    0
};



// class SwDropCapsPict *****************************************************

class SwDropCapsPict : public Control
{
    String          maText;
    String          maScriptText;
    Color           maBackColor;
    Color           maTextLineColor;
    BYTE            mnLines;
    long            mnTotLineH;
    long            mnLineH;
    long            mnTextH;
    USHORT          mnDistance;
    USHORT          mnLeading;
    Printer*        mpPrinter;
    BOOL            mbDelPrinter;
    SvULongs        maTextWidth;
    SvXub_StrLens   maScriptChg;
    SvUShorts       maScriptType;
    SvxFont         maFont;
    SvxFont         maCJKFont;
    SvxFont         maCTLFont;
    Size            maTextSize;
    Reference< I18N::XBreakIterator >   xBreak;

    virtual void    Paint(const Rectangle &rRect);
    void            CheckScript( void );
    Size            CalcTextSize( void );
    inline void     InitPrinter( void );
    void            _InitPrinter( void );
    void            GetFontSettings( const SwDropCapsPage& _rPage, Font& _rFont, USHORT _nWhich );
public:

     SwDropCapsPict(Window *pParent, const ResId &rResId) :
            Control(pParent, rResId), mpPrinter( NULL ), mbDelPrinter( FALSE ) {}
    ~SwDropCapsPict();

    void UpdatePaintSettings( void );       // also invalidates control!

    inline void SetText( const String& rT );
    inline void SetLines( BYTE nL );
    inline void SetDistance( USHORT nD );
    inline void SetValues( const String& rText, BYTE nLines, USHORT nDistance );

    void        DrawPrev( const Point& rPt );
};

inline void SwDropCapsPict::SetText( const String& rT )
{
    maText = rT;
    UpdatePaintSettings();
}

inline void SwDropCapsPict::SetLines( BYTE nL )
{
    mnLines = nL;
    UpdatePaintSettings();
}

inline void SwDropCapsPict::SetDistance( USHORT nD )
{
    mnDistance = nD;
    UpdatePaintSettings();
}

inline void SwDropCapsPict::SetValues( const String& rText, BYTE nLines, USHORT nDistance )
{
    maText = rText;
    mnLines = nLines;
    mnDistance = nDistance;

    UpdatePaintSettings();
}

inline void SwDropCapsPict::InitPrinter( void )
{
    if( !mpPrinter )
        _InitPrinter();
}

/****************************************************************************
Default-String aus Zeichenanzahl erzeugen (A, AB, ABC, ...)
****************************************************************************/


String GetDefaultString(USHORT nChars)
{
    String aStr;
    for (USHORT i = 0; i < nChars; i++)
        aStr += String((char) (i + 65));
    return aStr;
}

static void calcFontHeightAnyAscent( OutputDevice* _pWin, Font& _rFont, long& _nHeight, long& _nAscent )
{
    if ( !_nHeight )
    {
        _pWin->SetFont( _rFont );
        FontMetric aMetric( _pWin->GetFontMetric() );
        _nHeight = aMetric.GetLineHeight();
        _nAscent = aMetric.GetAscent();
    }
}

/****************************************************************************
Pict: Dtor
****************************************************************************/


 SwDropCapsPict::~SwDropCapsPict()
{
     if( mbDelPrinter )
         delete mpPrinter;
}

/****************************************************************************
Pict: Update Font
****************************************************************************/

#define LINES  10
#define BORDER  2

void SwDropCapsPict::GetFontSettings( const SwDropCapsPage& _rPage, Font& _rFont, USHORT _nWhich )
{
    SfxItemSet aSet( _rPage.rSh.GetAttrPool(), _nWhich, _nWhich);
    _rPage.rSh.GetAttr(aSet);
    SvxFontItem aFmtFont((SvxFontItem &) aSet.Get(_nWhich));

    _rFont.SetFamily (aFmtFont.GetFamily());
    _rFont.SetName   (aFmtFont.GetFamilyName());
    _rFont.SetPitch  (aFmtFont.GetPitch());
    _rFont.SetCharSet(aFmtFont.GetCharSet());
}

void SwDropCapsPict::UpdatePaintSettings( void )
{
    maBackColor = GetSettings().GetStyleSettings().GetWindowColor();
    maTextLineColor = Color( COL_LIGHTGRAY );

    // gray lines
    mnTotLineH = (GetOutputSizePixel().Height() - 2 * BORDER) / LINES;
    mnLineH = mnTotLineH - 2;
    mnLeading = (USHORT) GetFontMetric().GetIntLeading();

    Font aFont;
    {
        SwDropCapsPage* pPage = ( SwDropCapsPage* ) GetParent();
        if (!pPage->aTemplateBox.GetSelectEntryPos())
        {
            // Font an Absatzanfang erfragen
            pPage->rSh.SttCrsrMove();
            pPage->rSh.Push();
            pPage->rSh.ClearMark();
            pPage->rSh.MovePara(fnParaCurr,fnParaStart);

            // normal
            GetFontSettings( *pPage, aFont, RES_CHRATR_FONT );

            // CJK
            GetFontSettings( *pPage, maCJKFont, RES_CHRATR_CJK_FONT );

            // CTL
            GetFontSettings( *pPage, maCTLFont, RES_CHRATR_CTL_FONT );

            pPage->rSh.Pop(FALSE);
            pPage->rSh.EndCrsrMove();
        }
        else
        {
            // Font an Zeichenvorlage erfragen
            SwCharFmt *pFmt = pPage->rSh.GetCharStyle(
                                    pPage->aTemplateBox.GetSelectEntry(),
                                    SwWrtShell::GETSTYLE_CREATEANY );
            ASSERT(pFmt, "Zeichenvorlage existiert nicht!");
            const SvxFontItem &rFmtFont = pFmt->GetFont();

            aFont.SetFamily (rFmtFont.GetFamily());
            aFont.SetName   (rFmtFont.GetFamilyName());
            aFont.SetPitch  (rFmtFont.GetPitch());
            aFont.SetCharSet(rFmtFont.GetCharSet());
        }
    }

    mnTextH = mnLines * mnTotLineH;
    aFont.SetSize(Size(0, mnTextH));
    maCJKFont.SetSize(Size(0, mnTextH));
    maCTLFont.SetSize(Size(0, mnTextH));

    aFont.SetTransparent(TRUE);
    maCJKFont.SetTransparent(TRUE);
    maCTLFont.SetTransparent(TRUE);

    aFont.SetColor( SwViewOption::GetFontColor() );
    maCJKFont.SetColor( SwViewOption::GetFontColor() );
    maCTLFont.SetColor( SwViewOption::GetFontColor() );

    aFont.SetFillColor(GetSettings().GetStyleSettings().GetWindowColor());
    maCJKFont.SetFillColor(GetSettings().GetStyleSettings().GetWindowColor());
    maCTLFont.SetFillColor(GetSettings().GetStyleSettings().GetWindowColor());

    maCJKFont.SetSize(Size(0, maCJKFont.GetSize().Height() + mnLeading));
    maCTLFont.SetSize(Size(0, maCTLFont.GetSize().Height() + mnLeading));

    SetFont(aFont);
    aFont.SetSize(Size(0, aFont.GetSize().Height() + mnLeading));
    SetFont(aFont);
    maFont = aFont;

    CheckScript();

    maTextSize = CalcTextSize();

    Invalidate();
}

/****************************************************************************
Pict: Paint-Overload
****************************************************************************/

void  SwDropCapsPict::Paint(const Rectangle &rRect)
{
    if (!IsVisible())
        return;

    SetMapMode(MapMode(MAP_PIXEL));
    SetLineColor();

    SetFillColor( maBackColor );

    Size aOutputSizePixel( GetOutputSizePixel() );

    DrawRect(Rectangle(Point(0, 0), aOutputSizePixel ));
    SetClipRegion(Region(Rectangle(
        Point(BORDER, BORDER),
        Size (aOutputSizePixel.Width () - 2 * BORDER,
              aOutputSizePixel.Height() - 2 * BORDER))));

    ASSERT(mnLineH > 0, "So klein lassen wir uns nicht machen");
    long nY0 = (aOutputSizePixel.Height() - (LINES * mnTotLineH)) / 2;
    SetFillColor( maTextLineColor );
    for (USHORT i = 0; i < LINES; ++i)
        DrawRect(Rectangle(Point(BORDER, nY0 + i * mnTotLineH), Size(aOutputSizePixel.Width() - 2 * BORDER, mnLineH)));

    // Texthintergrund mit Abstand (240 twips ~ 1 Zeilenhoehe)
    ULONG lDistance = mnDistance;
    USHORT nDistW = (USHORT) (ULONG) (((lDistance * 100) / 240) * mnTotLineH) / 100;
    SetFillColor( maBackColor );
    if(((SwDropCapsPage*)GetParent())->aDropCapsBox.IsChecked())
    {
        Size    aTextSize( maTextSize );
        aTextSize.Width() += nDistW;
        DrawRect( Rectangle( Point( BORDER, nY0 ), aTextSize ) );

        // Text zeichnen
        DrawPrev( Point( BORDER, nY0 - mnLeading ) );
    }

    SetClipRegion();
}

void SwDropCapsPict::DrawPrev( const Point& rPt )
{
    Point aPt(rPt);
    InitPrinter();

    Font        aOldFont = mpPrinter->GetFont();
    USHORT      nScript;
    USHORT      nIdx = 0;
    xub_StrLen  nStart = 0;
    xub_StrLen  nEnd;
    USHORT      nCnt = maScriptChg.Count();

    if( nCnt )
    {
        nEnd = maScriptChg[ nIdx ];
        nScript = maScriptType[ nIdx ];
    }
    else
    {
        nEnd = maText.Len();
        nScript = I18N_SCRIPTTYPE::LATIN;
    }
    do
    {
        SvxFont&    rFnt = (nScript==I18N_SCRIPTTYPE::ASIAN) ? maCJKFont : ((nScript==I18N_SCRIPTTYPE::COMPLEX) ? maCTLFont : maFont);
        mpPrinter->SetFont( rFnt );

        rFnt.DrawPrev( this, mpPrinter, aPt, maText, nStart, nEnd - nStart );

        aPt.X() += maTextWidth[ nIdx++ ];
        if( nEnd < maText.Len() && nIdx < nCnt )
        {
            nStart = nEnd;
            nEnd = maScriptChg[ nIdx ];
            nScript = maScriptType[ nIdx ];
        }
        else
            break;
    }
    while( TRUE );
    mpPrinter->SetFont( aOldFont );
}

void SwDropCapsPict::CheckScript( void )
{
    if( maScriptText == maText )
        return;

    maScriptText = maText;
    USHORT nCnt = maScriptChg.Count();
    if( nCnt )
    {
        maScriptChg.Remove( 0, nCnt );
        maScriptType.Remove( 0, nCnt );
        maTextWidth.Remove( 0, nCnt );
        nCnt = 0;
    }
    if( !xBreak.is() )
    {
        Reference< XMultiServiceFactory > xMSF = ::comphelper::getProcessServiceFactory();
        xBreak = Reference< I18N::XBreakIterator >(xMSF->createInstance(
                ::rtl::OUString::createFromAscii( "com.sun.star.i18n.BreakIterator" ) ),UNO_QUERY);
    }
    if( xBreak.is() )
    {
        USHORT nScript = xBreak->getScriptType( maText, 0 );
        USHORT nChg = 0;
        if( I18N_SCRIPTTYPE::WEAK == nScript )
        {
            nChg = (xub_StrLen)xBreak->endOfScript( maText, nChg, nScript );
            if( nChg < maText.Len() )
                nScript = xBreak->getScriptType( maText, nChg );
            else
                nScript = I18N_SCRIPTTYPE::LATIN;
        }

        do
        {
            nChg = (xub_StrLen)xBreak->endOfScript( maText, nChg, nScript );
            maScriptChg.Insert( nChg, nCnt );
            maScriptType.Insert( nScript, nCnt );
            maTextWidth.Insert( ULONG(0), nCnt++ );

            if( nChg < maText.Len() )
                nScript = xBreak->getScriptType( maText, nChg );
            else
                break;
        } while( TRUE );
    }
}

Size SwDropCapsPict::CalcTextSize( void )
{
    InitPrinter();

    USHORT      nScript;
    USHORT      nIdx = 0;
    xub_StrLen  nStart = 0;
    xub_StrLen  nEnd;
    USHORT      nCnt = maScriptChg.Count();
    if( nCnt )
    {
        nEnd = maScriptChg[ nIdx ];
        nScript = maScriptType[ nIdx ];
    }
    else
    {
        nEnd = maText.Len();
        nScript = I18N_SCRIPTTYPE::LATIN;
    }
    long        nTxtWidth = 0;
    long        nCJKHeight = 0;
    long        nCTLHeight = 0;
    long        nHeight = 0;
    long        nAscent = 0;
    long        nCJKAscent = 0;
    long        nCTLAscent = 0;
    do
    {
        SvxFont&    rFnt = ( nScript == I18N_SCRIPTTYPE::ASIAN )? maCJKFont :
                                ( ( nScript == I18N_SCRIPTTYPE::COMPLEX )? maCTLFont : maFont );
        ULONG       nWidth = rFnt.GetTxtSize( mpPrinter, maText, nStart, nEnd-nStart ).Width();

        if( nIdx < maTextWidth.Count() )
            maTextWidth[ nIdx++ ] = nWidth;
        nTxtWidth += nWidth;
        switch(nScript)
        {
            case I18N_SCRIPTTYPE::ASIAN:
                calcFontHeightAnyAscent( this, maCJKFont, nCJKHeight, nCJKAscent );
                break;
            case I18N_SCRIPTTYPE::COMPLEX:
                calcFontHeightAnyAscent( this, maCTLFont, nCTLHeight, nCTLAscent );
                break;
            default:
                calcFontHeightAnyAscent( this, maFont, nHeight, nAscent );
        }

        if( nEnd < maText.Len() && nIdx < nCnt )
        {
            nStart = nEnd;
            nEnd = maScriptChg[ nIdx ];
            nScript = maScriptType[ nIdx ];
        }
        else
            break;
    }
    while( TRUE );
    nHeight -= nAscent;
    nCJKHeight -= nCJKAscent;
    nCTLHeight -= nCTLAscent;
    if( nHeight < nCJKHeight )
        nHeight = nCJKHeight;
    if( nAscent < nCJKAscent )
        nAscent = nCJKAscent;
    if( nHeight < nCTLHeight )
        nHeight = nCTLHeight;
    if( nAscent < nCTLAscent )
        nAscent = nCTLAscent;
    nHeight += nAscent;

    Size aTxtSize( nTxtWidth, nHeight );
    return aTxtSize;
}

void SwDropCapsPict::_InitPrinter()
{
    SfxViewShell*   pSh = SfxViewShell::Current();

    if ( pSh )
        mpPrinter = pSh->GetPrinter();

    if ( !mpPrinter )
    {
        mpPrinter = new Printer;
        mbDelPrinter = TRUE;
    }
}

/****************************************************************************
Dlg: Ctor
****************************************************************************/


SwDropCapsDlg::SwDropCapsDlg(Window *pParent, const SfxItemSet &rSet ) :

    SfxSingleTabDialog(pParent, rSet, 0)

{
    SwDropCapsPage* pPage = (SwDropCapsPage*) SwDropCapsPage::Create(this, rSet);
    pPage->SetFormat(FALSE);
    SetTabPage(pPage);
}

/****************************************************************************
Dlg: Dtor
****************************************************************************/


 SwDropCapsDlg::~SwDropCapsDlg()
{
}

/****************************************************************************
Page: Ctor
****************************************************************************/


SwDropCapsPage::SwDropCapsPage(Window *pParent, const SfxItemSet &rSet) :

    SfxTabPage(pParent, SW_RES(TP_DROPCAPS), rSet),

    aDropCapsBox  (this, SW_RES(CB_SWITCH   )),
    aWholeWordCB  (this, SW_RES(CB_WORD     )),
    aSwitchText   (this, SW_RES(FT_DROPCAPS )),
    aDropCapsField(this, SW_RES(FLD_DROPCAPS)),
    aLinesText    (this, SW_RES(TXT_LINES   )),
    aLinesField   (this, SW_RES(FLD_LINES   )),
    aDistanceText (this, SW_RES(TXT_DISTANCE)),
    aDistanceField(this, SW_RES(FLD_DISTANCE)),
    aTextText     (this, SW_RES(TXT_TEXT    )),
    aTextEdit     (this, SW_RES(EDT_TEXT    )),
    aTemplateText (this, SW_RES(TXT_TEMPLATE)),
    aTemplateBox  (this, SW_RES(BOX_TEMPLATE)),
    aSettingsFL   (this, SW_RES(FL_SETTINGS)),
    aContentFL    (this, SW_RES(FL_CONTENT )),
    pPict         (new SwDropCapsPict(this, SW_RES(CNT_PICT))),
    bModified(FALSE),
    bFormat(TRUE),
    rSh(::GetActiveView()->GetWrtShell())
{
    FreeResource();
    SetExchangeSupport();

    USHORT nHtmlMode = ::GetHtmlMode((const SwDocShell*)SfxObjectShell::Current());
    bHtmlMode = nHtmlMode & HTMLMODE_ON ? TRUE : FALSE;

    //Im Vorlagendialog kann der Text nicht beeinflusst werden
    aTextText.Enable( !bFormat );
    aTextEdit.Enable( !bFormat );

    // Metriken
    SetMetric( aDistanceField, GetDfltMetric(bHtmlMode) );

    pPict->SetBorderStyle( WINDOW_BORDER_MONO );

    // Handler installieren
    Link aLk = LINK(this, SwDropCapsPage, ModifyHdl);
    aDropCapsField.SetModifyHdl( aLk );
    aLinesField   .SetModifyHdl( aLk );
    aDistanceField.SetModifyHdl( aLk );
    aTextEdit     .SetModifyHdl( aLk );
    aDropCapsBox  .SetClickHdl (LINK(this, SwDropCapsPage, ClickHdl ));
    aTemplateBox  .SetSelectHdl(LINK(this, SwDropCapsPage, SelectHdl));
    aWholeWordCB  .SetClickHdl (LINK(this, SwDropCapsPage, WholeWordHdl ));
}

/****************************************************************************
Page: Dtor
****************************************************************************/


 SwDropCapsPage::~SwDropCapsPage()
{
    delete pPict;
}


int  SwDropCapsPage::DeactivatePage(SfxItemSet * pSet)
{
    if ( pSet )
        FillSet( *pSet );

    return LEAVE_PAGE;
}

/****************************************************************************
Page: Factory
****************************************************************************/


SfxTabPage*  SwDropCapsPage::Create(Window *pParent,
    const SfxItemSet &rSet)
{
    return new SwDropCapsPage(pParent, rSet);
}

/****************************************************************************
Page: FillItemSet-Overload
****************************************************************************/


BOOL  SwDropCapsPage::FillItemSet(SfxItemSet &rSet)
{
    if(bModified)
        FillSet(rSet);
    return bModified;
}

/****************************************************************************
Page: Reset-Overload
****************************************************************************/


void  SwDropCapsPage::Reset(const SfxItemSet &rSet)
{
    // Zeichen, Zeilen, Abstand und Text
    SwFmtDrop aFmtDrop((SwFmtDrop &) rSet.Get(RES_PARATR_DROP));
    if (aFmtDrop.GetLines() > 1)
    {
        aDropCapsField.SetValue(aFmtDrop.GetChars());
        aLinesField   .SetValue(aFmtDrop.GetLines());
        aDistanceField.SetValue(aDistanceField.Normalize(aFmtDrop.GetDistance()), FUNIT_TWIP);
        aWholeWordCB  .Check   (aFmtDrop.GetWholeWord());
    }
    else
    {
        aDropCapsField.SetValue(1);
        aLinesField   .SetValue(3);
        aDistanceField.SetValue(0);
    }

    ::FillCharStyleListBox(aTemplateBox, rSh.GetView().GetDocShell(), TRUE);

    aTemplateBox.InsertEntry(SW_RESSTR(SW_STR_NONE), 0);
    // Vorlage defaulten
    aTemplateBox.SelectEntryPos(0);
    if (aFmtDrop.GetCharFmt())
        aTemplateBox.SelectEntry(aFmtDrop.GetCharFmt()->GetName());

    // Controls enablen
    aDropCapsBox.Check(aFmtDrop.GetLines() > 1);
    const USHORT nVal = USHORT(aDropCapsField.GetValue());
    if (bFormat)
        aTextEdit.SetText(GetDefaultString(nVal));
    else
    {
        aTextEdit.SetText(rSh.GetDropTxt(nVal));
        aTextEdit.Enable();
        aTextText.Enable();
    }

    // Preview
    pPict->SetValues(   aTextEdit.GetText(),
                        BYTE( aLinesField.GetValue() ),
                        USHORT( aDistanceField.Denormalize( aDistanceField.GetValue( FUNIT_TWIP ) ) ) );

    ClickHdl(&aDropCapsBox);
    bModified = FALSE;
}

/****************************************************************************
Page: Click-Handler der CheckBox
****************************************************************************/


IMPL_LINK( SwDropCapsPage, ClickHdl, Button *, EMPTYARG )
{
    BOOL bChecked = aDropCapsBox.IsChecked();

    aWholeWordCB  .Enable( bChecked && !bHtmlMode );

    aSwitchText.Enable( bChecked && !aWholeWordCB.IsChecked() );
    aDropCapsField.Enable( bChecked && !aWholeWordCB.IsChecked() );
    aLinesText   .Enable( bChecked );
    aLinesField   .Enable( bChecked );
    aDistanceText.Enable( bChecked );
    aDistanceField.Enable( bChecked );
    aTemplateText .Enable( bChecked );
    aTemplateBox  .Enable( bChecked );
    aTextEdit     .Enable( bChecked && !bFormat );
    aTextText     .Enable( bChecked && !bFormat );

    if ( bChecked )
    {
        ModifyHdl(&aDropCapsField);
        aDropCapsField.GrabFocus();
    }
    else
        pPict->SetText(aEmptyStr);

    bModified = TRUE;

    return 0;
}

/****************************************************************************
Page: Click-Handler der CheckBox
****************************************************************************/


IMPL_LINK( SwDropCapsPage, WholeWordHdl, CheckBox *, EMPTYARG )
{
    aDropCapsField.Enable( !aWholeWordCB.IsChecked() );

    ModifyHdl(&aDropCapsField);

    bModified = TRUE;

    return 0;
}

/****************************************************************************
Page: Modify-Handler der SpinFields
****************************************************************************/


IMPL_LINK( SwDropCapsPage, ModifyHdl, Edit *, pEdit )
{
    String sPreview;

    // Ggf. Text setzen
    if (pEdit == &aDropCapsField)
    {
        USHORT nVal;
        BOOL bSetText = FALSE;

        if (!aWholeWordCB.IsChecked())
            nVal = (USHORT)aDropCapsField.GetValue();
        else
            nVal = 0;

        if (bFormat || !rSh.GetDropTxt(1).Len())
            sPreview = GetDefaultString(nVal);
        else
        {
            bSetText = TRUE;
            sPreview = rSh.GetDropTxt(nVal);
        }

        String sEdit(aTextEdit.GetText());

        if (sEdit.Len() && sPreview.CompareTo(sEdit, sEdit.Len()) != COMPARE_EQUAL)
        {
            sPreview = sEdit.Copy(0, sPreview.Len());
            bSetText = FALSE;
        }

        if (bSetText)
            aTextEdit.SetText(sPreview);
    }
    else if (pEdit == &aTextEdit)   // Ggf. Anzahl setzen
    {
        USHORT nTmp = aTextEdit.GetText().Len();
        aDropCapsField.SetValue(Max((USHORT)1, nTmp));

        sPreview = aTextEdit.GetText().Copy(0, nTmp);
    }

    // Bild anpassen
    if (pEdit == &aDropCapsField || pEdit == &aTextEdit)
        pPict->SetText (sPreview);
    else if (pEdit == &aLinesField)
        pPict->SetLines((BYTE)aLinesField.GetValue());
    else
        pPict->SetDistance((USHORT)aDistanceField.Denormalize(aDistanceField.GetValue(FUNIT_TWIP)));

    bModified = TRUE;

    return 0;
}

/****************************************************************************
Page: Select-Handler der Template-Box.
*****************************************************************************/


IMPL_LINK_INLINE_START( SwDropCapsPage, SelectHdl, ListBox *, EMPTYARG )
{
    pPict->UpdatePaintSettings();
    bModified = TRUE;
    return 0;
}
IMPL_LINK_INLINE_END( SwDropCapsPage, SelectHdl, ListBox *, EMPTYARG )


USHORT*  SwDropCapsPage::GetRanges()
{
    return aPageRg;
}


void SwDropCapsPage::FillSet( SfxItemSet &rSet )
{
    if(bModified)
    {
        SwFmtDrop aFmt;

        BOOL bOn = aDropCapsBox.IsChecked();
        if(bOn)
        {
            // Anzahl, Zeilen, Abstand
            aFmt.GetChars()     = (BYTE) aDropCapsField.GetValue();
            aFmt.GetLines()     = (BYTE) aLinesField.GetValue();
            aFmt.GetDistance()  = (USHORT) aDistanceField.Denormalize(aDistanceField.GetValue(FUNIT_TWIP));
            aFmt.GetWholeWord() = aWholeWordCB.IsChecked();

            // Vorlage
            if (aTemplateBox.GetSelectEntryPos())
                aFmt.SetCharFmt(rSh.GetCharStyle(aTemplateBox.GetSelectEntry()));
        }
        else
        {
            aFmt.GetChars()    = 1;
            aFmt.GetLines()    = 1;
            aFmt.GetDistance() = 0;
        }

        // Attribute setzen
        const SfxPoolItem* pOldItem;
        if(0 == (pOldItem = GetOldItem( rSet, FN_FORMAT_DROPCAPS )) ||
                    aFmt != *pOldItem )
            rSet.Put(aFmt);

        // Harte Textformatierung
        // Bug 24974: In Gestalter/Vorlagenkatoplog macht das keinen Sinn!!
        if( !bFormat && aDropCapsBox.IsChecked() )
        {
            String sText(aTextEdit.GetText());

            if (!aWholeWordCB.IsChecked())
                sText.Erase(aDropCapsField.GetValue());

            SfxStringItem aStr(FN_PARAM_1, sText);
            rSet.Put( aStr );
        }
    }
}




