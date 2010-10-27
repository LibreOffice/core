/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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
        ::rtl::OUString                     m_sColorEntry;
        sal_Int32                           m_nColor;
        sal_Int32                           m_nTextBoundaries;
        sal_Bool                            m_bCollapsed;
        sal_Bool                            m_bMarked;

        virtual void ImplInitSettings() = 0;
    protected:
        virtual void DataChanged( const DataChangedEvent& rDCEvt );
    public:
        OColorListener(Window* _pParent,const ::rtl::OUString& _sColorEntry);
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
