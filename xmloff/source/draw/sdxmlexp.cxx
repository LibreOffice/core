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

#include <xmloff/unointerfacetouniqueidentifiermapper.hxx>
#include <xmloff/nmspmap.hxx>
#include "xmloff/xmlnmspe.hxx"
#include <xmloff/xmluconv.hxx>
#include <xmloff/xmltoken.hxx>
#include <xmloff/xmlmetae.hxx>
#include <com/sun/star/lang/ServiceNotRegisteredException.hpp>
#include <com/sun/star/presentation/XPresentationSupplier.hpp>
#include <com/sun/star/presentation/XCustomPresentationSupplier.hpp>
#include <com/sun/star/geometry/RealPoint2D.hpp>
#include <com/sun/star/task/XStatusIndicatorSupplier.hpp>
#include <com/sun/star/office/XAnnotationAccess.hpp>
#include <com/sun/star/lang/Locale.hpp>
#include <com/sun/star/uno/Any.hxx>
#include "sdxmlexp_impl.hxx"
#include <com/sun/star/drawing/XDrawPagesSupplier.hpp>
#include <com/sun/star/drawing/XMasterPagesSupplier.hpp>
#include <com/sun/star/presentation/XHandoutMasterSupplier.hpp>
#include <com/sun/star/container/XIndexContainer.hpp>
#include <com/sun/star/view/PaperOrientation.hpp>
#include <com/sun/star/style/XStyleFamiliesSupplier.hpp>
#include <com/sun/star/style/XStyle.hpp>

#include <com/sun/star/form/XFormsSupplier2.hpp>
#include <com/sun/star/presentation/XPresentationPage.hpp>
#include <com/sun/star/drawing/XMasterPageTarget.hpp>
#include <com/sun/star/text/XText.hpp>
#include <com/sun/star/chart/XChartDocument.hpp>
#include <com/sun/star/animations/XAnimationNodeSupplier.hpp>
#include <com/sun/star/container/XNamed.hpp>
#include <com/sun/star/util/Duration.hpp>
#include <com/sun/star/util/MeasureUnit.hpp>
#include <rtl/ustrbuf.hxx>
#include <tools/gen.hxx>
#include <tools/debug.hxx>
#include <sax/tools/converter.hxx>
#include <xmloff/xmlaustp.hxx>
#include <xmloff/families.hxx>
#include <xmloff/styleexp.hxx>
#include <xmloff/settingsstore.hxx>
#include "sdpropls.hxx"
#include <xmloff/xmlexppr.hxx>
#include <com/sun/star/beans/XPropertyState.hpp>
#include "xexptran.hxx"

#include <cppuhelper/implbase1.hxx>
#include <comphelper/extract.hxx>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include "PropertySetMerger.hxx"
#include "layerexp.hxx"

#include "XMLNumberStylesExport.hxx"

#include <xmloff/animationexport.hxx>

#include <com/sun/star/document/XDocumentProperties.hpp>
#include <com/sun/star/document/XDocumentPropertiesSupplier.hpp>

using ::rtl::OUString;
using ::rtl::OUStringBuffer;

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::util;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::drawing;
using namespace ::com::sun::star::office;
using namespace ::com::sun::star::presentation;
using namespace ::com::sun::star::geometry;
using namespace ::com::sun::star::text;
using namespace ::xmloff::token;


//////////////////////////////////////////////////////////////////////////////

class ImpXMLEXPPageMasterInfo
{
    sal_Int32                   mnBorderBottom;
    sal_Int32                   mnBorderLeft;
    sal_Int32                   mnBorderRight;
    sal_Int32                   mnBorderTop;
    sal_Int32                   mnWidth;
    sal_Int32                   mnHeight;
    view::PaperOrientation      meOrientation;
    OUString                    msName;
    OUString                    msMasterPageName;

public:
    ImpXMLEXPPageMasterInfo(const SdXMLExport& rExp, const Reference<XDrawPage>& xPage);
    sal_Bool operator==(const ImpXMLEXPPageMasterInfo& rInfo) const;
    void SetName(const OUString& rStr);

    const OUString& GetName() const { return msName; }
    const OUString& GetMasterPageName() const { return msMasterPageName; }

    sal_Int32 GetBorderBottom() const { return mnBorderBottom; }
    sal_Int32 GetBorderLeft() const { return mnBorderLeft; }
    sal_Int32 GetBorderRight() const { return mnBorderRight; }
    sal_Int32 GetBorderTop() const { return mnBorderTop; }
    sal_Int32 GetWidth() const { return mnWidth; }
    sal_Int32 GetHeight() const { return mnHeight; }
    view::PaperOrientation GetOrientation() const { return meOrientation; }
};

ImpXMLEXPPageMasterInfo::ImpXMLEXPPageMasterInfo(
    const SdXMLExport& rExp,
    const Reference<XDrawPage>& xPage)
:   mnBorderBottom(0),
    mnBorderLeft(0),
    mnBorderRight(0),
    mnBorderTop(0),
    mnWidth(0),
    mnHeight(0),
    meOrientation(rExp.IsDraw() ? view::PaperOrientation_PORTRAIT : view::PaperOrientation_LANDSCAPE)
{
    Reference <beans::XPropertySet> xPropSet(xPage, UNO_QUERY);
    if(xPropSet.is())
    {
        Any aAny;

        Reference< beans::XPropertySetInfo > xPropsInfo( xPropSet->getPropertySetInfo() );
        if( xPropsInfo.is() && xPropsInfo->hasPropertyByName(OUString(RTL_CONSTASCII_USTRINGPARAM("BorderBottom") )))
        {
            aAny = xPropSet->getPropertyValue(OUString(RTL_CONSTASCII_USTRINGPARAM("BorderBottom")));
            aAny >>= mnBorderBottom;

            aAny = xPropSet->getPropertyValue(OUString(RTL_CONSTASCII_USTRINGPARAM("BorderLeft")));
            aAny >>= mnBorderLeft;

            aAny = xPropSet->getPropertyValue(OUString(RTL_CONSTASCII_USTRINGPARAM("BorderRight")));
            aAny >>= mnBorderRight;

            aAny = xPropSet->getPropertyValue(OUString(RTL_CONSTASCII_USTRINGPARAM("BorderTop")));
            aAny >>= mnBorderTop;
        }

        if( xPropsInfo.is() && xPropsInfo->hasPropertyByName(OUString(RTL_CONSTASCII_USTRINGPARAM("Width") )))
        {
            aAny = xPropSet->getPropertyValue(OUString(RTL_CONSTASCII_USTRINGPARAM("Width")));
            aAny >>= mnWidth;

            aAny = xPropSet->getPropertyValue(OUString(RTL_CONSTASCII_USTRINGPARAM("Height")));
            aAny >>= mnHeight;
        }

        if( xPropsInfo.is() && xPropsInfo->hasPropertyByName(OUString(RTL_CONSTASCII_USTRINGPARAM("Orientation") )))
        {
            aAny = xPropSet->getPropertyValue(OUString(RTL_CONSTASCII_USTRINGPARAM("Orientation")));
            aAny >>= meOrientation;
        }
    }

    Reference <container::XNamed> xMasterNamed(xPage, UNO_QUERY);
    if(xMasterNamed.is())
    {
        msMasterPageName = xMasterNamed->getName();
    }
}

sal_Bool ImpXMLEXPPageMasterInfo::operator==(const ImpXMLEXPPageMasterInfo& rInfo) const
{
    return ((mnBorderBottom == rInfo.mnBorderBottom)
        && (mnBorderLeft == rInfo.mnBorderLeft)
        && (mnBorderRight == rInfo.mnBorderRight)
        && (mnBorderTop == rInfo.mnBorderTop)
        && (mnWidth == rInfo.mnWidth)
        && (mnHeight == rInfo.mnHeight)
        && (meOrientation == rInfo.meOrientation));
}

void ImpXMLEXPPageMasterInfo::SetName(const OUString& rStr)
{
    msName = rStr;
}

//////////////////////////////////////////////////////////////////////////////

#define IMP_AUTOLAYOUT_INFO_MAX         (35L)

class ImpXMLAutoLayoutInfo
{
    sal_uInt16                  mnType;
    ImpXMLEXPPageMasterInfo*    mpPageMasterInfo;
    OUString                    msLayoutName;
    Rectangle                   maTitleRect;
    Rectangle                   maPresRect;
    sal_Int32                   mnGapX;
    sal_Int32                   mnGapY;

public:
    ImpXMLAutoLayoutInfo(sal_uInt16 nTyp, ImpXMLEXPPageMasterInfo* pInf);

    sal_Bool operator==(const ImpXMLAutoLayoutInfo& rInfo) const;

    sal_uInt16 GetLayoutType() const { return mnType; }
    sal_Int32 GetGapX() const { return mnGapX; }
    sal_Int32 GetGapY() const { return mnGapY; }

    const OUString& GetLayoutName() const { return msLayoutName; }
    void SetLayoutName(const OUString& rNew) { msLayoutName = rNew; }

    const Rectangle& GetTitleRectangle() const { return maTitleRect; }
    const Rectangle& GetPresRectangle() const { return maPresRect; }

    static sal_Bool IsCreateNecessary(sal_uInt16 nTyp);
};

sal_Bool ImpXMLAutoLayoutInfo::IsCreateNecessary(sal_uInt16 nTyp)
{
    if(nTyp == 5 /* AUTOLAYOUT_ORG */
        || nTyp == 20 /* AUTOLAYOUT_NONE */
        || nTyp >= IMP_AUTOLAYOUT_INFO_MAX)
        return sal_False;
    return sal_True;
}

sal_Bool ImpXMLAutoLayoutInfo::operator==(const ImpXMLAutoLayoutInfo& rInfo) const
{
    return ((mnType == rInfo.mnType
        && mpPageMasterInfo == rInfo.mpPageMasterInfo));
}

ImpXMLAutoLayoutInfo::ImpXMLAutoLayoutInfo(sal_uInt16 nTyp, ImpXMLEXPPageMasterInfo* pInf)
:   mnType(nTyp),
    mpPageMasterInfo(pInf)
{
    // create full info (initialze with typical values)
    Point aPagePos(0,0);
    Size aPageSize(28000, 21000);
    Size aPageInnerSize(28000, 21000);

    if(mpPageMasterInfo)
    {
        aPagePos = Point(mpPageMasterInfo->GetBorderLeft(), mpPageMasterInfo->GetBorderTop());
        aPageSize = Size(mpPageMasterInfo->GetWidth(), mpPageMasterInfo->GetHeight());
        aPageInnerSize = aPageSize;
        aPageInnerSize.Width() -= mpPageMasterInfo->GetBorderLeft() + mpPageMasterInfo->GetBorderRight();
        aPageInnerSize.Height() -= mpPageMasterInfo->GetBorderTop() + mpPageMasterInfo->GetBorderBottom();
    }

    // title rectangle aligning
    Point aTitlePos(aPagePos);
    Size aTitleSize(aPageInnerSize);

    if(mnType == 21 /* AUTOLAYOUT_NOTES */)
    {
        aTitleSize.Height() = (long) (aTitleSize.Height() / 2.5);
        Point aPos = aTitlePos;
        aPos.Y() += long( aTitleSize.Height() * 0.083 );
        Size aPartArea = aTitleSize;
        Size aSize;

        // scale handout rectangle using actual page size
        double fH = (double) aPartArea.Width()  / aPageSize.Width();
        double fV = (double) aPartArea.Height() / aPageSize.Height();

        if ( fH > fV )
            fH = fV;
        aSize.Width()  = (long) (fH * aPageSize.Width());
        aSize.Height() = (long) (fH * aPageSize.Height());

        aPos.X() += (aPartArea.Width() - aSize.Width()) / 2;
        aPos.Y() += (aPartArea.Height()- aSize.Height())/ 2;

        aTitlePos = aPos;
        aTitleSize = aSize;
    }
    else if(mnType == 27 || mnType == 28)
    {
        // AUTOLAYOUT_VERTICAL_TITLE_TEXT_CHART or
        // AUTOLAYOUT_VERTICAL_TITLE_VERTICAL_OUTLINE
        Point aClassicTPos(
            aTitlePos.X() + long( aTitleSize.Width() * 0.0735 ),
            aTitlePos.Y() + long( aTitleSize.Height() * 0.083 ));
        Size aClassicTSize(
            long( aTitleSize.Width() * 0.854 ),
            long( aTitleSize.Height() * 0.167 ));
        Point aLPos(aPagePos);
        Size aLSize(aPageInnerSize);
        Point aClassicLPos(
            aLPos.X() + long( aLSize.Width() * 0.0735 ),
            aLPos.Y() + long( aLSize.Height() * 0.472 ));
        Size aClassicLSize(
            long( aLSize.Width() * 0.854 ),
            long( aLSize.Height() * 0.444 ));

        aTitlePos.X() = (aClassicTPos.X() + aClassicTSize.Width()) - aClassicTSize.Height();
        aTitlePos.Y() = aClassicTPos.Y();
        aTitleSize.Width() = aClassicTSize.Height();
        aTitleSize.Height() = (aClassicLPos.Y() + aClassicLSize.Height()) - aClassicTPos.Y();
    }
    else
    {
        aTitlePos.X() += long( aTitleSize.Width() * 0.0735 );
        aTitlePos.Y() += long( aTitleSize.Height() * 0.083 );
        aTitleSize.Width() = long( aTitleSize.Width() * 0.854 );
        aTitleSize.Height() = long( aTitleSize.Height() * 0.167 );
    }

    maTitleRect.SetPos(aTitlePos);
    maTitleRect.SetSize(aTitleSize);

    // layout rectangle aligning
    Point aLayoutPos(aPagePos);
    Size aLayoutSize(aPageInnerSize);

    if(mnType == 21 /* AUTOLAYOUT_NOTES */)
    {
        aLayoutPos.X() += long( aLayoutSize.Width() * 0.0735 );
        aLayoutPos.Y() += long( aLayoutSize.Height() * 0.472 );
        aLayoutSize.Width() = long( aLayoutSize.Width() * 0.854 );
        aLayoutSize.Height() = long( aLayoutSize.Height() * 0.444 );
    }
    else if((mnType >= 22 && mnType <= 26) || (mnType == 31)) // AUTOLAYOUT_HANDOUT
    {
        // keep info for inner area in maPresRect, put info for gap size
        // to maTitleRect position
        mnGapX = (aPageSize.Width() - aPageInnerSize.Width()) / 2;
        mnGapY = (aPageSize.Height() - aPageInnerSize.Height()) / 2;

        if(!mnGapX)
            mnGapX = aPageSize.Width() / 10;

        if(!mnGapY)
            mnGapY = aPageSize.Height() / 10;

        if(mnGapX < aPageInnerSize.Width() / 10)
            mnGapX = aPageInnerSize.Width() / 10;

        if(mnGapY < aPageInnerSize.Height() / 10)
            mnGapY = aPageInnerSize.Height() / 10;
    }
    else if(mnType == 27 || mnType == 28)
    {
        // AUTOLAYOUT_VERTICAL_TITLE_TEXT_CHART or
        // AUTOLAYOUT_VERTICAL_TITLE_VERTICAL_OUTLINE
        Point aClassicTPos(
            aTitlePos.X() + long( aTitleSize.Width() * 0.0735 ),
            aTitlePos.Y() + long( aTitleSize.Height() * 0.083 ));
        Size aClassicTSize(
            long( aTitleSize.Width() * 0.854 ),
            long( aTitleSize.Height() * 0.167 ));
        Point aClassicLPos(
            aLayoutPos.X() + long( aLayoutSize.Width() * 0.0735 ),
            aLayoutPos.Y() + long( aLayoutSize.Height() * 0.472 ));
        Size aClassicLSize(
            long( aLayoutSize.Width() * 0.854 ),
            long( aLayoutSize.Height() * 0.444 ));

        aLayoutPos.X() = aClassicLPos.X();
        aLayoutPos.Y() = aClassicTPos.Y();
        aLayoutSize.Width() = (aClassicLPos.X() + aClassicLSize.Width())
            - (aClassicTSize.Height() + (aClassicLPos.Y() - (aClassicTPos.Y() + aClassicTSize.Height())));
        aLayoutSize.Height() = (aClassicLPos.Y() + aClassicLSize.Height()) - aClassicTPos.Y();
    }
    else if( mnType == 32 )
    {
        // AUTOLAYOUT_ONLY_TEXT
        aLayoutPos = aTitlePos;
        aLayoutSize.Width() = aTitleSize.Width();
        aLayoutSize.Height() = long( aLayoutSize.Height() * 0.825 );
    }
    else
    {
        aLayoutPos.X() += long( aLayoutSize.Width() * 0.0735 );
        aLayoutPos.Y() += long( aLayoutSize.Height() * 0.278 );
        aLayoutSize.Width() = long( aLayoutSize.Width() * 0.854 );
        aLayoutSize.Height() = long( aLayoutSize.Height() * 0.630 );
    }

    maPresRect.SetPos(aLayoutPos);
    maPresRect.SetSize(aLayoutSize);
}

