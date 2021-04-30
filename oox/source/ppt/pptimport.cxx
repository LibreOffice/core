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

#include <config_wasm_strip.h>

#include <sal/config.h>
#include <sal/log.hxx>

#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/document/XUndoManager.hpp>
#include <com/sun/star/document/XUndoManagerSupplier.hpp>
#include <comphelper/propertysequence.hxx>
#include <comphelper/scopeguard.hxx>
#include <vcl/svapp.hxx>
#include <vcl/weld.hxx>
#include <svtools/sfxecode.hxx>
#include <svtools/ehdl.hxx>
#include <tools/urlobj.hxx>
#include <svx/dialmgr.hxx>
#include <svx/strings.hrc>
#include <oox/ppt/pptimport.hxx>
#include <oox/drawingml/chart/chartconverter.hxx>
#include <oox/dump/pptxdumper.hxx>
#include <drawingml/table/tablestylelistfragmenthandler.hxx>
#include <oox/helper/graphichelper.hxx>
#include <oox/ole/vbaproject.hxx>
#include <oox/ppt/presentationfragmenthandler.hxx>
#include <oox/ppt/presPropsfragmenthandler.hxx>
#include <oox/token/tokens.hxx>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::xml::sax;
using namespace oox::core;

using ::com::sun::star::beans::PropertyValue;
using ::com::sun::star::lang::XComponent;

namespace oox::ppt {

#if OSL_DEBUG_LEVEL > 0
XmlFilterBase* PowerPointImport::mpDebugFilterBase = nullptr;
#endif

PowerPointImport::PowerPointImport( const Reference< XComponentContext >& rxContext ) :
    XmlFilterBase( rxContext ),
#ifdef ENABLE_WASM_STRIP_CHART
    // WASM_CHART change
    mxChartConv( )
#else
    mxChartConv( std::make_shared<::oox::drawingml::chart::ChartConverter>() )
#endif
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

    uno::Reference< document::XUndoManagerSupplier > xUndoManagerSupplier (getModel(), UNO_QUERY );
    uno::Reference< util::XLockable > xUndoManager;
    bool bWasUnLocked = true;
    if(xUndoManagerSupplier.is())
    {
        xUndoManager = xUndoManagerSupplier->getUndoManager();
        if(xUndoManager.is())
        {
            bWasUnLocked = !xUndoManager->isLocked();
            xUndoManager->lock();
        }
    }

    importDocumentProperties();

    OUString aFragmentPath = getFragmentPathFromFirstTypeFromOfficeDoc( u"officeDocument" );
    FragmentHandlerRef xPresentationFragmentHandler( new PresentationFragmentHandler( *this, aFragmentPath ) );
    maTableStyleListPath = xPresentationFragmentHandler->getFragmentPathFromFirstTypeFromOfficeDoc( u"tableStyles" );
    const OUString sPresPropsPath
        = xPresentationFragmentHandler->getFragmentPathFromFirstTypeFromOfficeDoc(u"presProps");

    bool bRet = importFragment(xPresentationFragmentHandler);
    if (bRet && !sPresPropsPath.isEmpty())
    {
        FragmentHandlerRef xPresPropsFragmentHandler(
            new PresPropsFragmentHandler(*this, sPresPropsPath));
        importFragment(xPresPropsFragmentHandler);
    }

    static bool bNoSmartartWarning = getenv("OOX_NO_SMARTART_WARNING");
    if (!bNoSmartartWarning && mbMissingExtDrawing)
    {
        // Construct a warning message.
        INetURLObject aURL(getFileUrl());
        SfxErrorContext aContext(ERRCTX_SFX_OPENDOC, aURL.getName(INetURLObject::LAST_SEGMENT, true, INetURLObject::DecodeMechanism::WithCharset), nullptr, RID_ERRCTX);
        OUString aWarning;
        aContext.GetString(ERRCODE_NONE.MakeWarning(), aWarning);
        aWarning += ":\n" + SvxResId(RID_SVXSTR_WARN_MISSING_SMARTART);

        // Show it.
        std::unique_ptr<weld::MessageDialog> xWarn(Application::CreateMessageDialog(nullptr,
                                                   VclMessageType::Warning, VclButtonsType::Ok,
                                                   aWarning));
        xWarn->run();
    }

    if(xUndoManager.is() && bWasUnLocked)
        xUndoManager->unlock();

    return bRet;

}

bool PowerPointImport::exportDocument() noexcept
{
    return false;
}

::Color PowerPointImport::getSchemeColor( sal_Int32 nToken ) const
{
    ::Color nColor;
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

    if (isExportFilter())
    {
        uno::Sequence<uno::Any> aArguments(comphelper::InitAnyPropertySequence(
        {
            {"IsPPTM", uno::makeAny(exportVBA())},
            {"IsTemplate", uno::makeAny(isExportTemplate())},
        }));

        Reference<css::lang::XMultiServiceFactory> aFactory(getComponentContext()->getServiceManager(), UNO_QUERY_THROW);
        Reference< XExporter > xExporter(aFactory->createInstanceWithArguments("com.sun.star.comp.Impress.oox.PowerPointExport", aArguments), UNO_QUERY);

        if (Reference<XFilter> xFilter{ xExporter, UNO_QUERY })
        {
            Reference<util::XLockable> xUndoManager;
            bool bWasUnLocked = true;
            if (Reference<document::XUndoManagerSupplier> xUMS{ getModel(), UNO_QUERY })
            {
                xUndoManager = xUMS->getUndoManager();
                if (xUndoManager.is())
                {
                    bWasUnLocked = !xUndoManager->isLocked();
                    xUndoManager->lock();
                }
            }
            comphelper::ScopeGuard aGuard([xUndoManager, bWasUnLocked] {
                if (xUndoManager && bWasUnLocked)
                    xUndoManager->unlock();
            });

            Reference< XComponent > xDocument = getModel();
            xExporter->setSourceDocument(xDocument);
            if (xFilter->filter(rDescriptor))
                return true;
        }
    }

    return false;
}

::oox::vml::Drawing* PowerPointImport::getVmlDrawing()
{
    return mpActualSlidePersist ? mpActualSlidePersist->getDrawing() : nullptr;
}

oox::drawingml::table::TableStyleListPtr PowerPointImport::getTableStyles()
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
    virtual ::Color     getSchemeColor( sal_Int32 nToken ) const override;
    virtual sal_Int32   getDefaultChartAreaFillStyle() const override;
private:
    const PowerPointImport& mrFilter;
};

PptGraphicHelper::PptGraphicHelper( const PowerPointImport& rFilter ) :
    GraphicHelper( rFilter.getComponentContext(), rFilter.getTargetFrame(), rFilter.getStorage() ),
    mrFilter( rFilter )
{
}

::Color PptGraphicHelper::getSchemeColor( sal_Int32 nToken ) const
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
    return new ::oox::ole::VbaProject( getComponentContext(), getModel(), u"Impress" );
}

OUString PowerPointImport::getImplementationName()
{
    return "com.sun.star.comp.oox.ppt.PowerPointImport";
}

}

extern "C" SAL_DLLPUBLIC_EXPORT uno::XInterface*
com_sun_star_comp_oox_ppt_PowerPointImport_get_implementation(
    uno::XComponentContext* pCtx, uno::Sequence<uno::Any> const& /*rSeq*/)
{
    return cppu::acquire(new oox::ppt::PowerPointImport(pCtx));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
