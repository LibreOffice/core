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

#include <SchXMLImport.hxx>
#include "SchXMLChartContext.hxx"
#include "contexts.hxx"
#include "SchXMLTools.hxx"

#include <sal/log.hxx>
#include <comphelper/processfactory.hxx>
#include <xmloff/namespacemap.hxx>
#include <xmloff/prstylei.hxx>
#include <xmloff/xmlnamespace.hxx>
#include <xmloff/xmltoken.hxx>
#include <xmloff/xmluconv.hxx>
#include <xmloff/xmlictxt.hxx>
#include <xmloff/xmlstyle.hxx>
#include <com/sun/star/chart/XChartDocument.hpp>
#include <com/sun/star/container/XChild.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/chart2/data/XDataReceiver.hpp>
#include <com/sun/star/chart2/XChartDocument.hpp>
#include <com/sun/star/chart2/XCoordinateSystemContainer.hpp>
#include <com/sun/star/chart2/XChartTypeContainer.hpp>
#include <com/sun/star/chart2/XDataSeriesContainer.hpp>

#include <com/sun/star/document/XDocumentPropertiesSupplier.hpp>

#include <tools/diagnose_ex.h>

using namespace com::sun::star;
using namespace ::xmloff::token;

using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::Sequence;

namespace
{
class lcl_MatchesChartType
{
public:
    explicit lcl_MatchesChartType( const OUString & aChartTypeName ) :
            m_aChartTypeName( aChartTypeName )
    {}

    bool operator () ( const Reference< chart2::XChartType > & xChartType ) const
    {
        return (xChartType.is() &&
                xChartType->getChartType() == m_aChartTypeName );
    }

private:
    OUString m_aChartTypeName;
};
} // anonymous namespace

   // TokenMaps for distinguishing different
   // tokens in different contexts

// element maps

// attribute maps

SchXMLImportHelper::SchXMLImportHelper() :
        mpAutoStyles( nullptr )
{
}

SvXMLImportContext* SchXMLImportHelper::CreateChartContext(
    SvXMLImport& rImport,
    const Reference< frame::XModel >& rChartModel )
{
    SvXMLImportContext* pContext = nullptr;

    Reference< chart::XChartDocument > xDoc( rChartModel, uno::UNO_QUERY );
    if( xDoc.is())
    {
        mxChartDoc = xDoc;
        pContext = new SchXMLChartContext( *this, rImport );
    }
    else
    {
        SAL_WARN("xmloff.chart", "No valid XChartDocument given as XModel" );
    }

    return pContext;
}

void SchXMLImportHelper::FillAutoStyle(const OUString& rAutoStyleName, const uno::Reference<beans::XPropertySet>& rProp)
{
    if (!rProp.is())
        return;

    const SvXMLStylesContext* pStylesCtxt = GetAutoStylesContext();
    if (pStylesCtxt)
    {
        SvXMLStyleContext* pStyle = const_cast<SvXMLStyleContext*>(pStylesCtxt->FindStyleChildContext(SchXMLImportHelper::GetChartFamilyID(), rAutoStyleName));

        if (XMLPropStyleContext* pPropStyle = dynamic_cast<XMLPropStyleContext*>(pStyle))
            pPropStyle->FillPropertySet(rProp);
    }
}

// get various token maps


//static
void SchXMLImportHelper::DeleteDataSeries(
                    const Reference< chart2::XDataSeries > & xSeries,
                    const Reference< chart2::XChartDocument > & xDoc )
{
    if( !xDoc.is() )
        return;
    try
    {
        Reference< chart2::XCoordinateSystemContainer > xCooSysCnt(
            xDoc->getFirstDiagram(), uno::UNO_QUERY_THROW );
        const Sequence< Reference< chart2::XCoordinateSystem > > aCooSysSeq(
            xCooSysCnt->getCoordinateSystems());

        for( const auto& rCooSys : aCooSysSeq )
        {
            Reference< chart2::XChartTypeContainer > xCTCnt( rCooSys, uno::UNO_QUERY_THROW );
            const Sequence< Reference< chart2::XChartType > > aChartTypes( xCTCnt->getChartTypes());

            for( const auto& rChartType : aChartTypes )
            {
                Reference< chart2::XDataSeriesContainer > xSeriesCnt( rChartType, uno::UNO_QUERY_THROW );
                Sequence< Reference< chart2::XDataSeries > > aSeriesSeq( xSeriesCnt->getDataSeries());

                if (std::find(aSeriesSeq.begin(), aSeriesSeq.end(), xSeries) != aSeriesSeq.end())
                {
                    xSeriesCnt->removeDataSeries(xSeries);
                    return;
                }
            }
        }
    }
    catch( const uno::Exception &)
    {
       DBG_UNHANDLED_EXCEPTION("xmloff.chart");
    }
}

