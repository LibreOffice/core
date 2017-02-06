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

#include <vector>

#include "ReportDefinition.hxx"

#include "FixedLine.hxx"
#include "FixedText.hxx"
#include "FormattedField.hxx"
#include "Functions.hxx"
#include "Groups.hxx"
#include "ImageControl.hxx"
#include "ReportComponent.hxx"
#include "ReportHelperImpl.hxx"
#include "RptDef.hxx"
#include "RptModel.hxx"
#include "Section.hxx"
#include "Shape.hxx"
#include "Tools.hxx"
#include "UndoEnv.hxx"
#include "core_resource.hrc"
#include "core_resource.hxx"
#include "corestrings.hrc"

#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <com/sun/star/beans/XMultiPropertyStates.hpp>
#include <com/sun/star/chart2/data/DatabaseDataProvider.hpp>
#include <com/sun/star/datatransfer/UnsupportedFlavorException.hpp>
#include <com/sun/star/document/DocumentProperties.hpp>
#include <com/sun/star/document/IndexedPropertyValues.hpp>
#include <com/sun/star/document/EventObject.hpp>
#include <com/sun/star/document/XEventListener.hpp>
#include <com/sun/star/document/XExporter.hpp>
#include <com/sun/star/document/XFilter.hpp>
#include <com/sun/star/document/XImporter.hpp>
#include <com/sun/star/embed/Aspects.hpp>
#include <com/sun/star/embed/ElementModes.hpp>
#include <com/sun/star/embed/EmbedMapUnits.hpp>
#include <com/sun/star/embed/EntryInitModes.hpp>
#include <com/sun/star/embed/XEmbedPersist.hpp>
#include <com/sun/star/embed/XTransactedObject.hpp>
#include <com/sun/star/embed/StorageFactory.hpp>
#include <com/sun/star/frame/Desktop.hpp>
#include <com/sun/star/frame/FrameSearchFlag.hpp>
#include <com/sun/star/frame/XComponentLoader.hpp>
#include <com/sun/star/io/IOException.hpp>
#include <com/sun/star/io/XActiveDataSource.hpp>
#include <com/sun/star/io/XSeekable.hpp>
#include <com/sun/star/lang/XSingleServiceFactory.hpp>
#include <com/sun/star/report/GroupKeepTogether.hpp>
#include <com/sun/star/report/ReportPrintOption.hpp>
#include <com/sun/star/report/XFunction.hpp>
#include <com/sun/star/sdb/CommandType.hpp>
#include <com/sun/star/sdb/XOfficeDatabaseDocument.hpp>
#include <com/sun/star/style/GraphicLocation.hpp>
#include <com/sun/star/style/NumberingType.hpp>
#include <com/sun/star/style/PageStyleLayout.hpp>
#include <com/sun/star/style/XStyle.hpp>
#include <com/sun/star/table/BorderLine2.hpp>
#include <com/sun/star/table/ShadowFormat.hpp>
#include <com/sun/star/task/InteractionHandler.hpp>
#include <com/sun/star/task/XStatusIndicator.hpp>
#include <com/sun/star/task/XStatusIndicatorFactory.hpp>
#include <com/sun/star/ui/UIConfigurationManager.hpp>
#include <com/sun/star/ui/XUIConfigurationStorage.hpp>
#include <com/sun/star/util/CloseVetoException.hpp>
#include <com/sun/star/util/NumberFormatsSupplier.hpp>
#include <com/sun/star/xml/AttributeData.hpp>
#include <com/sun/star/xml/sax/Writer.hpp>

#include <comphelper/broadcasthelper.hxx>
#include <comphelper/documentconstants.hxx>
#include <comphelper/genericpropertyset.hxx>
#include <unotools/mediadescriptor.hxx>
#include <comphelper/mimeconfighelper.hxx>
#include <comphelper/namecontainer.hxx>
#include <comphelper/namedvaluecollection.hxx>
#include <comphelper/numberedcollection.hxx>
#include <comphelper/proparrhlp.hxx>
#include <comphelper/property.hxx>
#include <comphelper/propertysetinfo.hxx>
#include <comphelper/propertystatecontainer.hxx>
#include <comphelper/seqstream.hxx>
#include <comphelper/sequence.hxx>
#include <comphelper/storagehelper.hxx>
#include <comphelper/uno3.hxx>
#include <connectivity/CommonTools.hxx>
#include <connectivity/dbconversion.hxx>
#include <connectivity/dbtools.hxx>
#include <cppuhelper/exc_hlp.hxx>
#include <cppuhelper/implbase.hxx>
#include <cppuhelper/interfacecontainer.h>
#include <cppuhelper/supportsservice.hxx>
#include <dbaccess/dbaundomanager.hxx>
#include <editeng/paperinf.hxx>
#include <framework/titlehelper.hxx>
#include <svl/itempool.hxx>
#include <svl/undo.hxx>
#include <svx/svdlayer.hxx>
#include <svx/unofill.hxx>
#include <svx/xmleohlp.hxx>
#include <svx/xmlgrhlp.hxx>
#include <tools/debug.hxx>
#include <tools/diagnose_ex.h>
#include <unotools/moduleoptions.hxx>
#include <unotools/saveopt.hxx>
#include <unotools/streamwrap.hxx>
#include <vcl/svapp.hxx>
#include <vcl/virdev.hxx>

//  page styles
#define SC_UNO_PAGE_LEFTBORDER      "LeftBorder"
#define SC_UNO_PAGE_RIGHTBORDER     "RightBorder"
#define SC_UNO_PAGE_BOTTBORDER      "BottomBorder"
#define SC_UNO_PAGE_TOPBORDER       "TopBorder"
#define SC_UNO_PAGE_LEFTBRDDIST     "LeftBorderDistance"
#define SC_UNO_PAGE_RIGHTBRDDIST    "RightBorderDistance"
#define SC_UNO_PAGE_BOTTBRDDIST     "BottomBorderDistance"
#define SC_UNO_PAGE_TOPBRDDIST      "TopBorderDistance"
#define SC_UNO_PAGE_BORDERDIST      "BorderDistance"
#define SC_UNO_PAGE_SHADOWFORM      "ShadowFormat"
#define SC_UNO_PAGE_PAPERTRAY       "PrinterPaperTray"
#define SC_UNO_PAGE_SCALEVAL        "PageScale"
#define SC_UNO_PAGE_SCALETOPAG      "ScaleToPages"
#define SC_UNO_PAGE_SCALETOX        "ScaleToPagesX"
#define SC_UNO_PAGE_SCALETOY        "ScaleToPagesY"
#define SC_UNO_PAGE_HDRBACKCOL      "HeaderBackColor"
#define SC_UNO_PAGE_HDRBACKTRAN     "HeaderBackTransparent"
#define SC_UNO_PAGE_HDRGRFFILT      "HeaderBackGraphicFilter"
#define SC_UNO_PAGE_HDRGRFLOC       "HeaderBackGraphicLocation"
#define SC_UNO_PAGE_HDRGRFURL       "HeaderBackGraphicURL"
#define SC_UNO_PAGE_HDRLEFTBOR      "HeaderLeftBorder"
#define SC_UNO_PAGE_HDRRIGHTBOR     "HeaderRightBorder"
#define SC_UNO_PAGE_HDRBOTTBOR      "HeaderBottomBorder"
#define SC_UNO_PAGE_HDRTOPBOR       "HeaderTopBorder"
#define SC_UNO_PAGE_HDRLEFTBDIS     "HeaderLeftBorderDistance"
#define SC_UNO_PAGE_HDRRIGHTBDIS    "HeaderRightBorderDistance"
#define SC_UNO_PAGE_HDRBOTTBDIS     "HeaderBottomBorderDistance"
#define SC_UNO_PAGE_HDRTOPBDIS      "HeaderTopBorderDistance"
#define SC_UNO_PAGE_HDRBRDDIST      "HeaderBorderDistance"
#define SC_UNO_PAGE_HDRSHADOW       "HeaderShadowFormat"
#define SC_UNO_PAGE_HDRLEFTMAR      "HeaderLeftMargin"
#define SC_UNO_PAGE_HDRRIGHTMAR     "HeaderRightMargin"
#define SC_UNO_PAGE_HDRBODYDIST     "HeaderBodyDistance"
#define SC_UNO_PAGE_HDRHEIGHT       "HeaderHeight"
#define SC_UNO_PAGE_HDRON           "HeaderIsOn"
#define SC_UNO_PAGE_HDRDYNAMIC      "HeaderIsDynamicHeight"
#define SC_UNO_PAGE_HDRSHARED       "HeaderIsShared"
#define SC_UNO_PAGE_FTRBACKCOL      "FooterBackColor"
#define SC_UNO_PAGE_FTRBACKTRAN     "FooterBackTransparent"
#define SC_UNO_PAGE_FTRGRFFILT      "FooterBackGraphicFilter"
#define SC_UNO_PAGE_FTRGRFLOC       "FooterBackGraphicLocation"
#define SC_UNO_PAGE_FTRGRFURL       "FooterBackGraphicURL"
#define SC_UNO_PAGE_FTRLEFTBOR      "FooterLeftBorder"
#define SC_UNO_PAGE_FTRRIGHTBOR     "FooterRightBorder"
#define SC_UNO_PAGE_FTRBOTTBOR      "FooterBottomBorder"
#define SC_UNO_PAGE_FTRTOPBOR       "FooterTopBorder"
#define SC_UNO_PAGE_FTRLEFTBDIS     "FooterLeftBorderDistance"
#define SC_UNO_PAGE_FTRRIGHTBDIS    "FooterRightBorderDistance"
#define SC_UNO_PAGE_FTRBOTTBDIS     "FooterBottomBorderDistance"
#define SC_UNO_PAGE_FTRTOPBDIS      "FooterTopBorderDistance"
#define SC_UNO_PAGE_FTRBRDDIST      "FooterBorderDistance"
#define SC_UNO_PAGE_FTRSHADOW       "FooterShadowFormat"
#define SC_UNO_PAGE_FTRLEFTMAR      "FooterLeftMargin"
#define SC_UNO_PAGE_FTRRIGHTMAR     "FooterRightMargin"
#define SC_UNO_PAGE_FTRBODYDIST     "FooterBodyDistance"
#define SC_UNO_PAGE_FTRHEIGHT       "FooterHeight"
#define SC_UNO_PAGE_FTRON           "FooterIsOn"
#define SC_UNO_PAGE_FTRDYNAMIC      "FooterIsDynamicHeight"
#define SC_UNO_PAGE_FTRSHARED       "FooterIsShared"