//////////////////////////////////////////////////////////////////////////////

// #110680#
SdXMLExport::SdXMLExport(
    const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& xServiceFactory,
    sal_Bool bIsDraw, sal_uInt16 nExportFlags )
:   SvXMLExport( util::MeasureUnit::CM, xServiceFactory,
        (bIsDraw) ? XML_GRAPHICS : XML_PRESENTATION, nExportFlags ),
    mnDocMasterPageCount(0L),
    mnDocDrawPageCount(0L),
    mnShapeStyleInfoIndex(0L),
    mnObjectCount(0L),
    mpPageMasterInfoList(new ImpXMLEXPPageMasterList()),
    mpPageMasterUsageList(new ImpXMLEXPPageMasterList()),
    mpNotesPageMasterUsageList(new ImpXMLEXPPageMasterList()),
    mpHandoutPageMaster(NULL),
    mpAutoLayoutInfoList(new ImpXMLAutoLayoutInfoList()),
    mpSdPropHdlFactory(0L),
    mpPropertySetMapper(0L),
    mpPresPagePropsMapper(0L),
    mbIsDraw(bIsDraw),
    mbFamilyGraphicUsed(sal_False),
    mbFamilyPresentationUsed(sal_False),
    msZIndex( GetXMLToken(XML_ZINDEX) ),
    msEmptyPres( RTL_CONSTASCII_USTRINGPARAM("IsEmptyPresentationObject") ),
    msModel( RTL_CONSTASCII_USTRINGPARAM("Model") ),
    msStartShape( RTL_CONSTASCII_USTRINGPARAM("StartShape") ),
    msEndShape( RTL_CONSTASCII_USTRINGPARAM("EndShape") ),
    msPageLayoutNames( RTL_CONSTASCII_USTRINGPARAM("PageLayoutNames") )
{


}

// XExporter
void SAL_CALL SdXMLExport::setSourceDocument( const Reference< lang::XComponent >& xDoc )
    throw(lang::IllegalArgumentException, uno::RuntimeException)
{
    SvXMLExport::setSourceDocument( xDoc );

    const OUString aEmpty;

    // prepare factory parts
    mpSdPropHdlFactory = new XMLSdPropHdlFactory( GetModel(), *this );
    if(mpSdPropHdlFactory)
    {
        // set lock to avoid deletion
        mpSdPropHdlFactory->acquire();

        // build one ref
        const UniReference< XMLPropertyHandlerFactory > aFactoryRef = mpSdPropHdlFactory;

        // construct PropertySetMapper
        UniReference < XMLPropertySetMapper > xMapper = new XMLShapePropertySetMapper( aFactoryRef);

        mpPropertySetMapper = new XMLShapeExportPropertyMapper( xMapper, *this );
        // set lock to avoid deletion
        mpPropertySetMapper->acquire();

        // chain text attributes
        mpPropertySetMapper->ChainExportMapper(XMLTextParagraphExport::CreateParaExtPropMapper(*this));

        // construct PresPagePropsMapper
        xMapper = new XMLPropertySetMapper((XMLPropertyMapEntry*)aXMLSDPresPageProps, aFactoryRef);

        mpPresPagePropsMapper = new XMLPageExportPropertyMapper( xMapper, *this  );
        if(mpPresPagePropsMapper)
        {
            // set lock to avoid deletion
            mpPresPagePropsMapper->acquire();
        }
    }

    // add family name
      GetAutoStylePool()->AddFamily(
        XML_STYLE_FAMILY_SD_GRAPHICS_ID,
        OUString(RTL_CONSTASCII_USTRINGPARAM(XML_STYLE_FAMILY_SD_GRAPHICS_NAME)),
          GetPropertySetMapper(),
          OUString(RTL_CONSTASCII_USTRINGPARAM(XML_STYLE_FAMILY_SD_GRAPHICS_PREFIX)));
    GetAutoStylePool()->AddFamily(
        XML_STYLE_FAMILY_SD_PRESENTATION_ID,
        OUString(RTL_CONSTASCII_USTRINGPARAM(XML_STYLE_FAMILY_SD_PRESENTATION_NAME)),
          GetPropertySetMapper(),
          OUString(RTL_CONSTASCII_USTRINGPARAM(XML_STYLE_FAMILY_SD_PRESENTATION_PREFIX)));
    GetAutoStylePool()->AddFamily(
        XML_STYLE_FAMILY_SD_DRAWINGPAGE_ID,
        OUString(RTL_CONSTASCII_USTRINGPARAM(XML_STYLE_FAMILY_SD_DRAWINGPAGE_NAME)),
          GetPresPagePropsMapper(),
          OUString(RTL_CONSTASCII_USTRINGPARAM(XML_STYLE_FAMILY_SD_DRAWINGPAGE_PREFIX)));
    // prepare access to styles
    Reference< style::XStyleFamiliesSupplier > xFamSup( GetModel(), UNO_QUERY );
    if(xFamSup.is())
    {
        mxDocStyleFamilies = xFamSup->getStyleFamilies();
    }

    // prepare access to master pages
    Reference < drawing::XMasterPagesSupplier > xMasterPagesSupplier(GetModel(), UNO_QUERY);
    if(xMasterPagesSupplier.is())
    {
        mxDocMasterPages = mxDocMasterPages.query( xMasterPagesSupplier->getMasterPages() );
        if(mxDocMasterPages.is())
        {
            mnDocMasterPageCount = mxDocMasterPages->getCount();
            maMasterPagesStyleNames.insert( maMasterPagesStyleNames.begin(), mnDocMasterPageCount, aEmpty );
        }
    }

    // prepare access to draw pages
    Reference <XDrawPagesSupplier> xDrawPagesSupplier(GetModel(), UNO_QUERY);
    if(xDrawPagesSupplier.is())
    {
        mxDocDrawPages = mxDocDrawPages.query( xDrawPagesSupplier->getDrawPages() );
        if(mxDocDrawPages.is())
        {
            mnDocDrawPageCount = mxDocDrawPages->getCount();
            maDrawPagesStyleNames.insert( maDrawPagesStyleNames.begin(), mnDocDrawPageCount, aEmpty );
            maDrawNotesPagesStyleNames.insert( maDrawNotesPagesStyleNames.begin(), mnDocDrawPageCount, aEmpty );
            if( !mbIsDraw )
                maDrawPagesAutoLayoutNames.realloc( mnDocDrawPageCount + 1 );

            HeaderFooterPageSettingsImpl aEmptySettings;
            maDrawPagesHeaderFooterSettings.insert( maDrawPagesHeaderFooterSettings.begin(), mnDocDrawPageCount, aEmptySettings );
            maDrawNotesPagesHeaderFooterSettings.insert( maDrawNotesPagesHeaderFooterSettings.begin(), mnDocDrawPageCount, aEmptySettings );
        }
    }

    // #82003# count all draw objects for use with progress bar.
    // #88245# init mnObjectCount once, use counter itself as flag. It
    // is initialized to 0.
    if(!mnObjectCount)
    {
        if( IsImpress() )
        {
             // #91587# add handout master count
            Reference<presentation::XHandoutMasterSupplier> xHandoutSupp(GetModel(), UNO_QUERY);
            if(xHandoutSupp.is())
            {
                Reference<XDrawPage> xHandoutPage(xHandoutSupp->getHandoutMasterPage());
                if(xHandoutPage.is())
                {
                    Reference<drawing::XShapes> xShapes(xHandoutPage, UNO_QUERY);
                    if(xShapes.is() && xShapes->getCount())
                    {
                        mnObjectCount += ImpRecursiveObjectCount(xShapes);
                    }
                }
            }
        }

        if(mxDocMasterPages.is())
        {
            for(sal_Int32 a(0); a < mnDocMasterPageCount; a++)
            {
                Any aAny(mxDocMasterPages->getByIndex(a));
                Reference< drawing::XShapes > xMasterPage;

                if((aAny >>= xMasterPage) && xMasterPage.is())
                {
                    mnObjectCount += ImpRecursiveObjectCount(xMasterPage);
                }

                if( IsImpress() )
                {
                    // #91587# take notes pages from master pages into account
                    Reference<presentation::XPresentationPage> xPresPage;
                    if((aAny >>= xPresPage) && xPresPage.is())
                    {
                        Reference<XDrawPage> xNotesPage(xPresPage->getNotesPage());
                        if(xNotesPage.is())
                        {
                            Reference<drawing::XShapes> xShapes(xNotesPage, UNO_QUERY);
                            if(xShapes.is() && xShapes->getCount())
                            {
                                mnObjectCount += ImpRecursiveObjectCount(xShapes);
                            }
                        }
                    }
                }
            }
        }

        if(mxDocDrawPages.is())
        {
            for(sal_Int32 a(0); a < mnDocDrawPageCount; a++)
            {
                Any aAny(mxDocDrawPages->getByIndex(a));
                Reference< drawing::XShapes > xPage;

                if((aAny >>= xPage) && xPage.is())
                {
                    mnObjectCount += ImpRecursiveObjectCount(xPage);
                }

                if( IsImpress() )
                {
                    // #91587# take notes pages from draw pages into account
                    Reference<presentation::XPresentationPage> xPresPage;
                    if((aAny >>= xPresPage) && xPresPage.is())
                    {
                        Reference<XDrawPage> xNotesPage(xPresPage->getNotesPage());
                        if(xNotesPage.is())
                        {
                            Reference<drawing::XShapes> xShapes(xNotesPage, UNO_QUERY);
                            if(xShapes.is() && xShapes->getCount())
                            {
                                mnObjectCount += ImpRecursiveObjectCount(xShapes);
                            }
                        }
                    }
                }
            }
        }

        // #82003# init progress bar
        GetProgressBarHelper()->SetReference(mnObjectCount);
    }

    // add namespaces
    _GetNamespaceMap().Add(
        GetXMLToken(XML_NP_PRESENTATION),
        GetXMLToken(XML_N_PRESENTATION),
        XML_NAMESPACE_PRESENTATION);

    _GetNamespaceMap().Add(
        GetXMLToken(XML_NP_SMIL),
        GetXMLToken(XML_N_SMIL_COMPAT),
        XML_NAMESPACE_SMIL);

    _GetNamespaceMap().Add(
        GetXMLToken(XML_NP_ANIMATION),
        GetXMLToken(XML_N_ANIMATION),
        XML_NAMESPACE_ANIMATION);

    if( getDefaultVersion() > SvtSaveOptions::ODFVER_012 )
    {
        _GetNamespaceMap().Add(
            GetXMLToken(XML_NP_OFFICE_EXT),
            GetXMLToken(XML_N_OFFICE_EXT),
            XML_NAMESPACE_OFFICE_EXT);
    }

    GetShapeExport()->enableLayerExport();

    // #88546# enable progress bar increments
    GetShapeExport()->enableHandleProgressBar();
}

//////////////////////////////////////////////////////////////////////////////
// #82003# helper function for recursive object count
sal_uInt32 SdXMLExport::ImpRecursiveObjectCount(Reference< drawing::XShapes > xShapes)
{
    sal_uInt32 nRetval(0L);

    if(xShapes.is())
    {
        sal_Int32 nCount = xShapes->getCount();

        for(sal_Int32 a(0L); a < nCount; a++)
        {
            Any aAny(xShapes->getByIndex(a));
            Reference< drawing::XShapes > xGroup;

            if((aAny >>= xGroup) && xGroup.is())
            {
                // #93180# count group objects, too.
                nRetval += 1 + ImpRecursiveObjectCount(xGroup);
            }
            else
            {
                nRetval++;
            }
        }
    }

    return nRetval;
}

//////////////////////////////////////////////////////////////////////////////

SdXMLExport::~SdXMLExport()
{
    // cleanup factory, decrease refcount. Should lead to destruction.
    if(mpSdPropHdlFactory)
    {
        mpSdPropHdlFactory->release();
        mpSdPropHdlFactory = 0L;
    }

    // cleanup mapper, decrease refcount. Should lead to destruction.
    if(mpPropertySetMapper)
    {
        mpPropertySetMapper->release();
        mpPropertySetMapper = 0L;
    }

    // cleanup presPage mapper, decrease refcount. Should lead to destruction.
    if(mpPresPagePropsMapper)
    {
        mpPresPagePropsMapper->release();
        mpPresPagePropsMapper = 0L;
    }

    // clear evtl. temporary page master infos
    if(mpPageMasterUsageList)
    {
        // note: all items in this list are also in mpPageMasterInfoList
        delete mpPageMasterUsageList;
        mpPageMasterUsageList = 0L;
    }

    if(mpNotesPageMasterUsageList)
    {
        // note: all items in this list are also in mpPageMasterInfoList
        delete mpNotesPageMasterUsageList;
        mpNotesPageMasterUsageList = 0L;
    }

    if(mpPageMasterInfoList)
    {
        for ( size_t i = 0, n = mpPageMasterInfoList->size(); i < n; ++i )
            delete mpPageMasterInfoList->at( i );
        mpPageMasterInfoList->clear();
        delete mpPageMasterInfoList;
        mpPageMasterInfoList = 0L;
    }

    // clear auto-layout infos
    if(mpAutoLayoutInfoList)
    {
        for ( size_t i = 0, n = mpAutoLayoutInfoList->size(); i < n; ++i )
            delete mpAutoLayoutInfoList->at( i );
        mpAutoLayoutInfoList->clear();
        delete mpAutoLayoutInfoList;
        mpAutoLayoutInfoList = 0L;
    }
}

//////////////////////////////////////////////////////////////////////////////

void SdXMLExport::ImpPrepAutoLayoutInfos()
{
    if(IsImpress())
    {
        OUString aStr;

        Reference< presentation::XHandoutMasterSupplier > xHandoutSupp( GetModel(), UNO_QUERY );
        if( xHandoutSupp.is() )
        {
            Reference< XDrawPage > xHandoutPage( xHandoutSupp->getHandoutMasterPage() );
            if( xHandoutPage.is() )
            {
                if(ImpPrepAutoLayoutInfo(xHandoutPage, aStr))
                    maDrawPagesAutoLayoutNames[0] = aStr;
            }
        }

        // prepare name creation
        for (sal_Int32 nCnt = 0; nCnt < mnDocDrawPageCount; nCnt++)
        {
            Any aAny(mxDocDrawPages->getByIndex(nCnt));
            Reference<XDrawPage> xDrawPage;

            if((aAny >>= xDrawPage) && xDrawPage.is())
            {
                if(ImpPrepAutoLayoutInfo(xDrawPage, aStr))
                    maDrawPagesAutoLayoutNames[nCnt+1] = aStr;
            }
        }
    }
}

sal_Bool SdXMLExport::ImpPrepAutoLayoutInfo(const Reference<XDrawPage>& xPage, OUString& rName)
{
    rName = OUString();
    sal_Bool bRetval(sal_False);

    Reference <beans::XPropertySet> xPropSet(xPage, UNO_QUERY);
    if(xPropSet.is())
    {
        sal_uInt16 nType = sal_uInt16();
        Any aAny;

        aAny = xPropSet->getPropertyValue(OUString(RTL_CONSTASCII_USTRINGPARAM("Layout")));
        if(aAny >>= nType)
        {
            if(ImpXMLAutoLayoutInfo::IsCreateNecessary(nType))
            {
                ImpXMLEXPPageMasterInfo* pInfo = 0L;

                // get master-page info
                Reference < drawing::XMasterPageTarget > xMasterPageInt(xPage, UNO_QUERY);
                if(xMasterPageInt.is())
                {
                    Reference<XDrawPage> xUsedMasterPage(xMasterPageInt->getMasterPage());
                    if(xUsedMasterPage.is())
                    {
                        Reference < container::XNamed > xMasterNamed(xUsedMasterPage, UNO_QUERY);
                        if(xMasterNamed.is())
                        {
                            OUString sMasterPageName = xMasterNamed->getName();
                            pInfo = ImpGetPageMasterInfoByName(sMasterPageName);
                        }
                    }
                }

                // create entry and look for existance
                ImpXMLAutoLayoutInfo* pNew = new ImpXMLAutoLayoutInfo(nType, pInfo);
                sal_Bool bDidExist(sal_False);

                for( size_t nCnt = 0; !bDidExist && nCnt < mpAutoLayoutInfoList->size(); nCnt++)
                {
                    if( *mpAutoLayoutInfoList->at( nCnt ) == *pNew)
                    {
                        delete pNew;
                        pNew = mpAutoLayoutInfoList->at( nCnt );
                        bDidExist = sal_True;
                    }
                }

                if(!bDidExist)
                {
                    mpAutoLayoutInfoList->push_back( pNew );
                    OUString sNewName = OUString(RTL_CONSTASCII_USTRINGPARAM("AL"));
                    sNewName += OUString::valueOf(sal_Int32( mpAutoLayoutInfoList->size() - 1 ));
                    sNewName += OUString(RTL_CONSTASCII_USTRINGPARAM("T"));
                    sNewName += OUString::valueOf(sal_Int32(nType));
                    pNew->SetLayoutName(sNewName);
                }

                rName = pNew->GetLayoutName();
                bRetval = sal_True;
            }
        }
    }

    return bRetval;
}