// static
Reference< chart2::XDataSeries > SchXMLImportHelper::GetNewDataSeries(
    const Reference< chart2::XChartDocument > & xDoc,
    sal_Int32 nCoordinateSystemIndex,
    const OUString & rChartTypeName,
    bool bPushLastChartType /* = false */ )
{
    Reference< chart2::XDataSeries > xResult;
    if(!xDoc.is())
        return xResult;

    try
    {
        Reference< chart2::XCoordinateSystemContainer > xCooSysCnt(
            xDoc->getFirstDiagram(), uno::UNO_QUERY_THROW );
        Sequence< Reference< chart2::XCoordinateSystem > > aCooSysSeq(
            xCooSysCnt->getCoordinateSystems());
        Reference< uno::XComponentContext > xContext(
            comphelper::getProcessComponentContext() );

        if( nCoordinateSystemIndex < aCooSysSeq.getLength())
        {
            Reference< chart2::XChartType > xCurrentType;
            {
                Reference< chart2::XChartTypeContainer > xCTCnt( aCooSysSeq[ nCoordinateSystemIndex ], uno::UNO_QUERY_THROW );
                Sequence< Reference< chart2::XChartType > > aChartTypes( xCTCnt->getChartTypes());
                // find matching chart type group
                const Reference< chart2::XChartType > * pBegin = aChartTypes.getConstArray();
                const Reference< chart2::XChartType > * pEnd = pBegin + aChartTypes.getLength();
                const Reference< chart2::XChartType > * pIt =
                    ::std::find_if( pBegin, pEnd, lcl_MatchesChartType( rChartTypeName ));
                if( pIt != pEnd )
                    xCurrentType.set( *pIt );
                // if chart type is set at series and differs from current one,
                // create a new chart type
                if( !xCurrentType.is())
                {
                    xCurrentType.set(
                        xContext->getServiceManager()->createInstanceWithContext( rChartTypeName, xContext ),
                        uno::UNO_QUERY );
                    if( xCurrentType.is())
                    {
                        if( bPushLastChartType && aChartTypes.hasElements())
                        {
                            sal_Int32 nIndex( aChartTypes.getLength() - 1 );
                            aChartTypes.realloc( aChartTypes.getLength() + 1 );
                            aChartTypes[ nIndex + 1 ] = aChartTypes[ nIndex ];
                            aChartTypes[ nIndex ] = xCurrentType;
                            xCTCnt->setChartTypes( aChartTypes );
                        }
                        else
                            xCTCnt->addChartType( xCurrentType );
                    }
                }
            }

            if( xCurrentType.is())
            {
                Reference< chart2::XDataSeriesContainer > xSeriesCnt( xCurrentType, uno::UNO_QUERY_THROW );

                if( xContext.is() )
                {
                    xResult.set(
                        xContext->getServiceManager()->createInstanceWithContext(
                            "com.sun.star.chart2.DataSeries",
                            xContext ), uno::UNO_QUERY_THROW );
                }
                if( xResult.is() )
                    xSeriesCnt->addDataSeries( xResult );
            }
        }
    }
    catch( const uno::Exception & )
    {
        DBG_UNHANDLED_EXCEPTION("xmloff.chart");
    }
    return xResult;
}

SchXMLImport::SchXMLImport(
    const Reference< uno::XComponentContext >& xContext,
    OUString const & implementationName, SvXMLImportFlags nImportFlags ) :
    SvXMLImport( xContext, implementationName, nImportFlags ),
    maImportHelper(new SchXMLImportHelper)
{
    GetNamespaceMap().Add( GetXMLToken(XML_NP_XLINK), GetXMLToken(XML_N_XLINK), XML_NAMESPACE_XLINK );
    GetNamespaceMap().Add( GetXMLToken(XML_NP_CHART_EXT), GetXMLToken(XML_N_CHART_EXT), XML_NAMESPACE_CHART_EXT);
}

SchXMLImport::~SchXMLImport() noexcept
{
    uno::Reference< chart2::XChartDocument > xChartDoc( GetModel(), uno::UNO_QUERY );
    if( xChartDoc.is() && xChartDoc->hasControllersLocked() )
        xChartDoc->unlockControllers();
}

