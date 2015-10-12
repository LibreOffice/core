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
#ifndef INCLUDED_REPORTDESIGN_SOURCE_UI_INC_SECTIONWINDOW_HXX
#define INCLUDED_REPORTDESIGN_SOURCE_UI_INC_SECTIONWINDOW_HXX

#include <com/sun/star/report/XSection.hpp>
#include <vcl/window.hxx>
#include <vcl/split.hxx>
#include <svtools/colorcfg.hxx>
#include <comphelper/propmultiplex.hxx>
#include <cppuhelper/basemutex.hxx>

#include "UITools.hxx"
#include "UndoActions.hxx"
#include "StartMarker.hxx"
#include "EndMarker.hxx"
#include "ReportSection.hxx"

#include <list>
#include <map>

namespace comphelper
{
    class OPropertyChangeMultiplexer;
}
namespace rptui
{
    class OViewsWindow;
    class OSectionWindow :      public vcl::Window
                            ,   public ::cppu::BaseMutex
                            ,   public ::comphelper::OPropertyChangeListener
    {
        VclPtr<OViewsWindow>    m_pParent;
        VclPtr<OStartMarker>    m_aStartMarker;
        VclPtr<OReportSection>  m_aReportSection;
        VclPtr<Splitter>        m_aSplitter;
        VclPtr<OEndMarker>      m_aEndMarker;

        ::rtl::Reference< comphelper::OPropertyChangeMultiplexer> m_pSectionMulti;
        ::rtl::Reference< comphelper::OPropertyChangeMultiplexer> m_pGroupMulti;

        OSectionWindow(OSectionWindow&) = delete;
        void operator =(OSectionWindow&) = delete;

        /** set the title of the group header or footer
        *
        * \param _xGroup
        * \param _nResId
        * \param _pGetSection
        * \param _pIsSectionOn
        * @return sal_True when title was set otherwise FALSE
        */
        bool setGroupSectionTitle(const ::com::sun::star::uno::Reference< ::com::sun::star::report::XGroup>& _xGroup,sal_uInt16 _nResId,::std::mem_fun_t< ::com::sun::star::uno::Reference< ::com::sun::star::report::XSection> , OGroupHelper> _pGetSection, const ::std::mem_fun_t<bool, OGroupHelper>& _pIsSectionOn);

        /** set the title of the (report/page) header or footer
        *
        * \param _xGroup
        * \param _nResId
        * \param _pGetSection
        * \param _pIsSectionOn
        * @return sal_True when title was set otherwise FALSE
        */
        bool setReportSectionTitle(const ::com::sun::star::uno::Reference< ::com::sun::star::report::XReportDefinition>& _xReport,sal_uInt16 _nResId,::std::mem_fun_t< ::com::sun::star::uno::Reference< ::com::sun::star::report::XSection> , OReportHelper> _pGetSection, const ::std::mem_fun_t<bool, OReportHelper>& _pIsSectionOn);
        void ImplInitSettings();

        DECL_LINK_TYPED(Collapsed, OColorListener&, void);
        DECL_LINK_TYPED(StartSplitHdl, Splitter*, void);
        DECL_LINK_TYPED(SplitHdl, Splitter*, void);
        DECL_LINK_TYPED(EndSplitHdl, Splitter*, void);


        virtual void DataChanged( const DataChangedEvent& rDCEvt ) SAL_OVERRIDE;
        // Window overrides
        virtual void Resize() SAL_OVERRIDE;

    protected:
        virtual void    _propertyChanged(const ::com::sun::star::beans::PropertyChangeEvent& _rEvent)
            throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    public:
        OSectionWindow( OViewsWindow* _pParent
                        ,const ::com::sun::star::uno::Reference< ::com::sun::star::report::XSection >& _xSection
                        ,const OUString& _sColorEntry);
        virtual ~OSectionWindow();
        virtual void dispose() SAL_OVERRIDE;

        inline OStartMarker&    getStartMarker()    { return *m_aStartMarker.get();     }
        inline OReportSection&  getReportSection()  { return *m_aReportSection.get();   }
        inline OEndMarker&      getEndMarker()      { return *m_aEndMarker.get();       }
        inline OViewsWindow*    getViewsWindow()    { return m_pParent;          }

        void    setCollapsed(bool _bCollapsed);

        /** triggers the property browser with the section
            @param  _pStartMarker
        */
        void    showProperties();

        /** set the marker as marked or not marked
            @param  _bMark  set the new state of the marker
        */
        void    setMarked(bool _bMark);

        /** zoom the ruler and view windows
        */
        void zoom(const Fraction& _aZoom);

        void scrollChildren(long _nThumbPosX);
    };

} // rptui

#endif // INCLUDED_REPORTDESIGN_SOURCE_UI_INC_SECTIONWINDOW_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