namespace reportdesign
{
    using namespace com::sun::star;
    using namespace comphelper;
    using namespace rptui;

void lcl_setModelReadOnly(const uno::Reference< embed::XStorage >& _xStorage,std::shared_ptr<rptui::OReportModel>& _rModel)
{
    uno::Reference<beans::XPropertySet> xProp(_xStorage,uno::UNO_QUERY);
    sal_Int32 nOpenMode = embed::ElementModes::READ;
    if ( xProp.is() )
        xProp->getPropertyValue("OpenMode") >>= nOpenMode;

    _rModel->SetReadOnly((nOpenMode & embed::ElementModes::WRITE) != embed::ElementModes::WRITE);
}
void lcl_stripLoadArguments( utl::MediaDescriptor& _rDescriptor, uno::Sequence< beans::PropertyValue >& _rArgs )
{
    _rDescriptor.erase( OUString( "StatusIndicator" ) );
    _rDescriptor.erase( OUString( "InteractionHandler" ) );
    _rDescriptor.erase( OUString( "Model" ) );
    _rDescriptor >> _rArgs;
}

void lcl_extractAndStartStatusIndicator( const utl::MediaDescriptor& _rDescriptor, uno::Reference< task::XStatusIndicator >& _rxStatusIndicator,
    uno::Sequence< uno::Any >& _rCallArgs )
{
    try
    {
        _rxStatusIndicator = _rDescriptor.getUnpackedValueOrDefault( utl::MediaDescriptor::PROP_STATUSINDICATOR(), _rxStatusIndicator );
        if ( _rxStatusIndicator.is() )
        {
            _rxStatusIndicator->start( OUString(), (sal_Int32)1000000 );

            sal_Int32 nLength = _rCallArgs.getLength();
            _rCallArgs.realloc( nLength + 1 );
            _rCallArgs[ nLength ] <<= _rxStatusIndicator;
        }
    }
    catch (const uno::Exception&)
    {
        OSL_FAIL( "lcl_extractAndStartStatusIndicator: caught an exception!" );
    }
}

typedef ::comphelper::OPropertyStateContainer       OStyle_PBASE;
class OStyle;
typedef ::comphelper::OPropertyArrayUsageHelper <   OStyle
                                                >   OStyle_PABASE;
typedef ::cppu::WeakImplHelper< style::XStyle, beans::XMultiPropertyStates> TStyleBASE;

class OStyle :   public ::comphelper::OMutexAndBroadcastHelper
                ,public TStyleBASE
                ,public OStyle_PBASE
                ,public OStyle_PABASE
{
    awt::Size m_aSize;

protected:
    void getPropertyDefaultByHandle( sal_Int32 _nHandle, uno::Any& _rDefault ) const override;
    virtual ~OStyle() override {}
public:
    OStyle();


    DECLARE_XINTERFACE( )

    // XPropertySet
    css::uno::Reference<css::beans::XPropertySetInfo>  SAL_CALL getPropertySetInfo() override;
    ::cppu::IPropertyArrayHelper& SAL_CALL getInfoHelper() override;
    ::cppu::IPropertyArrayHelper* createArrayHelper( ) const override;

    // XStyle
    sal_Bool SAL_CALL isUserDefined(  ) override;
    sal_Bool SAL_CALL isInUse(  ) override;
    OUString SAL_CALL getParentStyle(  ) override;
    void SAL_CALL setParentStyle( const OUString& aParentStyle ) override;

    // XNamed
    OUString SAL_CALL getName(  ) override;
    void SAL_CALL setName( const OUString& aName ) override;

    // XMultiPropertyState
    uno::Sequence< beans::PropertyState > SAL_CALL getPropertyStates( const uno::Sequence< OUString >& aPropertyNames ) override
    {
        return OStyle_PBASE::getPropertyStates(aPropertyNames);
    }
    void SAL_CALL setAllPropertiesToDefault(  ) override;
    void SAL_CALL setPropertiesToDefault( const uno::Sequence< OUString >& aPropertyNames ) override;
    uno::Sequence< uno::Any > SAL_CALL getPropertyDefaults( const uno::Sequence< OUString >& aPropertyNames ) override;
};

OStyle::OStyle()
:OStyle_PBASE(m_aBHelper)
,m_aSize(21000,29700)
{
    const ::Size aDefaultSize = SvxPaperInfo::GetDefaultPaperSize( MapUnit::Map100thMM );
    m_aSize.Height = aDefaultSize.Height();
    m_aSize.Width = aDefaultSize.Width();

    const sal_Int32 nMargin = 2000;
    const sal_Int32 nBound = beans::PropertyAttribute::BOUND;
    const sal_Int32 nMayBeVoid = beans::PropertyAttribute::MAYBEVOID;

    sal_Int32 i = 0;
    registerPropertyNoMember( PROPERTY_NAME, ++i, nBound, cppu::UnoType<OUString>::get(), css::uno::Any(OUString("Default")) );

    registerPropertyNoMember(PROPERTY_BACKCOLOR,                    ++i,nBound, cppu::UnoType<sal_Int32>::get(), css::uno::makeAny<sal_Int32>(COL_TRANSPARENT));

    registerPropertyNoMember(PROPERTY_BACKGRAPHICLOCATION,  ++i,nBound, cppu::UnoType<style::GraphicLocation>::get(), css::uno::Any(style::GraphicLocation_NONE));
    registerPropertyNoMember(PROPERTY_BACKTRANSPARENT,  ++i,nBound,cppu::UnoType<bool>::get(), css::uno::Any(true));
    registerPropertyNoMember(SC_UNO_PAGE_BORDERDIST,  ++i,nBound, cppu::UnoType<sal_Int32>::get(), css::uno::makeAny<sal_Int32>(0));
    registerPropertyNoMember(SC_UNO_PAGE_BOTTBORDER,  ++i,nBound, cppu::UnoType<table::BorderLine2>::get(), css::uno::Any(table::BorderLine2()));
    registerPropertyNoMember(SC_UNO_PAGE_BOTTBRDDIST, ++i,nBound, cppu::UnoType<sal_Int32>::get(), css::uno::makeAny<sal_Int32>(0));
    registerPropertyNoMember(PROPERTY_BOTTOMMARGIN, ++i,nBound, cppu::UnoType<sal_Int32>::get(), css::uno::Any(nMargin));
    registerPropertyNoMember("DisplayName",       ++i,nBound, cppu::UnoType<OUString>::get(), css::uno::Any(OUString()));
    registerPropertyNoMember(SC_UNO_PAGE_FTRBACKCOL,  ++i,nBound, cppu::UnoType<sal_Int32>::get(), css::uno::makeAny<sal_Int32>(COL_TRANSPARENT));
    registerPropertyNoMember(SC_UNO_PAGE_FTRGRFFILT,  ++i,nBound, cppu::UnoType<OUString>::get(), css::uno::Any(OUString()));
    registerPropertyNoMember(SC_UNO_PAGE_FTRGRFLOC,   ++i,nBound, cppu::UnoType<style::GraphicLocation>::get(), css::uno::Any(style::GraphicLocation_NONE));
    registerPropertyNoMember(SC_UNO_PAGE_FTRGRFURL,   ++i,nBound, cppu::UnoType<OUString>::get(), css::uno::Any(OUString()));
    registerPropertyNoMember(SC_UNO_PAGE_FTRBACKTRAN, ++i,nBound,cppu::UnoType<bool>::get(), css::uno::Any(true));
    registerPropertyNoMember(SC_UNO_PAGE_FTRBODYDIST, ++i,nBound, cppu::UnoType<sal_Int32>::get(), css::uno::makeAny<sal_Int32>(0));
    registerPropertyNoMember(SC_UNO_PAGE_FTRBRDDIST,  ++i,nBound, cppu::UnoType<sal_Int32>::get(), css::uno::makeAny<sal_Int32>(0));
    registerPropertyNoMember(SC_UNO_PAGE_FTRBOTTBOR,  ++i,nBound, cppu::UnoType<table::BorderLine2>::get(), css::uno::Any(table::BorderLine2()));
    registerPropertyNoMember(SC_UNO_PAGE_FTRBOTTBDIS, ++i,nBound, cppu::UnoType<sal_Int32>::get(), css::uno::makeAny<sal_Int32>(0));
    registerPropertyNoMember(SC_UNO_PAGE_FTRHEIGHT,   ++i,nBound, cppu::UnoType<sal_Int32>::get(), css::uno::makeAny<sal_Int32>(0));
    registerPropertyNoMember(SC_UNO_PAGE_FTRDYNAMIC,  ++i,nBound,cppu::UnoType<bool>::get(), css::uno::Any(false));
    registerPropertyNoMember(SC_UNO_PAGE_FTRON,       ++i,nBound,cppu::UnoType<bool>::get(), css::uno::Any(false));
    registerPropertyNoMember(SC_UNO_PAGE_FTRSHARED,   ++i,nBound,cppu::UnoType<bool>::get(), css::uno::Any(false));
    registerPropertyNoMember(SC_UNO_PAGE_FTRLEFTBOR,  ++i,nBound, cppu::UnoType<table::BorderLine2>::get(), css::uno::Any(table::BorderLine2()));
    registerPropertyNoMember(SC_UNO_PAGE_FTRLEFTBDIS, ++i,nBound, cppu::UnoType<sal_Int32>::get(), css::uno::makeAny<sal_Int32>(0));
    registerPropertyNoMember(SC_UNO_PAGE_FTRLEFTMAR,  ++i,nBound, cppu::UnoType<sal_Int32>::get(), css::uno::makeAny<sal_Int32>(0));
    registerPropertyNoMember(SC_UNO_PAGE_FTRRIGHTBOR, ++i,nBound, cppu::UnoType<table::BorderLine2>::get(), css::uno::Any(table::BorderLine2()));
    registerPropertyNoMember(SC_UNO_PAGE_FTRRIGHTBDIS,++i,nBound, cppu::UnoType<sal_Int32>::get(), css::uno::makeAny<sal_Int32>(0));
    registerPropertyNoMember(SC_UNO_PAGE_FTRRIGHTMAR, ++i,nBound, cppu::UnoType<sal_Int32>::get(), css::uno::makeAny<sal_Int32>(0));
    registerPropertyNoMember(SC_UNO_PAGE_FTRSHADOW,   ++i,nBound, cppu::UnoType<table::ShadowFormat>::get(), css::uno::Any(table::ShadowFormat()));
    registerPropertyNoMember(SC_UNO_PAGE_FTRTOPBOR,   ++i,nBound, cppu::UnoType<table::BorderLine2>::get(), css::uno::Any(table::BorderLine2()));
    registerPropertyNoMember(SC_UNO_PAGE_FTRTOPBDIS,  ++i,nBound, cppu::UnoType<sal_Int32>::get(), css::uno::makeAny<sal_Int32>(0));

    registerPropertyNoMember(SC_UNO_PAGE_HDRBACKCOL,  ++i,nBound|nMayBeVoid, cppu::UnoType<sal_Int32>::get(), css::uno::makeAny<sal_Int32>(COL_TRANSPARENT));
    registerPropertyNoMember(SC_UNO_PAGE_HDRGRFFILT,  ++i,nBound|nMayBeVoid, cppu::UnoType<OUString>::get(), css::uno::Any(OUString()));
    registerPropertyNoMember(SC_UNO_PAGE_HDRGRFLOC,   ++i,nBound|nMayBeVoid, cppu::UnoType<style::GraphicLocation>::get(), css::uno::Any(style::GraphicLocation_NONE));
    registerPropertyNoMember(SC_UNO_PAGE_HDRGRFURL,   ++i,nBound|nMayBeVoid, cppu::UnoType<OUString>::get(), css::uno::Any(OUString()));
    registerPropertyNoMember(SC_UNO_PAGE_HDRBACKTRAN, ++i,nBound|nMayBeVoid,cppu::UnoType<bool>::get(), css::uno::Any(true));
    registerPropertyNoMember(SC_UNO_PAGE_HDRBODYDIST, ++i,nBound|nMayBeVoid, cppu::UnoType<sal_Int32>::get(), css::uno::makeAny<sal_Int32>(0));
    registerPropertyNoMember(SC_UNO_PAGE_HDRBRDDIST,  ++i,nBound|nMayBeVoid, cppu::UnoType<sal_Int32>::get(), css::uno::makeAny<sal_Int32>(0));
    registerPropertyNoMember(SC_UNO_PAGE_HDRBOTTBOR,  ++i,nBound|nMayBeVoid, cppu::UnoType<table::BorderLine2>::get(), css::uno::Any(table::BorderLine2()));
    registerPropertyNoMember(SC_UNO_PAGE_HDRBOTTBDIS, ++i,nBound|nMayBeVoid, cppu::UnoType<sal_Int32>::get(), css::uno::makeAny<sal_Int32>(0));
    registerPropertyNoMember(SC_UNO_PAGE_HDRHEIGHT,   ++i,nBound|nMayBeVoid, cppu::UnoType<sal_Int32>::get(), css::uno::makeAny<sal_Int32>(0));
    registerPropertyNoMember(SC_UNO_PAGE_HDRDYNAMIC,  ++i,nBound|nMayBeVoid,cppu::UnoType<bool>::get(), css::uno::Any(false));
    registerPropertyNoMember(SC_UNO_PAGE_HDRON,       ++i,nBound|nMayBeVoid,cppu::UnoType<bool>::get(), css::uno::Any(false));
    registerPropertyNoMember(SC_UNO_PAGE_HDRSHARED,   ++i,nBound|nMayBeVoid,cppu::UnoType<bool>::get(), css::uno::Any(false));
    registerPropertyNoMember(SC_UNO_PAGE_HDRLEFTBOR,  ++i,nBound|nMayBeVoid, cppu::UnoType<table::BorderLine2>::get(), css::uno::Any(table::BorderLine2()));
    registerPropertyNoMember(SC_UNO_PAGE_HDRLEFTBDIS, ++i,nBound|nMayBeVoid, cppu::UnoType<sal_Int32>::get(), css::uno::makeAny<sal_Int32>(0));
    registerPropertyNoMember(SC_UNO_PAGE_HDRLEFTMAR,  ++i,nBound|nMayBeVoid, cppu::UnoType<sal_Int32>::get(), css::uno::makeAny<sal_Int32>(0));
    registerPropertyNoMember(SC_UNO_PAGE_HDRRIGHTBOR, ++i,nBound|nMayBeVoid, cppu::UnoType<table::BorderLine2>::get(), css::uno::Any(table::BorderLine2()));
    registerPropertyNoMember(SC_UNO_PAGE_HDRRIGHTBDIS,++i,nBound|nMayBeVoid, cppu::UnoType<sal_Int32>::get(), css::uno::makeAny<sal_Int32>(0));
    registerPropertyNoMember(SC_UNO_PAGE_HDRRIGHTMAR, ++i,nBound|nMayBeVoid, cppu::UnoType<sal_Int32>::get(), css::uno::makeAny<sal_Int32>(0));
    registerPropertyNoMember(SC_UNO_PAGE_HDRSHADOW,   ++i,nBound|nMayBeVoid, cppu::UnoType<table::ShadowFormat>::get(), css::uno::Any(table::ShadowFormat()));
    registerPropertyNoMember(SC_UNO_PAGE_HDRTOPBOR,   ++i,nBound|nMayBeVoid, cppu::UnoType<table::BorderLine2>::get(), css::uno::Any(table::BorderLine2()));
    registerPropertyNoMember(SC_UNO_PAGE_HDRTOPBDIS,  ++i,nBound|nMayBeVoid, cppu::UnoType<sal_Int32>::get(), css::uno::makeAny<sal_Int32>(0));

    registerProperty(PROPERTY_HEIGHT,       ++i,nBound,&m_aSize.Height,     ::cppu::UnoType<sal_Int32>::get() );
    registerPropertyNoMember(PROPERTY_ISLANDSCAPE,                  ++i,nBound,         cppu::UnoType<bool>::get(), css::uno::Any(false));
    registerPropertyNoMember(SC_UNO_PAGE_LEFTBORDER,  ++i,nBound, cppu::UnoType<table::BorderLine2>::get(), css::uno::Any(table::BorderLine2()));
    registerPropertyNoMember(SC_UNO_PAGE_LEFTBRDDIST, ++i,nBound, cppu::UnoType<sal_Int32>::get(), css::uno::makeAny<sal_Int32>(0));
    registerPropertyNoMember(PROPERTY_LEFTMARGIN,   ++i,beans::PropertyAttribute::BOUND,        ::cppu::UnoType<sal_Int32>::get(), css::uno::Any(nMargin));
    registerPropertyNoMember(PROPERTY_NUMBERINGTYPE,                ++i,nBound, cppu::UnoType<sal_Int16>::get(), css::uno::Any(style::NumberingType::ARABIC));
    registerPropertyNoMember(SC_UNO_PAGE_SCALEVAL,    ++i,nBound, cppu::UnoType<sal_Int16>::get(), css::uno::makeAny<sal_Int16>(0));
    registerPropertyNoMember(PROPERTY_PAGESTYLELAYOUT,              ++i,nBound, cppu::UnoType<style::PageStyleLayout>::get(), css::uno::Any(style::PageStyleLayout_ALL));
    registerPropertyNoMember(SC_UNO_PAGE_PAPERTRAY,   ++i,nBound, cppu::UnoType<OUString>::get(), css::uno::Any(OUString("[From printer settings]")));
    registerPropertyNoMember(SC_UNO_PAGE_RIGHTBORDER, ++i,nBound, cppu::UnoType<table::BorderLine2>::get(), css::uno::Any(table::BorderLine2()));
    registerPropertyNoMember(SC_UNO_PAGE_RIGHTBRDDIST,++i,nBound, cppu::UnoType<sal_Int32>::get(), css::uno::makeAny<sal_Int32>(0));
    registerPropertyNoMember(PROPERTY_RIGHTMARGIN,  ++i,beans::PropertyAttribute::BOUND,::cppu::UnoType<sal_Int32>::get(), css::uno::Any(nMargin));
    registerPropertyNoMember(SC_UNO_PAGE_SCALETOPAG,  ++i,nBound, cppu::UnoType<sal_Int16>::get(), css::uno::makeAny<sal_Int16>(0));
    registerPropertyNoMember(SC_UNO_PAGE_SCALETOX,    ++i,nBound, cppu::UnoType<sal_Int16>::get(), css::uno::makeAny<sal_Int16>(0));
    registerPropertyNoMember(SC_UNO_PAGE_SCALETOY,    ++i,nBound, cppu::UnoType<sal_Int16>::get(), css::uno::makeAny<sal_Int16>(0));
    registerPropertyNoMember(SC_UNO_PAGE_SHADOWFORM,  ++i,nBound, cppu::UnoType<table::ShadowFormat>::get(), css::uno::Any(table::ShadowFormat()));
    registerProperty(PROPERTY_PAPERSIZE,                    ++i,beans::PropertyAttribute::BOUND,&m_aSize, cppu::UnoType<awt::Size>::get() );
    registerPropertyNoMember(SC_UNO_PAGE_TOPBORDER,   ++i,nBound, cppu::UnoType<table::BorderLine2>::get(), css::uno::Any(table::BorderLine2()));
    registerPropertyNoMember(SC_UNO_PAGE_TOPBRDDIST,  ++i,nBound,::cppu::UnoType<sal_Int32>::get(), css::uno::makeAny<sal_Int32>(0));
    registerPropertyNoMember(PROPERTY_TOPMARGIN,    ++i,nBound,::cppu::UnoType<sal_Int32>::get(), css::uno::Any(nMargin));
    registerPropertyNoMember("UserDefinedAttributes",     ++i,nBound, cppu::UnoType<container::XNameContainer>::get(), css::uno::Any(comphelper::NameContainer_createInstance(cppu::UnoType<xml::AttributeData>::get())));
    registerProperty(PROPERTY_WIDTH,        ++i,nBound,&m_aSize.Width, cppu::UnoType<sal_Int32>::get() );
    registerPropertyNoMember("PrinterName",               ++i,nBound, cppu::UnoType<OUString>::get(), css::uno::Any(OUString()));
    registerPropertyNoMember("PrinterSetup",              ++i,nBound,cppu::UnoType<uno::Sequence<sal_Int8>>::get(), css::uno::Any(uno::Sequence<sal_Int8>()));


}

IMPLEMENT_FORWARD_XINTERFACE2(OStyle,TStyleBASE,OStyle_PBASE)

uno::Reference< beans::XPropertySetInfo>  SAL_CALL OStyle::getPropertySetInfo()
{
    return createPropertySetInfo( getInfoHelper() );
}

void OStyle::getPropertyDefaultByHandle( sal_Int32 /*_nHandle*/, uno::Any& /*_rDefault*/ ) const
{
}

::cppu::IPropertyArrayHelper& OStyle::getInfoHelper()
{
    return *getArrayHelper();
}

::cppu::IPropertyArrayHelper* OStyle::createArrayHelper( ) const
{
    uno::Sequence< beans::Property > aProps;
    describeProperties(aProps);
    return new ::cppu::OPropertyArrayHelper(aProps);
}

// XStyle
sal_Bool SAL_CALL OStyle::isUserDefined(  )
{
    return false;
}

sal_Bool SAL_CALL OStyle::isInUse(  )
{
    return true;
}

OUString SAL_CALL OStyle::getParentStyle(  )
{
    return OUString();
}

void SAL_CALL OStyle::setParentStyle( const OUString& /*aParentStyle*/ )
{
}

// XNamed
OUString SAL_CALL OStyle::getName(  )
{
    OUString sName;
    getPropertyValue(PROPERTY_NAME) >>= sName;
    return sName;
}

void SAL_CALL OStyle::setName( const OUString& aName )
{
    setPropertyValue(PROPERTY_NAME,uno::makeAny(aName));
}

void SAL_CALL OStyle::setAllPropertiesToDefault(  )
{
}

void SAL_CALL OStyle::setPropertiesToDefault( const uno::Sequence< OUString >& aPropertyNames )
{
    const OUString* pIter = aPropertyNames.getConstArray();
    const OUString* pEnd   = pIter + aPropertyNames.getLength();
    for(;pIter != pEnd;++pIter)
        setPropertyToDefault(*pIter);
}

uno::Sequence< uno::Any > SAL_CALL OStyle::getPropertyDefaults( const uno::Sequence< OUString >& aPropertyNames )
{
    uno::Sequence< uno::Any > aRet(aPropertyNames.getLength());
    const OUString* pIter = aPropertyNames.getConstArray();
    const OUString* pEnd   = pIter + aPropertyNames.getLength();
    for(sal_Int32 i = 0;pIter != pEnd;++pIter,++i)
        aRet[i] = getPropertyDefault(*pIter);
    return aRet;
}

struct OReportDefinitionImpl
{
    uno::WeakReference< uno::XInterface >                   m_xParent;
    ::comphelper::OInterfaceContainerHelper2                      m_aStorageChangeListeners;
    ::comphelper::OInterfaceContainerHelper2                      m_aCloseListener;
    ::comphelper::OInterfaceContainerHelper2                      m_aModifyListeners;
    ::comphelper::OInterfaceContainerHelper2                      m_aLegacyEventListeners;
    ::comphelper::OInterfaceContainerHelper2                      m_aDocEventListeners;
    ::std::vector< uno::Reference< frame::XController> >    m_aControllers;
    uno::Sequence< beans::PropertyValue >                   m_aArgs;