// create the main context (subcontexts are created
// by the one created here)
SvXMLImportContext *SchXMLImport::CreateFastContext( sal_Int32 nElement,
        const uno::Reference< xml::sax::XFastAttributeList >& /*xAttrList*/ )
{
    SvXMLImportContext* pContext = nullptr;

    switch (nElement)
    {
        case XML_ELEMENT( OFFICE, XML_DOCUMENT ):
        case XML_ELEMENT( OFFICE, XML_DOCUMENT_META ):
        {
            uno::Reference<document::XDocumentPropertiesSupplier> xDPS(
                GetModel(), uno::UNO_QUERY);
            // mst@: right now, this seems to be not supported, so it is untested
            if (xDPS.is()) {
                pContext = (nElement == XML_ELEMENT(OFFICE, XML_DOCUMENT_META))
                               ? new SvXMLMetaDocumentContext(*this, xDPS->getDocumentProperties())
                               // flat OpenDocument file format
                               : new SchXMLFlatDocContext_Impl(*maImportHelper, *this, nElement,
                                                               xDPS->getDocumentProperties());
            }
        }
        break;
        // accept <office:document>
        case XML_ELEMENT(OFFICE, XML_DOCUMENT_STYLES):
        case XML_ELEMENT(OFFICE, XML_DOCUMENT_CONTENT):
            pContext = new SchXMLDocContext(*maImportHelper, *this, nElement);
        break;
    }
    return pContext;
}

SvXMLImportContext* SchXMLImport::CreateStylesContext()
{
    //#i103287# make sure that the version information is set before importing all the properties (especially stroke-opacity!)
    SchXMLTools::setBuildIDAtImportInfo( GetModel(), getImportInfo() );

    SvXMLStylesContext* pStylesCtxt = new SvXMLStylesContext( *this );

    // set context at base class, so that all auto-style classes are imported
    SetAutoStyles( pStylesCtxt );
    maImportHelper->SetAutoStylesContext( pStylesCtxt );

    return pStylesCtxt;
}

void SAL_CALL SchXMLImport::setTargetDocument(const uno::Reference<lang::XComponent>& xDoc)
{
    uno::Reference<chart2::XChartDocument> xOldDoc(GetModel(), uno::UNO_QUERY);
    if (xOldDoc.is() && xOldDoc->hasControllersLocked())
        xOldDoc->unlockControllers();

    SvXMLImport::setTargetDocument(xDoc);

    uno::Reference<chart2::XChartDocument> xChartDoc(GetModel(), uno::UNO_QUERY);

    if (!xChartDoc.is())
        return;
    try
    {
        // prevent rebuild of view during load (necessary especially if loaded not
        // via load api, which is the case for example if binary files are loaded)
        xChartDoc->lockControllers();

        uno::Reference<container::XChild> xChild(xChartDoc, uno::UNO_QUERY);
        uno::Reference<chart2::data::XDataReceiver> xDataReceiver(xChartDoc, uno::UNO_QUERY);
        if (xChild.is() && xDataReceiver.is())
        {
            Reference<lang::XMultiServiceFactory> xFact(xChild->getParent(), uno::UNO_QUERY);
            if (xFact.is())
            {
                //if the parent has a number formatter we will use the numberformatter of the parent
                Reference<util::XNumberFormatsSupplier> xNumberFormatsSupplier(xFact, uno::UNO_QUERY);
                xDataReceiver->attachNumberFormatsSupplier(xNumberFormatsSupplier);
            }
        }
    }
    catch (const uno::Exception &)
    {
        TOOLS_INFO_EXCEPTION("xmloff.chart", "SchXMLChartContext::StartElement(): Exception caught");
    }
}

// first version: everything comes from one storage

extern "C" SAL_DLLPUBLIC_EXPORT uno::XInterface*
com_sun_star_comp_Chart_XMLOasisImporter_get_implementation(uno::XComponentContext* pCtx,
                                                            uno::Sequence<uno::Any> const& /*rSeq*/)
{
    return cppu::acquire(new SchXMLImport(pCtx, "SchXMLImport", SvXMLImportFlags::ALL));
}

// multiple storage version: one for content / styles / meta

extern "C" SAL_DLLPUBLIC_EXPORT uno::XInterface*
com_sun_star_comp_Chart_XMLOasisMetaImporter_get_implementation(
    uno::XComponentContext* pCtx, uno::Sequence<uno::Any> const& /*rSeq*/)
{
    return cppu::acquire(new SchXMLImport(pCtx, "SchXMLImport.Meta", SvXMLImportFlags::META));
}

extern "C" SAL_DLLPUBLIC_EXPORT uno::XInterface*
com_sun_star_comp_Chart_XMLOasisStylesImporter_get_implementation(
    uno::XComponentContext* pCtx, uno::Sequence<uno::Any> const& /*rSeq*/)
{
    return cppu::acquire(new SchXMLImport(pCtx, "SchXMLImport.Styles", SvXMLImportFlags::STYLES));
}

extern "C" SAL_DLLPUBLIC_EXPORT uno::XInterface*
com_sun_star_comp_Chart_XMLOasisContentImporter_get_implementation(
    uno::XComponentContext* pCtx, uno::Sequence<uno::Any> const& /*rSeq*/)
{
    return cppu::acquire(new SchXMLImport(pCtx, "SchXMLImport.Content",
                                          SvXMLImportFlags::CONTENT | SvXMLImportFlags::AUTOSTYLES
                                              | SvXMLImportFlags::FONTDECLS));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
