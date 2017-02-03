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

#include <sal/config.h>

#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <osl/diagnose.h>
#include <vcl/msgbox.hxx>
#include <vcl/svapp.hxx>
#include <svtools/sfxecode.hxx>
#include <svtools/ehdl.hxx>
#include <svtools/svtools.hrc>
#include <tools/urlobj.hxx>
#include <svx/dialogs.hrc>
#include "oox/ppt/pptimport.hxx"
#include "oox/drawingml/chart/chartconverter.hxx"
#include "oox/dump/pptxdumper.hxx"
#include "drawingml/table/tablestylelistfragmenthandler.hxx"
#include "oox/helper/graphichelper.hxx"
#include "oox/ole/vbaproject.hxx"
#include <oox/ppt/presentationfragmenthandler.hxx>
#include <oox/token/tokens.hxx>

#include <services.hxx>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::xml::sax;
using namespace oox::core;

using ::com::sun::star::beans::PropertyValue;
using ::com::sun::star::lang::XComponent;

namespace oox { namespace ppt {

OUString SAL_CALL PowerPointImport_getImplementationName()
{
    return OUString( "com.sun.star.comp.oox.ppt.PowerPointImport" );
}

uno::Sequence< OUString > SAL_CALL PowerPointImport_getSupportedServiceNames()
{
    Sequence< OUString > aSeq( 2 );
    aSeq[ 0 ] = "com.sun.star.document.ImportFilter";
    aSeq[ 1 ] = "com.sun.star.document.ExportFilter";
    return aSeq;
}

uno::Reference< uno::XInterface > SAL_CALL PowerPointImport_createInstance( const Reference< XComponentContext >& rxContext )
{
    return static_cast< ::cppu::OWeakObject* >( new PowerPointImport( rxContext ) );
}

#if OSL_DEBUG_LEVEL > 0
XmlFilterBase* PowerPointImport::mpDebugFilterBase = nullptr;
#endif

PowerPointImport::PowerPointImport( const Reference< XComponentContext >& rxContext ) :
    XmlFilterBase( rxContext ),
    mxChartConv( new ::oox::drawingml::chart::ChartConverter )

{
#if OSL_DEBUG_LEVEL > 0
    mpDebugFilterBase = this;
#endif
}

PowerPointImport::~PowerPointImport()
{
}

bool PowerPointImport::importDocument()
{
    /*  to activate the PPTX dumper, define the environment variable
        OOO_PPTXDUMPER and insert the full path to the file
        file:///<path-to-oox-module>/source/dump/pptxdumper.ini. */
    OOX_DUMP_FILE( ::oox::dump::pptx::Dumper );

    importDocumentProperties();

    OUString aFragmentPath = getFragmentPathFromFirstTypeFromOfficeDoc( "officeDocument" );
    FragmentHandlerRef xPresentationFragmentHandler( new PresentationFragmentHandler( *this, aFragmentPath ) );
    maTableStyleListPath = xPresentationFragmentHandler->getFragmentPathFromFirstTypeFromOfficeDoc( "tableStyles" );
    bool bRet = importFragment(xPresentationFragmentHandler);

    if (mbMissingExtDrawing)
    {
        // Construct a warning message.
        INetURLObject aURL(getFileUrl());
        SfxErrorContext aContext(ERRCTX_SFX_OPENDOC, aURL.getName(INetURLObject::LAST_SEGMENT), nullptr, RID_ERRCTX);
        OUString aWarning;
        aContext.GetString(ERRCODE_WARNING_MASK, aWarning);
        aWarning += ":\n";
        static ResMgr* pResMgr = ResMgr::CreateResMgr("svx", Application::GetSettings().GetUILanguageTag());
        aWarning += ResId(RID_SVXSTR_WARN_MISSING_SMARTART, *pResMgr).toString();

        // Show it.
        WinBits eBits = WB_OK | WB_DEF_OK;
        ScopedVclPtrInstance<WarningBox> pBox(nullptr, eBits, aWarning);
        pBox->Execute();
    }

    return bRet;

}

bool PowerPointImport::exportDocument() throw()
{
    return false;
}

sal_Int32 PowerPointImport::getSchemeColor( sal_Int32 nToken ) const
{
    sal_Int32 nColor = 0;
    if ( mpActualSlidePersist )
    {
        bool bColorMapped = false;
        oox::drawingml::ClrMapPtr pClrMapPtr( mpActualSlidePersist->getClrMap() );
        if ( pClrMapPtr )
            bColorMapped = pClrMapPtr->getColorMap( nToken );

        if ( !bColorMapped )    // try masterpage mapping
        {
            SlidePersistPtr pMasterPersist = mpActualSlidePersist->getMasterPersist();
            if ( pMasterPersist )
            {
                pClrMapPtr = pMasterPersist->getClrMap();
                if ( pClrMapPtr )
                    pClrMapPtr->getColorMap( nToken );
            }
        }
        oox::drawingml::ClrSchemePtr pClrSchemePtr( mpActualSlidePersist->getClrScheme() );
        if ( pClrSchemePtr )
            pClrSchemePtr->getColor( nToken, nColor );
        else
        {
            ::oox::drawingml::ThemePtr pTheme = mpActualSlidePersist->getTheme();
            if( pTheme )
            {
                pTheme->getClrScheme().getColor( nToken, nColor );
            }
            else
            {
                SAL_WARN("oox", "OOX: PowerPointImport::mpThemePtr is NULL");
            }
        }
    }
    return nColor;
}

const ::oox::drawingml::Theme* PowerPointImport::getCurrentTheme() const
{
    return mpActualSlidePersist ? mpActualSlidePersist->getTheme().get() : nullptr;
}

sal_Bool SAL_CALL PowerPointImport::filter( const Sequence< PropertyValue >& rDescriptor )
{
    if( XmlFilterBase::filter( rDescriptor ) )
        return true;

    if( isExportFilter() ) {
        Reference< XExporter > xExporter( Reference<css::lang::XMultiServiceFactory>(getComponentContext()->getServiceManager(), UNO_QUERY_THROW)->createInstance( "com.sun.star.comp.Impress.oox.PowerPointExport" ), UNO_QUERY );

        if( xExporter.is() ) {
            Reference< XComponent > xDocument( getModel(), UNO_QUERY );
            Reference< XFilter > xFilter( xExporter, UNO_QUERY );

            if( xFilter.is() ) {
                xExporter->setSourceDocument( xDocument );
                if( xFilter->filter( rDescriptor ) )
                    return true;
            }
        }
    }

    return false;
}

::oox::vml::Drawing* PowerPointImport::getVmlDrawing()
{
    return mpActualSlidePersist ? mpActualSlidePersist->getDrawing() : nullptr;
}

const oox::drawingml::table::TableStyleListPtr PowerPointImport::getTableStyles()
{
    if ( !mpTableStyleList && !maTableStyleListPath.isEmpty() )
    {
        mpTableStyleList = std::make_shared<oox::drawingml::table::TableStyleList>( );
        importFragment( new oox::drawingml::table::TableStyleListFragmentHandler(
            *this, maTableStyleListPath, *mpTableStyleList ) );
    }
    return mpTableStyleList;
}

::oox::drawingml::chart::ChartConverter* PowerPointImport::getChartConverter()
{
    return mxChartConv.get();
}

namespace {

class PptGraphicHelper : public GraphicHelper
{
public:
    explicit            PptGraphicHelper( const PowerPointImport& rFilter );
    virtual sal_Int32   getSchemeColor( sal_Int32 nToken ) const override;
    virtual sal_Int32 getDefaultChartAreaFillStyle() const override;
private:
    const PowerPointImport& mrFilter;
};

PptGraphicHelper::PptGraphicHelper( const PowerPointImport& rFilter ) :
    GraphicHelper( rFilter.getComponentContext(), rFilter.getTargetFrame(), rFilter.getStorage() ),
    mrFilter( rFilter )
{
}

sal_Int32 PptGraphicHelper::getSchemeColor( sal_Int32 nToken ) const
{
    return mrFilter.getSchemeColor( nToken );
}

sal_Int32 PptGraphicHelper::getDefaultChartAreaFillStyle() const
{
    return XML_noFill;
}

} // namespace

GraphicHelper* PowerPointImport::implCreateGraphicHelper() const
{
    return new PptGraphicHelper( *this );
}

::oox::ole::VbaProject* PowerPointImport::implCreateVbaProject() const
{
    return new ::oox::ole::VbaProject( getComponentContext(), getModel(), "Impress" );
}

OUString PowerPointImport::getImplementationName()
{
    return PowerPointImport_getImplementationName();
}

}}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