    uno::Reference< report::XGroups >                       m_xGroups;
    uno::Reference< report::XSection>                       m_xReportHeader;
    uno::Reference< report::XSection>                       m_xReportFooter;
    uno::Reference< report::XSection>                       m_xPageHeader;
    uno::Reference< report::XSection>                       m_xPageFooter;
    uno::Reference< report::XSection>                       m_xDetail;
    uno::Reference< embed::XStorage >                       m_xStorage;
    uno::Reference< frame::XController >                    m_xCurrentController;
    uno::Reference< container::XIndexAccess >               m_xViewData;
    uno::Reference< container::XNameAccess >                m_xStyles;
    uno::Reference< container::XNameAccess>                 m_xXMLNamespaceMap;
    uno::Reference< container::XNameAccess>                 m_xGradientTable;
    uno::Reference< container::XNameAccess>                 m_xHatchTable;
    uno::Reference< container::XNameAccess>                 m_xBitmapTable;
    uno::Reference< container::XNameAccess>                 m_xTransparencyGradientTable;
    uno::Reference< container::XNameAccess>                 m_xDashTable;
    uno::Reference< container::XNameAccess>                 m_xMarkerTable;
    uno::Reference< report::XFunctions >                    m_xFunctions;
    uno::Reference< ui::XUIConfigurationManager2>           m_xUIConfigurationManager;
    uno::Reference< util::XNumberFormatsSupplier>           m_xNumberFormatsSupplier;
    uno::Reference< sdbc::XConnection>                      m_xActiveConnection;
    uno::Reference< frame::XTitle >                         m_xTitleHelper;
    uno::Reference< frame::XUntitledNumbers >               m_xNumberedControllers;
    uno::Reference< document::XDocumentProperties >         m_xDocumentProperties;

    std::shared_ptr< ::comphelper::EmbeddedObjectContainer>
                                                            m_pObjectContainer;
    std::shared_ptr<rptui::OReportModel>                m_pReportModel;
    ::rtl::Reference< ::dbaui::UndoManager >                m_pUndoManager;
    OUString                                         m_sCaption;
    OUString                                         m_sCommand;
    OUString                                         m_sFilter;
    OUString                                         m_sMimeType;
    OUString                                         m_sIdentifier;
    OUString                                         m_sDataSourceName;
    awt::Size                                               m_aVisualAreaSize;
    ::sal_Int64                                             m_nAspect;
    ::sal_Int16                                             m_nGroupKeepTogether;
    ::sal_Int16                                             m_nPageHeaderOption;
    ::sal_Int16                                             m_nPageFooterOption;
    ::sal_Int32                                             m_nCommandType;
    bool                                                    m_bControllersLocked;
    bool                                                    m_bModified;
    bool                                                    m_bEscapeProcessing;
    bool                                                    m_bSetModifiedEnabled;

