/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: StartMarker.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: rt $ $Date: 2008-01-29 13:52:06 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/
#include "precompiled_reportdesign.hxx"

#ifndef RPTUI_STARTMARKER_HXX
#include "StartMarker.hxx"
#endif
#ifndef _SV_IMAGE_HXX
#include <vcl/image.hxx>
#endif
#ifndef _SV_SVAPP_HXX
#include <vcl/svapp.hxx>
#endif
#ifndef _RPTUI_DLGRESID_HRC
#include "RptResId.hrc"
#endif
#ifndef _RPTUI_MODULE_HELPER_RPT_HXX_
#include "ModuleHelper.hxx"
#endif
#ifndef RPTUI_COLORCHANGER_HXX
#include "ColorChanger.hxx"
#endif
#ifndef RPTUI_REPORT_DEFINES_HXX
#include "ReportDefines.hxx"
#endif
#ifndef RPTUI_SECTIONSWINDOW_HXX
#include "SectionsWindow.hxx"
#endif
#ifndef RTPUI_REPORTDESIGN_HELPID_HRC
#include "helpids.hrc"
#endif
#ifndef _SV_HELP_HXX
#include <vcl/help.hxx>
#endif
#include <vcl/gradient.hxx>
#include <vcl/lineinfo.hxx>
#include <toolkit/helper/vclunohelper.hxx>
#include <svtools/syslocale.hxx>
#ifndef _SFXSMPLHINT_HXX
#include <svtools/smplhint.hxx>
#endif

#define CORNER_SPACE     5
#define TEXT_WIDTH      10
#define STRT_BORDER      6

