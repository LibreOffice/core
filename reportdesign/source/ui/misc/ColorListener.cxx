/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: ColorListener.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: ihi $ $Date: 2007-11-20 19:11:46 $
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

#ifndef RPTUI_COLORLISTENER_HXX
#include "ColorListener.hxx"
#endif
#ifndef _SFXSMPLHINT_HXX
#include <svtools/smplhint.hxx>
#endif
#ifndef REPORTDESIGN_SHARED_UISTRINGS_HRC
#include "uistrings.hrc"
#endif

#define BORDERCOLORCHANGE 191
//=====================================================================
namespace rptui
{
//=====================================================================
    DBG_NAME( rpt_OColorListener )
OColorListener::OColorListener(Window* _pParent ,const ::rtl::OUString& _sColorEntry)
: Window(_pParent)
,m_sColorEntry(_sColorEntry)
,m_nColor(COL_LIGHTBLUE)
,m_bCollapsed(sal_False)
,m_bMarked(sal_False)
{
    DBG_CTOR( rpt_OColorListener,NULL);
    StartListening(m_aExtendedColorConfig);
    m_nColor = m_aExtendedColorConfig.GetColorValue(CFG_REPORTDESIGNER,m_sColorEntry).getColor();
    m_nTextBoundaries = m_aColorConfig.GetColorValue(::svtools::DOCBOUNDARIES).nColor;
}
// -----------------------------------------------------------------------------
OColorListener::~OColorListener()
{
    DBG_DTOR( rpt_OColorListener,NULL);
    EndListening(m_aExtendedColorConfig);
}
// -----------------------------------------------------------------------
void OColorListener::Notify(SfxBroadcaster & /*rBc*/, SfxHint const & rHint)
{
    if (rHint.ISA(SfxSimpleHint)
        && (static_cast< SfxSimpleHint const & >(rHint).GetId()
            == SFX_HINT_COLORS_CHANGED))
    {
        m_nColor = m_aExtendedColorConfig.GetColorValue(CFG_REPORTDESIGNER,m_sColorEntry).getColor();
        m_nTextBoundaries = m_aColorConfig.GetColorValue(::svtools::DOCBOUNDARIES).nColor;
        Invalidate(INVALIDATE_NOCHILDREN|INVALIDATE_NOERASE);
    }
}
//-----------------------------------------------------------------------------
void OColorListener::DataChanged( const DataChangedEvent& rDCEvt )
{
    Window::DataChanged( rDCEvt );

    if ( (rDCEvt.GetType() == DATACHANGED_SETTINGS) &&
         (rDCEvt.GetFlags() & SETTINGS_STYLE) )
    {
        ImplInitSettings();
        Invalidate();
    }
}
// -----------------------------------------------------------------------------
void OColorListener::setCollapsed(sal_Bool _bCollapsed)
{
    m_bCollapsed = _bCollapsed;
    if ( m_aCollapsedLink.IsSet() )
        m_aCollapsedLink.Call(this);
}
// -----------------------------------------------------------------------------
void OColorListener::setMarked(sal_Bool _bMark)
{
    m_bMarked = _bMark;
}
// =======================================================================
}
// =======================================================================