    explicit OReportDefinitionImpl(::osl::Mutex& _aMutex)
    :m_aStorageChangeListeners(_aMutex)
    ,m_aCloseListener(_aMutex)
    ,m_aModifyListeners(_aMutex)
    ,m_aLegacyEventListeners(_aMutex)
    ,m_aDocEventListeners(_aMutex)
    ,m_sMimeType(MIMETYPE_OASIS_OPENDOCUMENT_TEXT_ASCII)
    ,m_sIdentifier(SERVICE_REPORTDEFINITION)
    // default visual area is 8 x 7 cm
    ,m_aVisualAreaSize( 8000, 7000 )
    ,m_nAspect(embed::Aspects::MSOLE_CONTENT)
    ,m_nGroupKeepTogether(0)
    ,m_nPageHeaderOption(0)
    ,m_nPageFooterOption(0)
    ,m_nCommandType(sdb::CommandType::TABLE)
    ,m_bControllersLocked(false)
    ,m_bModified(false)
    ,m_bEscapeProcessing(true)
    ,m_bSetModifiedEnabled( true )
    {}
};

OReportDefinition::OReportDefinition(uno::Reference< uno::XComponentContext > const & _xContext)
: ReportDefinitionBase(m_aMutex)
,ReportDefinitionPropertySet(_xContext,static_cast< Implements >(IMPLEMENTS_PROPERTY_SET),uno::Sequence< OUString >())
,m_aProps(new OReportComponentProperties(_xContext))
,m_pImpl(new OReportDefinitionImpl(m_aMutex))
{
    m_aProps->m_sName  = RPT_RESSTRING(RID_STR_REPORT);
    osl_atomic_increment(&m_refCount);
    {
        init();
        m_pImpl->m_xGroups = new OGroups(this,m_aProps->m_xContext);
        m_pImpl->m_xDetail = OSection::createOSection(this,m_aProps->m_xContext);
        m_pImpl->m_xDetail->setName(RPT_RESSTRING(RID_STR_DETAIL));
    }
    osl_atomic_decrement( &m_refCount );
}

OReportDefinition::OReportDefinition(uno::Reference< uno::XComponentContext > const & _xContext
                                     ,const uno::Reference< lang::XMultiServiceFactory>& _xFactory
                                     ,uno::Reference< drawing::XShape >& _xShape)
: ReportDefinitionBase(m_aMutex)
,ReportDefinitionPropertySet(_xContext,static_cast< Implements >(IMPLEMENTS_PROPERTY_SET),uno::Sequence< OUString >())
,m_aProps(new OReportComponentProperties(_xContext))
,m_pImpl(new OReportDefinitionImpl(m_aMutex))
{
    m_aProps->m_sName  = RPT_RESSTRING(RID_STR_REPORT);
    m_aProps->m_xFactory = _xFactory;
    osl_atomic_increment(&m_refCount);
    {
        m_aProps->setShape(_xShape,this,m_refCount);
        init();
        m_pImpl->m_xGroups = new OGroups(this,m_aProps->m_xContext);
        m_pImpl->m_xDetail = OSection::createOSection(this,m_aProps->m_xContext);
        m_pImpl->m_xDetail->setName(RPT_RESSTRING(RID_STR_DETAIL));
    }
    osl_atomic_decrement( &m_refCount );
}

OReportDefinition::~OReportDefinition()
{
    if ( !ReportDefinitionBase::rBHelper.bInDispose && !ReportDefinitionBase::rBHelper.bDisposed )
    {
        acquire();
        dispose();
    }
}

IMPLEMENT_FORWARD_REFCOUNT( OReportDefinition, ReportDefinitionBase )
void OReportDefinition::init()
{
    try
    {
        m_pImpl->m_pReportModel.reset(new OReportModel(this));
        m_pImpl->m_pReportModel->GetItemPool().FreezeIdRanges();
        m_pImpl->m_pReportModel->SetScaleUnit( MapUnit::Map100thMM );
        SdrLayerAdmin& rAdmin = m_pImpl->m_pReportModel->GetLayerAdmin();
        rAdmin.NewStandardLayer(RPT_LAYER_FRONT);
        rAdmin.NewLayer("back", RPT_LAYER_BACK);
        rAdmin.NewLayer("HiddenLayer", RPT_LAYER_HIDDEN);

        m_pImpl->m_pUndoManager = new ::dbaui::UndoManager( *this, m_aMutex );
        m_pImpl->m_pReportModel->SetSdrUndoManager( &m_pImpl->m_pUndoManager->GetSfxUndoManager() );

        m_pImpl->m_xFunctions = new OFunctions(this,m_aProps->m_xContext);
        if ( !m_pImpl->m_xStorage.is() )
            m_pImpl->m_xStorage = ::comphelper::OStorageHelper::GetTemporaryStorage();

        uno::Reference<beans::XPropertySet> xStorProps(m_pImpl->m_xStorage,uno::UNO_QUERY);
        if ( xStorProps.is())
        {
            OUString sMediaType;
            xStorProps->getPropertyValue("MediaType") >>= sMediaType;
            if ( sMediaType.isEmpty() )
                xStorProps->setPropertyValue("MediaType",uno::makeAny<OUString>(MIMETYPE_OASIS_OPENDOCUMENT_REPORT_ASCII));
        }
        m_pImpl->m_pObjectContainer.reset( new comphelper::EmbeddedObjectContainer(m_pImpl->m_xStorage , static_cast<cppu::OWeakObject*>(this) ) );
    }
    catch (const uno::Exception&)
    {
        DBG_UNHANDLED_EXCEPTION();
    }
}

void SAL_CALL OReportDefinition::dispose()
{
    ReportDefinitionPropertySet::dispose();
    cppu::WeakComponentImplHelperBase::dispose();
}

void SAL_CALL OReportDefinition::disposing()
{
    notifyEvent("OnUnload");

    uno::Reference< frame::XModel > xHoldAlive( this );

    lang::EventObject aDisposeEvent( static_cast< ::cppu::OWeakObject* >( this ) );
    m_pImpl->m_aModifyListeners.disposeAndClear( aDisposeEvent );
    m_pImpl->m_aCloseListener.disposeAndClear( aDisposeEvent );
    m_pImpl->m_aLegacyEventListeners.disposeAndClear( aDisposeEvent );
    m_pImpl->m_aDocEventListeners.disposeAndClear( aDisposeEvent );
    m_pImpl->m_aStorageChangeListeners.disposeAndClear( aDisposeEvent );

    // SYNCHRONIZED --->
    {
    SolarMutexGuard aSolarGuard;
    ::osl::ResettableMutexGuard aGuard(m_aMutex);

    m_pImpl->m_aControllers.clear();

    ::comphelper::disposeComponent(m_pImpl->m_xGroups);
    m_pImpl->m_xReportHeader.clear();
    m_pImpl->m_xReportFooter.clear();
    m_pImpl->m_xPageHeader.clear();
    m_pImpl->m_xPageFooter.clear();
    m_pImpl->m_xDetail.clear();
    ::comphelper::disposeComponent(m_pImpl->m_xFunctions);

    //::comphelper::disposeComponent(m_pImpl->m_xStorage);
        // don't dispose, this currently is the task of either the ref count going to
        // 0, or of the embedded object (if we're embedded, which is the only possible
        // case so far)
        // #i78366#
    m_pImpl->m_xStorage.clear();
    m_pImpl->m_xViewData.clear();
    m_pImpl->m_xCurrentController.clear();
    m_pImpl->m_xNumberFormatsSupplier.clear();
    m_pImpl->m_xStyles.clear();
    m_pImpl->m_xXMLNamespaceMap.clear();
    m_pImpl->m_xGradientTable.clear();
    m_pImpl->m_xHatchTable.clear();
    m_pImpl->m_xBitmapTable.clear();
    m_pImpl->m_xTransparencyGradientTable.clear();
    m_pImpl->m_xDashTable.clear();
    m_pImpl->m_xMarkerTable.clear();
    m_pImpl->m_xUIConfigurationManager.clear();
    m_pImpl->m_pReportModel.reset();
    m_pImpl->m_pObjectContainer.reset();
    m_pImpl->m_aArgs.realloc(0);
    m_pImpl->m_xTitleHelper.clear();
    m_pImpl->m_xNumberedControllers.clear();
    }
    // <--- SYNCHRONIZED
}


OUString OReportDefinition::getImplementationName_Static(  )
{
    return OUString("com.sun.star.comp.report.OReportDefinition");
}

OUString SAL_CALL OReportDefinition::getImplementationName(  )
{
    return getImplementationName_Static();
}

uno::Sequence< OUString > OReportDefinition::getSupportedServiceNames_Static(  )
{
    uno::Sequence< OUString > aServices { SERVICE_REPORTDEFINITION };

    return aServices;
}

uno::Sequence< OUString > SAL_CALL OReportDefinition::getSupportedServiceNames(  )
{
    // first collect the services which are supported by our aggregate
    uno::Sequence< OUString > aSupported;
    if ( m_aProps->m_xServiceInfo.is() )
        aSupported = m_aProps->m_xServiceInfo->getSupportedServiceNames();

    // append our own service, if necessary
    if ( 0 == ::comphelper::findValue( aSupported, SERVICE_REPORTDEFINITION, true ).getLength() )
    {
        sal_Int32 nLen = aSupported.getLength();
        aSupported.realloc( nLen + 1 );
        aSupported[ nLen ] = SERVICE_REPORTDEFINITION;
    }

    // outta here
    return aSupported;
}

sal_Bool SAL_CALL OReportDefinition::supportsService( const OUString& _rServiceName )
{
    return cppu::supportsService(this, _rServiceName);
}

uno::Any SAL_CALL OReportDefinition::queryInterface( const uno::Type& _rType )
{
    uno::Any aReturn = ReportDefinitionBase::queryInterface(_rType);
    if ( !aReturn.hasValue() )
        aReturn = ReportDefinitionPropertySet::queryInterface(_rType);

    return aReturn.hasValue() ? aReturn : (m_aProps->m_xProxy.is() ? m_aProps->m_xProxy->queryAggregation(_rType) : aReturn);
}
uno::Sequence< uno::Type > SAL_CALL OReportDefinition::getTypes(  )
{
    if ( m_aProps->m_xTypeProvider.is() )
        return ::comphelper::concatSequences(
            ReportDefinitionBase::getTypes(),
            m_aProps->m_xTypeProvider->getTypes()
        );
    return ReportDefinitionBase::getTypes();
}

uno::Reference< uno::XInterface > OReportDefinition::create(uno::Reference< uno::XComponentContext > const & xContext)
{
    return *(new OReportDefinition(xContext));
}

// XReportDefinition
OUString SAL_CALL OReportDefinition::getCaption()
{
    ::osl::MutexGuard aGuard(m_aMutex);
    return m_pImpl->m_sCaption;
}

void SAL_CALL OReportDefinition::setCaption( const OUString& _caption )
{
    set(PROPERTY_CAPTION,_caption,m_pImpl->m_sCaption);
}

::sal_Int16 SAL_CALL OReportDefinition::getGroupKeepTogether()
{
    ::osl::MutexGuard aGuard(m_aMutex);
    return m_pImpl->m_nGroupKeepTogether;
}

void SAL_CALL OReportDefinition::setGroupKeepTogether( ::sal_Int16 _groupkeeptogether )
{
    if ( _groupkeeptogether < report::GroupKeepTogether::PER_PAGE || _groupkeeptogether > report::GroupKeepTogether::PER_COLUMN )
        throwIllegallArgumentException("css::report::GroupKeepTogether"
                        ,*this
                        ,1);
    set(PROPERTY_GROUPKEEPTOGETHER,_groupkeeptogether,m_pImpl->m_nGroupKeepTogether);
}

::sal_Int16 SAL_CALL OReportDefinition::getPageHeaderOption()
{
    ::osl::MutexGuard aGuard(m_aMutex);
    return m_pImpl->m_nPageHeaderOption;
}

void SAL_CALL OReportDefinition::setPageHeaderOption( ::sal_Int16 _pageheaderoption )
{
    if ( _pageheaderoption < report::ReportPrintOption::ALL_PAGES || _pageheaderoption > report::ReportPrintOption::NOT_WITH_REPORT_HEADER_FOOTER )
        throwIllegallArgumentException("css::report::ReportPrintOption"
                        ,*this
                        ,1);
    set(PROPERTY_PAGEHEADEROPTION,_pageheaderoption,m_pImpl->m_nPageHeaderOption);
}

::sal_Int16 SAL_CALL OReportDefinition::getPageFooterOption()
{
    ::osl::MutexGuard aGuard(m_aMutex);
    return m_pImpl->m_nPageFooterOption;
}

void SAL_CALL OReportDefinition::setPageFooterOption( ::sal_Int16 _pagefooteroption )
{
    if ( _pagefooteroption < report::ReportPrintOption::ALL_PAGES || _pagefooteroption > report::ReportPrintOption::NOT_WITH_REPORT_HEADER_FOOTER )
        throwIllegallArgumentException("css::report::ReportPrintOption"
                        ,*this
                        ,1);
    set(PROPERTY_PAGEFOOTEROPTION,_pagefooteroption,m_pImpl->m_nPageFooterOption);
}

OUString SAL_CALL OReportDefinition::getCommand()
{
    ::osl::MutexGuard aGuard(m_aMutex);
    return m_pImpl->m_sCommand;
}

void SAL_CALL OReportDefinition::setCommand( const OUString& _command )
{
    set(PROPERTY_COMMAND,_command,m_pImpl->m_sCommand);
}

::sal_Int32 SAL_CALL OReportDefinition::getCommandType()
{
    ::osl::MutexGuard aGuard(m_aMutex);
    return m_pImpl->m_nCommandType;
}

void SAL_CALL OReportDefinition::setCommandType( ::sal_Int32 _commandtype )
{
    if ( _commandtype < sdb::CommandType::TABLE || _commandtype > sdb::CommandType::COMMAND )
        throwIllegallArgumentException("css::sdb::CommandType"
                        ,*this
                        ,1);
    set(PROPERTY_COMMANDTYPE,_commandtype,m_pImpl->m_nCommandType);
}

OUString SAL_CALL OReportDefinition::getFilter()
{
    ::osl::MutexGuard aGuard(m_aMutex);
    return m_pImpl->m_sFilter;
}

void SAL_CALL OReportDefinition::setFilter( const OUString& _filter )
{
    set(PROPERTY_FILTER,_filter,m_pImpl->m_sFilter);
}

sal_Bool SAL_CALL OReportDefinition::getEscapeProcessing()
{
    ::osl::MutexGuard aGuard(m_aMutex);
    return m_pImpl->m_bEscapeProcessing;
}

void SAL_CALL OReportDefinition::setEscapeProcessing( sal_Bool _escapeprocessing )
{
    set(PROPERTY_ESCAPEPROCESSING,_escapeprocessing,m_pImpl->m_bEscapeProcessing);
}

sal_Bool SAL_CALL OReportDefinition::getReportHeaderOn()
{
    ::osl::MutexGuard aGuard(m_aMutex);
    return m_pImpl->m_xReportHeader.is();
}

void SAL_CALL OReportDefinition::setReportHeaderOn( sal_Bool _reportheaderon )
{
    if ( bool(_reportheaderon) != m_pImpl->m_xReportHeader.is() )
    {
        setSection(PROPERTY_REPORTHEADERON,_reportheaderon,RPT_RESSTRING(RID_STR_REPORT_HEADER),m_pImpl->m_xReportHeader);
    }
}

sal_Bool SAL_CALL OReportDefinition::getReportFooterOn()
{
    ::osl::MutexGuard aGuard(m_aMutex);
    return m_pImpl->m_xReportFooter.is();
}

void SAL_CALL OReportDefinition::setReportFooterOn( sal_Bool _reportfooteron )
{
    if ( bool(_reportfooteron) != m_pImpl->m_xReportFooter.is() )
    {
        setSection(PROPERTY_REPORTFOOTERON,_reportfooteron,RPT_RESSTRING(RID_STR_REPORT_FOOTER),m_pImpl->m_xReportFooter);
    }
}

sal_Bool SAL_CALL OReportDefinition::getPageHeaderOn()
{
    ::osl::MutexGuard aGuard(m_aMutex);
    return m_pImpl->m_xPageHeader.is();
}

void SAL_CALL OReportDefinition::setPageHeaderOn( sal_Bool _pageheaderon )
{
    if ( bool(_pageheaderon) != m_pImpl->m_xPageHeader.is() )
    {
        setSection(PROPERTY_PAGEHEADERON,_pageheaderon,RPT_RESSTRING(RID_STR_PAGE_HEADER),m_pImpl->m_xPageHeader);
    }
}

sal_Bool SAL_CALL OReportDefinition::getPageFooterOn()
{
    ::osl::MutexGuard aGuard(m_aMutex);
    return m_pImpl->m_xPageFooter.is();
}

void SAL_CALL OReportDefinition::setPageFooterOn( sal_Bool _pagefooteron )
{
    if ( bool(_pagefooteron) != m_pImpl->m_xPageFooter.is() )
    {
        setSection(PROPERTY_PAGEFOOTERON,_pagefooteron,RPT_RESSTRING(RID_STR_PAGE_FOOTER),m_pImpl->m_xPageFooter);
    }
}

uno::Reference< report::XGroups > SAL_CALL OReportDefinition::getGroups()
{
    ::osl::MutexGuard aGuard(m_aMutex);
    return m_pImpl->m_xGroups;
}

uno::Reference< report::XSection > SAL_CALL OReportDefinition::getReportHeader()
{
    ::osl::MutexGuard aGuard(m_aMutex);
    if ( !m_pImpl->m_xReportHeader.is() )
        throw container::NoSuchElementException();
    return m_pImpl->m_xReportHeader;
}

uno::Reference< report::XSection > SAL_CALL OReportDefinition::getPageHeader()
{
    ::osl::MutexGuard aGuard(m_aMutex);
    if ( !m_pImpl->m_xPageHeader.is() )
        throw container::NoSuchElementException();
    return m_pImpl->m_xPageHeader;
}

uno::Reference< report::XSection > SAL_CALL OReportDefinition::getDetail()
{
    ::osl::MutexGuard aGuard(m_aMutex);
    return m_pImpl->m_xDetail;
}

uno::Reference< report::XSection > SAL_CALL OReportDefinition::getPageFooter()
{
    ::osl::MutexGuard aGuard(m_aMutex);
    if ( !m_pImpl->m_xPageFooter.is() )
        throw container::NoSuchElementException();
    return m_pImpl->m_xPageFooter;
}

uno::Reference< report::XSection > SAL_CALL OReportDefinition::getReportFooter()
{
    ::osl::MutexGuard aGuard(m_aMutex);
    if ( !m_pImpl->m_xReportFooter.is() )
        throw container::NoSuchElementException();
    return m_pImpl->m_xReportFooter;
}

uno::Reference< document::XEventBroadcaster > SAL_CALL OReportDefinition::getEventBroadcaster(  )
{
    ::connectivity::checkDisposed(ReportDefinitionBase::rBHelper.bDisposed);
    return this;
}

// XReportComponent
REPORTCOMPONENT_MASTERDETAIL(OReportDefinition,*m_aProps)
REPORTCOMPONENT_IMPL(OReportDefinition,*m_aProps)
REPORTCOMPONENT_IMPL2(OReportDefinition,*m_aProps)

uno::Reference< beans::XPropertySetInfo > SAL_CALL OReportDefinition::getPropertySetInfo(  )
{
    return ReportDefinitionPropertySet::getPropertySetInfo();
}

void SAL_CALL OReportDefinition::setPropertyValue( const OUString& aPropertyName, const uno::Any& aValue )
{
    ReportDefinitionPropertySet::setPropertyValue( aPropertyName, aValue );
}

uno::Any SAL_CALL OReportDefinition::getPropertyValue( const OUString& PropertyName )
{
    return ReportDefinitionPropertySet::getPropertyValue( PropertyName);
}

void SAL_CALL OReportDefinition::addPropertyChangeListener( const OUString& aPropertyName, const uno::Reference< beans::XPropertyChangeListener >& xListener )
{
    ReportDefinitionPropertySet::addPropertyChangeListener( aPropertyName, xListener );
}

void SAL_CALL OReportDefinition::removePropertyChangeListener( const OUString& aPropertyName, const uno::Reference< beans::XPropertyChangeListener >& aListener )
{
    ReportDefinitionPropertySet::removePropertyChangeListener( aPropertyName, aListener );
}

void SAL_CALL OReportDefinition::addVetoableChangeListener( const OUString& PropertyName, const uno::Reference< beans::XVetoableChangeListener >& aListener )
{
    ReportDefinitionPropertySet::addVetoableChangeListener( PropertyName, aListener );
}

void SAL_CALL OReportDefinition::removeVetoableChangeListener( const OUString& PropertyName, const uno::Reference< beans::XVetoableChangeListener >& aListener )
{
    ReportDefinitionPropertySet::removeVetoableChangeListener( PropertyName, aListener );
}

// XChild
uno::Reference< uno::XInterface > SAL_CALL OReportDefinition::getParent(  )
{
    ::osl::MutexGuard aGuard(m_aMutex);
    uno::Reference< container::XChild > xChild;
    comphelper::query_aggregation(m_aProps->m_xProxy,xChild);
    if ( xChild.is() )
        return xChild->getParent();
    return m_pImpl->m_xParent;
}

void SAL_CALL OReportDefinition::setParent( const uno::Reference< uno::XInterface >& Parent )
{
    ::osl::MutexGuard aGuard(m_aMutex);
    m_aProps->m_xParent = uno::Reference< container::XChild >(Parent,uno::UNO_QUERY);
    m_pImpl->m_xParent = Parent;
    uno::Reference< container::XChild > xChild;
    comphelper::query_aggregation(m_aProps->m_xProxy,xChild);
    if ( xChild.is() )
        xChild->setParent(Parent);
}

// XCloneable
uno::Reference< util::XCloneable > SAL_CALL OReportDefinition::createClone(  )
{
    OSL_FAIL("Not yet implemented correctly");
    uno::Reference< report::XReportComponent> xSource = this;
    uno::Reference< report::XReportDefinition> xSet(cloneObject(xSource,m_aProps->m_xFactory,SERVICE_REPORTDEFINITION),uno::UNO_QUERY_THROW);
    return xSet.get();
}

void OReportDefinition::setSection(  const OUString& _sProperty
                            ,bool _bOn
                            ,const OUString& _sName
                            ,uno::Reference< report::XSection>& _member)
{
    BoundListeners l;
    {
        ::osl::MutexGuard aGuard(m_aMutex);
        prepareSet(_sProperty, uno::makeAny(_member), uno::makeAny(_bOn), &l);
        lcl_createSectionIfNeeded(_bOn ,this,_member,_sProperty == PROPERTY_PAGEHEADERON || _sProperty == PROPERTY_PAGEFOOTERON);
        if ( _member.is() )
            _member->setName(_sName);
    }
    l.notify();
}

// XCloseBroadcaster
void SAL_CALL OReportDefinition::addCloseListener( const uno::Reference< util::XCloseListener >& _xListener )
{
    ::connectivity::checkDisposed(ReportDefinitionBase::rBHelper.bDisposed);
    if ( _xListener.is() )
        m_pImpl->m_aCloseListener.addInterface(_xListener);
}

void SAL_CALL OReportDefinition::removeCloseListener( const uno::Reference< util::XCloseListener >& _xListener )
{
    ::connectivity::checkDisposed(ReportDefinitionBase::rBHelper.bDisposed);
    m_pImpl->m_aCloseListener.removeInterface(_xListener);
}

// XCloseable
void SAL_CALL OReportDefinition::close(sal_Bool bDeliverOwnership)
{
    SolarMutexGuard aSolarGuard;

    ::osl::ResettableMutexGuard aGuard(m_aMutex);
    ::connectivity::checkDisposed(ReportDefinitionBase::rBHelper.bDisposed);
    // notify our container listeners
    lang::EventObject aEvt( static_cast< ::cppu::OWeakObject* >( this ) );
    aGuard.clear();
    m_pImpl->m_aCloseListener.forEach<util::XCloseListener>(
        [&aEvt, &bDeliverOwnership] (uno::Reference<util::XCloseListener> const& xListener) {
            return xListener->queryClosing(aEvt, bDeliverOwnership);
        });
    aGuard.reset();


    ::std::vector< uno::Reference< frame::XController> > aCopy = m_pImpl->m_aControllers;
    ::std::vector< uno::Reference< frame::XController> >::iterator aIter = aCopy.begin();
    ::std::vector< uno::Reference< frame::XController> >::const_iterator aEnd = aCopy.end();
    for (;aIter != aEnd ; ++aIter)
    {
        if ( aIter->is() )
        {
            try
            {
                uno::Reference< util::XCloseable> xFrame( (*aIter)->getFrame(), uno::UNO_QUERY );
                if ( xFrame.is() )
                    xFrame->close( bDeliverOwnership );
            }
            catch (const util::CloseVetoException&) { throw; }
            catch (const uno::Exception&)
            {
                OSL_FAIL( "ODatabaseDocument::impl_closeControllerFrames: caught an unexpected exception!" );
            }
        }
    }

    aGuard.clear();
    m_pImpl->m_aCloseListener.notifyEach(&util::XCloseListener::notifyClosing,aEvt);
    aGuard.reset();

    dispose();
}

// XModel
sal_Bool SAL_CALL OReportDefinition::attachResource( const OUString& /*_rURL*/, const uno::Sequence< beans::PropertyValue >& _aArguments )
{
    // LLA: we had a deadlock problem in our context, so we get the SolarMutex earlier.
    SolarMutexGuard aSolarGuard;

    ::osl::MutexGuard aGuard(m_aMutex);
    ::connectivity::checkDisposed( ReportDefinitionBase::rBHelper.bDisposed );
    utl::MediaDescriptor aDescriptor( _aArguments );

    m_pImpl->m_pUndoManager->GetSfxUndoManager().EnableUndo( false );
    try
    {
        fillArgs(aDescriptor);
        m_pImpl->m_pReportModel->SetModified(false);
    }
    catch (...)
    {
        m_pImpl->m_pUndoManager->GetSfxUndoManager().EnableUndo( true );
        throw;
    }
    m_pImpl->m_pUndoManager->GetSfxUndoManager().EnableUndo( true );
    return true;
}

void OReportDefinition::fillArgs(utl::MediaDescriptor& _aDescriptor)
{
    uno::Sequence<beans::PropertyValue> aComponentData;
    aComponentData = _aDescriptor.getUnpackedValueOrDefault("ComponentData",aComponentData);
    if ( aComponentData.getLength() && (!m_pImpl->m_xActiveConnection.is() || !m_pImpl->m_xNumberFormatsSupplier.is()) )
    {
        ::comphelper::SequenceAsHashMap aComponentDataMap( aComponentData );
        m_pImpl->m_xActiveConnection = aComponentDataMap.getUnpackedValueOrDefault("ActiveConnection",m_pImpl->m_xActiveConnection);
        m_pImpl->m_xNumberFormatsSupplier = dbtools::getNumberFormats(m_pImpl->m_xActiveConnection);
    }
    if ( !m_pImpl->m_xNumberFormatsSupplier.is() )
    {
        m_pImpl->m_xNumberFormatsSupplier.set( util::NumberFormatsSupplier::createWithDefaultLocale( m_aProps->m_xContext ) );
    }
    lcl_stripLoadArguments( _aDescriptor, m_pImpl->m_aArgs );
    OUString sCaption;
    sCaption = _aDescriptor.getUnpackedValueOrDefault("DocumentTitle",sCaption);
    setCaption(sCaption);
}

OUString SAL_CALL OReportDefinition::getURL(  )
{
    return OUString();
}

uno::Sequence< beans::PropertyValue > SAL_CALL OReportDefinition::getArgs(  )
{
    ::osl::MutexGuard aGuard(m_aMutex);
    ::connectivity::checkDisposed(ReportDefinitionBase::rBHelper.bDisposed);
    return m_pImpl->m_aArgs;
}

void SAL_CALL OReportDefinition::connectController( const uno::Reference< frame::XController >& _xController )
{
    ::osl::MutexGuard aGuard(m_aMutex);
    ::connectivity::checkDisposed(ReportDefinitionBase::rBHelper.bDisposed);
    m_pImpl->m_aControllers.push_back(_xController);
    sal_Int32 nCount;
    if ( _xController.is() && m_pImpl->m_xViewData.is() && ( nCount = m_pImpl->m_xViewData->getCount()) != 0)
    {
        _xController->restoreViewData(m_pImpl->m_xViewData->getByIndex(nCount - 1));
    }
}

void SAL_CALL OReportDefinition::disconnectController( const uno::Reference< frame::XController >& _xController )
{
    ::osl::MutexGuard aGuard(m_aMutex);
    ::connectivity::checkDisposed(ReportDefinitionBase::rBHelper.bDisposed);
    ::std::vector< uno::Reference< frame::XController> >::iterator aFind = ::std::find(m_pImpl->m_aControllers.begin(),m_pImpl->m_aControllers.end(),_xController);
    if ( aFind != m_pImpl->m_aControllers.end() )
        m_pImpl->m_aControllers.erase(aFind);
    if ( m_pImpl->m_xCurrentController == _xController )
        m_pImpl->m_xCurrentController.clear();
}

void SAL_CALL OReportDefinition::lockControllers(  )
{
    ::osl::MutexGuard aGuard(m_aMutex);
    ::connectivity::checkDisposed(ReportDefinitionBase::rBHelper.bDisposed);
    m_pImpl->m_bControllersLocked = true;
}

void SAL_CALL OReportDefinition::unlockControllers(  )
{
    ::osl::MutexGuard aGuard(m_aMutex);
    ::connectivity::checkDisposed(ReportDefinitionBase::rBHelper.bDisposed);
    m_pImpl->m_bControllersLocked = false;
}

sal_Bool SAL_CALL OReportDefinition::hasControllersLocked(  )
{
    ::osl::MutexGuard aGuard(m_aMutex);
    ::connectivity::checkDisposed(ReportDefinitionBase::rBHelper.bDisposed);
    return m_pImpl->m_bControllersLocked;
}

uno::Reference< frame::XController > SAL_CALL OReportDefinition::getCurrentController(  )
{
    ::osl::MutexGuard aGuard(m_aMutex);
    ::connectivity::checkDisposed(ReportDefinitionBase::rBHelper.bDisposed);
    return m_pImpl->m_xCurrentController;
}

void SAL_CALL OReportDefinition::setCurrentController( const uno::Reference< frame::XController >& _xController )
{
    ::osl::MutexGuard aGuard(m_aMutex);
    ::connectivity::checkDisposed(ReportDefinitionBase::rBHelper.bDisposed);
    if ( ::std::find(m_pImpl->m_aControllers.begin(),m_pImpl->m_aControllers.end(),_xController) == m_pImpl->m_aControllers.end() )
        throw container::NoSuchElementException();
    m_pImpl->m_xCurrentController = _xController;
}

uno::Reference< uno::XInterface > SAL_CALL OReportDefinition::getCurrentSelection(  )
{
    return uno::Reference< uno::XInterface >();
}

void OReportDefinition::impl_loadFromStorage_nolck_throw( const uno::Reference< embed::XStorage >& _xStorageToLoadFrom,
        const uno::Sequence< beans::PropertyValue >& _aMediaDescriptor )
{
    m_pImpl->m_xStorage = _xStorageToLoadFrom;

    utl::MediaDescriptor aDescriptor( _aMediaDescriptor );
    fillArgs(aDescriptor);
    aDescriptor.createItemIfMissing("Storage",uno::makeAny(_xStorageToLoadFrom));

    uno::Sequence< uno::Any > aDelegatorArguments(_aMediaDescriptor.getLength());
    uno::Any* pIter = aDelegatorArguments.getArray();
    uno::Any* pEnd  = pIter + aDelegatorArguments.getLength();
    for(sal_Int32 i = 0;pIter != pEnd;++pIter,++i)
    {
        *pIter <<= _aMediaDescriptor[i];
    }
    sal_Int32 nPos = aDelegatorArguments.getLength();
    aDelegatorArguments.realloc(nPos+1);
    beans::PropertyValue aPropVal;
    aPropVal.Name = "Storage";
    aPropVal.Value <<= _xStorageToLoadFrom;
    aDelegatorArguments[nPos] <<= aPropVal;

    rptui::OXUndoEnvironment& rEnv = m_pImpl->m_pReportModel->GetUndoEnv();
    rptui::OXUndoEnvironment::OUndoEnvLock aLock(rEnv);
    {
        uno::Reference< document::XFilter > xFilter(
            m_aProps->m_xContext->getServiceManager()->createInstanceWithArgumentsAndContext("com.sun.star.comp.report.OReportFilter",aDelegatorArguments,m_aProps->m_xContext),
            uno::UNO_QUERY_THROW );

        uno::Reference< document::XImporter> xImporter(xFilter,uno::UNO_QUERY_THROW);
        uno::Reference<XComponent> xComponent(static_cast<OWeakObject*>(this),uno::UNO_QUERY);
        xImporter->setTargetDocument(xComponent);

        utl::MediaDescriptor aTemp;
        aTemp << aDelegatorArguments;
        xFilter->filter(aTemp.getAsConstPropertyValueList());

        lcl_setModelReadOnly(m_pImpl->m_xStorage,m_pImpl->m_pReportModel);
        m_pImpl->m_pObjectContainer->SwitchPersistence(m_pImpl->m_xStorage);
    }
}

// XStorageBasedDocument
void SAL_CALL OReportDefinition::loadFromStorage( const uno::Reference< embed::XStorage >& _xStorageToLoadFrom
                                                 , const uno::Sequence< beans::PropertyValue >& _aMediaDescriptor )
{
    ::osl::MutexGuard aGuard(m_aMutex);
    ::connectivity::checkDisposed(ReportDefinitionBase::rBHelper.bDisposed);

    impl_loadFromStorage_nolck_throw( _xStorageToLoadFrom, _aMediaDescriptor );
}

void SAL_CALL OReportDefinition::storeToStorage( const uno::Reference< embed::XStorage >& _xStorageToSaveTo, const uno::Sequence< beans::PropertyValue >& _aMediaDescriptor )
{
    if ( !_xStorageToSaveTo.is() )
        throw lang::IllegalArgumentException(RPT_RESSTRING(RID_STR_ARGUMENT_IS_NULL),*this,1);

    SolarMutexGuard aSolarGuard;
    ::osl::MutexGuard aGuard(m_aMutex);
    ::connectivity::checkDisposed(ReportDefinitionBase::rBHelper.bDisposed);
    // create XStatusIndicator
    uno::Reference<task::XStatusIndicator> xStatusIndicator;
    uno::Sequence< uno::Any > aDelegatorArguments;
    utl::MediaDescriptor aDescriptor( _aMediaDescriptor );
    lcl_extractAndStartStatusIndicator( aDescriptor, xStatusIndicator, aDelegatorArguments );

    // properties
    uno::Sequence < beans::PropertyValue > aProps;

    // export sub streams for package, else full stream into a file
    bool bErr = false;
    OUString sErrFile;

    uno::Reference< beans::XPropertySet> xProp(_xStorageToSaveTo,uno::UNO_QUERY);
    if ( xProp.is() )
    {
        static const char sPropName[] = "MediaType";
        OUString sOldMediaType;
        xProp->getPropertyValue(sPropName) >>= sOldMediaType;
        if ( !xProp->getPropertyValue(sPropName).hasValue() || sOldMediaType.isEmpty() || MIMETYPE_OASIS_OPENDOCUMENT_REPORT_ASCII != sOldMediaType )
            xProp->setPropertyValue( sPropName, uno::makeAny<OUString>(MIMETYPE_OASIS_OPENDOCUMENT_REPORT_ASCII) );
    }

    /** property map for export info set */
    comphelper::PropertyMapEntry const aExportInfoMap[] =
    {
        { OUString("UsePrettyPrinting") , 0, cppu::UnoType<sal_Bool>::get(),          beans::PropertyAttribute::MAYBEVOID, 0 },
        { OUString("StreamName")        , 0, cppu::UnoType<OUString>::get(), beans::PropertyAttribute::MAYBEVOID, 0 },
        { OUString("StreamRelPath")     , 0, cppu::UnoType<OUString>::get(), beans::PropertyAttribute::MAYBEVOID, 0 },
        { OUString("BaseURI")           , 0, cppu::UnoType<OUString>::get(), beans::PropertyAttribute::MAYBEVOID, 0 },
        { OUString(), 0, css::uno::Type(), 0, 0 }
    };
    uno::Reference< beans::XPropertySet > xInfoSet( comphelper::GenericPropertySet_CreateInstance( new comphelper::PropertySetInfo( aExportInfoMap ) ) );

    SvtSaveOptions aSaveOpt;
    xInfoSet->setPropertyValue("UsePrettyPrinting", uno::makeAny(aSaveOpt.IsPrettyPrinting()));
    if ( aSaveOpt.IsSaveRelFSys() )
    {
        const OUString sVal( aDescriptor.getUnpackedValueOrDefault(utl::MediaDescriptor::PROP_DOCUMENTBASEURL(),OUString()) );
        xInfoSet->setPropertyValue("BaseURI", uno::makeAny(sVal));
    }
    const OUString sHierarchicalDocumentName( aDescriptor.getUnpackedValueOrDefault("HierarchicalDocumentName",OUString()) );
    xInfoSet->setPropertyValue("StreamRelPath", uno::makeAny(sHierarchicalDocumentName));


    sal_Int32 nArgsLen = aDelegatorArguments.getLength();
    aDelegatorArguments.realloc(nArgsLen+1);
    aDelegatorArguments[nArgsLen++] <<= xInfoSet;

     uno::Reference< document::XEmbeddedObjectResolver > xObjectResolver;
    uno::Reference< document::XGraphicObjectResolver >      xGrfResolver;
    SvXMLGraphicHelper* pGraphicHelper = SvXMLGraphicHelper::Create(_xStorageToSaveTo,SvXMLGraphicHelperMode::Write);
    xGrfResolver = pGraphicHelper;
    pGraphicHelper->release();
    SvXMLEmbeddedObjectHelper* pEmbeddedObjectHelper = SvXMLEmbeddedObjectHelper::Create( _xStorageToSaveTo,*this, SvXMLEmbeddedObjectHelperMode::Write );
    xObjectResolver = pEmbeddedObjectHelper;
    pEmbeddedObjectHelper->release();

    aDelegatorArguments.realloc(nArgsLen+2);
    aDelegatorArguments[nArgsLen++] <<= xGrfResolver;
    aDelegatorArguments[nArgsLen++] <<= xObjectResolver;

    uno::Reference<XComponent> xCom(static_cast<OWeakObject*>(this),uno::UNO_QUERY);
    if( !bErr )
    {
        xInfoSet->setPropertyValue("StreamName", uno::makeAny(OUString("settings.xml")));
        WriteThroughComponent(
            xCom, "settings.xml",
            "com.sun.star.comp.report.XMLSettingsExporter",
            aDelegatorArguments, aProps, _xStorageToSaveTo );
    }

    if( !bErr )
    {
        xInfoSet->setPropertyValue("StreamName", uno::makeAny(OUString("meta.xml")));
        WriteThroughComponent(
            xCom, "meta.xml",
            "com.sun.star.comp.report.XMLMetaExporter",
            aDelegatorArguments, aProps, _xStorageToSaveTo );
    }

    if( !bErr )
    {
        xInfoSet->setPropertyValue("StreamName", uno::makeAny(OUString("styles.xml")));
        WriteThroughComponent(
            xCom, "styles.xml",
            "com.sun.star.comp.report.XMLStylesExporter",
            aDelegatorArguments, aProps, _xStorageToSaveTo );
    }

    if ( !bErr )
    {
        xInfoSet->setPropertyValue("StreamName", uno::makeAny(OUString("content.xml")));
        if( !WriteThroughComponent(
                xCom, "content.xml",
                "com.sun.star.comp.report.ExportFilter",
                aDelegatorArguments, aProps, _xStorageToSaveTo ) )
        {
            bErr = true;
            sErrFile = "content.xml";
        }
    }

    uno::Any aImage;
    uno::Reference< embed::XVisualObject > xCurrentController(getCurrentController(),uno::UNO_QUERY);
    if ( xCurrentController.is() )
    {
        xCurrentController->setVisualAreaSize(m_pImpl->m_nAspect,m_pImpl->m_aVisualAreaSize);
        aImage = xCurrentController->getPreferredVisualRepresentation( m_pImpl->m_nAspect ).Data;
    }
    if ( aImage.hasValue() )
    {
        OUString sObject1("report");
        OUString sPng("image/png");

        uno::Sequence<sal_Int8> aSeq;
        aImage >>= aSeq;
        uno::Reference<io::XInputStream> xStream = new ::comphelper::SequenceInputStream( aSeq );
        m_pImpl->m_pObjectContainer->InsertGraphicStreamDirectly(xStream,sObject1,sPng);
    }

    if ( !bErr )
    {
        bool bPersist = false;
        if ( _xStorageToSaveTo == m_pImpl->m_xStorage )
            bPersist = m_pImpl->m_pObjectContainer->StoreChildren(true,false);
        else
            bPersist = m_pImpl->m_pObjectContainer->StoreAsChildren(true,true,_xStorageToSaveTo);

        if( bPersist )
            m_pImpl->m_pObjectContainer->SetPersistentEntries(m_pImpl->m_xStorage);
        try
        {
            uno::Reference<embed::XTransactedObject> xTransact(_xStorageToSaveTo,uno::UNO_QUERY);
            if ( xTransact.is() )
                xTransact->commit();
        }
        catch (const uno::Exception&)
        {
            OSL_FAIL("Exception Caught: Could not commit report storage!");
            throw io::IOException();
        }

        if ( _xStorageToSaveTo == m_pImpl->m_xStorage )
            setModified(false);
    }
    if ( xStatusIndicator.is() )
        xStatusIndicator->end();
}

void SAL_CALL OReportDefinition::switchToStorage(
        const uno::Reference< embed::XStorage >& xStorage)
{
    if (!xStorage.is())
        throw lang::IllegalArgumentException(RPT_RESSTRING(RID_STR_ARGUMENT_IS_NULL),*this,1);
    {
        ::osl::MutexGuard aGuard(m_aMutex);
        ::connectivity::checkDisposed(ReportDefinitionBase::rBHelper.bDisposed);
        m_pImpl->m_xStorage = xStorage;
        lcl_setModelReadOnly(m_pImpl->m_xStorage,m_pImpl->m_pReportModel);
        m_pImpl->m_pObjectContainer->SwitchPersistence(m_pImpl->m_xStorage);
    }
    // notify our container listeners
    m_pImpl->m_aStorageChangeListeners.forEach<document::XStorageChangeListener>(
        [this, &xStorage] (uno::Reference<document::XStorageChangeListener> const& xListener) {
            return xListener->notifyStorageChange(static_cast<OWeakObject*>(this), xStorage);
        });
}

uno::Reference< embed::XStorage > SAL_CALL OReportDefinition::getDocumentStorage(  )
{
    ::osl::MutexGuard aGuard(m_aMutex);
    ::connectivity::checkDisposed(ReportDefinitionBase::rBHelper.bDisposed);
    return m_pImpl->m_xStorage;
}

void SAL_CALL OReportDefinition::addStorageChangeListener( const uno::Reference< document::XStorageChangeListener >& xListener )
{
    ::osl::MutexGuard aGuard(m_aMutex);
    ::connectivity::checkDisposed(ReportDefinitionBase::rBHelper.bDisposed);
    if ( xListener.is() )
        m_pImpl->m_aStorageChangeListeners.addInterface(xListener);
}

void SAL_CALL OReportDefinition::removeStorageChangeListener( const uno::Reference< document::XStorageChangeListener >& xListener )
{
    ::osl::MutexGuard aGuard(m_aMutex);
    ::connectivity::checkDisposed(ReportDefinitionBase::rBHelper.bDisposed);
    m_pImpl->m_aStorageChangeListeners.removeInterface(xListener);
}

bool OReportDefinition::WriteThroughComponent(
    const uno::Reference<lang::XComponent> & xComponent,
    const sal_Char* pStreamName,
    const sal_Char* pServiceName,
    const uno::Sequence<uno::Any> & rArguments,
    const uno::Sequence<beans::PropertyValue> & rMediaDesc,
    const uno::Reference<embed::XStorage>& _xStorageToSaveTo)
{
    OSL_ENSURE( nullptr != pStreamName, "Need stream name!" );
    OSL_ENSURE( nullptr != pServiceName, "Need service name!" );
    try
    {
        // open stream
        OUString sStreamName = OUString::createFromAscii( pStreamName );
        uno::Reference<io::XStream> xStream = _xStorageToSaveTo->openStreamElement( sStreamName,embed::ElementModes::READWRITE | embed::ElementModes::TRUNCATE );
        if ( !xStream.is() )
            return false;
        uno::Reference<io::XOutputStream> xOutputStream = xStream->getOutputStream();
        OSL_ENSURE(xOutputStream.is(), "Can't create output stream in package!");
        if ( ! xOutputStream.is() )
            return false;

        uno::Reference<beans::XPropertySet> xStreamProp(xOutputStream,uno::UNO_QUERY);
        OSL_ENSURE(xStreamProp.is(),"No valid preoperty set for the output stream!");

        uno::Reference<io::XSeekable> xSeek(xStreamProp,uno::UNO_QUERY);
        if ( xSeek.is() )
        {
            xSeek->seek(0);
        }

        OUString aPropName("MediaType");
        OUString aMime("text/xml");
        xStreamProp->setPropertyValue( aPropName, uno::Any(aMime) );

        // encrypt all streams
        xStreamProp->setPropertyValue( "UseCommonStoragePasswordEncryption",
                                       uno::makeAny( true ) );

        // set buffer and create outputstream

        // write the stuff
        bool bRet = WriteThroughComponent(
            xOutputStream, xComponent,
            pServiceName, rArguments, rMediaDesc );
        // finally, commit stream.
        return bRet;
    }
    catch (const uno::Exception&)
    {
        throw;
    }
}

bool OReportDefinition::WriteThroughComponent(
    const uno::Reference<io::XOutputStream> & xOutputStream,
    const uno::Reference<lang::XComponent> & xComponent,
    const sal_Char* pServiceName,
    const uno::Sequence<uno::Any> & rArguments,
    const uno::Sequence<beans::PropertyValue> & rMediaDesc)
{
    OSL_ENSURE( xOutputStream.is(), "I really need an output stream!" );
    OSL_ENSURE( xComponent.is(), "Need component!" );
    OSL_ENSURE( nullptr != pServiceName, "Need component name!" );

    // get component
    uno::Reference< xml::sax::XWriter > xSaxWriter(
        xml::sax::Writer::create(m_aProps->m_xContext) );

    // connect XML writer to output stream
    xSaxWriter->setOutputStream( xOutputStream );

    // prepare arguments (prepend doc handler to given arguments)
    uno::Sequence<uno::Any> aArgs( 1 + rArguments.getLength() );
    aArgs[0] <<= xSaxWriter;
    for(sal_Int32 i = 0; i < rArguments.getLength(); i++)
        aArgs[i+1] = rArguments[i];

    // get filter component
    uno::Reference< document::XExporter > xExporter(
        m_aProps->m_xContext->getServiceManager()->createInstanceWithArgumentsAndContext(
            OUString::createFromAscii(pServiceName), aArgs,m_aProps->m_xContext), uno::UNO_QUERY);
    OSL_ENSURE( xExporter.is(),
            "can't instantiate export filter component" );
    if( !xExporter.is() )
        return false;

    // connect model and filter
    xExporter->setSourceDocument( xComponent );

    // filter!
    uno::Reference<document::XFilter> xFilter( xExporter, uno::UNO_QUERY );
    return xFilter->filter( rMediaDesc );
}

// XLoadable
void SAL_CALL OReportDefinition::initNew(  )
{
     setPageHeaderOn( true );
     setPageFooterOn( true );
}

void SAL_CALL OReportDefinition::load( const uno::Sequence< beans::PropertyValue >& _rArguments )
{
    ::osl::MutexGuard aGuard(m_aMutex);
    ::connectivity::checkDisposed(ReportDefinitionBase::rBHelper.bDisposed);

    // TODO: this code is pretty similar to what happens in ODatabaseModelImpl::getOrCreateRootStorage,
    //       perhaps we can share code here.

    ::comphelper::NamedValueCollection aArguments( _rArguments );

    // the source for the to-be-created storage: either an URL, or a stream
    uno::Reference< io::XInputStream > xStream;
    OUString sURL;

    if ( aArguments.has( "Stream" ) )
    {
        aArguments.get_ensureType( "Stream", xStream );
        aArguments.remove( "Stream" );
    }
    else if ( aArguments.has( "InputStream" ) )
    {
        aArguments.get_ensureType( "InputStream", xStream );
        aArguments.remove( "InputStream" );
    }

    if ( aArguments.has( "FileName" ) )
    {
        aArguments.get_ensureType( "FileName", sURL );
        aArguments.remove( "FileName" );
    }
    else if ( aArguments.has( "URL" ) )
    {
        aArguments.get_ensureType( "URL", sURL );
        aArguments.remove( "URL" );
    }

    uno::Any aStorageSource;
    if ( xStream.is() )
        aStorageSource = aStorageSource;
    else if ( !sURL.isEmpty() )
        aStorageSource <<= sURL;
    else
        throw lang::IllegalArgumentException(
            "No input source (URL or InputStream) found.",
                // TODO: resource
            *this,
            1
        );

    uno::Reference< lang::XSingleServiceFactory > xStorageFactory( embed::StorageFactory::create( m_aProps->m_xContext ) );

    // open read-write per default, unless told otherwise in the MediaDescriptor
    uno::Reference< embed::XStorage > xDocumentStorage;
    const sal_Int32 nOpenModes[2] = {
        embed::ElementModes::READWRITE,
        embed::ElementModes::READ
    };
    size_t nFirstOpenMode = 0;
    if ( aArguments.has( "ReadOnly" ) )
    {
        bool bReadOnly = false;
        aArguments.get_ensureType( "ReadOnly", bReadOnly );
        nFirstOpenMode = bReadOnly ? 1 : 0;
    }
    const size_t nLastOpenMode = SAL_N_ELEMENTS( nOpenModes ) - 1;
    for ( size_t i=nFirstOpenMode; i <= nLastOpenMode; ++i )
    {
        uno::Sequence< uno::Any > aStorageCreationArgs(2);
        aStorageCreationArgs[0] = aStorageSource;
        aStorageCreationArgs[1] <<= nOpenModes[i];

        try
        {
            xDocumentStorage.set( xStorageFactory->createInstanceWithArguments( aStorageCreationArgs ), uno::UNO_QUERY_THROW );
        }
        catch (const uno::Exception&)
        {
            if ( i == nLastOpenMode )
                throw lang::WrappedTargetException(
                    "An error occurred while creating the document storage.",
                        // TODO: resource
                    *this,
                    ::cppu::getCaughtException()
                );
        }
    }

    if ( !xDocumentStorage.is() )
    {
        throw uno::RuntimeException();
    }

    if (!aArguments.has("DocumentBaseURL") && !sURL.isEmpty())
    {
        aArguments.put("DocumentBaseURL", sURL);
    }

    impl_loadFromStorage_nolck_throw( xDocumentStorage, aArguments.getPropertyValues() );
    // TODO: do we need to take ownership of the storage? In opposite to loadFromStorage, we created the storage
    // ourself here, and perhaps this means we're also responsible for it ...?
}

// XVisualObject
void SAL_CALL OReportDefinition::setVisualAreaSize( ::sal_Int64 _nAspect, const awt::Size& _aSize )
{
    ::osl::MutexGuard aGuard(m_aMutex);
    ::connectivity::checkDisposed(ReportDefinitionBase::rBHelper.bDisposed);
        bool bChanged =
            (m_pImpl->m_aVisualAreaSize.Width != _aSize.Width ||
             m_pImpl->m_aVisualAreaSize.Height != _aSize.Height);
        m_pImpl->m_aVisualAreaSize = _aSize;
        if( bChanged )
            setModified( true );
    m_pImpl->m_nAspect = _nAspect;
}

awt::Size SAL_CALL OReportDefinition::getVisualAreaSize( ::sal_Int64 /*_nAspect*/ )
{
    ::osl::MutexGuard aGuard(m_aMutex);
    ::connectivity::checkDisposed(ReportDefinitionBase::rBHelper.bDisposed);
    return m_pImpl->m_aVisualAreaSize;
}

embed::VisualRepresentation SAL_CALL OReportDefinition::getPreferredVisualRepresentation( ::sal_Int64 /*_nAspect*/ )
{
    ::osl::MutexGuard aGuard(m_aMutex);
    ::connectivity::checkDisposed(ReportDefinitionBase::rBHelper.bDisposed);
    embed::VisualRepresentation aResult;
    OUString sImageName("report");
    OUString sMimeType;
    uno::Reference<io::XInputStream> xStream = m_pImpl->m_pObjectContainer->GetGraphicStream(sImageName,&sMimeType);
    if ( xStream.is() )
    {
        uno::Sequence<sal_Int8> aSeq;
        xStream->readBytes(aSeq,xStream->available());
        xStream->closeInput();
        aResult.Data <<= aSeq;
        aResult.Flavor.MimeType = sMimeType;
        aResult.Flavor.DataType = cppu::UnoType<decltype(aSeq)>::get();
    }

    return aResult;
}

::sal_Int32 SAL_CALL OReportDefinition::getMapUnit( ::sal_Int64 /*nAspect*/ )
{
    return embed::EmbedMapUnits::ONE_100TH_MM;
}

// XModifiable
sal_Bool SAL_CALL OReportDefinition::disableSetModified(  )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    ::connectivity::checkDisposed( ReportDefinitionBase::rBHelper.bDisposed );