//////////////////////////////////////////////////////////////////////////////

void SdXMLExport::ImpWriteAutoLayoutInfos()
{
    if( !mpAutoLayoutInfoList->empty() )
    {
        for(size_t nCnt = 0; nCnt < mpAutoLayoutInfoList->size(); nCnt++)
        {
            ImpXMLAutoLayoutInfo* pInfo = mpAutoLayoutInfoList->at( nCnt );
            if(pInfo)
            {
                // prepare presentation-page layout attributes, style-name
                AddAttribute(XML_NAMESPACE_STYLE, XML_NAME, pInfo->GetLayoutName());

                // write draw-style attributes
                SvXMLElementExport aDSE(*this, XML_NAMESPACE_STYLE, XML_PRESENTATION_PAGE_LAYOUT, sal_True, sal_True);

                // write presentation placeholders
                switch(pInfo->GetLayoutType())
                {
                    case 0 : // AUTOLAYOUT_TITLE
                    {
                        ImpWriteAutoLayoutPlaceholder(XmlPlaceholderTitle, pInfo->GetTitleRectangle());
                        ImpWriteAutoLayoutPlaceholder(XmlPlaceholderSubtitle, pInfo->GetPresRectangle());
                        break;
                    }
                    case 1 : // AUTOLAYOUT_ENUM
                    {
                        ImpWriteAutoLayoutPlaceholder(XmlPlaceholderTitle, pInfo->GetTitleRectangle());
                        ImpWriteAutoLayoutPlaceholder(XmlPlaceholderOutline, pInfo->GetPresRectangle());
                        break;
                    }
                    case 2 : // AUTOLAYOUT_CHART
                    {
                        ImpWriteAutoLayoutPlaceholder(XmlPlaceholderTitle, pInfo->GetTitleRectangle());
                        ImpWriteAutoLayoutPlaceholder(XmlPlaceholderChart, pInfo->GetPresRectangle());
                        break;
                    }
                    case 3 : // AUTOLAYOUT_2TEXT
                    {
                        Rectangle aLeft(pInfo->GetPresRectangle());
                        aLeft.setWidth(long(aLeft.GetWidth() * 0.488));
                        Rectangle aRight(aLeft);
                        aRight.Left() = long(aRight.Left() + aRight.GetWidth() * 1.05);

                        ImpWriteAutoLayoutPlaceholder(XmlPlaceholderTitle, pInfo->GetTitleRectangle());
                        ImpWriteAutoLayoutPlaceholder(XmlPlaceholderOutline, aLeft);
                        ImpWriteAutoLayoutPlaceholder(XmlPlaceholderOutline, aRight);
                        break;
                    }
                    case 4 : // AUTOLAYOUT_TEXTCHART
                    {
                        Rectangle aLeft(pInfo->GetPresRectangle());
                        aLeft.setWidth(long(aLeft.GetWidth() * 0.488));
                        Rectangle aRight(aLeft);
                        aRight.Left() = long(aRight.Left() + aRight.GetWidth() * 1.05);

                        ImpWriteAutoLayoutPlaceholder(XmlPlaceholderTitle, pInfo->GetTitleRectangle());
                        ImpWriteAutoLayoutPlaceholder(XmlPlaceholderOutline, aLeft);
                        ImpWriteAutoLayoutPlaceholder(XmlPlaceholderChart, aRight);
                        break;
                    }
                    case 6 : // AUTOLAYOUT_TEXTCLIP
                    {
                        Rectangle aLeft(pInfo->GetPresRectangle());
                        aLeft.setWidth(long(aLeft.GetWidth() * 0.488));
                        Rectangle aRight(aLeft);
                        aRight.Left() = long(aRight.Left() + aRight.GetWidth() * 1.05);

                        ImpWriteAutoLayoutPlaceholder(XmlPlaceholderTitle, pInfo->GetTitleRectangle());
                        ImpWriteAutoLayoutPlaceholder(XmlPlaceholderOutline, aLeft);
                        ImpWriteAutoLayoutPlaceholder(XmlPlaceholderGraphic, aRight);
                        break;
                    }
                    case 7 : // AUTOLAYOUT_CHARTTEXT
                    {
                        Rectangle aLeft(pInfo->GetPresRectangle());
                        aLeft.setWidth(long(aLeft.GetWidth() * 0.488));
                        Rectangle aRight(aLeft);
                        aRight.Left() = long(aRight.Left() + aRight.GetWidth() * 1.05);

                        ImpWriteAutoLayoutPlaceholder(XmlPlaceholderTitle, pInfo->GetTitleRectangle());
                        ImpWriteAutoLayoutPlaceholder(XmlPlaceholderChart, aLeft);
                        ImpWriteAutoLayoutPlaceholder(XmlPlaceholderOutline, aRight);
                        break;
                    }
                    case 8 : // AUTOLAYOUT_TAB
                    {
                        ImpWriteAutoLayoutPlaceholder(XmlPlaceholderTitle, pInfo->GetTitleRectangle());
                        ImpWriteAutoLayoutPlaceholder(XmlPlaceholderTable, pInfo->GetPresRectangle());
                        break;
                    }
                    case 9 : // AUTOLAYOUT_CLIPTEXT
                    {
                        Rectangle aLeft(pInfo->GetPresRectangle());
                        aLeft.setWidth(long(aLeft.GetWidth() * 0.488));
                        Rectangle aRight(aLeft);
                        aRight.Left() = long(aRight.Left() + aRight.GetWidth() * 1.05);

                        ImpWriteAutoLayoutPlaceholder(XmlPlaceholderTitle, pInfo->GetTitleRectangle());
                        ImpWriteAutoLayoutPlaceholder(XmlPlaceholderGraphic, aLeft);
                        ImpWriteAutoLayoutPlaceholder(XmlPlaceholderOutline, aRight);
                        break;
                    }
                    case 10 : // AUTOLAYOUT_TEXTOBJ
                    {
                        Rectangle aLeft(pInfo->GetPresRectangle());
                        aLeft.setWidth(long(aLeft.GetWidth() * 0.488));
                        Rectangle aRight(aLeft);
                        aRight.Left() = long(aRight.Left() + aRight.GetWidth() * 1.05);

                        ImpWriteAutoLayoutPlaceholder(XmlPlaceholderTitle, pInfo->GetTitleRectangle());
                        ImpWriteAutoLayoutPlaceholder(XmlPlaceholderOutline, aLeft);
                        ImpWriteAutoLayoutPlaceholder(XmlPlaceholderObject, aRight);
                        break;
                    }
                    case 11 : // AUTOLAYOUT_OBJ
                    {
                        ImpWriteAutoLayoutPlaceholder(XmlPlaceholderTitle, pInfo->GetTitleRectangle());
                        ImpWriteAutoLayoutPlaceholder(XmlPlaceholderObject, pInfo->GetPresRectangle());
                        break;
                    }
                    case 12 : // AUTOLAYOUT_TEXT2OBJ
                    {
                        Rectangle aLeft(pInfo->GetPresRectangle());
                        aLeft.setWidth(long(aLeft.GetWidth() * 0.488));
                        Rectangle aRightTop(aLeft);
                        aRightTop.Left() = long(aRightTop.Left() + aRightTop.GetWidth() * 1.05);
                        aRightTop.setHeight(long(aRightTop.GetHeight() * 0.477));
                        Rectangle aRightBottom(aRightTop);
                        aRightBottom.Top() = long(aRightBottom.Top() + aRightBottom.GetHeight() * 1.095);

                        ImpWriteAutoLayoutPlaceholder(XmlPlaceholderTitle, pInfo->GetTitleRectangle());
                        ImpWriteAutoLayoutPlaceholder(XmlPlaceholderOutline, aLeft);
                        ImpWriteAutoLayoutPlaceholder(XmlPlaceholderObject, aRightTop);
                        ImpWriteAutoLayoutPlaceholder(XmlPlaceholderObject, aRightBottom);
                        break;
                    }
                    case 13 : // AUTOLAYOUT_OBJTEXT
                    {
                        Rectangle aLeft(pInfo->GetPresRectangle());
                        aLeft.setWidth(long(aLeft.GetWidth() * 0.488));
                        Rectangle aRight(aLeft);
                        aRight.Left() = long(aRight.Left() + aRight.GetWidth() * 1.05);

                        ImpWriteAutoLayoutPlaceholder(XmlPlaceholderTitle, pInfo->GetTitleRectangle());
                        ImpWriteAutoLayoutPlaceholder(XmlPlaceholderObject, aLeft);
                        ImpWriteAutoLayoutPlaceholder(XmlPlaceholderOutline, aRight);
                        break;
                    }
                    case 14 : // AUTOLAYOUT_OBJOVERTEXT
                    {
                        Rectangle aTop(pInfo->GetPresRectangle());
                        aTop.setHeight(long(aTop.GetHeight() * 0.477));
                        Rectangle aBottom(aTop);
                        aBottom.Top() = long(aBottom.Top() + aBottom.GetHeight() * 1.095);

                        ImpWriteAutoLayoutPlaceholder(XmlPlaceholderTitle, pInfo->GetTitleRectangle());
                        ImpWriteAutoLayoutPlaceholder(XmlPlaceholderObject, aTop);
                        ImpWriteAutoLayoutPlaceholder(XmlPlaceholderOutline, aBottom);
                        break;
                    }
                    case 15 : // AUTOLAYOUT_2OBJTEXT
                    {
                        Rectangle aLeftTop(pInfo->GetPresRectangle());
                        aLeftTop.setWidth(long(aLeftTop.GetWidth() * 0.488));
                        Rectangle aRight(aLeftTop);
                        aRight.Left() = long(aRight.Left() + aRight.GetWidth() * 1.05);
                        aLeftTop.setHeight(long(aLeftTop.GetHeight() * 0.477));
                        Rectangle aLeftBottom(aLeftTop);
                        aLeftBottom.Top() = long(aLeftBottom.Top() + aLeftBottom.GetHeight() * 1.095);

                        ImpWriteAutoLayoutPlaceholder(XmlPlaceholderTitle, pInfo->GetTitleRectangle());
                        ImpWriteAutoLayoutPlaceholder(XmlPlaceholderObject, aLeftTop);
                        ImpWriteAutoLayoutPlaceholder(XmlPlaceholderObject, aLeftBottom);
                        ImpWriteAutoLayoutPlaceholder(XmlPlaceholderOutline, aRight);
                        break;
                    }
                    case 16 : // AUTOLAYOUT_2OBJOVERTEXT
                    {
                        Rectangle aTopLeft(pInfo->GetPresRectangle());
                        aTopLeft.setHeight(long(aTopLeft.GetHeight() * 0.477));
                        Rectangle aBottom(aTopLeft);
                        aBottom.Top() = long(aBottom.Top() + aBottom.GetHeight() * 1.095);
                        aTopLeft.setWidth(long(aTopLeft.GetWidth() * 0.488));
                        Rectangle aTopRight(aTopLeft);
                        aTopRight.Left() = long(aTopRight.Left() + aTopRight.GetWidth() * 1.05);

                        ImpWriteAutoLayoutPlaceholder(XmlPlaceholderTitle, pInfo->GetTitleRectangle());
                        ImpWriteAutoLayoutPlaceholder(XmlPlaceholderObject, aTopLeft);
                        ImpWriteAutoLayoutPlaceholder(XmlPlaceholderObject, aTopRight);
                        ImpWriteAutoLayoutPlaceholder(XmlPlaceholderOutline, aBottom);
                        break;
                    }
                    case 17 : // AUTOLAYOUT_TEXTOVEROBJ
                    {
                        Rectangle aTop(pInfo->GetPresRectangle());
                        aTop.setHeight(long(aTop.GetHeight() * 0.477));
                        Rectangle aBottom(aTop);
                        aBottom.Top() = long(aBottom.Top() + aBottom.GetHeight() * 1.095);

                        ImpWriteAutoLayoutPlaceholder(XmlPlaceholderTitle, pInfo->GetTitleRectangle());
                        ImpWriteAutoLayoutPlaceholder(XmlPlaceholderOutline, aTop);
                        ImpWriteAutoLayoutPlaceholder(XmlPlaceholderObject, aBottom);
                        break;
                    }
                    case 18 : // AUTOLAYOUT_4OBJ
                    {
                        Rectangle aTopLeft(pInfo->GetPresRectangle());
                        aTopLeft.setHeight(long(aTopLeft.GetHeight() * 0.477));
                        aTopLeft.setWidth(long(aTopLeft.GetWidth() * 0.488));
                        Rectangle aBottomLeft(aTopLeft);
                        aBottomLeft.Top() = long(aBottomLeft.Top() + aBottomLeft.GetHeight() * 1.095);
                        Rectangle aTopRight(aTopLeft);
                        aTopRight.Left() = long(aTopRight.Left() + aTopRight.GetWidth() * 1.05);
                        Rectangle aBottomRight(aTopRight);
                        aBottomRight.Top() = long(aBottomRight.Top() + aBottomRight.GetHeight() * 1.095);

                        ImpWriteAutoLayoutPlaceholder(XmlPlaceholderTitle, pInfo->GetTitleRectangle());
                        ImpWriteAutoLayoutPlaceholder(XmlPlaceholderObject, aTopLeft);
                        ImpWriteAutoLayoutPlaceholder(XmlPlaceholderObject, aTopRight);
                        ImpWriteAutoLayoutPlaceholder(XmlPlaceholderObject, aBottomLeft);
                        ImpWriteAutoLayoutPlaceholder(XmlPlaceholderObject, aBottomRight);
                        break;
                    }
                    case 19 : // AUTOLAYOUT_ONLY_TITLE
                    {
                        ImpWriteAutoLayoutPlaceholder(XmlPlaceholderTitle, pInfo->GetTitleRectangle());
                        break;
                    }
                    case 21 : // AUTOLAYOUT_NOTES
                    {
                        ImpWriteAutoLayoutPlaceholder(XmlPlaceholderPage, pInfo->GetTitleRectangle());
                        ImpWriteAutoLayoutPlaceholder(XmlPlaceholderNotes, pInfo->GetPresRectangle());
                        break;
                    }
                    case 22 : // AUTOLAYOUT_HANDOUT1
                    case 23 : // AUTOLAYOUT_HANDOUT2
                    case 24 : // AUTOLAYOUT_HANDOUT3
                    case 25 : // AUTOLAYOUT_HANDOUT4
                    case 26 : // AUTOLAYOUT_HANDOUT6
                    case 31 : // AUTOLAYOUT_HANDOUT9
                    {
                        sal_Int32 nColCnt, nRowCnt;
                        sal_Int32 nGapX = pInfo->GetGapX();
                        sal_Int32 nGapY = pInfo->GetGapY();

                        switch(pInfo->GetLayoutType())
                        {
                            case 22 : nColCnt = 1; nRowCnt = 1; break;
                            case 23 : nColCnt = 1; nRowCnt = 2; break;
                            case 24 : nColCnt = 1; nRowCnt = 3; break;
                            case 25 : nColCnt = 2; nRowCnt = 2; break;
                            case 26 : nColCnt = 3; nRowCnt = 2; break;
                            case 31 : nColCnt = 3; nRowCnt = 3; break;
                            default:  nColCnt = 0; nRowCnt = 0; break;  // FIXME - What is correct values?
                        }

                        Size aPartSize(pInfo->GetTitleRectangle().GetSize());
                        Point aPartPos(pInfo->GetTitleRectangle().TopLeft());

                        if(aPartSize.Width() > aPartSize.Height())
                        {
                            sal_Int32 nZwi(nColCnt);
                            nColCnt = nRowCnt;
                            nRowCnt = nZwi;
                        }

                        aPartSize.Width() = (aPartSize.Width() - ((nColCnt - 1) * nGapX)) / nColCnt;
                        aPartSize.Height() = (aPartSize.Height() - ((nRowCnt - 1) * nGapY)) / nRowCnt;

                        Point aTmpPos(aPartPos);

                        for (sal_Int32 a = 0; a < nRowCnt; a++)
                        {
                            aTmpPos.X() = aPartPos.X();

                            for (sal_Int32 b = 0; b < nColCnt; b++)
                            {
                                Rectangle aTmpRect(aTmpPos, aPartSize);

                                ImpWriteAutoLayoutPlaceholder(XmlPlaceholderHandout, aTmpRect);
                                aTmpPos.X() += aPartSize.Width() + nGapX;
                            }

                            aTmpPos.Y() += aPartSize.Height() + nGapY;
                        }
                        break;
                    }
                    case 27 : // AUTOLAYOUT_VERTICAL_TITLE_TEXT_CHART
                    {
                        Rectangle aTop(pInfo->GetPresRectangle());
                        aTop.setHeight(long(aTop.GetHeight() * 0.488));
                        Rectangle aBottom(aTop);
                        aBottom.Top() = long(aBottom.Top() + aBottom.GetHeight() * 1.05);

                        ImpWriteAutoLayoutPlaceholder(XmlPlaceholderVerticalTitle, pInfo->GetTitleRectangle());
                        ImpWriteAutoLayoutPlaceholder(XmlPlaceholderVerticalOutline, aTop);
                        ImpWriteAutoLayoutPlaceholder(XmlPlaceholderChart, aBottom);
                        break;
                    }
                    case 28 : // AUTOLAYOUT_VERTICAL_TITLE_VERTICAL_OUTLINE
                    {
                        ImpWriteAutoLayoutPlaceholder(XmlPlaceholderVerticalTitle, pInfo->GetTitleRectangle());
                        ImpWriteAutoLayoutPlaceholder(XmlPlaceholderVerticalOutline, pInfo->GetPresRectangle());
                        break;
                    }
                    case 29 : // AUTOLAYOUT_TITLE_VERTICAL_OUTLINE
                    {
                        ImpWriteAutoLayoutPlaceholder(XmlPlaceholderTitle, pInfo->GetTitleRectangle());
                        ImpWriteAutoLayoutPlaceholder(XmlPlaceholderVerticalOutline, pInfo->GetPresRectangle());
                        break;
                    }
                    case 30 : // AUTOLAYOUT_TITLE_VERTICAL_OUTLINE_CLIPART
                    {
                        Rectangle aLeft(pInfo->GetPresRectangle());
                        aLeft.setWidth(long(aLeft.GetWidth() * 0.488));
                        Rectangle aRight(aLeft);
                        aRight.Left() = long(aRight.Left() + aRight.GetWidth() * 1.05);

                        ImpWriteAutoLayoutPlaceholder(XmlPlaceholderTitle, pInfo->GetTitleRectangle());
                        ImpWriteAutoLayoutPlaceholder(XmlPlaceholderGraphic, aLeft);
                        ImpWriteAutoLayoutPlaceholder(XmlPlaceholderVerticalOutline, aRight);
                        break;
                    }
                    case 32 : // AUTOLAYOUT_TITLE
                    {
                        ImpWriteAutoLayoutPlaceholder(XmlPlaceholderSubtitle, pInfo->GetPresRectangle());
                        break;
                    }

                    case 33 : // AUTOLAYOUT_4CLIPART
                    {
                        Rectangle aTopLeft(pInfo->GetPresRectangle());
                        aTopLeft.setHeight(long(aTopLeft.GetHeight() * 0.477));
                        aTopLeft.setWidth(long(aTopLeft.GetWidth() * 0.488));
                        Rectangle aBottomLeft(aTopLeft);
                        aBottomLeft.Top() = long(aBottomLeft.Top() + aBottomLeft.GetHeight() * 1.095);
                        Rectangle aTopRight(aTopLeft);
                        aTopRight.Left() = long(aTopRight.Left() + aTopRight.GetWidth() * 1.05);
                        Rectangle aBottomRight(aTopRight);
                        aBottomRight.Top() = long(aBottomRight.Top() + aBottomRight.GetHeight() * 1.095);

                        ImpWriteAutoLayoutPlaceholder(XmlPlaceholderTitle, pInfo->GetTitleRectangle());
                        ImpWriteAutoLayoutPlaceholder(XmlPlaceholderGraphic, aTopLeft);
                        ImpWriteAutoLayoutPlaceholder(XmlPlaceholderGraphic, aTopRight);
                        ImpWriteAutoLayoutPlaceholder(XmlPlaceholderGraphic, aBottomLeft);
                        ImpWriteAutoLayoutPlaceholder(XmlPlaceholderGraphic, aBottomRight);
                        break;
                    }

                    case 34 : // AUTOLAYOUT_6CLIPART
                    {
                        Rectangle aTopLeft(pInfo->GetPresRectangle());
                        aTopLeft.setHeight(long(aTopLeft.GetHeight() * 0.477));
                        aTopLeft.setWidth(long(aTopLeft.GetWidth() * 0.322));
                        Rectangle aTopCenter(aTopLeft);
                        aTopCenter.Left() = long(aTopCenter.Left() + aTopCenter.GetWidth() * 1.05);
                        Rectangle aTopRight(aTopLeft);
                        aTopRight.Left() = long(aTopRight.Left() + aTopRight.GetWidth() * 2 * 1.05);

                        Rectangle aBottomLeft(aTopLeft);
                        aBottomLeft.Top() = long(aBottomLeft.Top() + aBottomLeft.GetHeight() * 1.095);
                        Rectangle aBottomCenter(aTopCenter);
                        aBottomCenter.Top() = long(aBottomCenter.Top() + aBottomCenter.GetHeight() * 1.095);
                        Rectangle aBottomRight(aTopRight);
                        aBottomRight.Top() = long(aBottomRight.Top() + aBottomRight.GetHeight() * 1.095);

                        ImpWriteAutoLayoutPlaceholder(XmlPlaceholderTitle, pInfo->GetTitleRectangle());
                        ImpWriteAutoLayoutPlaceholder(XmlPlaceholderGraphic, aTopLeft);
                        ImpWriteAutoLayoutPlaceholder(XmlPlaceholderGraphic, aTopCenter);
                        ImpWriteAutoLayoutPlaceholder(XmlPlaceholderGraphic, aTopRight);
                        ImpWriteAutoLayoutPlaceholder(XmlPlaceholderGraphic, aBottomLeft);
                        ImpWriteAutoLayoutPlaceholder(XmlPlaceholderGraphic, aBottomCenter);
                        ImpWriteAutoLayoutPlaceholder(XmlPlaceholderGraphic, aBottomRight);
                        break;
                    }
                    default:
                    {
                        OSL_FAIL("XMLEXP: unknown autolayout export");
                        break;
                    }
                }
            }
        }
    }
}

