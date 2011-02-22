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
#include "precompiled_reportdesign.hxx"
#include "ColorListener.hxx"
#include <svl/smplhint.hxx>
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
    if ( m_bCollapsed != _bCollapsed )
    {
        m_bCollapsed = _bCollapsed;
        if ( m_aCollapsedLink.IsSet() )
            m_aCollapsedLink.Call(this);
    }
}
// -----------------------------------------------------------------------------
void OColorListener::setMarked(sal_Bool _bMark)
{
    if ( m_bMarked != _bMark)
    {
        m_bMarked = _bMark;
        Invalidate(INVALIDATE_NOCHILDREN|INVALIDATE_NOERASE);
    }
}
// =======================================================================
}
// =======================================================================
