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
#include <ColorListener.hxx>
#include <svl/hint.hxx>
#include <vcl/settings.hxx>
#include <vcl/event.hxx>

#include <strings.hxx>


namespace rptui
{

OColorListener::OColorListener(vcl::Window* _pParent ,const OUString& _sColorEntry)
: Window(_pParent)
,m_sColorEntry(_sColorEntry)
,m_nColor(COL_LIGHTBLUE)
,m_bCollapsed(false)
,m_bMarked(false)
{
    StartListening(m_aExtendedColorConfig);
    m_nColor = m_aExtendedColorConfig.GetColorValue(CFG_REPORTDESIGNER,m_sColorEntry).getColor();
    m_nTextBoundaries = m_aColorConfig.GetColorValue(::svtools::DOCBOUNDARIES).nColor;
}

OColorListener::~OColorListener()
{
    disposeOnce();
}

void OColorListener::dispose()
{
    EndListening(m_aExtendedColorConfig);
    vcl::Window::dispose();
}

void OColorListener::Notify(SfxBroadcaster & /*rBc*/, SfxHint const & rHint)
{
    if (rHint.GetId() == SfxHintId::ColorsChanged)
    {
        m_nColor = m_aExtendedColorConfig.GetColorValue(CFG_REPORTDESIGNER,m_sColorEntry).getColor();
        m_nTextBoundaries = m_aColorConfig.GetColorValue(::svtools::DOCBOUNDARIES).nColor;
        Invalidate(InvalidateFlags::NoChildren|InvalidateFlags::NoErase);
    }
}

void OColorListener::DataChanged( const DataChangedEvent& rDCEvt )
{
    Window::DataChanged( rDCEvt );

    if ( (rDCEvt.GetType() == DataChangedEventType::SETTINGS) &&
         (rDCEvt.GetFlags() & AllSettingsFlags::STYLE) )
    {
        ImplInitSettings();
        Invalidate();
    }
}

void OColorListener::setCollapsed(bool _bCollapsed)
{
    if ( m_bCollapsed != _bCollapsed )
    {
        m_bCollapsed = _bCollapsed;
        m_aCollapsedLink.Call(*this);
    }
}

void OColorListener::setMarked(bool _bMark)
{
    if ( m_bMarked != _bMark)
    {
        m_bMarked = _bMark;
        Invalidate(InvalidateFlags::NoChildren|InvalidateFlags::NoErase);
    }
}

}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
