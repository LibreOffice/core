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
#ifndef RPTUI_COLORLISTENER_HXX
#define RPTUI_COLORLISTENER_HXX

#include <vcl/window.hxx>
#include <svtools/colorcfg.hxx>
#include <svtools/extcolorcfg.hxx>
#include "ModuleHelper.hxx"
#include <tools/link.hxx>

namespace rptui
{
    class OColorListener : public Window, public SfxListener
    {
        OColorListener(const OColorListener&);
        void operator =(const OColorListener&);
    protected:
        OModuleClient                       m_aModuleClient;
        Link                                m_aCollapsedLink;
        svtools::ColorConfig                m_aColorConfig;
        svtools::ExtendedColorConfig        m_aExtendedColorConfig;
        OUString                     m_sColorEntry;
        sal_Int32                           m_nColor;
        sal_Int32                           m_nTextBoundaries;
        sal_Bool                            m_bCollapsed;
        sal_Bool                            m_bMarked;

        virtual void ImplInitSettings() = 0;
    protected:
        virtual void DataChanged( const DataChangedEvent& rDCEvt );
    public:
        OColorListener(Window* _pParent,const OUString& _sColorEntry);
        virtual ~OColorListener();

        using Window::Notify;
        // SfxListener
        virtual void Notify(SfxBroadcaster & rBc, SfxHint const & rHint);

        /** set the marker as marked or not marked
            @param  _bMark  set the new state of the marker
        */
        void    setMarked(sal_Bool _bMark);

        /** returns if the section is marked
        */
        inline sal_Bool isMarked() const { return m_bMarked; }

        inline void     setCollapsedHdl(const Link& _aLink ){ m_aCollapsedLink = _aLink; }
        inline sal_Bool isCollapsed() const { return m_bCollapsed; }

        /** collapse or expand
         *
         * \param _bCollapsed
         */
        virtual void    setCollapsed(sal_Bool _bCollapsed);
    };
}
#endif // RPTUI_COLORLISTENER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