//////////////////////////////////////////////////////////////////////////////

void SdXMLExport::ImpWriteAutoLayoutPlaceholder(XmlPlaceholder ePl, const Rectangle& rRect)
{
    OUString aStr;
    OUStringBuffer sStringBuffer;

    // prepare presentation-placeholder attributes, presentation:object
    switch(ePl)
    {
        case XmlPlaceholderTitle: aStr = OUString(RTL_CONSTASCII_USTRINGPARAM("title")); break;
        case XmlPlaceholderOutline: aStr = OUString(RTL_CONSTASCII_USTRINGPARAM("outline")); break;
        case XmlPlaceholderSubtitle: aStr = OUString(RTL_CONSTASCII_USTRINGPARAM("subtitle")); break;
        case XmlPlaceholderText: aStr = OUString(RTL_CONSTASCII_USTRINGPARAM("text")); break;
        case XmlPlaceholderGraphic: aStr = OUString(RTL_CONSTASCII_USTRINGPARAM("graphic")); break;
        case XmlPlaceholderObject: aStr = OUString(RTL_CONSTASCII_USTRINGPARAM("object")); break;
        case XmlPlaceholderChart: aStr = OUString(RTL_CONSTASCII_USTRINGPARAM("chart")); break;
        case XmlPlaceholderOrgchart: aStr = OUString(RTL_CONSTASCII_USTRINGPARAM("orgchart")); break;
        case XmlPlaceholderTable: aStr = OUString(RTL_CONSTASCII_USTRINGPARAM("table")); break;
        case XmlPlaceholderPage: aStr = OUString(RTL_CONSTASCII_USTRINGPARAM("page")); break;
        case XmlPlaceholderNotes: aStr = OUString(RTL_CONSTASCII_USTRINGPARAM("notes")); break;
        case XmlPlaceholderHandout: aStr = OUString(RTL_CONSTASCII_USTRINGPARAM("handout")); break;
        case XmlPlaceholderVerticalTitle: aStr = OUString(RTL_CONSTASCII_USTRINGPARAM("vertical_title")); break;
        case XmlPlaceholderVerticalOutline: aStr = OUString(RTL_CONSTASCII_USTRINGPARAM("vertical_outline")); break;
    }

    AddAttribute(XML_NAMESPACE_PRESENTATION, XML_OBJECT, aStr);

    // svg:x,y,width,height
    GetMM100UnitConverter().convertMeasureToXML(sStringBuffer, rRect.Left());
    aStr = sStringBuffer.makeStringAndClear();
    AddAttribute(XML_NAMESPACE_SVG, XML_X, aStr);

    GetMM100UnitConverter().convertMeasureToXML(sStringBuffer, rRect.Top());
    aStr = sStringBuffer.makeStringAndClear();
    AddAttribute(XML_NAMESPACE_SVG, XML_Y, aStr);

    GetMM100UnitConverter().convertMeasureToXML(sStringBuffer,
            rRect.GetWidth());
    aStr = sStringBuffer.makeStringAndClear();
    AddAttribute(XML_NAMESPACE_SVG, XML_WIDTH, aStr);

    GetMM100UnitConverter().convertMeasureToXML(sStringBuffer,
            rRect.GetHeight());
    aStr = sStringBuffer.makeStringAndClear();
    AddAttribute(XML_NAMESPACE_SVG, XML_HEIGHT, aStr);

    // write presentation-placeholder
    SvXMLElementExport aPPL(*this, XML_NAMESPACE_PRESENTATION, XML_PLACEHOLDER, sal_True, sal_True);
}

//////////////////////////////////////////////////////////////////////////////

ImpXMLEXPPageMasterInfo* SdXMLExport::ImpGetOrCreatePageMasterInfo( Reference< XDrawPage > xMasterPage )
{
    bool bDoesExist = false;

    ImpXMLEXPPageMasterInfo* pNewInfo = new ImpXMLEXPPageMasterInfo(*this, xMasterPage);

    // compare with prev page-master infos
    for( size_t a = 0; !bDoesExist && a < mpPageMasterInfoList->size(); a++)
    {
        if (   mpPageMasterInfoList->at(a)
           && *mpPageMasterInfoList->at(a) == *pNewInfo
           )
        {
            delete pNewInfo;
            pNewInfo = mpPageMasterInfoList->at(a);
            bDoesExist = true;
        }
    }
    // add entry when not found same page-master infos
    if(!bDoesExist)
        mpPageMasterInfoList->push_back( pNewInfo );

    return pNewInfo;
}

void SdXMLExport::ImpPrepPageMasterInfos()
{
    if( IsImpress() )
    {
        // create page master info for handout master page

        Reference< XHandoutMasterSupplier > xHMS( GetModel(), UNO_QUERY );
        if( xHMS.is() )
        {
            Reference< XDrawPage > xMasterPage( xHMS->getHandoutMasterPage() );
            if( xMasterPage.is() )
                mpHandoutPageMaster = ImpGetOrCreatePageMasterInfo(xMasterPage);
        }
    }

    // create page master infos for master pages
    if(mnDocMasterPageCount)
    {
        // look for needed page-masters, create these
        for (sal_Int32 nMPageId = 0; nMPageId < mnDocMasterPageCount; nMPageId++)
        {
            Reference< XDrawPage > xMasterPage( mxDocMasterPages->getByIndex(nMPageId), UNO_QUERY );
            ImpXMLEXPPageMasterInfo* pNewInfo = 0L;

            if(xMasterPage.is())
                pNewInfo = ImpGetOrCreatePageMasterInfo(xMasterPage);

            mpPageMasterUsageList->push_back( pNewInfo );

            // look for page master of handout page
            if(IsImpress())
            {
                pNewInfo = NULL;
                Reference< presentation::XPresentationPage > xPresPage(xMasterPage, UNO_QUERY);
                if(xPresPage.is())
                {
                    Reference< XDrawPage > xNotesPage(xPresPage->getNotesPage());
                    if(xNotesPage.is())
                    {
                        pNewInfo = ImpGetOrCreatePageMasterInfo(xNotesPage);
                    }
                }
                mpNotesPageMasterUsageList->push_back( pNewInfo );
            }
        }
    }
}

//////////////////////////////////////////////////////////////////////////////

void SdXMLExport::ImpWritePageMasterInfos()
{
    // write created page-masters, create names for these
    for( size_t nCnt = 0; nCnt < mpPageMasterInfoList->size(); nCnt++)
    {
        ImpXMLEXPPageMasterInfo* pInfo = mpPageMasterInfoList->at(nCnt);
        if(pInfo)
        {
            // create name
            OUString sNewName = OUString(RTL_CONSTASCII_USTRINGPARAM("PM"));

            sNewName += OUString::valueOf((sal_Int32)nCnt);
            pInfo->SetName(sNewName);

            // prepare page-master attributes
            OUString sString;
            OUStringBuffer sStringBuffer;

            sString = sNewName;
            AddAttribute(XML_NAMESPACE_STYLE, XML_NAME, sString);

            // write page-layout
            SvXMLElementExport aPME(*this, XML_NAMESPACE_STYLE, XML_PAGE_LAYOUT, sal_True, sal_True);

            // prepare style:properties inside page-master
            GetMM100UnitConverter().convertMeasureToXML(sStringBuffer,
                    pInfo->GetBorderTop());
            sString = sStringBuffer.makeStringAndClear();
            AddAttribute(XML_NAMESPACE_FO, XML_MARGIN_TOP, sString);

            GetMM100UnitConverter().convertMeasureToXML(sStringBuffer,
                    pInfo->GetBorderBottom());
            sString = sStringBuffer.makeStringAndClear();
            AddAttribute(XML_NAMESPACE_FO, XML_MARGIN_BOTTOM, sString);

            GetMM100UnitConverter().convertMeasureToXML(sStringBuffer,
                    pInfo->GetBorderLeft());
            sString = sStringBuffer.makeStringAndClear();
            AddAttribute(XML_NAMESPACE_FO, XML_MARGIN_LEFT, sString);

            GetMM100UnitConverter().convertMeasureToXML(sStringBuffer,
                    pInfo->GetBorderRight());
            sString = sStringBuffer.makeStringAndClear();
            AddAttribute(XML_NAMESPACE_FO, XML_MARGIN_RIGHT, sString);

            GetMM100UnitConverter().convertMeasureToXML(sStringBuffer,
                    pInfo->GetWidth());
            sString = sStringBuffer.makeStringAndClear();
            AddAttribute(XML_NAMESPACE_FO, XML_PAGE_WIDTH, sString);

            GetMM100UnitConverter().convertMeasureToXML(sStringBuffer,
                    pInfo->GetHeight());
            sString = sStringBuffer.makeStringAndClear();
            AddAttribute(XML_NAMESPACE_FO, XML_PAGE_HEIGHT, sString);

            if(pInfo->GetOrientation() == view::PaperOrientation_PORTRAIT)
                AddAttribute(XML_NAMESPACE_STYLE, XML_PRINT_ORIENTATION, XML_PORTRAIT);
            else
                AddAttribute(XML_NAMESPACE_STYLE, XML_PRINT_ORIENTATION, XML_LANDSCAPE);

            // write style:properties
            SvXMLElementExport aPMF(*this, XML_NAMESPACE_STYLE, XML_PAGE_LAYOUT_PROPERTIES, sal_True, sal_True);
        }
    }
}

//////////////////////////////////////////////////////////////////////////////

ImpXMLEXPPageMasterInfo* SdXMLExport::ImpGetPageMasterInfoByName(const OUString& rName)
{
    if(!rName.isEmpty() && !mpPageMasterInfoList->empty())
    {
        for( size_t nCnt = 0; nCnt < mpPageMasterInfoList->size(); nCnt++)
        {
            ImpXMLEXPPageMasterInfo* pInfo = mpPageMasterInfoList->at(nCnt);
            if(pInfo)
            {
                if(!pInfo->GetMasterPageName().isEmpty() && rName.equals(pInfo->GetMasterPageName()))
                {
                    return pInfo;
                }
            }
        }
    }
    return 0L;
}

//////////////////////////////////////////////////////////////////////////////

