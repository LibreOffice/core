/*************************************************************************
 *
 *  $RCSfile: ImplChartModel.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: bm $ $Date: 2003-10-17 14:48:14 $
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
 *  Copyright: 2003 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/
#ifndef CHART_IMPLCHARTMODEL_HXX
#define CHART_IMPLCHARTMODEL_HXX

#ifndef _DRAFTS_COM_SUN_STAR_CHART2_XDATASOURCE_HPP_
#include <drafts/com/sun/star/chart2/XDataSource.hpp>
#endif
#ifndef _DRAFTS_COM_SUN_STAR_CHART2_XDATASERIES_HPP_
#include <drafts/com/sun/star/chart2/XDataSeries.hpp>
#endif
#ifndef _DRAFTS_COM_SUN_STAR_CHART2_XDIAGRAM_HPP_
#include <drafts/com/sun/star/chart2/XDiagram.hpp>
#endif
#ifndef _DRAFTS_COM_SUN_STAR_CHART2_XDATAPROVIDER_HPP_
#include <drafts/com/sun/star/chart2/XDataProvider.hpp>
#endif
#ifndef _DRAFTS_COM_SUN_STAR_CHART2_XCHARTTYPEMANAGER_HPP_
#include <drafts/com/sun/star/chart2/XChartTypeManager.hpp>
#endif
#ifndef _DRAFTS_COM_SUN_STAR_CHART2_XCHARTTYPETEMPLATE_HPP_
#include <drafts/com/sun/star/chart2/XChartTypeTemplate.hpp>
#endif
#ifndef _DRAFTS_COM_SUN_STAR_CHART2_XTITLE_HPP_
#include <drafts/com/sun/star/chart2/XTitle.hpp>
#endif
#ifndef _DRAFTS_COM_SUN_STAR_CHART2_XCHARTDOCUMENT_HPP_
#include <drafts/com/sun/star/chart2/XChartDocument.hpp>
#endif
// #ifndef _DRAFTS_COM_SUN_STAR_LAYOUT_XSPLITLAYOUTCONTAINER_HPP_
// #include <drafts/com/sun/star/layout/XSplitLayoutContainer.hpp>
// #endif

#ifndef _COM_SUN_STAR_CONTAINER_NOSUCHELEMENTEXCEPTION_HPP_
#include <com/sun/star/container/NoSuchElementException.hpp>
#endif

#ifndef _COM_SUN_STAR_CHART_XCHARTDOCUMENT_HPP_
#include <com/sun/star/chart/XChartDocument.hpp>
#endif

#ifndef _CPPUHELPER_WEAKREF_HXX_
#include <cppuhelper/weakref.hxx>
#endif

#include <vector>

namespace com { namespace sun { namespace star {
    namespace container {
        class XNameAccess;
    }
    namespace uno {
        class XComponentContext;
    }
}}}

namespace chart
{
namespace impl
{

class ImplChartModel
{
public:
    ImplChartModel( ::com::sun::star::uno::Reference<
                    ::com::sun::star::uno::XComponentContext > const & xContext );

//     ::com::sun::star::uno::Sequence<
//         ::com::sun::star::uno::Reference<
//         ::drafts::com::sun::star::chart2::XDataSeries > >
//         GetDataSeries() const;

    ::com::sun::star::uno::Reference<
        ::com::sun::star::container::XNameAccess >
        GetStyleFamilies();

    // Diagram Access
    void RemoveAllDiagrams();
    /** @return true, if the chart was found and removed, false otherwise.
     */
    bool RemoveDiagram( const ::com::sun::star::uno::Reference<
                        ::drafts::com::sun::star::chart2::XDiagram > & xDiagram );
    void AppendDiagram( const ::com::sun::star::uno::Reference<
                        ::drafts::com::sun::star::chart2::XDiagram > & xDiagram );
    ::com::sun::star::uno::Reference<
            ::drafts::com::sun::star::chart2::XDiagram >
        GetDiagram( size_t nIndex ) const
            throw( ::com::sun::star::container::NoSuchElementException );

    void SetDataProvider( const ::com::sun::star::uno::Reference<
                          ::drafts::com::sun::star::chart2::XDataProvider > & xProvider );

    void SAL_CALL SetRangeRepresentation( const ::rtl::OUString& aRangeRepresentation )
            throw (::com::sun::star::lang::IllegalArgumentException);

    void SetChartTypeManager(
        const ::com::sun::star::uno::Reference<
            ::drafts::com::sun::star::chart2::XChartTypeManager > & xManager );

    ::com::sun::star::uno::Reference<
        ::drafts::com::sun::star::chart2::XChartTypeManager >
        GetChartTypeManager();

    void SetChartTypeTemplate(
        const ::com::sun::star::uno::Reference<
            ::drafts::com::sun::star::chart2::XChartTypeTemplate > & xTemplate );

    ::com::sun::star::uno::Reference<
        ::drafts::com::sun::star::chart2::XChartTypeTemplate >
        GetChartTypeTemplate();

