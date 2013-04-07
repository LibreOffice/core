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
#include "ColorListener.hxx"
#include <svl/smplhint.hxx>
#include "uistrings.hrc"

//=====================================================================
namespace rptui
{
//=====================================================================
    DBG_NAME( rpt_OColorListener )
OColorListener::OColorListener(Window* _pParent ,const OUString& _sColorEntry)
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