void SdXMLExport::ImpPrepDrawPageInfos()
{
    // create draw:style-name entries for page export
    // containing presentation page attributes AND background attributes
    // fixed family for page-styles is "drawing-page" (XML_STYLE_FAMILY_SD_DRAWINGPAGE_NAME)

    sal_Int32 nCnt;
    for(nCnt = 0; nCnt < mnDocDrawPageCount; nCnt++)
    {
        Reference<XDrawPage> xDrawPage;
        mxDocDrawPages->getByIndex(nCnt) >>= xDrawPage;
        maDrawPagesStyleNames[nCnt] = ImpCreatePresPageStyleName( xDrawPage );

        Reference< presentation::XPresentationPage > xPresPage(xDrawPage, UNO_QUERY);
        if(xPresPage.is())
        {
            maDrawNotesPagesStyleNames[nCnt] = ImpCreatePresPageStyleName( xPresPage->getNotesPage(), false );

            maDrawPagesHeaderFooterSettings[nCnt] = ImpPrepDrawPageHeaderFooterDecls( xDrawPage );
            maDrawNotesPagesHeaderFooterSettings[nCnt] = ImpPrepDrawPageHeaderFooterDecls( xPresPage->getNotesPage() );
        }
    }
}

//////////////////////////////////////////////////////////////////////////////

static OUString findOrAppendImpl( std::vector< OUString >& rVector, const OUString& rText, const sal_Char* pPrefix )
{
    // search rVector if there is already a string that equals rText
    std::vector< OUString >::iterator aIter;
    sal_Int32 nIndex;
    for( nIndex = 1, aIter = rVector.begin(); aIter != rVector.end(); ++aIter, ++nIndex )
    {
        if( (*aIter) == rText )
            break;
    }

    // if nothing is found, append the string at the end of rVector
    if( aIter == rVector.end() )
        rVector.push_back( rText );

    // create a reference string with pPrefix and the index of the
    // found or created rText
    OUString aStr( OUString::createFromAscii( pPrefix ) );
    aStr += OUString::valueOf( nIndex );
    return aStr;
}

static OUString findOrAppendImpl( std::vector< DateTimeDeclImpl >& rVector, const OUString& rText, sal_Bool bFixed, sal_Int32 nFormat, const sal_Char* pPrefix )
{
    // search rVector if there is already a DateTimeDeclImpl with rText,bFixed and nFormat
    std::vector< DateTimeDeclImpl >::iterator aIter;
    sal_Int32 nIndex;
    for( nIndex = 1, aIter = rVector.begin(); aIter != rVector.end(); ++aIter, ++nIndex )
    {
        const DateTimeDeclImpl& rDecl = (*aIter);
        if( (rDecl.mbFixed == bFixed ) &&
            (!bFixed || rDecl.maStrText == rText) &&
            (bFixed || (rDecl.mnFormat == nFormat) ) )
            break;
    }

    // if nothing is found, append a new DateTimeDeclImpl
    if( aIter == rVector.end() )
    {
        DateTimeDeclImpl aDecl;
        aDecl.maStrText = rText;
        aDecl.mbFixed = bFixed;
        aDecl.mnFormat = nFormat;
        rVector.push_back( aDecl );
    }

    // create a reference string with pPrefix and the index of the
    // found or created DateTimeDeclImpl
    OUString aStr( OUString::createFromAscii( pPrefix ) );
    aStr += OUString::valueOf( nIndex );
    return aStr;

}

static const sal_Char* gpStrHeaderTextPrefix = "hdr";
static const sal_Char* gpStrFooterTextPrefix = "ftr";
static const sal_Char* gpStrDateTimeTextPrefix = "dtd";

HeaderFooterPageSettingsImpl SdXMLExport::ImpPrepDrawPageHeaderFooterDecls( const Reference<XDrawPage>& xDrawPage )
{
    HeaderFooterPageSettingsImpl aSettings;

    if( xDrawPage.is() ) try
    {
        Reference< XPropertySet > xSet( xDrawPage, UNO_QUERY_THROW );
        Reference< XPropertySetInfo > xInfo( xSet->getPropertySetInfo() );

        OUString aStrText;

        const OUString aStrHeaderTextProp( RTL_CONSTASCII_USTRINGPARAM( "HeaderText" ) );
        if( xInfo->hasPropertyByName( aStrHeaderTextProp ) )
        {
            xSet->getPropertyValue( aStrHeaderTextProp  ) >>= aStrText;
            if( !aStrText.isEmpty() )
                aSettings.maStrHeaderDeclName = findOrAppendImpl( maHeaderDeclsVector, aStrText, gpStrHeaderTextPrefix );
        }

        const OUString aStrFooterTextProp( RTL_CONSTASCII_USTRINGPARAM( "FooterText" ) );
        if( xInfo->hasPropertyByName( aStrFooterTextProp ) )
        {
            xSet->getPropertyValue( aStrFooterTextProp ) >>= aStrText;
            if( !aStrText.isEmpty() )
                aSettings.maStrFooterDeclName = findOrAppendImpl( maFooterDeclsVector, aStrText, gpStrFooterTextPrefix );
        }

        const OUString aStrDateTimeTextProp( RTL_CONSTASCII_USTRINGPARAM( "DateTimeText" ) );
        if( xInfo->hasPropertyByName( aStrDateTimeTextProp ) )
        {
            sal_Bool bFixed = false;
            sal_Int32 nFormat = 0;
            xSet->getPropertyValue( aStrDateTimeTextProp ) >>= aStrText;
            xSet->getPropertyValue( OUString( RTL_CONSTASCII_USTRINGPARAM( "IsDateTimeFixed" ) ) ) >>= bFixed;
            xSet->getPropertyValue( OUString( RTL_CONSTASCII_USTRINGPARAM( "DateTimeFormat" ) ) ) >>= nFormat;

            if( !bFixed || !aStrText.isEmpty() )
            {
                aSettings.maStrDateTimeDeclName = findOrAppendImpl( maDateTimeDeclsVector, aStrText, bFixed, nFormat, gpStrDateTimeTextPrefix );
                if( !bFixed )
                    addDataStyle( nFormat );
            }
        }
    }
    catch(const Exception&)
    {
        OSL_FAIL( "SdXMLExport::ImpPrepDrawPageHeaderFooterDecls(), unexpected exception cought!" );
    }

    return aSettings;
}

//////////////////////////////////////////////////////////////////////////////

void SdXMLExport::ImpWriteHeaderFooterDecls()
{
    OUStringBuffer sBuffer;

    if( !maHeaderDeclsVector.empty() )
    {
        // export header decls
        const OUString aPrefix( OUString::createFromAscii( gpStrHeaderTextPrefix ) );
        std::vector< OUString >::iterator aIter;
        sal_Int32 nIndex;
        for( nIndex = 1, aIter = maHeaderDeclsVector.begin(); aIter != maHeaderDeclsVector.end(); ++aIter, ++nIndex )
        {
            sBuffer.append( aPrefix );
            sBuffer.append( nIndex );
            AddAttribute(XML_NAMESPACE_PRESENTATION, XML_NAME, sBuffer.makeStringAndClear());

            SvXMLElementExport aElem(*this, XML_NAMESPACE_PRESENTATION, XML_HEADER_DECL, sal_True, sal_True);
            Characters((*aIter));
        }
    }

    if( !maFooterDeclsVector.empty() )
    {
        // export footer decls
        const OUString aPrefix( OUString::createFromAscii( gpStrFooterTextPrefix ) );
        std::vector< OUString >::iterator aIter;
        sal_Int32 nIndex;
        for( nIndex = 1, aIter = maFooterDeclsVector.begin(); aIter != maFooterDeclsVector.end(); ++aIter, ++nIndex )
        {
            sBuffer.append( aPrefix );
            sBuffer.append( nIndex );
            AddAttribute(XML_NAMESPACE_PRESENTATION, XML_NAME, sBuffer.makeStringAndClear());

            SvXMLElementExport aElem(*this, XML_NAMESPACE_PRESENTATION, XML_FOOTER_DECL, sal_False, sal_False);
            Characters((*aIter));
        }
    }

    if( !maDateTimeDeclsVector.empty() )
    {
        // export footer decls
        const OUString aPrefix( OUString::createFromAscii( gpStrDateTimeTextPrefix ) );
        std::vector< DateTimeDeclImpl >::iterator aIter;
        sal_Int32 nIndex;
        for( nIndex = 1, aIter = maDateTimeDeclsVector.begin(); aIter != maDateTimeDeclsVector.end(); ++aIter, ++nIndex )
        {
            const DateTimeDeclImpl& rDecl = (*aIter);

            sBuffer.append( aPrefix );
            sBuffer.append( nIndex );
            AddAttribute( XML_NAMESPACE_PRESENTATION, XML_NAME, sBuffer.makeStringAndClear());

            AddAttribute( XML_NAMESPACE_PRESENTATION, XML_SOURCE, rDecl.mbFixed ? XML_FIXED : XML_CURRENT_DATE );

            if( !rDecl.mbFixed )
                AddAttribute( XML_NAMESPACE_STYLE, XML_DATA_STYLE_NAME, getDataStyleName( rDecl.mnFormat ) );

            SvXMLElementExport aElem(*this, XML_NAMESPACE_PRESENTATION, XML_DATE_TIME_DECL, sal_False, sal_False);
            if( rDecl.mbFixed )
                Characters(rDecl.maStrText);
        }
    }
}

void SdXMLExport::ImplExportHeaderFooterDeclAttributes( const HeaderFooterPageSettingsImpl& aSettings )
{
    if( !aSettings.maStrHeaderDeclName.isEmpty() )
        AddAttribute( XML_NAMESPACE_PRESENTATION, XML_USE_HEADER_NAME, aSettings.maStrHeaderDeclName );

    if( !aSettings.maStrFooterDeclName.isEmpty() )
        AddAttribute( XML_NAMESPACE_PRESENTATION, XML_USE_FOOTER_NAME, aSettings.maStrFooterDeclName );

    if( !aSettings.maStrDateTimeDeclName.isEmpty() )
        AddAttribute( XML_NAMESPACE_PRESENTATION, XML_USE_DATE_TIME_NAME, aSettings.maStrDateTimeDeclName );
}

//////////////////////////////////////////////////////////////////////////////

OUString SdXMLExport::ImpCreatePresPageStyleName( Reference<XDrawPage> xDrawPage, bool bExportBackground /* = true */ )
{
    // create name
    OUString sStyleName;

    // create style for this page and add to auto style pool

    Reference< beans::XPropertySet > xPropSet1(xDrawPage, UNO_QUERY);
    if(xPropSet1.is())
    {
        Reference< beans::XPropertySet > xPropSet;

        if( bExportBackground )
        {
            // since the background items are in a different propertyset
            // which itself is a property of the pages property set
            // we now merge these two propertysets if possible to simulate
            // a single propertyset with all draw page properties
            const OUString aBackground(RTL_CONSTASCII_USTRINGPARAM("Background"));
            Reference< beans::XPropertySet > xPropSet2;
            Reference< beans::XPropertySetInfo > xInfo( xPropSet1->getPropertySetInfo() );
            if( xInfo.is() && xInfo->hasPropertyByName( aBackground ) )
            {
                Any aAny( xPropSet1->getPropertyValue( aBackground ) );
                aAny >>= xPropSet2;
            }

            if( xPropSet2.is() )
                xPropSet = PropertySetMerger_CreateInstance( xPropSet1, xPropSet2 );
            else
                xPropSet = xPropSet1;
        }
        else
        {
            xPropSet = xPropSet1;
        }

        const UniReference< SvXMLExportPropertyMapper > aMapperRef( GetPresPagePropsMapper() );

        std::vector< XMLPropertyState > xPropStates( aMapperRef->Filter( xPropSet ) );

        if( !xPropStates.empty() )
        {
            // there are filtered properties -> hard attributes
            // try to find this style in AutoStylePool
            sStyleName = GetAutoStylePool()->Find(XML_STYLE_FAMILY_SD_DRAWINGPAGE_ID, sStyleName, xPropStates);

            if(sStyleName.isEmpty())
            {
                // Style did not exist, add it to AutoStalePool
                sStyleName = GetAutoStylePool()->Add(XML_STYLE_FAMILY_SD_DRAWINGPAGE_ID, sStyleName, xPropStates);
            }
        }
    }

    return sStyleName;
}

//////////////////////////////////////////////////////////////////////////////

void SdXMLExport::ImpPrepMasterPageInfos()
{
    // create draw:style-name entries for master page export
    // containing only background attributes
    // fixed family for page-styles is "drawing-page" (XML_STYLE_FAMILY_SD_DRAWINGPAGE_NAME)

    sal_Int32 nCnt;
    for( nCnt = 0; nCnt < mnDocMasterPageCount; nCnt++)
    {
        Reference<XDrawPage> xDrawPage;
        mxDocMasterPages->getByIndex(nCnt) >>= xDrawPage;
        maMasterPagesStyleNames[nCnt] = ImpCreatePresPageStyleName( xDrawPage );
    }

    if( IsImpress() )
    {
        Reference< presentation::XHandoutMasterSupplier > xHandoutSupp( GetModel(), UNO_QUERY );
        if( xHandoutSupp.is() )
        {
            Reference< XDrawPage > xHandoutPage( xHandoutSupp->getHandoutMasterPage() );
            if( xHandoutPage.is() )
            {
                maHandoutPageHeaderFooterSettings = ImpPrepDrawPageHeaderFooterDecls( xHandoutPage );
                maHandoutMasterStyleName = ImpCreatePresPageStyleName( xHandoutPage, false );
            }
        }
    }
}

//////////////////////////////////////////////////////////////////////////////
void SdXMLExport::ImpWritePresentationStyles()
{
    if(IsImpress())
    {
        for (sal_Int32 nCnt = 0; nCnt < mnDocMasterPageCount; nCnt++)
        {
            Any aAny(mxDocMasterPages->getByIndex(nCnt));
            Reference<container::XNamed> xNamed;

            if(aAny >>= xNamed)
            {
                // write presentation styles (ONLY if presentation)
                if(IsImpress() && mxDocStyleFamilies.is() && xNamed.is())
                {
                    XMLStyleExport aStEx(*this, OUString(), GetAutoStylePool().get());
                    const UniReference< SvXMLExportPropertyMapper > aMapperRef( GetPropertySetMapper() );

                    OUString aPrefix( xNamed->getName() );

                    aPrefix += OUString(RTL_CONSTASCII_USTRINGPARAM("-"));
                    aStEx.exportStyleFamily(xNamed->getName(),
                        OUString(RTL_CONSTASCII_USTRINGPARAM(XML_STYLE_FAMILY_SD_PRESENTATION_NAME)),
                        aMapperRef, sal_False,
                        XML_STYLE_FAMILY_SD_PRESENTATION_ID, &aPrefix);
                }
            }
        }
    }
}

//////////////////////////////////////////////////////////////////////////////

void SdXMLExport::SetProgress(sal_Int32 nProg)
{
    // set progress view
    if(GetStatusIndicator().is())
        GetStatusIndicator()->setValue(nProg);
}

//////////////////////////////////////////////////////////////////////////////
// #82003#

void SdXMLExport::_ExportMeta()
{
    uno::Sequence<beans::NamedValue> stats(1);
    stats[0] = beans::NamedValue(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "ObjectCount" )),
                uno::makeAny(mnObjectCount));

    // update document statistics at the model
    uno::Reference<document::XDocumentPropertiesSupplier> xPropSup(GetModel(),
        uno::UNO_QUERY_THROW);
    uno::Reference<document::XDocumentProperties> xDocProps(
        xPropSup->getDocumentProperties());
    if (xDocProps.is()) {
        xDocProps->setDocumentStatistics(stats);
    }

    // call parent
    SvXMLExport::_ExportMeta();
}

//////////////////////////////////////////////////////////////////////////////