//     void SetSplitLayoutContainer(
//         const ::com::sun::star::uno::Reference<
//             ::drafts::com::sun::star::layout::XSplitLayoutContainer > & xLayoutCnt );

//     ::com::sun::star::uno::Reference<
//         ::drafts::com::sun::star::layout::XSplitLayoutContainer > GetSplitLayoutContainer();

    ::com::sun::star::uno::Reference< ::drafts::com::sun::star::chart2::XTitle >
        GetTitle();

    void SetTitle( const ::com::sun::star::uno::Reference<
                   ::drafts::com::sun::star::chart2::XTitle >& rTitle );

    ::com::sun::star::uno::Reference<
        ::com::sun::star::chart::XChartDocument > GetOldChartDocument(
            const ::com::sun::star::uno::Reference<
            ::drafts::com::sun::star::chart2::XChartDocument > & xNewModel );

    /** Is called by the ChartModel's XComponent::dispose() to notify the
        impl-class to release resources
     */
    void dispose();

    ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >
        GetPageBackground();

private:
    void ReadData( const ::rtl::OUString & rRangeRepresentation );
    void CreateDefaultChart();
//     void CreateDefaultLayout();

    ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >        m_xContext;
    ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess >        m_xFamilies;

    // Data Access (deprecated, temporary solution)
    ::com::sun::star::uno::Reference< ::drafts::com::sun::star::chart2::XDataSource >   m_xChartData;
    ::com::sun::star::uno::Reference< ::drafts::com::sun::star::chart2::XDataProvider > m_xDataProvider;

    ::std::vector< ::com::sun::star::uno::Reference<
        ::drafts::com::sun::star::chart2::XDataSeries > >                               m_aInterpretedData;

    ::com::sun::star::uno::Reference< ::drafts::com::sun::star::chart2::XChartTypeManager >
        m_xChartTypeManager;
    ::com::sun::star::uno::Reference< ::drafts::com::sun::star::chart2::XChartTypeTemplate >
        m_xChartTypeTemplate;

    // Diagram Access
    typedef ::std::vector< ::com::sun::star::uno::Reference<
                                ::drafts::com::sun::star::chart2::XDiagram > >
        tDiagramContainer;

    tDiagramContainer                     m_aDiagrams;

//     ::com::sun::star::uno::Reference< ::drafts::com::sun::star::layout::XSplitLayoutContainer >
//                                           m_xLayoutContainer;

    ::com::sun::star::uno::Reference< ::drafts::com::sun::star::chart2::XTitle >
                                          m_xTitle;

    ::com::sun::star::uno::WeakReference< ::com::sun::star::chart::XChartDocument >
                                          m_xOldModel;
    bool                                  m_bIsDisposed;
    ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >
                                          m_xPageBackground;
};

}  // namespace impl
}  // namespace chart

// CHART_IMPLCHARTMODEL_HXX
#endif