    const bool bWasEnabled = m_pImpl->m_bSetModifiedEnabled;
    m_pImpl->m_bSetModifiedEnabled = false;
    return bWasEnabled;
}

sal_Bool SAL_CALL OReportDefinition::enableSetModified(  )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    ::connectivity::checkDisposed( ReportDefinitionBase::rBHelper.bDisposed );

    const bool bWasEnabled = m_pImpl->m_bSetModifiedEnabled;
    m_pImpl->m_bSetModifiedEnabled = true;
    return bWasEnabled;
}

sal_Bool SAL_CALL OReportDefinition::isSetModifiedEnabled(  )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    ::connectivity::checkDisposed( ReportDefinitionBase::rBHelper.bDisposed );

    return m_pImpl->m_bSetModifiedEnabled;
}

// XModifiable
sal_Bool SAL_CALL OReportDefinition::isModified(  )
{
    ::osl::MutexGuard aGuard(m_aMutex);
    ::connectivity::checkDisposed(ReportDefinitionBase::rBHelper.bDisposed);
    return m_pImpl->m_bModified;
}

void SAL_CALL OReportDefinition::setModified( sal_Bool _bModified )
{
    ::osl::ResettableMutexGuard aGuard(m_aMutex);
    ::connectivity::checkDisposed(ReportDefinitionBase::rBHelper.bDisposed);

    if ( !m_pImpl->m_bSetModifiedEnabled )
        return;

    if ( m_pImpl->m_pReportModel->IsReadOnly() && _bModified )
        throw beans::PropertyVetoException();
    if ( m_pImpl->m_bModified != bool(_bModified) )
    {
        m_pImpl->m_bModified = _bModified;
        if ( m_pImpl->m_pReportModel->IsChanged() != bool(_bModified) )
            m_pImpl->m_pReportModel->SetChanged(_bModified);

        lang::EventObject aEvent(*this);
        aGuard.clear();
        m_pImpl->m_aModifyListeners.notifyEach(&util::XModifyListener::modified,aEvent);
        notifyEvent("OnModifyChanged");
    }
}