void SdXMLExport::_ExportContent()
{
    // export <pres:header-decl>, <pres:footer-decl> and <pres:date-time-decl> elements
    ImpWriteHeaderFooterDecls();

    // page export
    for(sal_Int32 nPageInd(0); nPageInd < mnDocDrawPageCount; nPageInd++)
    {
        uno::Reference<drawing::XDrawPage> xDrawPage( mxDocDrawPages->getByIndex(nPageInd), uno::UNO_QUERY );

        SetProgress(((nPageInd + 1) * 100) / mnDocDrawPageCount);

        if(xDrawPage.is())
        {
            // prepare page attributes, name of page
            Reference < container::XNamed > xNamed(xDrawPage, UNO_QUERY);
            if(xNamed.is())
                AddAttribute(XML_NAMESPACE_DRAW, XML_NAME, xNamed->getName());

            // draw:style-name (presentation page attributes AND background attributes)
            if( !maDrawPagesStyleNames[nPageInd].isEmpty() )
                AddAttribute(XML_NAMESPACE_DRAW, XML_STYLE_NAME,
                        maDrawPagesStyleNames[nPageInd]);

            // draw:master-page-name
            Reference < drawing::XMasterPageTarget > xMasterPageInt(xDrawPage, UNO_QUERY);
            if(xMasterPageInt.is())
            {
                Reference<XDrawPage> xUsedMasterPage(xMasterPageInt->getMasterPage());
                if(xUsedMasterPage.is())
                {
                    Reference < container::XNamed > xMasterNamed(xUsedMasterPage, UNO_QUERY);
                    if(xMasterNamed.is())
                    {
                        AddAttribute(XML_NAMESPACE_DRAW, XML_MASTER_PAGE_NAME,
                            EncodeStyleName( xMasterNamed->getName()) );
                    }
                }
            }

            // presentation:page-layout-name
            if( IsImpress() && !maDrawPagesAutoLayoutNames[nPageInd+1].isEmpty())
            {
                AddAttribute(XML_NAMESPACE_PRESENTATION, XML_PRESENTATION_PAGE_LAYOUT_NAME, maDrawPagesAutoLayoutNames[nPageInd+1] );
            }

            Reference< beans::XPropertySet > xProps( xDrawPage, UNO_QUERY );
            if( xProps.is() )
            {
                try
                {
                    OUString aBookmarkURL;
                    xProps->getPropertyValue( OUString( RTL_CONSTASCII_USTRINGPARAM( "BookmarkURL" ) ) ) >>= aBookmarkURL;

                    if( !aBookmarkURL.isEmpty() )
                    {
                        sal_Int32 nIndex = aBookmarkURL.lastIndexOf( (sal_Unicode)'#' );
                        if( nIndex != -1 )
                        {
                            OUString aFileName( aBookmarkURL.copy( 0, nIndex ) );
                            OUString aBookmarkName( aBookmarkURL.copy( nIndex+1 ) );

                            aBookmarkURL = GetRelativeReference( aFileName );
                            aBookmarkURL += rtl::OUString(static_cast<sal_Unicode>('#'));
                            aBookmarkURL += aBookmarkName;
                        }

                        AddAttribute ( XML_NAMESPACE_XLINK, XML_HREF, aBookmarkURL);
                        AddAttribute ( XML_NAMESPACE_XLINK, XML_TYPE, XML_SIMPLE );
                        AddAttribute ( XML_NAMESPACE_XLINK, XML_SHOW, XML_REPLACE );
                        AddAttribute ( XML_NAMESPACE_XLINK, XML_ACTUATE, XML_ONREQUEST );
                    }
                }
                catch(const Exception&)
                {
                    OSL_FAIL(" no \"BookmarkURL\" property at page?" );
                }
            }

            if( IsImpress() )
                ImplExportHeaderFooterDeclAttributes( maDrawPagesHeaderFooterSettings[nPageInd] );


            OUString sNavigationOrder( getNavigationOrder( xDrawPage ) );
            if( !sNavigationOrder.isEmpty() )
                AddAttribute ( XML_NAMESPACE_DRAW, XML_NAV_ORDER, sNavigationOrder );

            UniReference< xmloff::AnimationsExporter >  xAnimationsExporter;
            uno::Reference< ::com::sun::star::animations::XAnimationNodeSupplier > xAnimNodeSupplier;

            // prepare animation export
            if(IsImpress())
            {
                if( getExportFlags() & EXPORT_OASIS )
                {
                    // export new animations for oasis format
                    xAnimNodeSupplier.set( xDrawPage, UNO_QUERY );

                    // prepare animations exporter if impress
                    if(xAnimNodeSupplier.is())
                    {
                        xAnimationsExporter = new xmloff::AnimationsExporter( *this, xProps );
                        xAnimationsExporter->prepare( xAnimNodeSupplier->getAnimationNode() );
                    }
                }
                else
                {
                    // export old animations for ooo format
                    UniReference< XMLAnimationsExporter > xAnimExport = new XMLAnimationsExporter( GetShapeExport().get() );
                    GetShapeExport()->setAnimationsExporter( xAnimExport );
                }
            }

            // write draw:id
            const OUString aPageId = getInterfaceToIdentifierMapper().getIdentifier( xDrawPage );
            if( !aPageId.isEmpty() )
            {
                AddAttributeIdLegacy(XML_NAMESPACE_DRAW, aPageId);
            }

            // write page
            SvXMLElementExport aDPG(*this, XML_NAMESPACE_DRAW, XML_PAGE, sal_True, sal_True);

            // write optional office:forms
            exportFormsElement( xDrawPage );

            // write graphic objects on this page (if any)
            Reference< drawing::XShapes > xExportShapes(xDrawPage, UNO_QUERY);
            if(xExportShapes.is() && xExportShapes->getCount())
                GetShapeExport()->exportShapes( xExportShapes );

            // write animations and presentation notes (ONLY if presentation)
            if(IsImpress())
            {
                if(xAnimNodeSupplier.is())
                {
                    xAnimationsExporter->exportAnimations( xAnimNodeSupplier->getAnimationNode() );
                }
                else
                {
                    // animations
                    UniReference< XMLAnimationsExporter > xAnimExport( GetShapeExport()->getAnimationsExporter() );
                    if( xAnimExport.is() )
                        xAnimExport->exportAnimations( *this );

                    xAnimExport = NULL;
                    GetShapeExport()->setAnimationsExporter( xAnimExport );
                }

                // presentations
                Reference< presentation::XPresentationPage > xPresPage(xDrawPage, UNO_QUERY);
                if(xPresPage.is())
                {
                    Reference< XDrawPage > xNotesPage(xPresPage->getNotesPage());
                    if(xNotesPage.is())
                    {
                        Reference< drawing::XShapes > xShapes(xNotesPage, UNO_QUERY);
                        if(xShapes.is())
                        {
                            if( !maDrawNotesPagesStyleNames[nPageInd].isEmpty() )
                                AddAttribute(XML_NAMESPACE_DRAW, XML_STYLE_NAME, maDrawNotesPagesStyleNames[nPageInd]);

                            ImplExportHeaderFooterDeclAttributes( maDrawNotesPagesHeaderFooterSettings[nPageInd] );

                            // write presentation notes
                            SvXMLElementExport aPSY(*this, XML_NAMESPACE_PRESENTATION, XML_NOTES, sal_True, sal_True);

                            // write optional office:forms
                            exportFormsElement( xNotesPage );

                            // write shapes per se
                            GetShapeExport()->exportShapes( xShapes );
                        }
                    }
                }
            }

            exportAnnotations( xDrawPage );
        }
    }

    if( IsImpress() )
        exportPresentationSettings();
}

//////////////////////////////////////////////////////////////////////////////

void SdXMLExport::exportPresentationSettings()
{
    try
    {
        Reference< XPresentationSupplier > xPresSupplier( GetModel(), UNO_QUERY );
        if( !xPresSupplier.is() )
            return;

        Reference< XPropertySet > xPresProps( xPresSupplier->getPresentation(), UNO_QUERY );
        if( !xPresProps.is() )
            return;

        sal_Bool bHasAttr = sal_False;

        sal_Bool bTemp = false;

        // export range
        xPresProps->getPropertyValue( OUString( RTL_CONSTASCII_USTRINGPARAM( "IsShowAll" ) ) ) >>= bTemp;
        if( !bTemp )
        {
            OUString aFirstPage;
            xPresProps->getPropertyValue( OUString( RTL_CONSTASCII_USTRINGPARAM( "FirstPage" ) ) ) >>= aFirstPage;
            if( !aFirstPage.isEmpty() )
            {
                AddAttribute(XML_NAMESPACE_PRESENTATION, XML_START_PAGE, aFirstPage );
                bHasAttr = sal_True;
            }
            else
            {
                OUString aCustomShow;
                xPresProps->getPropertyValue( OUString( RTL_CONSTASCII_USTRINGPARAM( "CustomShow" ) ) ) >>= aCustomShow;
                if( !aCustomShow.isEmpty() )
                {
                    AddAttribute(XML_NAMESPACE_PRESENTATION, XML_SHOW, aCustomShow );
                    bHasAttr = sal_True;
                }
            }
        }

        xPresProps->getPropertyValue( OUString( RTL_CONSTASCII_USTRINGPARAM( "IsEndless" ) ) ) >>= bTemp;
        if( bTemp )
        {
            AddAttribute(XML_NAMESPACE_PRESENTATION, XML_ENDLESS, XML_TRUE );
            bHasAttr = sal_True;

            sal_Int32 nPause = 0;
            xPresProps->getPropertyValue( OUString( RTL_CONSTASCII_USTRINGPARAM( "Pause" ) ) ) >>= nPause;

            util::Duration aDuration;
            aDuration.Seconds = static_cast<sal_uInt16>(nPause);

            OUStringBuffer aOut;
            ::sax::Converter::convertDuration(aOut, aDuration);
            AddAttribute(XML_NAMESPACE_PRESENTATION, XML_PAUSE, aOut.makeStringAndClear() );
        }

        xPresProps->getPropertyValue( OUString( RTL_CONSTASCII_USTRINGPARAM( "AllowAnimations" ) ) ) >>= bTemp;
        if( !bTemp )
        {
            AddAttribute(XML_NAMESPACE_PRESENTATION, XML_ANIMATIONS, XML_DISABLED );
            bHasAttr = sal_True;
        }

        xPresProps->getPropertyValue( OUString( RTL_CONSTASCII_USTRINGPARAM( "IsAlwaysOnTop" ) ) ) >>= bTemp;
        if( bTemp )
        {
            AddAttribute(XML_NAMESPACE_PRESENTATION, XML_STAY_ON_TOP, XML_TRUE );
            bHasAttr = sal_True;
        }

        xPresProps->getPropertyValue( OUString( RTL_CONSTASCII_USTRINGPARAM( "IsAutomatic" ) ) ) >>= bTemp;
        if( bTemp )
        {
            AddAttribute(XML_NAMESPACE_PRESENTATION, XML_FORCE_MANUAL, XML_TRUE );
            bHasAttr = sal_True;
        }

        xPresProps->getPropertyValue( OUString( RTL_CONSTASCII_USTRINGPARAM( "IsFullScreen" ) ) ) >>= bTemp;
        if( !bTemp )
        {
            AddAttribute(XML_NAMESPACE_PRESENTATION, XML_FULL_SCREEN, XML_FALSE );
            bHasAttr = sal_True;
        }

        xPresProps->getPropertyValue( OUString( RTL_CONSTASCII_USTRINGPARAM( "IsMouseVisible" ) ) ) >>= bTemp;
        if( !bTemp )
        {
            AddAttribute(XML_NAMESPACE_PRESENTATION, XML_MOUSE_VISIBLE, XML_FALSE );
            bHasAttr = sal_True;
        }

        xPresProps->getPropertyValue( OUString( RTL_CONSTASCII_USTRINGPARAM( "StartWithNavigator" ) ) ) >>= bTemp;
        if( bTemp )
        {
            AddAttribute(XML_NAMESPACE_PRESENTATION, XML_START_WITH_NAVIGATOR, XML_TRUE );
            bHasAttr = sal_True;
        }

        xPresProps->getPropertyValue( OUString( RTL_CONSTASCII_USTRINGPARAM( "UsePen" ) ) ) >>= bTemp;
        if( bTemp )
        {
            AddAttribute(XML_NAMESPACE_PRESENTATION, XML_MOUSE_AS_PEN, XML_TRUE );
            bHasAttr = sal_True;
        }

        xPresProps->getPropertyValue( OUString( RTL_CONSTASCII_USTRINGPARAM( "IsTransitionOnClick" ) ) ) >>= bTemp;
        if( !bTemp )
        {
            AddAttribute(XML_NAMESPACE_PRESENTATION, XML_TRANSITION_ON_CLICK, XML_DISABLED );
            bHasAttr = sal_True;
        }

        xPresProps->getPropertyValue( OUString( RTL_CONSTASCII_USTRINGPARAM( "IsShowLogo" ) ) ) >>= bTemp;
        if( bTemp )
        {
            AddAttribute(XML_NAMESPACE_PRESENTATION, XML_SHOW_LOGO, XML_TRUE );
            bHasAttr = sal_True;
        }

        Reference< container::XNameContainer > xShows;
        Sequence< OUString > aShowNames;
        const OUString* pShowNames = NULL;
        sal_Int32 nShowCount = 0;

        Reference< XCustomPresentationSupplier > xSup( GetModel(), UNO_QUERY );
        if( xSup.is() )
        {
            xShows = xSup->getCustomPresentations();
            if( xShows.is() )
            {
                aShowNames = xShows->getElementNames();
                pShowNames = aShowNames.getArray();
                nShowCount = aShowNames.getLength();
            }
        }

        if( bHasAttr || nShowCount != 0 )
        {
            SvXMLElementExport aSettings(*this, XML_NAMESPACE_PRESENTATION, XML_SETTINGS, sal_True, sal_True);

            if( nShowCount == 0 )
                return;

            Reference< XIndexContainer > xShow;
            Reference< XNamed > xPageName;

            OUStringBuffer sTmp;

            for( sal_Int32 nIndex = 0; nIndex < nShowCount; nIndex++, pShowNames++ )
            {
                AddAttribute(XML_NAMESPACE_PRESENTATION, XML_NAME, *pShowNames );

                xShows->getByName( *pShowNames ) >>= xShow;
                DBG_ASSERT( xShow.is(), "invalid custom show!" );
                if( !xShow.is() )
                    continue;

                const sal_Int32 nPageCount = xShow->getCount();
                for( sal_Int32 nPage = 0; nPage < nPageCount; nPage++ )
                {
                    xShow->getByIndex( nPage ) >>= xPageName;

                    if( !xPageName.is() )
                        continue;

                    if( sTmp.getLength() != 0 )
                        sTmp.append( sal_Unicode( ',' ) );
                    sTmp.append( xPageName->getName() );

                }

                if( sTmp.getLength() )
                    AddAttribute(XML_NAMESPACE_PRESENTATION, XML_PAGES, sTmp.makeStringAndClear() );

                SvXMLElementExport aShows(*this, XML_NAMESPACE_PRESENTATION, XML_SHOW, sal_True, sal_True);
            }
        }
    }
    catch(const uno::Exception&)
    {
        OSL_FAIL( "uno::Exception while exporting <presentation:settings>" );
    }
}

//////////////////////////////////////////////////////////////////////////////

void SdXMLExport::_ExportStyles(sal_Bool bUsed)
{
    GetPropertySetMapper()->SetAutoStyles( sal_False );

    // export fill styles
    SvXMLExport::_ExportStyles( bUsed );

    // write draw:style-name for object graphic-styles
    GetShapeExport()->ExportGraphicDefaults();

    // do not export in ODF 1.1 or older
    if( getDefaultVersion() >= SvtSaveOptions::ODFVER_012 )
        GetShapeExport()->GetShapeTableExport()->exportTableStyles();

    // write presentation styles
    ImpWritePresentationStyles();

    // prepare draw:auto-layout-name for page export
    ImpPrepAutoLayoutInfos();

    // write draw:auto-layout-name for page export
    ImpWriteAutoLayoutInfos();

    Reference< beans::XPropertySet > xInfoSet( getExportInfo() );
    if( xInfoSet.is() )
    {
        Reference< beans::XPropertySetInfo > xInfoSetInfo( xInfoSet->getPropertySetInfo() );

        Any aAny;

        if( xInfoSetInfo->hasPropertyByName( msPageLayoutNames ) )
        {
            aAny <<= maDrawPagesAutoLayoutNames;
            xInfoSet->setPropertyValue( msPageLayoutNames, aAny );
        }
    }
}

//////////////////////////////////////////////////////////////////////////////

