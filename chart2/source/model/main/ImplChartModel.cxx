/*************************************************************************
 *
 *  $RCSfile: ImplChartModel.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: bm $ $Date: 2003-10-06 12:54:15 $
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
#include "ImplChartModel.hxx"
#include "CachedDataSequence.hxx"
#include "DataSeries.hxx"
#include "DataInterpreter.hxx"
#include "StyleFamilies.hxx"
#include "StyleFamily.hxx"
#include "macros.hxx"
#include "algohelper.hxx"
#include "ContextHelper.hxx"
// #include "SplitLayoutContainer.hxx"
#include "LayoutDefaults.hxx"
#include "ChartDocumentWrapper.hxx"

#ifndef _CPPUHELPER_COMPONENT_CONTEXT_HXX_
#include <cppuhelper/component_context.hxx>
#endif

#ifndef _DRAFTS_COM_SUN_STAR_CHART2_XDATASINK_HPP_
#include <drafts/com/sun/star/chart2/XDataSink.hpp>
#endif
#ifndef _DRAFTS_COM_SUN_STAR_CHART2_XDATASEQUENCE_HPP_
#include <drafts/com/sun/star/chart2/XDataSequence.hpp>
#endif
#ifndef _DRAFTS_COM_SUN_STAR_CHART2_XDATASERIES_HPP_
#include <drafts/com/sun/star/chart2/XDataSeries.hpp>
#endif
#ifndef _DRAFTS_COM_SUN_STAR_CHART2_XAXIS_HPP_
#include <drafts/com/sun/star/chart2/XAxis.hpp>
#endif

#ifndef _COM_SUN_STAR_LANG_XINITIALIZATION_HPP_
#include <com/sun/star/lang/XInitialization.hpp>
#endif
#ifndef _COM_SUN_STAR_UNO_XCOMPONENTCONTEXT_HPP_
#include <com/sun/star/uno/XComponentContext.hpp>
#endif

#include <vector>
#include <algorithm>

using namespace ::com::sun::star;
using namespace ::drafts::com::sun::star;

using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::Sequence;

using ::rtl::OUString;

namespace chart
{
namespace impl
{

ImplChartModel::ImplChartModel( Reference< uno::XComponentContext > const & xContext ) :
        m_xContext( xContext ),
        m_xChartTypeManager(
            xContext->getServiceManager()->createInstanceWithContext(
                C2U( "drafts.com.sun.star.chart2.ChartTypeManager" ),
                xContext ), uno::UNO_QUERY )
//         m_xLayoutContainer( new SplitLayoutContainer() )
{
    GetStyleFamilies();


    uno::Reference< beans::XPropertySet > xProp( m_xChartTypeManager, uno::UNO_QUERY );
    uno::Reference< lang::XMultiServiceFactory > xFact( m_xChartTypeManager, uno::UNO_QUERY );
    if( xProp.is() &&
        xFact.is() )
    {
        OUString aServiceName;
        if( xProp->getPropertyValue( C2U( "ChartStyleTemplateServiceName" )) >>= aServiceName )
        {
            m_xChartTypeTemplate.set( xFact->createInstance( aServiceName ), uno::UNO_QUERY );
        }
    }
}

// ImplChartModel::CreateStyles()
// {
//     // set style
//     Reference< container::XNameAccess > xGraphicsFamily(
//         GetStyleFamilies()->getByName( C2U( "graphics" )), uno::UNO_QUERY );

//     if( xGraphicsFamily.is())
//         const sal_Int32 nModul = Reference< container::XIndexAccess >(
//             xGraphicsFamily, uno::UNO_QUERY )->getCount();
//     for( size_t nI = 0; nI < nMaxSeries; ++nI )
//     {
//         try
//         {
//             Reference< ::com::sun::star::style::XStyle > xStyle(
//                 xGraphicsFamily->getByName(
//                     aStyleNameStub + OUString::valueOf(
//                         static_cast< sal_Int32 >( (nI % nModul)+1 ))), uno::UNO_QUERY );

//             Reference< beans::XPropertySet > xSeriesProp(
//                 aResult[ nI ], uno::UNO_QUERY );

//             if( xSeriesProp.is())
//             {
//                 xSeriesProp->setPropertyValue(
//                     C2U( "Color" ),
//                     uno::makeAny( nDefaultColors[ sal_Int32( nI % nMaxDefaultColors ) ]));

//                 // set style
//                 Reference< ::drafts::com::sun::star::style::XStyleSupplier >
//                     xStyleSupp( xSeriesProp->getPropertyValue( C2U( "FirstGraphicsFormat" )),
//                                 uno::UNO_QUERY );

//                 if( xStyleSupp.is())
//                 {
//                     xStyleSupp->setStyle( xStyle );
//                 }
//             }
//         }
//         catch( uno::Exception ex )
//         {
//             OSL_ENSURE( false, "Couldn't set style" );
//         }
//     }
// }


void ImplChartModel::ReadData( const ::rtl::OUString & rRangeRepresentation )
{
    static sal_Int32 nDefaultColors[] =  {
        0xd43b3a,
        0xd0976d,
        0x76934b,
        0x386a53,
        0x14816b,
        0x103841,
        0x12406c,
        0x5929ba,
        0xda42c0,
        0xc179a2,
        0x31181b,
        0x4e6129
    };
    static const sal_Int32 nMaxDefaultColors = sizeof( nDefaultColors ) / sizeof( sal_Int32 );

    if( m_xDataProvider.is())
    {
        m_xChartData = m_xDataProvider->getDataByRangeRepresentation( rRangeRepresentation );

        // create DataSeries objects using the standard data interpreter
        m_aInterpretedData = DataInterpreter::InterpretData( m_xChartData );

        const size_t nMaxSeries = m_aInterpretedData.size();
        const OUString aStyleNameStub( RTL_CONSTASCII_USTRINGPARAM( "Series " ));

        for( size_t nI = 0; nI < nMaxSeries; ++nI )
        {
            try
            {
                Reference< beans::XPropertySet > xSeriesProp(
                    m_aInterpretedData[ nI ], uno::UNO_QUERY );

                if( xSeriesProp.is())
                {
                    xSeriesProp->setPropertyValue(
                        C2U( "Color" ),
                        uno::makeAny( nDefaultColors[ sal_Int32( nI % nMaxDefaultColors ) ]));
                }
            }
            catch( uno::Exception ex )
            {
                OSL_ENSURE( false, "Couldn't set style" );
            }
        }
    }
}

Reference< container::XNameAccess > ImplChartModel::GetStyleFamilies()
{
    if( ! m_xFamilies.is())
    {
//         StyleFamilies * pStyleFamilies = new StyleFamilies();
//         StyleFamily *  pGraphicsFamily = new StyleFamily();
//         Reference< container::XNameAccess > xGraphicsFamily( pGraphicsFamily );

        // add some default styles for graphics

        // randomly generated default colors

//         sal_Int32 i = 0;
//         const sal_Int32 nMax = sizeof( nDefaults ) / sizeof( sal_Int32 );
//         for( ; i < nMax; ++i )
//         {
//             try
//             {
//                 Reference< ::com::sun::star::style::XStyle > xStyle(
//                     new GraphicsPropertiesStyle( xGraphicsFamily ) );
//                 xStyle->setName( C2U( "Series " ) + OUString::valueOf( static_cast< sal_Int32 >(i + 1) ) );
//                 Reference< beans::XPropertySet > xProp( xStyle, uno::UNO_QUERY );
//                 if( xProp.is())
//                 {
//                     xProp->setPropertyValue( C2U( "Color" ), uno::makeAny( nDefaults[ i ] ));
//                 }
//                 pGraphicsFamily->AddStyle( xStyle );
//             }
//             catch( uno::Exception ex )
//             {
//                 OSL_ENSURE( false, "Error on creating styles" );
//             }
//         }

//         bool bResult = pStyleFamilies->AddStyleFamily(
//             C2U( "graphics" ), xGraphicsFamily );

//         OSL_ASSERT( bResult );
//         m_xFamilies.set( pStyleFamilies );
    }

    return m_xFamilies;
}

// Diagram Access

void ImplChartModel::RemoveAllDiagrams()
{
    m_aDiagrams.clear();
}

bool ImplChartModel::RemoveDiagram( const Reference< chart2::XDiagram > & xDiagram )
{
    tDiagramContainer::iterator aIt( ::std::find(
                                         m_aDiagrams.begin(), m_aDiagrams.end(),
                                         xDiagram ));
    if( aIt == m_aDiagrams.end() )
        return false;

    m_aDiagrams.erase( aIt );
    return true;
}

void ImplChartModel::AppendDiagram( const Reference< chart2::XDiagram > & xDiagram )
{
    m_aDiagrams.push_back( xDiagram );
}

Reference< chart2::XDiagram > ImplChartModel::GetDiagram( size_t nIndex ) const
    throw( container::NoSuchElementException )
{
    if( nIndex >= m_aDiagrams.size() )
        throw container::NoSuchElementException();

    return m_aDiagrams[ nIndex ];
}

void ImplChartModel::SetDataProvider(
    const Reference< chart2::XDataProvider > & xProvider )
{
    m_xDataProvider = xProvider;

    // release other ressources
    m_xChartData = 0;
    m_aInterpretedData.clear();
}

void SAL_CALL ImplChartModel::SetRangeRepresentation( const ::rtl::OUString& aRangeRepresentation )
    throw (lang::IllegalArgumentException)
{
    OSL_TRACE( ::rtl::OUStringToOString(
                   ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Chart Range: \"" )) +
                   aRangeRepresentation +
                   ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "\"")),
                   RTL_TEXTENCODING_ASCII_US ).getStr() );
    ReadData( aRangeRepresentation );
    CreateDefaultChart();
}

void ImplChartModel::SetChartTypeManager(
    const Reference< chart2::XChartTypeManager > & xManager )
{
    m_xChartTypeManager = xManager;
}

Reference< chart2::XChartTypeManager > ImplChartModel::GetChartTypeManager()
{
    return m_xChartTypeManager;
}

void ImplChartModel::SetChartTypeTemplate(
    const Reference< chart2::XChartTypeTemplate > & xTemplate )
{
    m_xChartTypeTemplate = xTemplate;
}

Reference< chart2::XChartTypeTemplate > ImplChartModel::GetChartTypeTemplate()
{
    return m_xChartTypeTemplate;
}

void ImplChartModel::CreateDefaultChart()
{
    // clean up
    RemoveAllDiagrams();

    if( GetChartTypeTemplate().is())
    {
        AppendDiagram(
            GetChartTypeTemplate()->createDiagram( helper::VectorToSequence( m_aInterpretedData )));
    }
}

uno::Reference< chart2::XTitle > ImplChartModel::GetTitle()
{
    return m_xTitle;
}

void ImplChartModel::SetTitle( const uno::Reference< chart2::XTitle >& rTitle )
{
    m_xTitle = rTitle;
}

uno::Reference< ::com::sun::star::chart::XChartDocument > ImplChartModel::GetOldChartDocument(
    const uno::Reference< chart2::XChartDocument > & xNewModel )
{
    // put weak reference into a strong one
    uno::Reference< ::com::sun::star::chart::XChartDocument > xRef( m_xOldModel );

    if( ! xRef.is())
    {
        xRef.set( new wrapper::ChartDocumentWrapper( xNewModel, m_xContext ));

        // assing reference to weak reference
        m_xOldModel = xRef;
    }

    return xRef;
}

}  // namespace impl
}  // namespace chart