// XModifyBroadcaster
void SAL_CALL OReportDefinition::addModifyListener( const uno::Reference< util::XModifyListener >& _xListener )
{
    ::osl::MutexGuard aGuard(m_aMutex);
    ::connectivity::checkDisposed(ReportDefinitionBase::rBHelper.bDisposed);
    if ( _xListener.is() )
        m_pImpl->m_aModifyListeners.addInterface(_xListener);
}

void SAL_CALL OReportDefinition::removeModifyListener( const uno::Reference< util::XModifyListener >& _xListener )
{
    ::osl::MutexGuard aGuard(m_aMutex);
    ::connectivity::checkDisposed(ReportDefinitionBase::rBHelper.bDisposed);
    m_pImpl->m_aModifyListeners.removeInterface(_xListener);
}

void OReportDefinition::notifyEvent(const OUString& _sEventName)
{
    try
    {
        ::osl::ResettableMutexGuard aGuard(m_aMutex);
        ::connectivity::checkDisposed(ReportDefinitionBase::rBHelper.bDisposed);
        document::EventObject aEvt(*this, _sEventName);
        aGuard.clear();
        m_pImpl->m_aLegacyEventListeners.notifyEach(&document::XEventListener::notifyEvent,aEvt);
    }
    catch (const uno::Exception&)
    {
    }

    notifyDocumentEvent(_sEventName, nullptr, css::uno::Any());
}

// document::XDocumentEventBroadcaster
void SAL_CALL OReportDefinition::notifyDocumentEvent( const OUString& rEventName, const uno::Reference< frame::XController2 >& rViewController, const uno::Any& rSupplement )
{
    try
    {
        ::osl::ResettableMutexGuard aGuard(m_aMutex);
        ::connectivity::checkDisposed(ReportDefinitionBase::rBHelper.bDisposed);
        document::DocumentEvent aEvt(*this, rEventName, rViewController, rSupplement);
        aGuard.clear();
        m_pImpl->m_aDocEventListeners.notifyEach(&document::XDocumentEventListener::documentEventOccured,aEvt);
    }
    catch (const uno::Exception&)
    {
    }
}

void SAL_CALL OReportDefinition::addDocumentEventListener( const uno::Reference< document::XDocumentEventListener >& rListener )
{
    ::osl::MutexGuard aGuard(m_aMutex);
    ::connectivity::checkDisposed(ReportDefinitionBase::rBHelper.bDisposed);
    if ( rListener.is() )
        m_pImpl->m_aDocEventListeners.addInterface(rListener);
}

void SAL_CALL OReportDefinition::removeDocumentEventListener( const uno::Reference< document::XDocumentEventListener >& rListener )
{
    ::osl::MutexGuard aGuard(m_aMutex);
    ::connectivity::checkDisposed(ReportDefinitionBase::rBHelper.bDisposed);
    m_pImpl->m_aDocEventListeners.removeInterface(rListener);
}

// document::XEventBroadcaster
void SAL_CALL OReportDefinition::addEventListener(const uno::Reference< document::XEventListener >& _xListener )
{
    ::osl::MutexGuard aGuard(m_aMutex);
    ::connectivity::checkDisposed(ReportDefinitionBase::rBHelper.bDisposed);
    if ( _xListener.is() )
        m_pImpl->m_aLegacyEventListeners.addInterface(_xListener);
}

void SAL_CALL OReportDefinition::removeEventListener( const uno::Reference< document::XEventListener >& _xListener )
{
    ::osl::MutexGuard aGuard(m_aMutex);
    ::connectivity::checkDisposed(ReportDefinitionBase::rBHelper.bDisposed);
    m_pImpl->m_aLegacyEventListeners.removeInterface(_xListener);
}

// document::XViewDataSupplier
uno::Reference< container::XIndexAccess > SAL_CALL OReportDefinition::getViewData(  )
{
    ::osl::MutexGuard aGuard(m_aMutex);
    ::connectivity::checkDisposed(ReportDefinitionBase::rBHelper.bDisposed);
    if ( !m_pImpl->m_xViewData.is() )
    {
        m_pImpl->m_xViewData.set( document::IndexedPropertyValues::create(m_aProps->m_xContext), uno::UNO_QUERY);
        uno::Reference< container::XIndexContainer > xContainer(m_pImpl->m_xViewData,uno::UNO_QUERY);
        ::std::vector< uno::Reference< frame::XController> >::const_iterator aIter = m_pImpl->m_aControllers.begin();
        ::std::vector< uno::Reference< frame::XController> >::const_iterator aEnd = m_pImpl->m_aControllers.end();
        for (;aIter != aEnd ; ++aIter)
        {
            if ( aIter->is() )
            {
                try
                {
                    xContainer->insertByIndex(xContainer->getCount(),(*aIter)->getViewData());
                }
                catch (const uno::Exception&)
                {
                }
            }
        }

    }
    return m_pImpl->m_xViewData;
}

void SAL_CALL OReportDefinition::setViewData( const uno::Reference< container::XIndexAccess >& Data )
{
    ::osl::MutexGuard aGuard(m_aMutex);
    ::connectivity::checkDisposed(ReportDefinitionBase::rBHelper.bDisposed);
    m_pImpl->m_xViewData = Data;
}

uno::Reference< report::XFunctions > SAL_CALL OReportDefinition::getFunctions()
{
    ::osl::MutexGuard aGuard(m_aMutex);
    ::connectivity::checkDisposed(ReportDefinitionBase::rBHelper.bDisposed);
    return m_pImpl->m_xFunctions;
}

uno::Reference< ui::XUIConfigurationManager > SAL_CALL OReportDefinition::getUIConfigurationManager(  )
{
    return uno::Reference< ui::XUIConfigurationManager >( getUIConfigurationManager2(), uno::UNO_QUERY_THROW );
}

uno::Reference< ui::XUIConfigurationManager2 > OReportDefinition::getUIConfigurationManager2(  )
{
    ::osl::MutexGuard aGuard(m_aMutex);
    ::connectivity::checkDisposed(ReportDefinitionBase::rBHelper.bDisposed);

    if ( !m_pImpl->m_xUIConfigurationManager.is() )
    {
        m_pImpl->m_xUIConfigurationManager = ui::UIConfigurationManager::create(m_aProps->m_xContext);

        uno::Reference< embed::XStorage > xConfigStorage;
        // initialize ui configuration manager with document substorage
        m_pImpl->m_xUIConfigurationManager->setStorage( xConfigStorage );
    }

    return m_pImpl->m_xUIConfigurationManager;
}

uno::Reference< embed::XStorage > SAL_CALL OReportDefinition::getDocumentSubStorage( const OUString& aStorageName, sal_Int32 nMode )
{
    ::osl::MutexGuard aGuard(m_aMutex);
    ::connectivity::checkDisposed(ReportDefinitionBase::rBHelper.bDisposed);
    return m_pImpl->m_xStorage->openStorageElement(aStorageName, nMode);
}

uno::Sequence< OUString > SAL_CALL OReportDefinition::getDocumentSubStoragesNames(  )
{
    ::osl::MutexGuard aGuard(m_aMutex);
    ::connectivity::checkDisposed(ReportDefinitionBase::rBHelper.bDisposed);
    uno::Reference<container::XNameAccess> xNameAccess(m_pImpl->m_xStorage,uno::UNO_QUERY);
    return xNameAccess.is() ? xNameAccess->getElementNames() : uno::Sequence< OUString >();
}

OUString SAL_CALL OReportDefinition::getMimeType()
{
    ::osl::MutexGuard aGuard(m_aMutex);
    ::connectivity::checkDisposed(ReportDefinitionBase::rBHelper.bDisposed);
    return m_pImpl->m_sMimeType;
}

void SAL_CALL OReportDefinition::setMimeType( const OUString& _mimetype )
{
    ::osl::MutexGuard aGuard(m_aMutex);
    ::connectivity::checkDisposed(ReportDefinitionBase::rBHelper.bDisposed);
    uno::Sequence< OUString > aList = getAvailableMimeTypes();
    const OUString* pEnd = aList.getConstArray()+aList.getLength();
    if ( ::std::find(aList.getConstArray(),pEnd,_mimetype) == pEnd )
        throwIllegallArgumentException("getAvailableMimeTypes()"
                        ,*this
                        ,1);
    set(PROPERTY_MIMETYPE,_mimetype,m_pImpl->m_sMimeType);
}

uno::Sequence< OUString > SAL_CALL OReportDefinition::getAvailableMimeTypes(  )
{
    uno::Sequence< OUString > s_aList(2);
    s_aList[0] = MIMETYPE_OASIS_OPENDOCUMENT_TEXT_ASCII;
    s_aList[1] = MIMETYPE_OASIS_OPENDOCUMENT_SPREADSHEET_ASCII;
    return s_aList;
}