void SdXMLExport::_ExportAutoStyles()
{
    Reference< beans::XPropertySet > xInfoSet( getExportInfo() );
    if( xInfoSet.is() )
    {
        Reference< beans::XPropertySetInfo > xInfoSetInfo( xInfoSet->getPropertySetInfo() );

        if( xInfoSetInfo->hasPropertyByName( msPageLayoutNames ) )
        {
            xInfoSet->getPropertyValue( msPageLayoutNames ) >>= maDrawPagesAutoLayoutNames;
        }
    }

    GetPropertySetMapper()->SetAutoStyles( sal_True );

    if( getExportFlags() & EXPORT_STYLES )
    {
        // #80012# PageMaster export moved from _ExportStyles
        // prepare page-master infos
        ImpPrepPageMasterInfos();

        // write page-master infos
        ImpWritePageMasterInfos();

        // prepare draw:style-name for master page export
        ImpPrepMasterPageInfos();
    }

    if( getExportFlags() & EXPORT_CONTENT )
    {
        // prepare draw:style-name for page export
        ImpPrepDrawPageInfos();
    }

    // export draw-page styles
    GetAutoStylePool()->exportXML(
        XML_STYLE_FAMILY_SD_DRAWINGPAGE_ID
        , GetDocHandler(),
        GetMM100UnitConverter(),
        GetNamespaceMap()
        );

    if( getExportFlags() & EXPORT_STYLES )
    {
        // create auto style infos for shapes on master handout page
        if( IsImpress() )
        {
            Reference< presentation::XHandoutMasterSupplier > xHandoutSupp( GetModel(), UNO_QUERY );
            if( xHandoutSupp.is() )
            {
                Reference< XDrawPage > xHandoutPage( xHandoutSupp->getHandoutMasterPage() );
                if( xHandoutPage.is() )
                {
                    Reference< drawing::XShapes > xShapes(xHandoutPage, UNO_QUERY);
                    if(xShapes.is() && xShapes->getCount())
                        GetShapeExport()->collectShapesAutoStyles( xShapes );
                }
            }
        }

        // create auto style infos for objects on master pages
        for(sal_Int32 nMPageId(0L); nMPageId < mnDocMasterPageCount; nMPageId++)
        {
            Reference< XDrawPage > xMasterPage(mxDocMasterPages->getByIndex(nMPageId), UNO_QUERY );

            if( xMasterPage.is() )
            {
                // collect layer information
                GetFormExport()->examineForms( xMasterPage );

                // get MasterPage Name
                OUString aMasterPageNamePrefix;
                Reference < container::XNamed > xNamed(xMasterPage, UNO_QUERY);
                if(xNamed.is())
                {
                    aMasterPageNamePrefix = xNamed->getName();
                }
                if(!aMasterPageNamePrefix.isEmpty())
                {
                    aMasterPageNamePrefix += OUString(RTL_CONSTASCII_USTRINGPARAM("-"));
                }
                GetShapeExport()->setPresentationStylePrefix( aMasterPageNamePrefix );

                Reference< drawing::XShapes > xMasterShapes(xMasterPage, UNO_QUERY);
                if(xMasterShapes.is() && xMasterShapes->getCount())
                    GetShapeExport()->collectShapesAutoStyles( xMasterShapes );

                if(IsImpress())
                {
                    Reference< presentation::XPresentationPage > xPresPage(xMasterPage, UNO_QUERY);
                    if(xPresPage.is())
                    {
                        Reference< XDrawPage > xNotesPage(xPresPage->getNotesPage());
                        if(xNotesPage.is())
                        {
                            // collect layer information
                            GetFormExport()->examineForms( xNotesPage );

                            Reference< drawing::XShapes > xShapes(xNotesPage, UNO_QUERY);
                            if(xShapes.is() && xShapes->getCount())
                                GetShapeExport()->collectShapesAutoStyles( xShapes );
                        }
                    }
                }
                collectAnnotationAutoStyles(xMasterPage);
            }
        }
    }

    if( getExportFlags() & EXPORT_CONTENT )
    {
        // prepare animations exporter if impress
        if(IsImpress() && ((getExportFlags() & EXPORT_OASIS) == 0) )
        {
            UniReference< XMLAnimationsExporter > xAnimExport = new XMLAnimationsExporter( GetShapeExport().get() );
            GetShapeExport()->setAnimationsExporter( xAnimExport );
        }

        // create auto style infos for objects on pages
        for(sal_Int32 nPageInd(0); nPageInd < mnDocDrawPageCount; nPageInd++)
        {
            Reference<XDrawPage> xDrawPage( mxDocDrawPages->getByIndex(nPageInd), UNO_QUERY );
            if( xDrawPage.is() )
            {
                // collect layer information
                GetFormExport()->examineForms( xDrawPage );

                // get MasterPage Name
                OUString aMasterPageNamePrefix;
                Reference < drawing::XMasterPageTarget > xMasterPageInt(xDrawPage, UNO_QUERY);
                if(xMasterPageInt.is())
                {
                    Reference<XDrawPage> xUsedMasterPage(xMasterPageInt->getMasterPage());
                    if(xUsedMasterPage.is())
                    {
                        Reference < container::XNamed > xMasterNamed(xUsedMasterPage, UNO_QUERY);
                        if(xMasterNamed.is())
                        {
                            aMasterPageNamePrefix = xMasterNamed->getName();
                        }
                    }
                }
                if(!aMasterPageNamePrefix.isEmpty())
                {
                    aMasterPageNamePrefix += OUString(RTL_CONSTASCII_USTRINGPARAM("-"));
                }

                GetShapeExport()->setPresentationStylePrefix( aMasterPageNamePrefix );

                // prepare object infos
                Reference< drawing::XShapes > xDrawShapes(xDrawPage, UNO_QUERY);
                if(xDrawShapes.is() && xDrawShapes->getCount())
                    GetShapeExport()->collectShapesAutoStyles( xDrawShapes );

                // prepare presentation notes page object infos (ONLY if presentation)
                if(IsImpress())
                {
                    Reference< presentation::XPresentationPage > xPresPage(xDrawPage, UNO_QUERY);
                    if(xPresPage.is())
                    {
                        Reference< XDrawPage > xNotesPage(xPresPage->getNotesPage());
                        if(xNotesPage.is())
                        {
                            // collect layer information
                            GetFormExport()->examineForms( xNotesPage );

                            Reference< drawing::XShapes > xShapes(xNotesPage, UNO_QUERY);
                            if(xShapes.is() && xShapes->getCount())
                                GetShapeExport()->collectShapesAutoStyles( xShapes );
                        }
                    }
                }

                collectAnnotationAutoStyles( xDrawPage );
            }
        }
        if(IsImpress())
        {
            UniReference< XMLAnimationsExporter > xAnimExport;
            GetShapeExport()->setAnimationsExporter( xAnimExport );
        }
    }

    exportAutoDataStyles();

    GetShapeExport()->exportAutoStyles();

    sal_uInt16 nContentAutostyles = EXPORT_CONTENT | EXPORT_AUTOSTYLES;
    if ( ( getExportFlags() & nContentAutostyles ) == nContentAutostyles )
        GetFormExport()->exportAutoStyles( );

    // ...for text
    GetTextParagraphExport()->exportTextAutoStyles();
}

//////////////////////////////////////////////////////////////////////////////

void SdXMLExport::_ExportMasterStyles()
{
    // export layer
    SdXMLayerExporter::exportLayer( *this );

    // export handout master page if impress
    if( IsImpress() )
    {
        Reference< presentation::XHandoutMasterSupplier > xHandoutSupp( GetModel(), UNO_QUERY );
        if( xHandoutSupp.is() )
        {
            Reference< XDrawPage > xHandoutPage( xHandoutSupp->getHandoutMasterPage() );
            if( xHandoutPage.is() )
            {
                // presentation:page-layout-name
                if( IsImpress() && !maDrawPagesAutoLayoutNames[0].isEmpty())
                {
                    AddAttribute(XML_NAMESPACE_PRESENTATION, XML_PRESENTATION_PAGE_LAYOUT_NAME, EncodeStyleName( maDrawPagesAutoLayoutNames[0] ));
                }

                ImpXMLEXPPageMasterInfo* pInfo = mpHandoutPageMaster;
                if(pInfo)
                {
                    OUString sString = pInfo->GetName();
                    AddAttribute(XML_NAMESPACE_STYLE, XML_PAGE_LAYOUT_NAME, sString );
                }

                // draw:style-name
                if( !maHandoutMasterStyleName.isEmpty() )
                    AddAttribute(XML_NAMESPACE_DRAW, XML_STYLE_NAME, maHandoutMasterStyleName);

                ImplExportHeaderFooterDeclAttributes( maHandoutPageHeaderFooterSettings );

                // write masterpage
                SvXMLElementExport aMPG(*this, XML_NAMESPACE_STYLE, XML_HANDOUT_MASTER, sal_True, sal_True);

                // write graphic objects on this master page (if any)
                Reference< drawing::XShapes > xShapes(xHandoutPage, UNO_QUERY);
                if(xShapes.is() && xShapes->getCount())
                    GetShapeExport()->exportShapes( xShapes );
            }
        }
    }

    // export MasterPages in master-styles section
    for (sal_Int32 nMPageId = 0; nMPageId < mnDocMasterPageCount; nMPageId++)
    {
        Reference< XDrawPage > xMasterPage( mxDocMasterPages->getByIndex(nMPageId), UNO_QUERY );
        if(xMasterPage.is())
        {
            // prepare masterpage attributes
            OUString sMasterPageName;
            Reference < container::XNamed > xNamed(xMasterPage, UNO_QUERY);
            if(xNamed.is())
            {
                sal_Bool bEncoded = sal_False;
                sMasterPageName = xNamed->getName();
                AddAttribute(XML_NAMESPACE_STYLE, XML_NAME,
                    EncodeStyleName( sMasterPageName, &bEncoded ));
                if( bEncoded )
                    AddAttribute(
                        XML_NAMESPACE_STYLE, XML_DISPLAY_NAME,
                        sMasterPageName );
            }

            ImpXMLEXPPageMasterInfo* pInfo = mpPageMasterUsageList->at( nMPageId );
            if(pInfo)
            {
                OUString sString = pInfo->GetName();
                AddAttribute(XML_NAMESPACE_STYLE, XML_PAGE_LAYOUT_NAME, sString );
            }

            // draw:style-name (background attributes)
            if( !maMasterPagesStyleNames[nMPageId].isEmpty() )
                AddAttribute(XML_NAMESPACE_DRAW, XML_STYLE_NAME,
                        maMasterPagesStyleNames[nMPageId]);

            // write masterpage
            SvXMLElementExport aMPG(*this, XML_NAMESPACE_STYLE, XML_MASTER_PAGE, sal_True, sal_True);

            // write optional office:forms
            exportFormsElement( xMasterPage );

            // write graphic objects on this master page (if any)
            Reference< drawing::XShapes > xMasterShapes(xMasterPage, UNO_QUERY);
            if(xMasterShapes.is() && xMasterShapes->getCount())
                GetShapeExport()->exportShapes( xMasterShapes );

            // write presentation notes (ONLY if presentation)
            if(IsImpress())
            {
                Reference< presentation::XPresentationPage > xPresPage(xMasterPage, UNO_QUERY);
                if(xPresPage.is())
                {
                    Reference< XDrawPage > xNotesPage(xPresPage->getNotesPage());
                    if(xNotesPage.is())
                    {
                        Reference< drawing::XShapes > xShapes(xNotesPage, UNO_QUERY);
                        if(xShapes.is())
                        {
                            ImpXMLEXPPageMasterInfo* pMasterInfo = mpNotesPageMasterUsageList->at( nMPageId );
                            if(pMasterInfo)
                            {
                                OUString sString = pMasterInfo->GetName();
                                AddAttribute(XML_NAMESPACE_STYLE, XML_PAGE_LAYOUT_NAME, sString);
                            }

                            // write presentation notes
                            SvXMLElementExport aPSY(*this, XML_NAMESPACE_PRESENTATION, XML_NOTES, sal_True, sal_True);

                            // write optional office:forms
                            exportFormsElement( xNotesPage );

                            // write shapes per se
                            GetShapeExport()->exportShapes( xShapes );
                        }
                    }
                }
            }
            exportAnnotations( xMasterPage );
        }
    }
}

void SdXMLExport::exportFormsElement( Reference< XDrawPage > xDrawPage )
{
    if( xDrawPage.is() )
    {
        Reference< form::XFormsSupplier2 > xFormsSupplier( xDrawPage, UNO_QUERY );
        if ( xFormsSupplier.is() && xFormsSupplier->hasForms() )
        {
            // write masterpage
            ::xmloff::OOfficeFormsExport aForms(*this);
            GetFormExport()->exportForms( xDrawPage );
        }

        if(! GetFormExport()->seekPage( xDrawPage ) )
        {
            OSL_FAIL( "OFormLayerXMLExport::seekPage failed!" );
        }
    }
}

void SdXMLExport::GetViewSettings(uno::Sequence<beans::PropertyValue>& rProps)
{
    rProps.realloc(4);
    beans::PropertyValue* pProps = rProps.getArray();
    if(pProps)
    {
        Reference< beans::XPropertySet > xPropSet( GetModel(), UNO_QUERY );
        if( !xPropSet.is() )
            return;

        awt::Rectangle aVisArea;
        xPropSet->getPropertyValue( OUString( RTL_CONSTASCII_USTRINGPARAM( "VisibleArea" ) ) ) >>= aVisArea;

        sal_uInt16 i = 0;
        pProps[i].Name = rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("VisibleAreaTop"));
        pProps[i++].Value <<= aVisArea.Y;
        pProps[i].Name = rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("VisibleAreaLeft"));
        pProps[i++].Value <<= aVisArea.X;
        pProps[i].Name = rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("VisibleAreaWidth"));
        pProps[i++].Value <<= aVisArea.Width;
        pProps[i].Name = rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("VisibleAreaHeight"));
        pProps[i++].Value <<= aVisArea.Height;
    }
}

