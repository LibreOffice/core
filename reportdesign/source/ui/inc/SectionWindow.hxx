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
#ifndef RPTUI_SECTIONWINDOW_HXX
#define RPTUI_SECTIONWINDOW_HXX

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
#include <boost/shared_ptr.hpp>

namespace comphelper
{
    class OPropertyChangeMultiplexer;
}
namespace rptui
{
    class OReportWindow;
    class ODesignView;
    class OViewsWindow;
    class OSectionWindow :      public Window
                            ,   public ::cppu::BaseMutex
                            ,   public ::comphelper::OPropertyChangeListener
    {
        OViewsWindow*   m_pParent;
        OStartMarker    m_aStartMarker;
        OReportSection  m_aReportSection;
        Splitter        m_aSplitter;
        OEndMarker      m_aEndMarker;

        ::rtl::Reference< comphelper::OPropertyChangeMultiplexer> m_pSectionMulti;
        ::rtl::Reference< comphelper::OPropertyChangeMultiplexer> m_pGroupMulti;

        OSectionWindow(OSectionWindow&);
        void operator =(OSectionWindow&);

        /** set the title of the group header or footer
        *
        * \param _xGroup
        * \param _nResId
        * \param _pGetSection
        * \param _pIsSectionOn
        * @return sal_True when title was set otherwise FALSE
        */
        bool setGroupSectionTitle(const ::com::sun::star::uno::Reference< ::com::sun::star::report::XGroup>& _xGroup,sal_uInt16 _nResId,::std::mem_fun_t< ::com::sun::star::uno::Reference< ::com::sun::star::report::XSection> , OGroupHelper> _pGetSection,::std::mem_fun_t<sal_Bool, OGroupHelper> _pIsSectionOn);

        /** set the title of the (report/page) header or footer
        *
        * \param _xGroup
        * \param _nResId
        * \param _pGetSection
        * \param _pIsSectionOn
        * @return sal_True when title was set otherwise FALSE
        */
        bool setReportSectionTitle(const ::com::sun::star::uno::Reference< ::com::sun::star::report::XReportDefinition>& _xReport,sal_uInt16 _nResId,::std::mem_fun_t< ::com::sun::star::uno::Reference< ::com::sun::star::report::XSection> , OReportHelper> _pGetSection,::std::mem_fun_t<sal_Bool, OReportHelper> _pIsSectionOn);
        void ImplInitSettings();

        DECL_LINK(Collapsed,OColorListener*);
        DECL_LINK(StartSplitHdl, Splitter*);
        DECL_LINK(SplitHdl, Splitter*);
        DECL_LINK(EndSplitHdl, Splitter*);


        virtual void DataChanged( const DataChangedEvent& rDCEvt );
        // windows overload
        virtual void Resize();

    protected:
        virtual void    _propertyChanged(const ::com::sun::star::beans::PropertyChangeEvent& _rEvent) throw( ::com::sun::star::uno::RuntimeException);
    public:
        OSectionWindow( OViewsWindow* _pParent
                        ,const ::com::sun::star::uno::Reference< ::com::sun::star::report::XSection >& _xSection
                        ,const ::rtl::OUString& _sColorEntry);
        virtual ~OSectionWindow();

        inline OStartMarker&    getStartMarker()    { return m_aStartMarker;     }
        inline OReportSection&  getReportSection()  { return m_aReportSection;   }
        inline OEndMarker&      getEndMarker()      { return m_aEndMarker;       }
        inline OViewsWindow*    getViewsWindow()    { return m_pParent;          }

        void    setCollapsed(sal_Bool _bCollapsed);

        /** triggers the property browser with the section
            @param  _pStartMarker
        */
        void            showProperties();

        /** set the marker as marked or not marked
            @param  _bMark  set the new state of the marker
        */
        void    setMarked(sal_Bool _bMark);

        OViewsWindow* getViewsWindow() const { return m_pParent; }

        /** zoom the ruler and view windows
        */
        void zoom(const Fraction& _aZoom);

        void scrollChildren(long _nThumbPosX);
    };
//==============================================================================
} // rptui
//==============================================================================
#endif // RPTUI_SECTIONWINDOW_HXX