// css::XUnoTunnel
sal_Int64 SAL_CALL OReportDefinition::getSomething( const uno::Sequence< sal_Int8 >& rId )
{
    sal_Int64 nRet = 0;
    if (rId.getLength() == 16 && 0 == memcmp(getUnoTunnelImplementationId().getConstArray(),  rId.getConstArray(), 16 ) )
        nRet = reinterpret_cast<sal_Int64>(this);
    else
    {
        uno::Reference< lang::XUnoTunnel> xUnoTunnel(m_pImpl->m_xNumberFormatsSupplier,uno::UNO_QUERY);
        if ( xUnoTunnel.is() )
            nRet = xUnoTunnel->getSomething(rId);
    }
    if ( !nRet )
    {
        uno::Reference< lang::XUnoTunnel> xTunnel;
        ::comphelper::query_aggregation(m_aProps->m_xProxy,xTunnel);
        if ( xTunnel.is() )
            nRet = xTunnel->getSomething(rId);
    }

    return nRet;
}

uno::Sequence< sal_Int8 > SAL_CALL OReportDefinition::getImplementationId(  )
{
    return css::uno::Sequence<sal_Int8>();
}

uno::Sequence< sal_Int8 > OReportDefinition::getUnoTunnelImplementationId()
{
    static ::cppu::OImplementationId * pId = nullptr;
    if (! pId)
    {
        ::osl::MutexGuard aGuard( ::osl::Mutex::getGlobalMutex() );
        if (! pId)
        {
            static ::cppu::OImplementationId aId;
            pId = &aId;
        }
    }
    return pId->getImplementationId();
}

uno::Reference< uno::XComponentContext > OReportDefinition::getContext()
{
    ::osl::MutexGuard aGuard(m_aMutex);
    ::connectivity::checkDisposed(ReportDefinitionBase::rBHelper.bDisposed);
    return m_aProps->m_xContext;
}

std::shared_ptr<rptui::OReportModel> OReportDefinition::getSdrModel(const uno::Reference< report::XReportDefinition >& _xReportDefinition)
{
    std::shared_ptr<rptui::OReportModel> pReportModel;
    uno::Reference< lang::XUnoTunnel > xUT( _xReportDefinition, uno::UNO_QUERY );
    if( xUT.is() )
        pReportModel = reinterpret_cast<OReportDefinition*>(
                           sal::static_int_cast<sal_uIntPtr>(
                               xUT->getSomething( OReportDefinition::getUnoTunnelImplementationId()))
                        )->m_pImpl->m_pReportModel;
    return pReportModel;
}

uno::Reference< uno::XInterface > SAL_CALL OReportDefinition::createInstanceWithArguments( const OUString& aServiceSpecifier, const uno::Sequence< uno::Any >& _aArgs)
{
    ::osl::MutexGuard aGuard(m_aMutex);
    ::connectivity::checkDisposed(ReportDefinitionBase::rBHelper.bDisposed);

    uno::Reference< uno::XInterface > xRet;
    if ( aServiceSpecifier.startsWith( "com.sun.star.document.ImportEmbeddedObjectResolver") )
    {
        uno::Reference< embed::XStorage > xStorage;
        const uno::Any* pIter = _aArgs.getConstArray();
        const uno::Any* pEnd  = pIter + _aArgs.getLength();
        for(;pIter != pEnd ;++pIter)
        {
            beans::NamedValue aValue;
            *pIter >>= aValue;
            if ( aValue.Name == "Storage" )
                aValue.Value >>= xStorage;
        }
        m_pImpl->m_pObjectContainer->SwitchPersistence(xStorage);
        xRet = static_cast< ::cppu::OWeakObject* >(SvXMLEmbeddedObjectHelper::Create( xStorage,*this, SvXMLEmbeddedObjectHelperMode::Read ));
    }
    return xRet;
}

uno::Reference< uno::XInterface > SAL_CALL OReportDefinition::createInstance( const OUString& aServiceSpecifier )
{
    ::osl::MutexGuard aGuard(m_aMutex);
    ::connectivity::checkDisposed(ReportDefinitionBase::rBHelper.bDisposed);
    uno::Reference< drawing::XShape > xShape;
    if ( aServiceSpecifier.startsWith( "com.sun.star.report." ) )
    {
        if ( aServiceSpecifier == SERVICE_SHAPE )
            xShape.set(SvxUnoDrawMSFactory::createInstance("com.sun.star.drawing.CustomShape"),uno::UNO_QUERY_THROW);
        else if (   aServiceSpecifier == SERVICE_FORMATTEDFIELD
            ||      aServiceSpecifier == SERVICE_FIXEDTEXT
            ||      aServiceSpecifier == SERVICE_FIXEDLINE
            ||      aServiceSpecifier == SERVICE_IMAGECONTROL )
            xShape.set(SvxUnoDrawMSFactory::createInstance("com.sun.star.drawing.ControlShape"),uno::UNO_QUERY_THROW);
        else
            xShape.set(SvxUnoDrawMSFactory::createInstance("com.sun.star.drawing.OLE2Shape"),uno::UNO_QUERY_THROW);
    }
    else if ( aServiceSpecifier.startsWith( "com.sun.star.form.component." ) )
    {
        xShape.set(m_aProps->m_xContext->getServiceManager()->createInstanceWithContext(aServiceSpecifier,m_aProps->m_xContext),uno::UNO_QUERY);
    }
    else if ( aServiceSpecifier == "com.sun.star.style.PageStyle" ||
              aServiceSpecifier == "com.sun.star.style.FrameStyle" ||
              aServiceSpecifier == "com.sun.star.style.GraphicStyle"
              )
    {
        uno::Reference< style::XStyle> xStyle = new OStyle();
        xStyle->setName("Default");
        uno::Reference<beans::XPropertySet> xProp(xStyle,uno::UNO_QUERY);
        OUString sTray;
        xProp->getPropertyValue("PrinterPaperTray")>>= sTray;

        return xStyle.get();
    }
    else if ( aServiceSpecifier == "com.sun.star.document.Settings" )
    {
        uno::Reference<beans::XPropertySet> xProp = new OStyle();

        return xProp.get();
    }
    else if ( aServiceSpecifier == "com.sun.star.drawing.Defaults" )
    {
        uno::Reference<beans::XPropertySet> xProp = new OStyle();
        return xProp.get();
    }
    else if ( aServiceSpecifier == "com.sun.star.drawing.GradientTable" )
    {
        if ( !m_pImpl->m_xGradientTable.is() )
            m_pImpl->m_xGradientTable.set(SvxUnoGradientTable_createInstance(m_pImpl->m_pReportModel.get()),uno::UNO_QUERY);
        return m_pImpl->m_xGradientTable;
    }
    else if ( aServiceSpecifier == "com.sun.star.drawing.HatchTable" )
    {
        if ( !m_pImpl->m_xHatchTable.is() )
            m_pImpl->m_xHatchTable.set(SvxUnoHatchTable_createInstance(m_pImpl->m_pReportModel.get()),uno::UNO_QUERY);
        return m_pImpl->m_xHatchTable;
    }
    else if ( aServiceSpecifier == "com.sun.star.drawing.BitmapTable"  )
    {
        if ( !m_pImpl->m_xBitmapTable.is() )
            m_pImpl->m_xBitmapTable.set(SvxUnoBitmapTable_createInstance(m_pImpl->m_pReportModel.get()),uno::UNO_QUERY);
        return m_pImpl->m_xBitmapTable;
    }
    else if ( aServiceSpecifier == "com.sun.star.drawing.TransparencyGradientTable" )
    {
        if ( !m_pImpl->m_xTransparencyGradientTable.is() )
            m_pImpl->m_xTransparencyGradientTable.set(SvxUnoTransGradientTable_createInstance(m_pImpl->m_pReportModel.get()),uno::UNO_QUERY);
        return m_pImpl->m_xTransparencyGradientTable;
    }
    else if ( aServiceSpecifier == "com.sun.star.drawing.DashTable" )
    {
        if ( !m_pImpl->m_xDashTable.is() )
            m_pImpl->m_xDashTable.set(SvxUnoDashTable_createInstance(m_pImpl->m_pReportModel.get()),uno::UNO_QUERY);
        return m_pImpl->m_xDashTable;
    }
    else if( aServiceSpecifier == "com.sun.star.drawing.MarkerTable" )
    {
        if( !m_pImpl->m_xMarkerTable.is() )
            m_pImpl->m_xMarkerTable.set(SvxUnoMarkerTable_createInstance( m_pImpl->m_pReportModel.get() ),uno::UNO_QUERY);
        return m_pImpl->m_xMarkerTable;
    }
    else if ( aServiceSpecifier == "com.sun.star.document.ImportEmbeddedObjectResolver" )
        return static_cast< ::cppu::OWeakObject* >(SvXMLEmbeddedObjectHelper::Create( m_pImpl->m_xStorage,*this, SvXMLEmbeddedObjectHelperMode::Read ));
    else if ( aServiceSpecifier == "com.sun.star.document.ExportEmbeddedObjectResolver" )
        return static_cast< ::cppu::OWeakObject* >(SvXMLEmbeddedObjectHelper::Create( m_pImpl->m_xStorage,*this, SvXMLEmbeddedObjectHelperMode::Write ));
    else if ( aServiceSpecifier == "com.sun.star.document.ImportGraphicObjectResolver" )
    {
        SvXMLGraphicHelper* pGraphicHelper = SvXMLGraphicHelper::Create(m_pImpl->m_xStorage,SvXMLGraphicHelperMode::Write);
        uno::Reference< uno::XInterface> xRet(static_cast< ::cppu::OWeakObject* >(pGraphicHelper));
        pGraphicHelper->release();
        return xRet;
    }
    else if ( aServiceSpecifier == "com.sun.star.document.ExportGraphicObjectResolver" )
    {
        SvXMLGraphicHelper* pGraphicHelper = SvXMLGraphicHelper::Create(m_pImpl->m_xStorage,SvXMLGraphicHelperMode::Write);
        uno::Reference< uno::XInterface> xRet(static_cast< ::cppu::OWeakObject* >(pGraphicHelper));
        pGraphicHelper->release();
        return xRet;
    }
    else if ( aServiceSpecifier == "com.sun.star.chart2.data.DataProvider" )
    {
        uno::Reference<chart2::data::XDatabaseDataProvider> xDataProvider(chart2::data::DatabaseDataProvider::createWithConnection( m_aProps->m_xContext, m_pImpl->m_xActiveConnection ));
        xDataProvider->setRowLimit(10);
        uno::Reference< container::XChild > xChild(xDataProvider,uno::UNO_QUERY);
        if ( xChild.is() )
            xChild->setParent(*this);
        return uno::Reference< uno::XInterface >(xDataProvider,uno::UNO_QUERY);
    }
    else if ( aServiceSpecifier == "com.sun.star.xml.NamespaceMap" )
    {
        if ( !m_pImpl->m_xXMLNamespaceMap.is() )
            m_pImpl->m_xXMLNamespaceMap = comphelper::NameContainer_createInstance( cppu::UnoType<OUString>::get() ).get();
        return m_pImpl->m_xXMLNamespaceMap;
    }
    else
        xShape.set(SvxUnoDrawMSFactory::createInstance( aServiceSpecifier ),uno::UNO_QUERY_THROW);

    return m_pImpl->m_pReportModel->createShape(aServiceSpecifier,xShape);
}

uno::Sequence< OUString > SAL_CALL OReportDefinition::getAvailableServiceNames()
{
    static const OUStringLiteral aSvxComponentServiceNameList[] =
    {
        "com.sun.star.form.component.FixedText",
        "com.sun.star.form.component.DatabaseImageControl",
        "com.sun.star.style.PageStyle",
        "com.sun.star.style.GraphicStyle",
        "com.sun.star.style.FrameStyle",
        "com.sun.star.drawing.Defaults",
        "com.sun.star.document.ImportEmbeddedObjectResolver",
        "com.sun.star.document.ExportEmbeddedObjectResolver",
        "com.sun.star.document.ImportGraphicObjectResolver",
        "com.sun.star.document.ExportGraphicObjectResolver",
        "com.sun.star.chart2.data.DataProvider",
        "com.sun.star.xml.NamespaceMap",
        "com.sun.star.document.Settings",
        "com.sun.star.drawing.GradientTable",
        "com.sun.star.drawing.HatchTable",
        "com.sun.star.drawing.BitmapTable",
        "com.sun.star.drawing.TransparencyGradientTable",
        "com.sun.star.drawing.DashTable",
        "com.sun.star.drawing.MarkerTable"
    };

    static const sal_uInt16 nSvxComponentServiceNameListCount = SAL_N_ELEMENTS(aSvxComponentServiceNameList);

    uno::Sequence< OUString > aSeq( nSvxComponentServiceNameListCount );
    OUString* pStrings = aSeq.getArray();
    for( sal_uInt16 nIdx = 0; nIdx < nSvxComponentServiceNameListCount; nIdx++ )
        pStrings[nIdx] = aSvxComponentServiceNameList[nIdx];

    uno::Sequence< OUString > aParentSeq( SvxUnoDrawMSFactory::getAvailableServiceNames() );
    return concatServiceNames( aParentSeq, aSeq );
}

// XShape
awt::Point SAL_CALL OReportDefinition::getPosition(  )
{
    ::osl::MutexGuard aGuard(m_aMutex);
    ::connectivity::checkDisposed(ReportDefinitionBase::rBHelper.bDisposed);
    if ( m_aProps->m_xShape.is() )
        return m_aProps->m_xShape->getPosition();
    return awt::Point(m_aProps->m_nPosX,m_aProps->m_nPosY);
}

void SAL_CALL OReportDefinition::setPosition( const awt::Point& aPosition )
{
    ::osl::MutexGuard aGuard(m_aMutex);
    ::connectivity::checkDisposed(ReportDefinitionBase::rBHelper.bDisposed);
    if ( m_aProps->m_xShape.is() )
        m_aProps->m_xShape->setPosition(aPosition);
    set(PROPERTY_POSITIONX,aPosition.X,m_aProps->m_nPosX);
    set(PROPERTY_POSITIONY,aPosition.Y,m_aProps->m_nPosY);
}

awt::Size SAL_CALL OReportDefinition::getSize(  )
{
    ::osl::MutexGuard aGuard(m_aMutex);
    ::connectivity::checkDisposed(ReportDefinitionBase::rBHelper.bDisposed);
    if ( m_aProps->m_xShape.is() )
        return m_aProps->m_xShape->getSize();
    return awt::Size(m_aProps->m_nWidth,m_aProps->m_nHeight);
}

void SAL_CALL OReportDefinition::setSize( const awt::Size& aSize )
{
    ::osl::MutexGuard aGuard(m_aMutex);
    ::connectivity::checkDisposed(ReportDefinitionBase::rBHelper.bDisposed);
    if ( m_aProps->m_xShape.is() )
        m_aProps->m_xShape->setSize(aSize);
    set(PROPERTY_WIDTH,aSize.Width,m_aProps->m_nWidth);
    set(PROPERTY_HEIGHT,aSize.Height,m_aProps->m_nHeight);
}


// XShapeDescriptor
OUString SAL_CALL OReportDefinition::getShapeType(  )
{
    ::osl::MutexGuard aGuard(m_aMutex);
    ::connectivity::checkDisposed(ReportDefinitionBase::rBHelper.bDisposed);
    if ( m_aProps->m_xShape.is() )
        return m_aProps->m_xShape->getShapeType();
   return OUString("com.sun.star.drawing.OLE2Shape");
}

typedef ::cppu::WeakImplHelper< container::XNameContainer,
                             container::XIndexAccess
                            > TStylesBASE;
class OStylesHelper:
    public cppu::BaseMutex, public TStylesBASE
{
    typedef ::std::map< OUString, uno::Any  , ::comphelper::UStringMixLess> TStyleElements;
    TStyleElements                                  m_aElements;
    ::std::vector<TStyleElements::iterator>         m_aElementsPos;
    uno::Type                                       m_aType;

protected:
    virtual ~OStylesHelper() override {}
public:
    explicit OStylesHelper(const uno::Type& rType = cppu::UnoType<container::XElementAccess>::get());
    OStylesHelper(const OStylesHelper&) = delete;
    OStylesHelper& operator=(const OStylesHelper&) = delete;

    // XNameContainer
    virtual void SAL_CALL insertByName( const OUString& aName, const uno::Any& aElement ) override;
    virtual void SAL_CALL removeByName( const OUString& Name ) override;

    // XNameReplace
    virtual void SAL_CALL replaceByName( const OUString& aName, const uno::Any& aElement ) override;

    // container::XElementAccess
    virtual uno::Type SAL_CALL getElementType(  ) override;
    virtual sal_Bool SAL_CALL hasElements(  ) override;
    // container::XIndexAccess
    virtual sal_Int32 SAL_CALL getCount(  ) override;
    virtual uno::Any SAL_CALL getByIndex( sal_Int32 Index ) override;

    // container::XNameAccess
    virtual uno::Any SAL_CALL getByName( const OUString& aName ) override;
    virtual uno::Sequence< OUString > SAL_CALL getElementNames(  ) override;
    virtual sal_Bool SAL_CALL hasByName( const OUString& aName ) override;
};

OStylesHelper::OStylesHelper(const uno::Type& rType)
    : cppu::BaseMutex()
    , m_aType(rType)
{
}
;