void SdXMLExport::GetConfigurationSettings(uno::Sequence<beans::PropertyValue>& rProps)
{
    Reference< lang::XMultiServiceFactory > xFac( GetModel(), UNO_QUERY );
    if( xFac.is() )
    {
        Reference< beans::XPropertySet > xProps( xFac->createInstance( OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.document.Settings" ) ) ), UNO_QUERY );
        if( xProps.is() )
            SvXMLUnitConverter::convertPropertySet( rProps, xProps );
        DocumentSettingsSerializer *pFilter;
        pFilter = dynamic_cast<DocumentSettingsSerializer *>(xProps.get());
        if( pFilter )
            rProps = pFilter->filterStreamsToStorage( GetTargetStorage(), rProps );
    }
}

//////////////////////////////////////////////////////////////////////////////

void SdXMLExport::addDataStyle(const sal_Int32 nNumberFormat, sal_Bool bTimeFormat )
{
    sal_Int32 nFormat = nNumberFormat;
    if( (nNumberFormat > 1) && (nNumberFormat <= 0x0f) )
        nFormat -= 2;

    if( bTimeFormat )
    {
        if( maUsedTimeStyles.count( nFormat ) == 0 )
            maUsedTimeStyles.insert( nFormat );
    }
    else
    {
        if( maUsedDateStyles.count( nFormat ) == 0 )
            maUsedDateStyles.insert( nFormat );
    }
}

//////////////////////////////////////////////////////////////////////////////

void SdXMLExport::exportDataStyles()
{
    // there are no data styles to export in draw/impress yet
}

//////////////////////////////////////////////////////////////////////////////

void SdXMLExport::exportAutoDataStyles()
{
    SdXMLFormatMap::iterator aIter( maUsedDateStyles.begin() );
    SdXMLFormatMap::iterator aEnd( maUsedDateStyles.end() );

    while( aIter != aEnd )
        SdXMLNumberStylesExporter::exportDateStyle( *this, (*aIter++) );

    aIter = maUsedTimeStyles.begin();
    aEnd = maUsedTimeStyles.end();
    while( aIter != aEnd )
        SdXMLNumberStylesExporter::exportTimeStyle( *this, (*aIter++) );

    if(HasFormExport())
        GetFormExport()->exportAutoControlNumberStyles();
}

//////////////////////////////////////////////////////////////////////////////

OUString SdXMLExport::getDataStyleName(const sal_Int32 nNumberFormat, sal_Bool bTimeFormat ) const
{
    if( bTimeFormat )
    {
        return SdXMLNumberStylesExporter::getTimeStyleName( nNumberFormat );
    }
    else
    {
        return SdXMLNumberStylesExporter::getDateStyleName( nNumberFormat );
    }
}

OUString SdXMLExport::getNavigationOrder( const Reference< XDrawPage >& xDrawPage )
{
    OUStringBuffer sNavOrder;
    try
    {
        Reference< XPropertySet > xSet( xDrawPage, UNO_QUERY_THROW );
        Reference< XIndexAccess > xNavOrder( xSet->getPropertyValue( OUString( RTL_CONSTASCII_USTRINGPARAM( "NavigationOrder" ) ) ), UNO_QUERY_THROW );

        Reference< XIndexAccess > xZOrderAccess( xDrawPage, UNO_QUERY );

        // only export navigation order if it is different from the z-order
        if( (xNavOrder.get() != xZOrderAccess.get()) && (xNavOrder->getCount() == xDrawPage->getCount())  )
        {
            sal_Int32 nIndex;
            const sal_Int32 nCount = xNavOrder->getCount();
            for( nIndex = 0; nIndex < nCount; ++nIndex )
            {
                OUString sId( getInterfaceToIdentifierMapper().registerReference( Reference< XInterface >( xNavOrder->getByIndex( nIndex ), UNO_QUERY ) ) );
                if( !sId.isEmpty() )
                {
                    if( sNavOrder.getLength() != 0 )
                        sNavOrder.append( (sal_Unicode)' ' );
                    sNavOrder.append( sId );
                }
            }
        }
    }
    catch(const Exception&)
    {
    }
    return sNavOrder.makeStringAndClear();
}

//////////////////////////////////////////////////////////////////////////////

void SdXMLExport::collectAnnotationAutoStyles( const Reference<XDrawPage>& xDrawPage )
{
    Reference< XAnnotationAccess > xAnnotationAccess( xDrawPage, UNO_QUERY );
    if( xAnnotationAccess.is() ) try
    {
        Reference< XAnnotationEnumeration > xAnnotationEnumeration( xAnnotationAccess->createAnnotationEnumeration() );
        if( xAnnotationEnumeration.is() )
        {
            while( xAnnotationEnumeration->hasMoreElements() )
            {
                Reference< XAnnotation > xAnnotation( xAnnotationEnumeration->nextElement(), UNO_QUERY_THROW );
                Reference< XText > xText( xAnnotation->getTextRange() );
                if(xText.is() && !xText->getString().isEmpty())
                    GetTextParagraphExport()->collectTextAutoStyles( xText );
            }
        }
    }
    catch(const Exception&)
    {
        OSL_FAIL("SdXMLExport::collectAnnotationAutoStyles(), exception caught during export of annotation auto styles");
    }
}

void SdXMLExport::exportAnnotations( const Reference<XDrawPage>& xDrawPage )
{
    // do not export in ODF 1.2 or older
    if( getDefaultVersion() <= SvtSaveOptions::ODFVER_012 )
        return;

    Reference< XAnnotationAccess > xAnnotationAccess( xDrawPage, UNO_QUERY );
    if( xAnnotationAccess.is() ) try
    {
        Reference< XAnnotationEnumeration > xAnnotationEnumeration( xAnnotationAccess->createAnnotationEnumeration() );
        if( xAnnotationEnumeration.is() && xAnnotationEnumeration->hasMoreElements() )
        {
            OUStringBuffer sStringBuffer;
            do
            {
                Reference< XAnnotation > xAnnotation( xAnnotationEnumeration->nextElement(), UNO_QUERY_THROW );

                RealPoint2D aPosition( xAnnotation->getPosition() );

                GetMM100UnitConverter().convertMeasureToXML(sStringBuffer,
                        static_cast<sal_Int32>( aPosition.X * 100 ) );
                AddAttribute(XML_NAMESPACE_SVG, XML_X, sStringBuffer.makeStringAndClear());

                GetMM100UnitConverter().convertMeasureToXML(sStringBuffer,
                        static_cast<sal_Int32>( aPosition.Y * 100 ) );
                AddAttribute(XML_NAMESPACE_SVG, XML_Y, sStringBuffer.makeStringAndClear());

                RealSize2D aSize( xAnnotation->getSize() );

                if( aSize.Width || aSize.Height )
                {
                    GetMM100UnitConverter().convertMeasureToXML(sStringBuffer,
                            static_cast<sal_Int32>( aSize.Width * 100 ) );
                    AddAttribute(XML_NAMESPACE_SVG, XML_WIDTH, sStringBuffer.makeStringAndClear());
                    GetMM100UnitConverter().convertMeasureToXML(sStringBuffer,
                            static_cast<sal_Int32>( aSize.Height * 100 ) );
                    AddAttribute(XML_NAMESPACE_SVG, XML_HEIGHT, sStringBuffer.makeStringAndClear());
                }

                // annotation element + content
                SvXMLElementExport aElem(*this, XML_NAMESPACE_OFFICE_EXT, XML_ANNOTATION, sal_False, sal_True);

                // author
                OUString aAuthor( xAnnotation->getAuthor() );
                if( !aAuthor.isEmpty() )
                {
                    SvXMLElementExport aCreatorElem( *this, XML_NAMESPACE_DC, XML_CREATOR, sal_True, sal_False );
                    this->Characters(aAuthor);
                }

                {
                    // date time
                    DateTime aDate( xAnnotation->getDateTime() );
                    ::sax::Converter::convertDateTime(sStringBuffer, aDate, true);
                    SvXMLElementExport aDateElem( *this, XML_NAMESPACE_DC, XML_DATE, sal_True, sal_False );
                    Characters(sStringBuffer.makeStringAndClear());
                }

                com::sun::star::uno::Reference < com::sun::star::text::XText > xText( xAnnotation->getTextRange() );
                if( xText.is() )
                    this->GetTextParagraphExport()->exportText( xText );
            }
            while( xAnnotationEnumeration->hasMoreElements() );
        }
    }
    catch(const Exception&)
    {
        OSL_FAIL("SdXMLExport::exportAnnotations(), exception caught during export of annotations");
    }
}

//////////////////////////////////////////////////////////////////////////////

#define SERVICE( classname, servicename, implementationname, draw, flags )\
uno::Sequence< OUString > SAL_CALL classname##_getSupportedServiceNames() throw()\
{\
    const OUString aServiceName( RTL_CONSTASCII_USTRINGPARAM( servicename ) );\
    const uno::Sequence< OUString > aSeq( &aServiceName, 1 );\
    return aSeq;\
}\
OUString SAL_CALL classname##_getImplementationName() throw()\
{\
    return OUString( RTL_CONSTASCII_USTRINGPARAM( implementationname ) );\
}\
uno::Reference< uno::XInterface > SAL_CALL classname##_createInstance(const uno::Reference< lang::XMultiServiceFactory > & rSMgr) throw( uno::Exception )\
{\
    return (cppu::OWeakObject*)new SdXMLExport( rSMgr, draw, flags );\
}

SERVICE( XMLImpressExportOasis, "com.sun.star.comp.Impress.XMLOasisExporter", "XMLImpressExportOasis", sal_False, EXPORT_OASIS|EXPORT_META|EXPORT_STYLES|EXPORT_MASTERSTYLES|EXPORT_AUTOSTYLES|EXPORT_CONTENT|EXPORT_SCRIPTS|EXPORT_SETTINGS|EXPORT_FONTDECLS|EXPORT_EMBEDDED );
SERVICE( XMLImpressStylesExportOasis, "com.sun.star.comp.Impress.XMLOasisStylesExporter", "XMLImpressStylesExportOasis", sal_False, EXPORT_OASIS|EXPORT_STYLES|EXPORT_MASTERSTYLES|EXPORT_AUTOSTYLES );
SERVICE( XMLImpressContentExportOasis, "com.sun.star.comp.Impress.XMLOasisContentExporter", "XMLImpressContentExportOasis", sal_False, EXPORT_OASIS|EXPORT_AUTOSTYLES|EXPORT_CONTENT|EXPORT_SCRIPTS|EXPORT_FONTDECLS );
SERVICE( XMLImpressMetaExportOasis, "com.sun.star.comp.Impress.XMLOasisMetaExporter", "XMLImpressMetaExportOasis", sal_False, EXPORT_OASIS|EXPORT_META );
SERVICE( XMLImpressSettingsExportOasis, "com.sun.star.comp.Impress.XMLOasisSettingsExporter", "XMLImpressSettingsExportOasis", sal_False, EXPORT_OASIS|EXPORT_SETTINGS );

SERVICE( XMLImpressExportOOO, "com.sun.star.comp.Impress.XMLExporter", "XMLImpressExportOOO", sal_False, EXPORT_META|EXPORT_STYLES|EXPORT_MASTERSTYLES|EXPORT_AUTOSTYLES|EXPORT_CONTENT|EXPORT_SCRIPTS|EXPORT_SETTINGS|EXPORT_FONTDECLS|EXPORT_EMBEDDED );
SERVICE( XMLImpressStylesExportOOO, "com.sun.star.comp.Impress.XMLStylesExporter", "XMLImpressStylesExportOOO", sal_False, EXPORT_STYLES|EXPORT_MASTERSTYLES|EXPORT_AUTOSTYLES );
SERVICE( XMLImpressContentExportOOO, "com.sun.star.comp.Impress.XMLContentExporter", "XMLImpressContentExportOOO", sal_False, EXPORT_AUTOSTYLES|EXPORT_CONTENT|EXPORT_SCRIPTS|EXPORT_FONTDECLS );
SERVICE( XMLImpressMetaExportOOO, "com.sun.star.comp.Impress.XMLMetaExporter", "XMLImpressMetaExportOOO", sal_False, EXPORT_META );
SERVICE( XMLImpressSettingsExportOOO, "com.sun.star.comp.Impress.XMLSettingsExporter", "XMLImpressSettingsExportOOO", sal_False, EXPORT_SETTINGS );

SERVICE( XMLDrawExportOasis, "com.sun.star.comp.Draw.XMLOasisExporter", "XMLDrawExportOasis", sal_True, EXPORT_OASIS|EXPORT_META|EXPORT_STYLES|EXPORT_MASTERSTYLES|EXPORT_AUTOSTYLES|EXPORT_CONTENT|EXPORT_SCRIPTS|EXPORT_SETTINGS|EXPORT_FONTDECLS|EXPORT_EMBEDDED );
SERVICE( XMLDrawStylesExportOasis, "com.sun.star.comp.Draw.XMLOasisStylesExporter", "XMLDrawStylesExportOasis", sal_True, EXPORT_OASIS|EXPORT_STYLES|EXPORT_MASTERSTYLES|EXPORT_AUTOSTYLES );
SERVICE( XMLDrawContentExportOasis, "com.sun.star.comp.Draw.XMLOasisContentExporter", "XMLDrawContentExportOasis", sal_True, EXPORT_OASIS|EXPORT_AUTOSTYLES|EXPORT_CONTENT|EXPORT_SCRIPTS|EXPORT_FONTDECLS );
SERVICE( XMLDrawMetaExportOasis, "com.sun.star.comp.Draw.XMLOasisMetaExporter", "XMLDrawMetaExportOasis", sal_True, EXPORT_OASIS|EXPORT_META );
SERVICE( XMLDrawSettingsExportOasis, "com.sun.star.comp.Draw.XMLOasisSettingsExporter", "XMLDrawSettingsExportOasis", sal_True, EXPORT_OASIS|EXPORT_SETTINGS );

SERVICE( XMLDrawExportOOO, "com.sun.star.comp.Draw.XMLExporter", "XMLDrawExportOOO", sal_True, EXPORT_META|EXPORT_STYLES|EXPORT_MASTERSTYLES|EXPORT_AUTOSTYLES|EXPORT_CONTENT|EXPORT_SCRIPTS|EXPORT_SETTINGS|EXPORT_FONTDECLS|EXPORT_EMBEDDED );
SERVICE( XMLDrawStylesExportOOO, "com.sun.star.comp.Draw.XMLStylesExporter", "XMLDrawStylesExportOOO", sal_True, EXPORT_STYLES|EXPORT_MASTERSTYLES|EXPORT_AUTOSTYLES );
SERVICE( XMLDrawContentExportOOO, "com.sun.star.comp.Draw.XMLContentExporter", "XMLDrawContentExportOOO", sal_True, EXPORT_AUTOSTYLES|EXPORT_CONTENT|EXPORT_SCRIPTS|EXPORT_FONTDECLS );
SERVICE( XMLDrawMetaExportOOO, "com.sun.star.comp.Draw.XMLMetaExporter", "XMLDrawMetaExportOOO", sal_True, EXPORT_META );
SERVICE( XMLDrawSettingsExportOOO, "com.sun.star.comp.Draw.XMLSettingsExporter", "XMLDrawSettingsExportOOO", sal_True, EXPORT_SETTINGS );

SERVICE( XMLDrawingLayerExport, "com.sun.star.comp.DrawingLayer.XMLExporter", "XMLDrawingLayerExport", sal_True, EXPORT_OASIS|EXPORT_STYLES|EXPORT_AUTOSTYLES|EXPORT_CONTENT|EXPORT_FONTDECLS|EXPORT_EMBEDDED );
SERVICE( XMLImpressClipboardExport, "com.sun.star.comp.Impress.XMLClipboardExporter", "XMLImpressClipboardExport", sal_False, EXPORT_OASIS|EXPORT_STYLES|EXPORT_AUTOSTYLES|EXPORT_CONTENT|EXPORT_FONTDECLS|EXPORT_EMBEDDED );

// XServiceInfo
OUString SAL_CALL SdXMLExport::getImplementationName() throw( uno::RuntimeException )
{
    if( IsDraw())
    {
        // Draw

        switch( getExportFlags())
        {
            case EXPORT_META|EXPORT_STYLES|EXPORT_MASTERSTYLES|EXPORT_AUTOSTYLES|EXPORT_CONTENT|EXPORT_SCRIPTS|EXPORT_SETTINGS|EXPORT_FONTDECLS|EXPORT_EMBEDDED:
                return XMLDrawExportOOO_getImplementationName();
            case EXPORT_STYLES|EXPORT_MASTERSTYLES|EXPORT_AUTOSTYLES:
                return XMLDrawStylesExportOOO_getImplementationName();
            case EXPORT_AUTOSTYLES|EXPORT_CONTENT|EXPORT_SCRIPTS|EXPORT_FONTDECLS:
                return XMLDrawContentExportOOO_getImplementationName();
            case EXPORT_META:
                return XMLDrawMetaExportOOO_getImplementationName();
            case EXPORT_SETTINGS:
                return XMLDrawSettingsExportOOO_getImplementationName();

            case EXPORT_OASIS|EXPORT_META|EXPORT_STYLES|EXPORT_MASTERSTYLES|EXPORT_AUTOSTYLES|EXPORT_CONTENT|EXPORT_SCRIPTS|EXPORT_SETTINGS|EXPORT_FONTDECLS|EXPORT_EMBEDDED:
                return XMLDrawExportOasis_getImplementationName();
            case EXPORT_OASIS|EXPORT_STYLES|EXPORT_MASTERSTYLES|EXPORT_AUTOSTYLES:
                return XMLDrawStylesExportOasis_getImplementationName();
            case EXPORT_OASIS|EXPORT_AUTOSTYLES|EXPORT_CONTENT|EXPORT_SCRIPTS|EXPORT_FONTDECLS:
                return XMLDrawContentExportOasis_getImplementationName();
            case EXPORT_OASIS|EXPORT_META:
                return XMLDrawMetaExportOasis_getImplementationName();
            case EXPORT_OASIS|EXPORT_SETTINGS:
                return XMLDrawSettingsExportOasis_getImplementationName();

            default:
                return XMLDrawExportOOO_getImplementationName();
        }
    }
    else
    {
        // Impress

        switch( getExportFlags())
        {
            case EXPORT_META|EXPORT_STYLES|EXPORT_MASTERSTYLES|EXPORT_AUTOSTYLES|EXPORT_CONTENT|EXPORT_SCRIPTS|EXPORT_SETTINGS|EXPORT_FONTDECLS|EXPORT_EMBEDDED:
                return XMLImpressExportOOO_getImplementationName();
            case EXPORT_STYLES|EXPORT_MASTERSTYLES|EXPORT_AUTOSTYLES:
                return XMLImpressStylesExportOOO_getImplementationName();
            case EXPORT_AUTOSTYLES|EXPORT_CONTENT|EXPORT_SCRIPTS|EXPORT_FONTDECLS:
                return XMLImpressContentExportOOO_getImplementationName();
            case EXPORT_META:
                return XMLImpressMetaExportOOO_getImplementationName();
            case EXPORT_SETTINGS:
                return XMLImpressSettingsExportOOO_getImplementationName();
            case EXPORT_OASIS|EXPORT_META|EXPORT_STYLES|EXPORT_MASTERSTYLES|EXPORT_AUTOSTYLES|EXPORT_CONTENT|EXPORT_SCRIPTS|EXPORT_SETTINGS|EXPORT_FONTDECLS|EXPORT_EMBEDDED:
                return XMLImpressExportOasis_getImplementationName();
            case EXPORT_OASIS|EXPORT_STYLES|EXPORT_MASTERSTYLES|EXPORT_AUTOSTYLES:
                return XMLImpressStylesExportOasis_getImplementationName();
            case EXPORT_OASIS|EXPORT_AUTOSTYLES|EXPORT_CONTENT|EXPORT_SCRIPTS|EXPORT_FONTDECLS:
                return XMLImpressContentExportOasis_getImplementationName();
            case EXPORT_OASIS|EXPORT_META:
                return XMLImpressMetaExportOasis_getImplementationName();
            case EXPORT_OASIS|EXPORT_SETTINGS:
                return XMLImpressSettingsExportOasis_getImplementationName();

            default:
                return XMLImpressExportOOO_getImplementationName();
        }
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