//=====================================================================
namespace rptui
{
//=====================================================================

Image*  OStartMarker::s_pDefCollapsed       = NULL;
Image*  OStartMarker::s_pDefExpanded        = NULL;
Image*  OStartMarker::s_pDefCollapsedHC = NULL;
Image*  OStartMarker::s_pDefExpandedHC  = NULL;
oslInterlockedCount OStartMarker::s_nImageRefCount  = 0;

DBG_NAME( rpt_OStartMarker )
// -----------------------------------------------------------------------------
OStartMarker::OStartMarker(OSectionsWindow* _pParent,const ::rtl::OUString& _sColorEntry)
: OColorListener(_pParent,_sColorEntry)
,m_aVRuler(this,WB_VERT)
,m_aText(this,WB_WORDBREAK)
,m_aImage(this,WB_LEFT|WB_TOP)
,m_pParent(_pParent)
,m_nCornerSize(CORNER_SPACE)
,m_bShowRuler(sal_True)
{
    DBG_CTOR( rpt_OStartMarker,NULL);
    SetUniqueId(HID_RPT_STARTMARKER);
    osl_incrementInterlockedCount(&s_nImageRefCount);
    initDefaultNodeImages();
    ImplInitSettings();
    m_aText.SetHelpId(HID_RPT_START_TITLE);
    m_aImage.SetHelpId(HID_RPT_START_IMAGE);
    m_aText.Show();
    m_aImage.Show();
    m_aVRuler.Show();
    m_aVRuler.Activate();
    m_aVRuler.SetPagePos(0);
    m_aVRuler.SetBorders();
    m_aVRuler.SetIndents();
    m_aVRuler.SetMargin1();
    m_aVRuler.SetMargin2();
    const MeasurementSystem eSystem = SvtSysLocale().GetLocaleData().getMeasurementSystemEnum();
    m_aVRuler.SetUnit(MEASURE_METRIC == eSystem ? FUNIT_CM : FUNIT_INCH);

}
// -----------------------------------------------------------------------------
OStartMarker::~OStartMarker()
{
    DBG_DTOR( rpt_OStartMarker,NULL);
    if ( osl_decrementInterlockedCount(&s_nImageRefCount) == 0 )
    {
        DELETEZ(s_pDefCollapsed);
        DELETEZ(s_pDefExpanded);
        DELETEZ(s_pDefCollapsedHC);
        DELETEZ(s_pDefExpandedHC);
    } // if ( osl_decrementInterlockedCount(&s_nImageRefCount) == 0 )
}
// -----------------------------------------------------------------------------
sal_Int32 OStartMarker::getWidth() const
{
    return (GetDisplayBackground().GetColor().IsDark() ? s_pDefExpandedHC : s_pDefCollapsed)->GetSizePixel().Width() + GetTextWidth(m_aText.GetText(),0,::std::min<USHORT>(TEXT_WIDTH,m_aText.GetText().Len())) + 2*REPORT_EXTRA_SPACE;
}
// -----------------------------------------------------------------------------
sal_Int32 OStartMarker::getMinHeight() const
{
    return m_aText.GetTextHeight() + 2*STRT_BORDER + 2;
}
// -----------------------------------------------------------------------------
void OStartMarker::Paint( const Rectangle& rRect )
{
    Window::Paint( rRect );
    //SetUpdateMode(FALSE);
    Size aSize = GetSizePixel();
    long nSize = aSize.Width();
    if ( !isCollapsed() )
        nSize = aSize.Width() - m_aVRuler.GetSizePixel().Width() - m_nCornerSize;
    SetClipRegion(Region(Rectangle(Point(),Size( nSize,aSize.Height()))));
    aSize.Width() += m_nCornerSize;

    Point aGcc3WorkaroundTemporary;
    Rectangle aWholeRect(aGcc3WorkaroundTemporary,aSize);
    {
        const ColorChanger aColors( this, m_nTextBoundaries, m_nColor );

        //aGradient.SetBorder(static_cast<USHORT>(m_nCornerSize));
        PolyPolygon aPoly;
        aPoly.Insert(Polygon(aWholeRect,m_nCornerSize,m_nCornerSize));

        Color aStartColor(m_nColor);
        aStartColor.IncreaseLuminance(10);
        USHORT nHue = 0;
        USHORT nSat = 0;
        USHORT nBri = 0;
        aStartColor.RGBtoHSB(nHue, nSat, nBri);
        nSat += 40;
        Color aEndColor(Color::HSBtoRGB(nHue, nSat, nBri));
        Gradient aGradient(GRADIENT_LINEAR,aStartColor,aEndColor);
        aGradient.SetSteps(static_cast<USHORT>(aSize.Height()));

        DrawGradient(aPoly ,aGradient);
    }
    if ( m_bMarked )
    {
#define DIFF_DIST    2
        Rectangle aRect( Point(m_nCornerSize,m_nCornerSize),
                         Size(aSize.Width() - m_nCornerSize- m_nCornerSize,aSize.Height() - m_nCornerSize- m_nCornerSize));
        ColorChanger aColors( this, COL_WHITE, COL_WHITE );
        DrawPolyLine(Polygon(aRect),LineInfo(LINE_SOLID,2));
    }
}
// -----------------------------------------------------------------------------
void OStartMarker::setColor()
{
    const Color aColor(m_nColor);
    Color aTextColor = GetTextColor();
    if ( aColor.GetLuminance() < 128 )
        aTextColor = COL_WHITE;
    m_aText.SetTextColor(aTextColor);
    m_aText.SetLineColor(m_nColor);
}
// -----------------------------------------------------------------------
void OStartMarker::MouseButtonUp( const MouseEvent& rMEvt )
{
    if ( !rMEvt.IsLeft() )
        return;

    Point aPos( rMEvt.GetPosPixel());

    const Size aOutputSize = GetOutputSizePixel();
    if( aPos.X() > aOutputSize.Width() || aPos.Y() > aOutputSize.Height() )
        return;
    Rectangle aRect(m_aImage.GetPosPixel(),m_aImage.GetImage().GetSizePixel());
    if ( rMEvt.GetClicks() == 2 || aRect.IsInside( aPos ) )
    {
        m_bCollapsed = !m_bCollapsed;

        Image* pImage = NULL;
        if ( GetDisplayBackground().GetColor().IsDark() )
            pImage = m_bCollapsed ? s_pDefCollapsedHC : s_pDefExpandedHC;
        else
            pImage = m_bCollapsed ? s_pDefCollapsed : s_pDefExpanded;
        m_aImage.SetImage(*pImage);

        m_aVRuler.Show(!m_bCollapsed && m_bShowRuler);
        m_nCornerSize = CORNER_SPACE;
        if ( m_aCollapsedLink.IsSet() )
            m_aCollapsedLink.Call(this);
    }

    m_pParent->showProperties(this);
}
// -----------------------------------------------------------------------
void OStartMarker::initDefaultNodeImages()
{
    if ( !s_pDefCollapsed )
    {
        s_pDefCollapsed = new Image( ModuleRes( RID_IMG_TREENODE_COLLAPSED) );
        s_pDefCollapsedHC = new Image( ModuleRes( RID_IMG_TREENODE_COLLAPSED_HC ) );
        s_pDefExpanded = new Image( ModuleRes( RID_IMG_TREENODE_EXPANDED ) );
        s_pDefExpandedHC = new Image( ModuleRes( RID_IMG_TREENODE_EXPANDED_HC ) );
    }

    Image* pImage = NULL;
    if ( GetDisplayBackground().GetColor().IsDark() )
    {
        pImage = m_bCollapsed ? s_pDefCollapsedHC : s_pDefExpandedHC;
    }
    else
    {
        pImage = m_bCollapsed ? s_pDefCollapsed : s_pDefExpanded;
    }
    m_aImage.SetImage(*pImage);
    m_aImage.SetMouseTransparent(TRUE);
    m_aImage.SetBackground();
    m_aText.SetBackground();
    m_aText.SetMouseTransparent(TRUE);
}
// -----------------------------------------------------------------------
void OStartMarker::ImplInitSettings()
{
    SetBackground( Wallpaper( Application::GetSettings().GetStyleSettings().GetDialogColor() ) );
    SetFillColor( Application::GetSettings().GetStyleSettings().GetDialogColor() );
    //SetTextFillColor( Application::GetSettings().GetStyleSettings().GetDarkShadowColor() );
    setColor();
}
//------------------------------------------------------------------------------
void OStartMarker::Resize()
{
    const Size aOutputSize( GetOutputSize() );
    const long nOutputWidth = aOutputSize.Width();
    const long nOutputHeight    = aOutputSize.Height();

    const Size aImageSize = m_aImage.GetImage().GetSizePixel();
    sal_Int32 nY = ::std::min<sal_Int32>(static_cast<sal_Int32>(REPORT_EXTRA_SPACE),static_cast<sal_Int32>((nOutputHeight - aImageSize.Height()) * 0.5));
    if ( m_bCollapsed )
        nY = static_cast<sal_Int32>((nOutputHeight - aImageSize.Height()) * 0.5);
    Point aPos(REPORT_EXTRA_SPACE,nY);

    m_aImage.SetPosSizePixel(aPos,Size(aImageSize.Width() + REPORT_EXTRA_SPACE,nOutputHeight - 2*nY));
    aPos.X() += aImageSize.Width() + REPORT_EXTRA_SPACE;
    aPos.Y() -= 2;

    const long nVRulerWidth = m_aVRuler.GetSizePixel().Width();
    const Point aRulerPos(nOutputWidth - nVRulerWidth - 5,0);

    m_aText.SetPosSizePixel(aPos,Size(aRulerPos.X() - aPos.X(),nOutputHeight - 2*aPos.Y()));


    m_aVRuler.SetPosSizePixel(aRulerPos,Size(nVRulerWidth,nOutputHeight));
}
// -----------------------------------------------------------------------------
void OStartMarker::setTitle(const String& _sTitle)
{
    m_aText.SetText(_sTitle);
}
// -----------------------------------------------------------------------------
void OStartMarker::Notify(SfxBroadcaster & rBc, SfxHint const & rHint)
{
    OColorListener::Notify(rBc, rHint);
    if (rHint.ISA(SfxSimpleHint)
        && (static_cast< SfxSimpleHint const & >(rHint).GetId()
            == SFX_HINT_COLORS_CHANGED))
    {
        setColor();
        //m_aText.Invalidate();
        Invalidate(INVALIDATE_CHILDREN);
    }
}
//----------------------------------------------------------------------------
void OStartMarker::showRuler(sal_Bool _bShow)
{
    m_bShowRuler = _bShow;
    m_aVRuler.Show(!m_bCollapsed && m_bShowRuler);
}
//------------------------------------------------------------------------------
sal_Int32 OStartMarker::getRulerOffset() const
{
    return m_aVRuler.GetSizePixel().Width();
}

//------------------------------------------------------------------------------
void OStartMarker::RequestHelp( const HelpEvent& rHEvt )
{
    if( m_aText.GetText().Len())
    {
        // Hilfe anzeigen
        Rectangle aItemRect(rHEvt.GetMousePosPixel(),Size(GetSizePixel().Width(),getMinHeight()));
        //aItemRect = LogicToPixel( aItemRect );
        Point aPt = OutputToScreenPixel( aItemRect.TopLeft() );
        aItemRect.Left()   = aPt.X();
        aItemRect.Top()    = aPt.Y();
        aPt = OutputToScreenPixel( aItemRect.BottomRight() );
        aItemRect.Right()  = aPt.X();
        aItemRect.Bottom() = aPt.Y();
        if( rHEvt.GetMode() == HELPMODE_BALLOON )
            Help::ShowBalloon( this, aItemRect.Center(), aItemRect, m_aText.GetText());
        else
            Help::ShowQuickHelp( this, aItemRect, m_aText.GetText() );
    }
}
// -----------------------------------------------------------------------------
void OStartMarker::setCollapsed(sal_Bool _bCollapsed)
{
    OColorListener::setCollapsed(_bCollapsed);
    showRuler(_bCollapsed);
}
// -----------------------------------------------------------------------
// -----------------------------------------------------------------------
// -----------------------------------------------------------------------
// =======================================================================
}
// =======================================================================