// container::XElementAccess
uno::Type SAL_CALL OStylesHelper::getElementType(  )
{
    return m_aType;
}

sal_Bool SAL_CALL OStylesHelper::hasElements(  )
{
    ::osl::MutexGuard aGuard(m_aMutex);
    return !m_aElementsPos.empty();
}

// container::XIndexAccess
sal_Int32 SAL_CALL OStylesHelper::getCount(  )
{
    ::osl::MutexGuard aGuard(m_aMutex);
    return m_aElementsPos.size();
}

uno::Any SAL_CALL OStylesHelper::getByIndex( sal_Int32 Index )
{
    ::osl::MutexGuard aGuard(m_aMutex);
    if ( Index < 0 || Index >= static_cast<sal_Int32>(m_aElementsPos.size()) )
        throw lang::IndexOutOfBoundsException();
    return m_aElementsPos[Index]->second;
}

// container::XNameAccess
uno::Any SAL_CALL OStylesHelper::getByName( const OUString& aName )
{
    ::osl::MutexGuard aGuard(m_aMutex);
    TStyleElements::const_iterator aFind = m_aElements.find(aName);
    if ( aFind == m_aElements.end() )
        throw container::NoSuchElementException();
    return aFind->second;
}

uno::Sequence< OUString > SAL_CALL OStylesHelper::getElementNames(  )
{
    ::osl::MutexGuard aGuard(m_aMutex);
    uno::Sequence< OUString > aNameList(m_aElementsPos.size());

    OUString* pStringArray = aNameList.getArray();
    ::std::vector<TStyleElements::iterator>::const_iterator aEnd = m_aElementsPos.end();
    for(::std::vector<TStyleElements::iterator>::const_iterator aIter = m_aElementsPos.begin();         aIter != aEnd;++aIter,++pStringArray)
        *pStringArray = (*aIter)->first;

    return aNameList;
}

sal_Bool SAL_CALL OStylesHelper::hasByName( const OUString& aName )
{
    ::osl::MutexGuard aGuard(m_aMutex);
    return m_aElements.find(aName) != m_aElements.end();
}

// XNameContainer
void SAL_CALL OStylesHelper::insertByName( const OUString& aName, const uno::Any& aElement )
{
    ::osl::MutexGuard aGuard(m_aMutex);
    if ( m_aElements.find(aName) != m_aElements.end() )
        throw container::ElementExistException();

    if ( !aElement.isExtractableTo(m_aType) )
        throw lang::IllegalArgumentException();

    m_aElementsPos.push_back(m_aElements.insert(TStyleElements::value_type(aName,aElement)).first);
}

void SAL_CALL OStylesHelper::removeByName( const OUString& aName )
{
    ::osl::MutexGuard aGuard(m_aMutex);
    TStyleElements::const_iterator aFind = m_aElements.find(aName);
    if ( aFind != m_aElements.end() )
        throw container::NoSuchElementException();
    m_aElementsPos.erase(::std::find(m_aElementsPos.begin(),m_aElementsPos.end(),aFind));
    m_aElements.erase(aFind);
}

// XNameReplace
void SAL_CALL OStylesHelper::replaceByName( const OUString& aName, const uno::Any& aElement )
{
    ::osl::MutexGuard aGuard(m_aMutex);
    TStyleElements::iterator aFind = m_aElements.find(aName);
    if ( aFind == m_aElements.end() )
        throw container::NoSuchElementException();
    if ( !aElement.isExtractableTo(m_aType) )
        throw lang::IllegalArgumentException();
    aFind->second = aElement;
}

uno::Reference< container::XNameAccess > SAL_CALL OReportDefinition::getStyleFamilies(  )
{
    ::osl::MutexGuard aGuard(m_aMutex);
    ::connectivity::checkDisposed(ReportDefinitionBase::rBHelper.bDisposed);
    if ( !m_pImpl->m_xStyles.is() )
    {
        m_pImpl->m_xStyles = new OStylesHelper();
        uno::Reference< container::XNameContainer> xStyles(m_pImpl->m_xStyles,uno::UNO_QUERY);

        uno::Reference< container::XNameContainer> xPageStyles = new OStylesHelper(cppu::UnoType<style::XStyle>::get());
        xStyles->insertByName("PageStyles",uno::makeAny(xPageStyles));
        uno::Reference< style::XStyle> xPageStyle(createInstance("com.sun.star.style.PageStyle"),uno::UNO_QUERY);
        xPageStyles->insertByName(xPageStyle->getName(),uno::makeAny(xPageStyle));

        uno::Reference< container::XNameContainer> xFrameStyles = new OStylesHelper(cppu::UnoType<style::XStyle>::get());
        xStyles->insertByName("FrameStyles",uno::makeAny(xFrameStyles));
        uno::Reference< style::XStyle> xFrameStyle(createInstance("com.sun.star.style.FrameStyle"),uno::UNO_QUERY);
        xFrameStyles->insertByName(xFrameStyle->getName(),uno::makeAny(xFrameStyle));

        uno::Reference< container::XNameContainer> xGraphicStyles = new OStylesHelper(cppu::UnoType<style::XStyle>::get());
        xStyles->insertByName("graphics",uno::makeAny(xGraphicStyles));
        uno::Reference< style::XStyle> xGraphicStyle(createInstance("com.sun.star.style.GraphicStyle"),uno::UNO_QUERY);
        xGraphicStyles->insertByName(xGraphicStyle->getName(),uno::makeAny(xGraphicStyle));
    }
    return m_pImpl->m_xStyles;
}
OUString SAL_CALL  OReportDefinition::getIdentifier(  )
{
    ::osl::MutexGuard aGuard(m_aMutex);
    ::connectivity::checkDisposed(ReportDefinitionBase::rBHelper.bDisposed);
    return m_pImpl->m_sIdentifier;
}

void SAL_CALL OReportDefinition::setIdentifier( const OUString& Identifier )
{
    ::osl::MutexGuard aGuard(m_aMutex);
    ::connectivity::checkDisposed(ReportDefinitionBase::rBHelper.bDisposed);
    m_pImpl->m_sIdentifier = Identifier;
}

// XNumberFormatsSupplier
uno::Reference< beans::XPropertySet > SAL_CALL OReportDefinition::getNumberFormatSettings(  )
{
    ::osl::MutexGuard aGuard(m_aMutex);
    if ( m_pImpl->m_xNumberFormatsSupplier.is() )
        return m_pImpl->m_xNumberFormatsSupplier->getNumberFormatSettings();
    return uno::Reference< beans::XPropertySet >();
}

uno::Reference< util::XNumberFormats > SAL_CALL OReportDefinition::getNumberFormats(  )
{
    ::osl::MutexGuard aGuard(m_aMutex);
    if ( m_pImpl->m_xNumberFormatsSupplier.is() )
        return m_pImpl->m_xNumberFormatsSupplier->getNumberFormats();
    return uno::Reference< util::XNumberFormats >();
}

::comphelper::EmbeddedObjectContainer& OReportDefinition::getEmbeddedObjectContainer() const
{
    return *m_pImpl->m_pObjectContainer;
}

uno::Reference< embed::XStorage > OReportDefinition::getStorage() const
{
    return m_pImpl->m_xStorage;
}

uno::Reference< task::XInteractionHandler > OReportDefinition::getInteractionHandler() const
{
    uno::Reference< task::XInteractionHandler > xRet(
        task::InteractionHandler::createWithParent(m_aProps->m_xContext, nullptr), uno::UNO_QUERY_THROW);
    return xRet;
}

uno::Reference< sdbc::XConnection > SAL_CALL OReportDefinition::getActiveConnection()
{
    ::osl::MutexGuard aGuard(m_aMutex);
    return m_pImpl->m_xActiveConnection;
}

void SAL_CALL OReportDefinition::setActiveConnection( const uno::Reference< sdbc::XConnection >& _activeconnection )
{
    if ( !_activeconnection.is() )
        throw lang::IllegalArgumentException();
    set(PROPERTY_ACTIVECONNECTION,_activeconnection,m_pImpl->m_xActiveConnection);
}

OUString SAL_CALL OReportDefinition::getDataSourceName()
{
    osl::MutexGuard g(m_aMutex);
    return m_pImpl->m_sDataSourceName;
}

void SAL_CALL OReportDefinition::setDataSourceName(const OUString& the_value)
{
    set(PROPERTY_DATASOURCENAME,the_value,m_pImpl->m_sDataSourceName);
}

bool OReportDefinition::isEnableSetModified() const
{
    return true;
}

OUString OReportDefinition::getDocumentBaseURL() const
{
    // TODO: should this be in getURL()? not sure...
    uno::Reference<frame::XModel> const xParent(
        const_cast<OReportDefinition*>(this)->getParent(), uno::UNO_QUERY);
    if (xParent.is())
    {
        return xParent->getURL();
    }

    ::osl::MutexGuard aGuard(m_aMutex);
    ::connectivity::checkDisposed(ReportDefinitionBase::rBHelper.bDisposed);
    for (beans::PropertyValue const& it : m_pImpl->m_aArgs)
    {
        if (it.Name == "DocumentBaseURL")
            return it.Value.get<OUString>();
    }

    return OUString();
}

uno::Reference< frame::XTitle > OReportDefinition::impl_getTitleHelper_throw()
{
    SolarMutexGuard aSolarGuard;

    ::osl::MutexGuard aGuard(m_aMutex);
    ::connectivity::checkDisposed(ReportDefinitionBase::rBHelper.bDisposed);

    if ( ! m_pImpl->m_xTitleHelper.is ())
    {
        uno::Reference< frame::XDesktop2 >    xDesktop = frame::Desktop::create(m_aProps->m_xContext);
        uno::Reference< frame::XModel >       xThis(static_cast< frame::XModel* >(this), uno::UNO_QUERY_THROW);

        ::framework::TitleHelper* pHelper = new ::framework::TitleHelper( m_aProps->m_xContext );
        m_pImpl->m_xTitleHelper.set(static_cast< ::cppu::OWeakObject* >(pHelper), uno::UNO_QUERY_THROW);
        pHelper->setOwner                   (xThis   );
        pHelper->connectWithUntitledNumbers (uno::Reference<frame::XUntitledNumbers>(xDesktop, uno::UNO_QUERY_THROW));
    }

    return m_pImpl->m_xTitleHelper;
}

uno::Reference< frame::XUntitledNumbers > OReportDefinition::impl_getUntitledHelper_throw()
{
    SolarMutexGuard aSolarGuard;

    ::osl::MutexGuard aGuard(m_aMutex);
    ::connectivity::checkDisposed(ReportDefinitionBase::rBHelper.bDisposed);

    if ( ! m_pImpl->m_xNumberedControllers.is ())
    {
        uno::Reference< frame::XModel >    xThis(static_cast< frame::XModel* >(this), uno::UNO_QUERY_THROW);
        ::comphelper::NumberedCollection*  pHelper = new ::comphelper::NumberedCollection();
        m_pImpl->m_xNumberedControllers.set(static_cast< ::cppu::OWeakObject* >(pHelper), uno::UNO_QUERY_THROW);

        pHelper->setOwner          (xThis);
        pHelper->setUntitledPrefix (" : ");
    }

    return m_pImpl->m_xNumberedControllers;
}

// css.frame.XTitle
OUString SAL_CALL OReportDefinition::getTitle()
{
    // SYNCHRONIZED ->
    SolarMutexGuard aSolarGuard;

    ::osl::MutexGuard aGuard(m_aMutex);
    ::connectivity::checkDisposed(ReportDefinitionBase::rBHelper.bDisposed);

    return impl_getTitleHelper_throw()->getTitle ();
}

// css.frame.XTitle
void SAL_CALL OReportDefinition::setTitle( const OUString& sTitle )
{
    // SYNCHRONIZED ->
    SolarMutexGuard aSolarGuard;

    ::osl::MutexGuard aGuard(m_aMutex);
    ::connectivity::checkDisposed(ReportDefinitionBase::rBHelper.bDisposed);

    impl_getTitleHelper_throw()->setTitle (sTitle);
}

// css.frame.XTitleChangeBroadcaster
void SAL_CALL OReportDefinition::addTitleChangeListener( const uno::Reference< frame::XTitleChangeListener >& xListener )
{
    // SYNCHRONIZED ->
    SolarMutexGuard aSolarGuard;

    ::osl::MutexGuard aGuard(m_aMutex);
    ::connectivity::checkDisposed(ReportDefinitionBase::rBHelper.bDisposed);

    uno::Reference< frame::XTitleChangeBroadcaster > xBroadcaster(impl_getTitleHelper_throw(), uno::UNO_QUERY);
    if (xBroadcaster.is ())
        xBroadcaster->addTitleChangeListener (xListener);
}

// css.frame.XTitleChangeBroadcaster
void SAL_CALL OReportDefinition::removeTitleChangeListener( const uno::Reference< frame::XTitleChangeListener >& xListener )
{
    // SYNCHRONIZED ->
    SolarMutexGuard aSolarGuard;

    ::osl::MutexGuard aGuard(m_aMutex);
    ::connectivity::checkDisposed(ReportDefinitionBase::rBHelper.bDisposed);

    uno::Reference< frame::XTitleChangeBroadcaster > xBroadcaster(impl_getTitleHelper_throw(), uno::UNO_QUERY);
    if (xBroadcaster.is ())
        xBroadcaster->removeTitleChangeListener (xListener);
}

// css.frame.XUntitledNumbers
::sal_Int32 SAL_CALL OReportDefinition::leaseNumber( const uno::Reference< uno::XInterface >& xComponent )
{
    // object already disposed?
    SolarMutexGuard aSolarGuard;
    ::osl::MutexGuard aGuard(m_aMutex);
    ::connectivity::checkDisposed(ReportDefinitionBase::rBHelper.bDisposed);

    return impl_getUntitledHelper_throw()->leaseNumber (xComponent);
}

// css.frame.XUntitledNumbers
void SAL_CALL OReportDefinition::releaseNumber( ::sal_Int32 nNumber )
{
    // object already disposed?
    SolarMutexGuard aSolarGuard;
    ::osl::MutexGuard aGuard(m_aMutex);
    ::connectivity::checkDisposed(ReportDefinitionBase::rBHelper.bDisposed);

    impl_getUntitledHelper_throw()->releaseNumber (nNumber);
}

// css.frame.XUntitledNumbers
void SAL_CALL OReportDefinition::releaseNumberForComponent( const uno::Reference< uno::XInterface >& xComponent )
{
    // object already disposed?
    SolarMutexGuard aSolarGuard;
    ::osl::MutexGuard aGuard(m_aMutex);
    ::connectivity::checkDisposed(ReportDefinitionBase::rBHelper.bDisposed);

    impl_getUntitledHelper_throw()->releaseNumberForComponent (xComponent);
}

// css.frame.XUntitledNumbers
OUString SAL_CALL OReportDefinition::getUntitledPrefix()
{
    // object already disposed?
    SolarMutexGuard aSolarGuard;
    ::osl::MutexGuard aGuard(m_aMutex);
    ::connectivity::checkDisposed(ReportDefinitionBase::rBHelper.bDisposed);

    return impl_getUntitledHelper_throw()->getUntitledPrefix ();
}

uno::Reference< document::XDocumentProperties > SAL_CALL OReportDefinition::getDocumentProperties(  )
{
    ::osl::MutexGuard aGuard(m_aMutex);
    ::connectivity::checkDisposed(ReportDefinitionBase::rBHelper.bDisposed);
    if ( !m_pImpl->m_xDocumentProperties.is() )
    {
        m_pImpl->m_xDocumentProperties.set(document::DocumentProperties::create(m_aProps->m_xContext));
    }
    return m_pImpl->m_xDocumentProperties;
}

uno::Any SAL_CALL OReportDefinition::getTransferData( const datatransfer::DataFlavor& aFlavor )
{
    uno::Any aResult;
    if( isDataFlavorSupported( aFlavor ) )
    {
        try
        {
            aResult = getPreferredVisualRepresentation(0).Data;
        }
        catch (const uno::Exception &)
        {
            DBG_UNHANDLED_EXCEPTION();
        }
    }
    else
    {
        throw datatransfer::UnsupportedFlavorException(aFlavor.MimeType, static_cast< ::cppu::OWeakObject* >( this ));
    }

    return aResult;
}

uno::Sequence< datatransfer::DataFlavor > SAL_CALL OReportDefinition::getTransferDataFlavors(  )
{
    uno::Sequence< datatransfer::DataFlavor > aRet(1);

    aRet[0] = datatransfer::DataFlavor( "image/png",
        "PNG",
        cppu::UnoType<uno::Sequence< sal_Int8 >>::get() );

    return aRet;
}

sal_Bool SAL_CALL OReportDefinition::isDataFlavorSupported( const datatransfer::DataFlavor& aFlavor )
{
    return aFlavor.MimeType == "image/png";
}


uno::Reference< document::XUndoManager > SAL_CALL OReportDefinition::getUndoManager(  )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    return m_pImpl->m_pUndoManager.get();
}

}// namespace reportdesign

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
