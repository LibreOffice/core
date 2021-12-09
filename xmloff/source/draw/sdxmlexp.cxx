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

#include <string_view>

#include <xmloff/autolayout.hxx>
#include <xmloff/unointerfacetouniqueidentifiermapper.hxx>
#include <xmloff/namespacemap.hxx>
#include <xmloff/xmlnamespace.hxx>
#include <xmloff/xmluconv.hxx>
#include <xmloff/xmltoken.hxx>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/presentation/XPresentationSupplier.hpp>
#include <com/sun/star/presentation/XCustomPresentationSupplier.hpp>
#include <com/sun/star/geometry/RealPoint2D.hpp>
#include <com/sun/star/office/XAnnotationAccess.hpp>
#include <com/sun/star/uno/Any.hxx>
#include "sdxmlexp_impl.hxx"
#include <com/sun/star/drawing/XDrawPagesSupplier.hpp>
#include <com/sun/star/drawing/XMasterPagesSupplier.hpp>
#include <com/sun/star/presentation/XHandoutMasterSupplier.hpp>
#include <com/sun/star/container/XIndexContainer.hpp>
#include <com/sun/star/view/PaperOrientation.hpp>
#include <com/sun/star/style/XStyleFamiliesSupplier.hpp>

#include <com/sun/star/form/XFormsSupplier2.hpp>
#include <com/sun/star/presentation/XPresentationPage.hpp>
#include <com/sun/star/drawing/XMasterPageTarget.hpp>
#include <com/sun/star/text/XText.hpp>
#include <com/sun/star/animations/XAnimationNodeSupplier.hpp>
#include <com/sun/star/container/XNamed.hpp>
#include <com/sun/star/util/Duration.hpp>
#include <com/sun/star/util/MeasureUnit.hpp>
#include <rtl/ustrbuf.hxx>
#include <sal/log.hxx>
#include <tools/diagnose_ex.h>
#include <tools/gen.hxx>
#include <sax/tools/converter.hxx>
#include <xmloff/xmlaustp.hxx>
#include <xmloff/families.hxx>
#include <xmloff/styleexp.hxx>
#include <xmloff/settingsstore.hxx>
#include <xmloff/table/XMLTableExport.hxx>
#include <xmloff/ProgressBarHelper.hxx>
#include "sdpropls.hxx"
#include <xmloff/xmlexppr.hxx>

#include <PropertySetMerger.hxx>
#include "layerexp.hxx"

#include "XMLNumberStylesExport.hxx"

#include <xmloff/animationexport.hxx>

#include <com/sun/star/document/XDocumentProperties.hpp>
#include <com/sun/star/document/XDocumentPropertiesSupplier.hpp>
#include <com/sun/star/util/Color.hpp>

#include <comphelper/sequenceashashmap.hxx>

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
    bool operator==(const ImpXMLEXPPageMasterInfo& rInfo) const;
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
        if( xPropsInfo.is() && xPropsInfo->hasPropertyByName("BorderBottom"))
        {
            aAny = xPropSet->getPropertyValue("BorderBottom");
            aAny >>= mnBorderBottom;

            aAny = xPropSet->getPropertyValue("BorderLeft");
            aAny >>= mnBorderLeft;

            aAny = xPropSet->getPropertyValue("BorderRight");
            aAny >>= mnBorderRight;

            aAny = xPropSet->getPropertyValue("BorderTop");
            aAny >>= mnBorderTop;
        }

        if( xPropsInfo.is() && xPropsInfo->hasPropertyByName("Width"))
        {
            aAny = xPropSet->getPropertyValue("Width");
            aAny >>= mnWidth;

            aAny = xPropSet->getPropertyValue("Height");
            aAny >>= mnHeight;
        }

        if( xPropsInfo.is() && xPropsInfo->hasPropertyByName("Orientation"))
        {
            aAny = xPropSet->getPropertyValue("Orientation");
            aAny >>= meOrientation;
        }
    }

    Reference <container::XNamed> xMasterNamed(xPage, UNO_QUERY);
    if(xMasterNamed.is())
    {
        msMasterPageName = xMasterNamed->getName();
    }
}

bool ImpXMLEXPPageMasterInfo::operator==(const ImpXMLEXPPageMasterInfo& rInfo) const
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

#define IMP_AUTOLAYOUT_INFO_MAX         (35L)

class ImpXMLAutoLayoutInfo
{
    sal_uInt16                  mnType;
    ImpXMLEXPPageMasterInfo*    mpPageMasterInfo;
    OUString                    msLayoutName;
    tools::Rectangle            maTitleRect;
    tools::Rectangle            maPresRect;
    sal_Int32                   mnGapX;
    sal_Int32                   mnGapY;

public:
    ImpXMLAutoLayoutInfo(sal_uInt16 nTyp, ImpXMLEXPPageMasterInfo* pInf);

    sal_uInt16 GetLayoutType() const { return mnType; }
    ImpXMLEXPPageMasterInfo* GetPageMasterInfo() const { return mpPageMasterInfo; }
    sal_Int32 GetGapX() const { return mnGapX; }
    sal_Int32 GetGapY() const { return mnGapY; }

    const OUString& GetLayoutName() const { return msLayoutName; }
    void SetLayoutName(const OUString& rNew) { msLayoutName = rNew; }

    const tools::Rectangle& GetTitleRectangle() const { return maTitleRect; }
    const tools::Rectangle& GetPresRectangle() const { return maPresRect; }

    static bool IsCreateNecessary(sal_uInt16 nTyp);
};

bool ImpXMLAutoLayoutInfo::IsCreateNecessary(sal_uInt16 nTyp)
{
    if(nTyp == 5 /* AUTOLAYOUT_ORG */
        || nTyp == 20 /* AUTOLAYOUT_NONE */
        || nTyp >= IMP_AUTOLAYOUT_INFO_MAX)
        return false;
    return true;
}

ImpXMLAutoLayoutInfo::ImpXMLAutoLayoutInfo(sal_uInt16 nTyp, ImpXMLEXPPageMasterInfo* pInf)
    : mnType(nTyp)
    , mpPageMasterInfo(pInf)
    , mnGapX(0)
    , mnGapY(0)
{
    // create full info (initialize with typical values)
    Point aPagePos(0,0);
    Size aPageSize(28000, 21000);
    Size aPageInnerSize(28000, 21000);

    if(mpPageMasterInfo)
    {
        aPagePos = Point(mpPageMasterInfo->GetBorderLeft(), mpPageMasterInfo->GetBorderTop());
        aPageSize = Size(mpPageMasterInfo->GetWidth(), mpPageMasterInfo->GetHeight());
        aPageInnerSize = aPageSize;
        aPageInnerSize.AdjustWidth(-(mpPageMasterInfo->GetBorderLeft() + mpPageMasterInfo->GetBorderRight()));
        aPageInnerSize.AdjustHeight(-(mpPageMasterInfo->GetBorderTop() + mpPageMasterInfo->GetBorderBottom()));
    }

    // title rectangle aligning
    Point aTitlePos(aPagePos);
    Size aTitleSize(aPageInnerSize);

    if(mnType == 21 /* AUTOLAYOUT_NOTES */)
    {
        aTitleSize.setHeight(static_cast<tools::Long>(aTitleSize.Height() / 2.5));
        Point aPos = aTitlePos;
        aPos.AdjustY( tools::Long( aTitleSize.Height() * 0.083 ) );
        Size aPartArea = aTitleSize;
        Size aSize;

        // scale handout rectangle using actual page size
        double fH = static_cast<double>(aPartArea.Width())  / aPageSize.Width();
        double fV = static_cast<double>(aPartArea.Height()) / aPageSize.Height();

        if ( fH > fV )
            fH = fV;
        aSize.setWidth( static_cast<tools::Long>(fH * aPageSize.Width()) );
        aSize.setHeight( static_cast<tools::Long>(fH * aPageSize.Height()) );

        aPos.AdjustX((aPartArea.Width() - aSize.Width()) / 2);
        aPos.AdjustY((aPartArea.Height()- aSize.Height())/ 2);

        aTitlePos = aPos;
        aTitleSize = aSize;
    }
    else if(mnType == AUTOLAYOUT_VTITLE_VCONTENT_OVER_VCONTENT || mnType == AUTOLAYOUT_VTITLE_VCONTENT)
    {
        Point aClassicTPos(
            aTitlePos.X() + tools::Long( aTitleSize.Width() * 0.0735 ),
            aTitlePos.Y() + tools::Long( aTitleSize.Height() * 0.083 ));
        Size aClassicTSize(
            tools::Long( aTitleSize.Width() * 0.854 ),
            tools::Long( aTitleSize.Height() * 0.167 ));
        Point aLPos(aPagePos);
        Size aLSize(aPageInnerSize);
        Point aClassicLPos(
            aLPos.X() + tools::Long( aLSize.Width() * 0.0735 ),
            aLPos.Y() + tools::Long( aLSize.Height() * 0.472 ));
        Size aClassicLSize(
            tools::Long( aLSize.Width() * 0.854 ),
            tools::Long( aLSize.Height() * 0.444 ));

        aTitlePos.setX( (aClassicTPos.X() + aClassicTSize.Width()) - aClassicTSize.Height() );
        aTitlePos.setY( aClassicTPos.Y() );
        aTitleSize.setWidth( aClassicTSize.Height() );
        aTitleSize.setHeight( (aClassicLPos.Y() + aClassicLSize.Height()) - aClassicTPos.Y() );
    }
    else
    {
        aTitlePos.AdjustX( tools::Long( aTitleSize.Width() * 0.0735 ) );
        aTitlePos.AdjustY( tools::Long( aTitleSize.Height() * 0.083 ) );
        aTitleSize.setWidth( tools::Long( aTitleSize.Width() * 0.854 ) );
        aTitleSize.setHeight( tools::Long( aTitleSize.Height() * 0.167 ) );
    }

    maTitleRect.SetPos(aTitlePos);
    maTitleRect.SetSize(aTitleSize);

    // layout rectangle aligning
    Point aLayoutPos(aPagePos);
    Size aLayoutSize(aPageInnerSize);

    if(mnType == 21 /* AUTOLAYOUT_NOTES */)
    {
        aLayoutPos.AdjustX( tools::Long( aLayoutSize.Width() * 0.0735 ) );
        aLayoutPos.AdjustY( tools::Long( aLayoutSize.Height() * 0.472 ) );
        aLayoutSize.setWidth( tools::Long( aLayoutSize.Width() * 0.854 ) );
        aLayoutSize.setHeight( tools::Long( aLayoutSize.Height() * 0.444 ) );
    }
    else if((mnType >= 22 && mnType <= 26) || (mnType == 31)) // AUTOLAYOUT_HANDOUT*
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
    else if(mnType == AUTOLAYOUT_VTITLE_VCONTENT_OVER_VCONTENT || mnType == AUTOLAYOUT_VTITLE_VCONTENT)
    {
        Point aClassicTPos(
            aTitlePos.X() + tools::Long( aTitleSize.Width() * 0.0735 ),
            aTitlePos.Y() + tools::Long( aTitleSize.Height() * 0.083 ));
        Size aClassicTSize(
            tools::Long( aTitleSize.Width() * 0.854 ),
            tools::Long( aTitleSize.Height() * 0.167 ));
        Point aClassicLPos(
            aLayoutPos.X() + tools::Long( aLayoutSize.Width() * 0.0735 ),
            aLayoutPos.Y() + tools::Long( aLayoutSize.Height() * 0.472 ));
        Size aClassicLSize(
            tools::Long( aLayoutSize.Width() * 0.854 ),
            tools::Long( aLayoutSize.Height() * 0.444 ));

        aLayoutPos.setX( aClassicLPos.X() );
        aLayoutPos.setY( aClassicTPos.Y() );
        aLayoutSize.setWidth( (aClassicLPos.X() + aClassicLSize.Width())
            - (aClassicTSize.Height() + (aClassicLPos.Y() - (aClassicTPos.Y() + aClassicTSize.Height()))));
        aLayoutSize.setHeight( (aClassicLPos.Y() + aClassicLSize.Height()) - aClassicTPos.Y() );
    }
    else if( mnType == AUTOLAYOUT_ONLY_TEXT )
    {
        aLayoutPos = aTitlePos;
        aLayoutSize.setWidth( aTitleSize.Width() );
        aLayoutSize.setHeight( tools::Long( aLayoutSize.Height() * 0.825 ) );
    }
    else
    {
        aLayoutPos.AdjustX( tools::Long( aLayoutSize.Width() * 0.0735 ) );
        aLayoutPos.AdjustY( tools::Long( aLayoutSize.Height() * 0.278 ) );
        aLayoutSize.setWidth( tools::Long( aLayoutSize.Width() * 0.854 ) );
        aLayoutSize.setHeight( tools::Long( aLayoutSize.Height() * 0.630 ) );
    }

    maPresRect.SetPos(aLayoutPos);
    maPresRect.SetSize(aLayoutSize);
}

constexpr OUStringLiteral gsPageLayoutNames( u"PageLayoutNames" );

SdXMLExport::SdXMLExport(
    const css::uno::Reference< css::uno::XComponentContext >& xContext,
    OUString const & implementationName,
    bool bIsDraw, SvXMLExportFlags nExportFlags )
:   SvXMLExport( xContext, implementationName, util::MeasureUnit::CM,
        bIsDraw ? XML_GRAPHICS : XML_PRESENTATION, nExportFlags ),
    mnDocMasterPageCount(0),
    mnDocDrawPageCount(0),
    mnObjectCount(0),
    mpHandoutPageMaster(nullptr),
    mbIsDraw(bIsDraw)
{

}

// XExporter
void SAL_CALL SdXMLExport::setSourceDocument( const Reference< lang::XComponent >& xDoc )
{
    SvXMLExport::setSourceDocument( xDoc );

    // prepare factory parts
    mpSdPropHdlFactory = new XMLSdPropHdlFactory( GetModel(), *this );

    // construct PropertySetMapper
    rtl::Reference < XMLPropertySetMapper > xMapper = new XMLShapePropertySetMapper( mpSdPropHdlFactory, true);

    // get or create text paragraph export
    GetTextParagraphExport();
    mpPropertySetMapper = new XMLShapeExportPropertyMapper( xMapper, *this );

    // chain text attributes
    mpPropertySetMapper->ChainExportMapper(XMLTextParagraphExport::CreateParaExtPropMapper(*this));

    // construct PresPagePropsMapper
    xMapper = new XMLPropertySetMapper(aXMLSDPresPageProps, mpSdPropHdlFactory, true);

    mpPresPagePropsMapper = new XMLPageExportPropertyMapper( xMapper, *this  );

    // add family name
    GetAutoStylePool()->AddFamily(
        XmlStyleFamily::SD_GRAPHICS_ID,
        OUString(XML_STYLE_FAMILY_SD_GRAPHICS_NAME),
          GetPropertySetMapper(),
          OUString(XML_STYLE_FAMILY_SD_GRAPHICS_PREFIX));
    GetAutoStylePool()->AddFamily(
        XmlStyleFamily::SD_PRESENTATION_ID,
        OUString(XML_STYLE_FAMILY_SD_PRESENTATION_NAME),
          GetPropertySetMapper(),
          OUString(XML_STYLE_FAMILY_SD_PRESENTATION_PREFIX));
    GetAutoStylePool()->AddFamily(
        XmlStyleFamily::SD_DRAWINGPAGE_ID,
        OUString(XML_STYLE_FAMILY_SD_DRAWINGPAGE_NAME),
          GetPresPagePropsMapper(),
          OUString(XML_STYLE_FAMILY_SD_DRAWINGPAGE_PREFIX));
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
        mxDocMasterPages = xMasterPagesSupplier->getMasterPages();
        if(mxDocMasterPages.is())
        {
            mnDocMasterPageCount = mxDocMasterPages->getCount();
            maMasterPagesStyleNames.insert( maMasterPagesStyleNames.begin(), mnDocMasterPageCount, "" );
        }
    }

    // prepare access to draw pages
    Reference <XDrawPagesSupplier> xDrawPagesSupplier(GetModel(), UNO_QUERY);
    if(xDrawPagesSupplier.is())
    {
        mxDocDrawPages = xDrawPagesSupplier->getDrawPages();
        if(mxDocDrawPages.is())
        {
            mnDocDrawPageCount = mxDocDrawPages->getCount();
            maDrawPagesStyleNames.insert( maDrawPagesStyleNames.begin(), mnDocDrawPageCount, "" );
            maDrawNotesPagesStyleNames.insert( maDrawNotesPagesStyleNames.begin(), mnDocDrawPageCount, "" );
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
                if(xHandoutPage.is() && xHandoutPage->getCount())
                    mnObjectCount += ImpRecursiveObjectCount(xHandoutPage);
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
                        if(xNotesPage.is() && xNotesPage->getCount())
                            mnObjectCount += ImpRecursiveObjectCount(xNotesPage);
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
                        if(xNotesPage.is() && xNotesPage->getCount())
                            mnObjectCount += ImpRecursiveObjectCount(xNotesPage);
                    }
                }
            }
        }

        // #82003# init progress bar
        GetProgressBarHelper()->SetReference(mnObjectCount);
    }

    // add namespaces
    GetNamespaceMap_().Add(
        GetXMLToken(XML_NP_PRESENTATION),
        GetXMLToken(XML_N_PRESENTATION),
        XML_NAMESPACE_PRESENTATION);

    GetNamespaceMap_().Add(
        GetXMLToken(XML_NP_SMIL),
        GetXMLToken(XML_N_SMIL_COMPAT),
        XML_NAMESPACE_SMIL);

    GetNamespaceMap_().Add(
        GetXMLToken(XML_NP_ANIMATION),
        GetXMLToken(XML_N_ANIMATION),
        XML_NAMESPACE_ANIMATION);

    if (getSaneDefaultVersion() & SvtSaveOptions::ODFSVER_EXTENDED)
    {
        GetNamespaceMap_().Add(
            GetXMLToken(XML_NP_OFFICE_EXT),
            GetXMLToken(XML_N_OFFICE_EXT),
            XML_NAMESPACE_OFFICE_EXT);
    }

    GetShapeExport()->enableLayerExport();

    // #88546# enable progress bar increments
    GetShapeExport()->enableHandleProgressBar();
}

// #82003# helper function for recursive object count
sal_uInt32 SdXMLExport::ImpRecursiveObjectCount(const Reference< drawing::XShapes >& xShapes)
{
    sal_uInt32 nRetval(0);

    if(xShapes.is())
    {
        sal_Int32 nCount = xShapes->getCount();

        for(sal_Int32 a(0); a < nCount; a++)
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

SdXMLExport::~SdXMLExport()
{
    // cleanup factory, decrease refcount. Should lead to destruction.
    mpSdPropHdlFactory.clear();

    // cleanup mapper, decrease refcount. Should lead to destruction.
    mpPropertySetMapper.clear();

    // cleanup presPage mapper, decrease refcount. Should lead to destruction.
    mpPresPagePropsMapper.clear();

    mvPageMasterInfoList.clear();

    // clear auto-layout infos
    mvAutoLayoutInfoList.clear();
}

void SdXMLExport::ImpPrepAutoLayoutInfos()
{
    if(!IsImpress())
        return;

    OUString aStr;
    auto DrawPagesAutoLayoutNamesRange = asNonConstRange(maDrawPagesAutoLayoutNames);
    Reference< presentation::XHandoutMasterSupplier > xHandoutSupp( GetModel(), UNO_QUERY );
    if( xHandoutSupp.is() )
    {
        Reference< XDrawPage > xHandoutPage( xHandoutSupp->getHandoutMasterPage() );
        if( xHandoutPage.is() )
        {
            if(ImpPrepAutoLayoutInfo(xHandoutPage, aStr))
                DrawPagesAutoLayoutNamesRange[0] = aStr;
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
                DrawPagesAutoLayoutNamesRange[nCnt+1] = aStr;
        }
    }
}

bool SdXMLExport::ImpPrepAutoLayoutInfo(const Reference<XDrawPage>& xPage, OUString& rName)
{
    rName.clear();
    bool bRetval(false);

    Reference <beans::XPropertySet> xPropSet(xPage, UNO_QUERY);
    if(xPropSet.is())
    {
        sal_uInt16 nType = sal_uInt16();
        Any aAny = xPropSet->getPropertyValue("Layout");
        if(aAny >>= nType)
        {
            if(ImpXMLAutoLayoutInfo::IsCreateNecessary(nType))
            {
                ImpXMLEXPPageMasterInfo* pInfo = nullptr;

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

                // create entry and look for existence
                ImpXMLAutoLayoutInfo* pNew;
                auto it = std::find_if(mvAutoLayoutInfoList.begin(), mvAutoLayoutInfoList.end(),
                            [=](std::unique_ptr<ImpXMLAutoLayoutInfo> const & rInfo) { return nType == rInfo->GetLayoutType() && pInfo == rInfo->GetPageMasterInfo(); });
                if (it != mvAutoLayoutInfoList.end())
                {
                    pNew = it->get();
                }
                else
                {
                    pNew = new ImpXMLAutoLayoutInfo(nType, pInfo);
                    mvAutoLayoutInfoList.emplace_back( pNew );
                    OUString sNewName =
                        "AL" + OUString::number(mvAutoLayoutInfoList.size() - 1) +
                        "T" + OUString::number(nType);
                    pNew->SetLayoutName(sNewName);
                }

                rName = pNew->GetLayoutName();
                bRetval = true;
            }
        }
    }

    return bRetval;
}

void SdXMLExport::ImpWriteAutoLayoutInfos()
{
    for(const auto & pInfo : mvAutoLayoutInfoList)
    {
        if(pInfo)
        {
            // prepare presentation-page layout attributes, style-name
            AddAttribute(XML_NAMESPACE_STYLE, XML_NAME, pInfo->GetLayoutName());

            // write draw-style attributes
            SvXMLElementExport aDSE(*this, XML_NAMESPACE_STYLE, XML_PRESENTATION_PAGE_LAYOUT, true, true);

            // write presentation placeholders
            switch(pInfo->GetLayoutType())
            {
                case AUTOLAYOUT_TITLE :
                {
                    ImpWriteAutoLayoutPlaceholder(XmlPlaceholderTitle, pInfo->GetTitleRectangle());
                    ImpWriteAutoLayoutPlaceholder(XmlPlaceholderSubtitle, pInfo->GetPresRectangle());
                    break;
                }
                case AUTOLAYOUT_TITLE_CONTENT :
                {
                    ImpWriteAutoLayoutPlaceholder(XmlPlaceholderTitle, pInfo->GetTitleRectangle());
                    ImpWriteAutoLayoutPlaceholder(XmlPlaceholderOutline, pInfo->GetPresRectangle());
                    break;
                }
                case AUTOLAYOUT_CHART :
                {
                    ImpWriteAutoLayoutPlaceholder(XmlPlaceholderTitle, pInfo->GetTitleRectangle());
                    ImpWriteAutoLayoutPlaceholder(XmlPlaceholderChart, pInfo->GetPresRectangle());
                    break;
                }
                case AUTOLAYOUT_TITLE_2CONTENT :
                {
                    tools::Rectangle aLeft(pInfo->GetPresRectangle());
                    aLeft.setWidth(tools::Long(aLeft.GetWidth() * 0.488));
                    tools::Rectangle aRight(aLeft);
                    aRight.SetLeft(tools::Long(aRight.Left() + aRight.GetWidth() * 1.05));

                    ImpWriteAutoLayoutPlaceholder(XmlPlaceholderTitle, pInfo->GetTitleRectangle());
                    ImpWriteAutoLayoutPlaceholder(XmlPlaceholderOutline, aLeft);
                    ImpWriteAutoLayoutPlaceholder(XmlPlaceholderOutline, aRight);
                    break;
                }
                case AUTOLAYOUT_TEXTCHART :
                {
                    tools::Rectangle aLeft(pInfo->GetPresRectangle());
                    aLeft.setWidth(tools::Long(aLeft.GetWidth() * 0.488));
                    tools::Rectangle aRight(aLeft);
                    aRight.SetLeft( tools::Long(aRight.Left() + aRight.GetWidth() * 1.05) );

                    ImpWriteAutoLayoutPlaceholder(XmlPlaceholderTitle, pInfo->GetTitleRectangle());
                    ImpWriteAutoLayoutPlaceholder(XmlPlaceholderOutline, aLeft);
                    ImpWriteAutoLayoutPlaceholder(XmlPlaceholderChart, aRight);
                    break;
                }
                case AUTOLAYOUT_TEXTCLIP :
                {
                    tools::Rectangle aLeft(pInfo->GetPresRectangle());
                    aLeft.setWidth(tools::Long(aLeft.GetWidth() * 0.488));
                    tools::Rectangle aRight(aLeft);
                    aRight.SetLeft(tools::Long(aRight.Left() + aRight.GetWidth() * 1.05));

                    ImpWriteAutoLayoutPlaceholder(XmlPlaceholderTitle, pInfo->GetTitleRectangle());
                    ImpWriteAutoLayoutPlaceholder(XmlPlaceholderOutline, aLeft);
                    ImpWriteAutoLayoutPlaceholder(XmlPlaceholderGraphic, aRight);
                    break;
                }
                case AUTOLAYOUT_CHARTTEXT :
                {
                    tools::Rectangle aLeft(pInfo->GetPresRectangle());
                    aLeft.setWidth(tools::Long(aLeft.GetWidth() * 0.488));
                    tools::Rectangle aRight(aLeft);
                    aRight.SetLeft(tools::Long(aRight.Left() + aRight.GetWidth() * 1.05));

                    ImpWriteAutoLayoutPlaceholder(XmlPlaceholderTitle, pInfo->GetTitleRectangle());
                    ImpWriteAutoLayoutPlaceholder(XmlPlaceholderChart, aLeft);
                    ImpWriteAutoLayoutPlaceholder(XmlPlaceholderOutline, aRight);
                    break;
                }
                case AUTOLAYOUT_TAB :
                {
                    ImpWriteAutoLayoutPlaceholder(XmlPlaceholderTitle, pInfo->GetTitleRectangle());
                    ImpWriteAutoLayoutPlaceholder(XmlPlaceholderTable, pInfo->GetPresRectangle());
                    break;
                }
                case AUTOLAYOUT_CLIPTEXT :
                {
                    tools::Rectangle aLeft(pInfo->GetPresRectangle());
                    aLeft.setWidth(tools::Long(aLeft.GetWidth() * 0.488));
                    tools::Rectangle aRight(aLeft);
                    aRight.SetLeft(tools::Long(aRight.Left() + aRight.GetWidth() * 1.05));

                    ImpWriteAutoLayoutPlaceholder(XmlPlaceholderTitle, pInfo->GetTitleRectangle());
                    ImpWriteAutoLayoutPlaceholder(XmlPlaceholderGraphic, aLeft);
                    ImpWriteAutoLayoutPlaceholder(XmlPlaceholderOutline, aRight);
                    break;
                }
                case AUTOLAYOUT_TEXTOBJ :
                {
                    tools::Rectangle aLeft(pInfo->GetPresRectangle());
                    aLeft.setWidth(tools::Long(aLeft.GetWidth() * 0.488));
                    tools::Rectangle aRight(aLeft);
                    aRight.SetLeft(tools::Long(aRight.Left() + aRight.GetWidth() * 1.05));

                    ImpWriteAutoLayoutPlaceholder(XmlPlaceholderTitle, pInfo->GetTitleRectangle());
                    ImpWriteAutoLayoutPlaceholder(XmlPlaceholderOutline, aLeft);
                    ImpWriteAutoLayoutPlaceholder(XmlPlaceholderObject, aRight);
                    break;
                }
                case AUTOLAYOUT_OBJ :
                {
                    ImpWriteAutoLayoutPlaceholder(XmlPlaceholderTitle, pInfo->GetTitleRectangle());
                    ImpWriteAutoLayoutPlaceholder(XmlPlaceholderObject, pInfo->GetPresRectangle());
                    break;
                }
                case AUTOLAYOUT_TITLE_CONTENT_2CONTENT :
                {
                    tools::Rectangle aLeft(pInfo->GetPresRectangle());
                    aLeft.setWidth(tools::Long(aLeft.GetWidth() * 0.488));
                    tools::Rectangle aRightTop(aLeft);
                    aRightTop.SetLeft(tools::Long(aRightTop.Left() + aRightTop.GetWidth() * 1.05));
                    aRightTop.setHeight(tools::Long(aRightTop.GetHeight() * 0.477));
                    tools::Rectangle aRightBottom(aRightTop);
                    aRightBottom.SetTop(tools::Long(aRightBottom.Top() + aRightBottom.GetHeight() * 1.095));

                    ImpWriteAutoLayoutPlaceholder(XmlPlaceholderTitle, pInfo->GetTitleRectangle());
                    ImpWriteAutoLayoutPlaceholder(XmlPlaceholderOutline, aLeft);
                    ImpWriteAutoLayoutPlaceholder(XmlPlaceholderObject, aRightTop);
                    ImpWriteAutoLayoutPlaceholder(XmlPlaceholderObject, aRightBottom);
                    break;
                }
                case AUTOLAYOUT_OBJTEXT :
                {
                    tools::Rectangle aLeft(pInfo->GetPresRectangle());
                    aLeft.setWidth(tools::Long(aLeft.GetWidth() * 0.488));
                    tools::Rectangle aRight(aLeft);
                    aRight.SetLeft(tools::Long(aRight.Left() + aRight.GetWidth() * 1.05));

                    ImpWriteAutoLayoutPlaceholder(XmlPlaceholderTitle, pInfo->GetTitleRectangle());
                    ImpWriteAutoLayoutPlaceholder(XmlPlaceholderObject, aLeft);
                    ImpWriteAutoLayoutPlaceholder(XmlPlaceholderOutline, aRight);
                    break;
                }
                case AUTOLAYOUT_TITLE_CONTENT_OVER_CONTENT :
                {
                    tools::Rectangle aTop(pInfo->GetPresRectangle());
                    aTop.setHeight(tools::Long(aTop.GetHeight() * 0.477));
                    tools::Rectangle aBottom(aTop);
                    aBottom.SetTop(tools::Long(aBottom.Top() + aBottom.GetHeight() * 1.095));

                    ImpWriteAutoLayoutPlaceholder(XmlPlaceholderTitle, pInfo->GetTitleRectangle());
                    ImpWriteAutoLayoutPlaceholder(XmlPlaceholderObject, aTop);
                    ImpWriteAutoLayoutPlaceholder(XmlPlaceholderOutline, aBottom);
                    break;
                }
                case AUTOLAYOUT_TITLE_2CONTENT_CONTENT :
                {
                    tools::Rectangle aLeftTop(pInfo->GetPresRectangle());
                    aLeftTop.setWidth(tools::Long(aLeftTop.GetWidth() * 0.488));
                    tools::Rectangle aRight(aLeftTop);
                    aRight.SetLeft(tools::Long(aRight.Left() + aRight.GetWidth() * 1.05));
                    aLeftTop.setHeight(tools::Long(aLeftTop.GetHeight() * 0.477));
                    tools::Rectangle aLeftBottom(aLeftTop);
                    aLeftBottom.SetTop(tools::Long(aLeftBottom.Top() + aLeftBottom.GetHeight() * 1.095));

                    ImpWriteAutoLayoutPlaceholder(XmlPlaceholderTitle, pInfo->GetTitleRectangle());
                    ImpWriteAutoLayoutPlaceholder(XmlPlaceholderObject, aLeftTop);
                    ImpWriteAutoLayoutPlaceholder(XmlPlaceholderObject, aLeftBottom);
                    ImpWriteAutoLayoutPlaceholder(XmlPlaceholderOutline, aRight);
                    break;
                }
                case AUTOLAYOUT_TITLE_2CONTENT_OVER_CONTENT :
                {
                    tools::Rectangle aTopLeft(pInfo->GetPresRectangle());
                    aTopLeft.setHeight(tools::Long(aTopLeft.GetHeight() * 0.477));
                    tools::Rectangle aBottom(aTopLeft);
                    aBottom.SetTop(tools::Long(aBottom.Top() + aBottom.GetHeight() * 1.095));
                    aTopLeft.setWidth(tools::Long(aTopLeft.GetWidth() * 0.488));
                    tools::Rectangle aTopRight(aTopLeft);
                    aTopRight.SetLeft(tools::Long(aTopRight.Left() + aTopRight.GetWidth() * 1.05));

                    ImpWriteAutoLayoutPlaceholder(XmlPlaceholderTitle, pInfo->GetTitleRectangle());
                    ImpWriteAutoLayoutPlaceholder(XmlPlaceholderObject, aTopLeft);
                    ImpWriteAutoLayoutPlaceholder(XmlPlaceholderObject, aTopRight);
                    ImpWriteAutoLayoutPlaceholder(XmlPlaceholderOutline, aBottom);
                    break;
                }
                case AUTOLAYOUT_TEXTOVEROBJ :
                {
                    tools::Rectangle aTop(pInfo->GetPresRectangle());
                    aTop.setHeight(tools::Long(aTop.GetHeight() * 0.477));
                    tools::Rectangle aBottom(aTop);
                    aBottom.SetTop(tools::Long(aBottom.Top() + aBottom.GetHeight() * 1.095));

                    ImpWriteAutoLayoutPlaceholder(XmlPlaceholderTitle, pInfo->GetTitleRectangle());
                    ImpWriteAutoLayoutPlaceholder(XmlPlaceholderOutline, aTop);
                    ImpWriteAutoLayoutPlaceholder(XmlPlaceholderObject, aBottom);
                    break;
                }
                case AUTOLAYOUT_TITLE_4CONTENT :
                {
                    tools::Rectangle aTopLeft(pInfo->GetPresRectangle());
                    aTopLeft.setHeight(tools::Long(aTopLeft.GetHeight() * 0.477));
                    aTopLeft.setWidth(tools::Long(aTopLeft.GetWidth() * 0.488));
                    tools::Rectangle aBottomLeft(aTopLeft);
                    aBottomLeft.SetTop(tools::Long(aBottomLeft.Top() + aBottomLeft.GetHeight() * 1.095));
                    tools::Rectangle aTopRight(aTopLeft);
                    aTopRight.SetLeft(tools::Long(aTopRight.Left() + aTopRight.GetWidth() * 1.05));
                    tools::Rectangle aBottomRight(aTopRight);
                    aBottomRight.SetTop(tools::Long(aBottomRight.Top() + aBottomRight.GetHeight() * 1.095));

                    ImpWriteAutoLayoutPlaceholder(XmlPlaceholderTitle, pInfo->GetTitleRectangle());
                    ImpWriteAutoLayoutPlaceholder(XmlPlaceholderObject, aTopLeft);
                    ImpWriteAutoLayoutPlaceholder(XmlPlaceholderObject, aTopRight);
                    ImpWriteAutoLayoutPlaceholder(XmlPlaceholderObject, aBottomLeft);
                    ImpWriteAutoLayoutPlaceholder(XmlPlaceholderObject, aBottomRight);
                    break;
                }
                case AUTOLAYOUT_TITLE_ONLY :
                {
                    ImpWriteAutoLayoutPlaceholder(XmlPlaceholderTitle, pInfo->GetTitleRectangle());
                    break;
                }
                case AUTOLAYOUT_NOTES :
                {
                    ImpWriteAutoLayoutPlaceholder(XmlPlaceholderPage, pInfo->GetTitleRectangle());
                    ImpWriteAutoLayoutPlaceholder(XmlPlaceholderNotes, pInfo->GetPresRectangle());
                    break;
                }
                case AUTOLAYOUT_HANDOUT1 :
                case AUTOLAYOUT_HANDOUT2 :
                case AUTOLAYOUT_HANDOUT3 :
                case AUTOLAYOUT_HANDOUT4 :
                case AUTOLAYOUT_HANDOUT6 :
                case AUTOLAYOUT_HANDOUT9 :
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

                    if (nColCnt == 0 || nRowCnt == 0)
                        break;

                    aPartSize.setWidth( (aPartSize.Width() - ((nColCnt - 1) * nGapX)) / nColCnt );
                    aPartSize.setHeight( (aPartSize.Height() - ((nRowCnt - 1) * nGapY)) / nRowCnt );

                    Point aTmpPos(aPartPos);

                    for (sal_Int32 a = 0; a < nRowCnt; a++)
                    {
                        aTmpPos.setX(aPartPos.X());

                        for (sal_Int32 b = 0; b < nColCnt; b++)
                        {
                            tools::Rectangle aTmpRect(aTmpPos, aPartSize);

                            ImpWriteAutoLayoutPlaceholder(XmlPlaceholderHandout, aTmpRect);
                            aTmpPos.AdjustX( aPartSize.Width() + nGapX );
                        }

                        aTmpPos.AdjustY( aPartSize.Height() + nGapY );
                    }
                    break;
                }
                case AUTOLAYOUT_VTITLE_VCONTENT_OVER_VCONTENT :
                {
                    tools::Rectangle aTop(pInfo->GetPresRectangle());
                    aTop.setHeight(tools::Long(aTop.GetHeight() * 0.488));
                    tools::Rectangle aBottom(aTop);
                    aBottom.SetTop(tools::Long(aBottom.Top() + aBottom.GetHeight() * 1.05));

                    ImpWriteAutoLayoutPlaceholder(XmlPlaceholderVerticalTitle, pInfo->GetTitleRectangle());
                    ImpWriteAutoLayoutPlaceholder(XmlPlaceholderVerticalOutline, aTop);
                    ImpWriteAutoLayoutPlaceholder(XmlPlaceholderChart, aBottom);
                    break;
                }
                case AUTOLAYOUT_VTITLE_VCONTENT :
                {
                    ImpWriteAutoLayoutPlaceholder(XmlPlaceholderVerticalTitle, pInfo->GetTitleRectangle());
                    ImpWriteAutoLayoutPlaceholder(XmlPlaceholderVerticalOutline, pInfo->GetPresRectangle());
                    break;
                }
                case AUTOLAYOUT_TITLE_VCONTENT :
                {
                    ImpWriteAutoLayoutPlaceholder(XmlPlaceholderTitle, pInfo->GetTitleRectangle());
                    ImpWriteAutoLayoutPlaceholder(XmlPlaceholderVerticalOutline, pInfo->GetPresRectangle());
                    break;
                }
                case AUTOLAYOUT_TITLE_2VTEXT :
                {
                    tools::Rectangle aLeft(pInfo->GetPresRectangle());
                    aLeft.setWidth(tools::Long(aLeft.GetWidth() * 0.488));
                    tools::Rectangle aRight(aLeft);
                    aRight.SetLeft(tools::Long(aRight.Left() + aRight.GetWidth() * 1.05));

                    ImpWriteAutoLayoutPlaceholder(XmlPlaceholderTitle, pInfo->GetTitleRectangle());
                    ImpWriteAutoLayoutPlaceholder(XmlPlaceholderGraphic, aLeft);
                    ImpWriteAutoLayoutPlaceholder(XmlPlaceholderVerticalOutline, aRight);
                    break;
                }
                case AUTOLAYOUT_ONLY_TEXT :
                {
                    ImpWriteAutoLayoutPlaceholder(XmlPlaceholderSubtitle, pInfo->GetPresRectangle());
                    break;
                }

                case AUTOLAYOUT_4CLIPART :
                {
                    tools::Rectangle aTopLeft(pInfo->GetPresRectangle());
                    aTopLeft.setHeight(tools::Long(aTopLeft.GetHeight() * 0.477));
                    aTopLeft.setWidth(tools::Long(aTopLeft.GetWidth() * 0.488));
                    tools::Rectangle aBottomLeft(aTopLeft);
                    aBottomLeft.SetTop(tools::Long(aBottomLeft.Top() + aBottomLeft.GetHeight() * 1.095));
                    tools::Rectangle aTopRight(aTopLeft);
                    aTopRight.SetLeft(tools::Long(aTopRight.Left() + aTopRight.GetWidth() * 1.05));
                    tools::Rectangle aBottomRight(aTopRight);
                    aBottomRight.SetTop(tools::Long(aBottomRight.Top() + aBottomRight.GetHeight() * 1.095));

                    ImpWriteAutoLayoutPlaceholder(XmlPlaceholderTitle, pInfo->GetTitleRectangle());
                    ImpWriteAutoLayoutPlaceholder(XmlPlaceholderGraphic, aTopLeft);
                    ImpWriteAutoLayoutPlaceholder(XmlPlaceholderGraphic, aTopRight);
                    ImpWriteAutoLayoutPlaceholder(XmlPlaceholderGraphic, aBottomLeft);
                    ImpWriteAutoLayoutPlaceholder(XmlPlaceholderGraphic, aBottomRight);
                    break;
                }

                case AUTOLAYOUT_TITLE_6CONTENT :
                {
                    tools::Rectangle aTopLeft(pInfo->GetPresRectangle());
                    aTopLeft.setHeight(tools::Long(aTopLeft.GetHeight() * 0.477));
                    aTopLeft.setWidth(tools::Long(aTopLeft.GetWidth() * 0.322));
                    tools::Rectangle aTopCenter(aTopLeft);
                    aTopCenter.SetLeft(tools::Long(aTopCenter.Left() + aTopCenter.GetWidth() * 1.05));
                    tools::Rectangle aTopRight(aTopLeft);
                    aTopRight.SetLeft(tools::Long(aTopRight.Left() + aTopRight.GetWidth() * 2 * 1.05));

                    tools::Rectangle aBottomLeft(aTopLeft);
                    aBottomLeft.SetTop(tools::Long(aBottomLeft.Top() + aBottomLeft.GetHeight() * 1.095));
                    tools::Rectangle aBottomCenter(aTopCenter);
                    aBottomCenter.SetTop(tools::Long(aBottomCenter.Top() + aBottomCenter.GetHeight() * 1.095));
                    tools::Rectangle aBottomRight(aTopRight);
                    aBottomRight.SetTop(tools::Long(aBottomRight.Top() + aBottomRight.GetHeight() * 1.095));

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

void SdXMLExport::ImpWriteAutoLayoutPlaceholder(XmlPlaceholder ePl, const tools::Rectangle& rRect)
{
    OUString aStr;
    OUStringBuffer sStringBuffer;

    // prepare presentation-placeholder attributes, presentation:object
    switch(ePl)
    {
        case XmlPlaceholderTitle: aStr = "title"; break;
        case XmlPlaceholderOutline: aStr = "outline"; break;
        case XmlPlaceholderSubtitle: aStr = "subtitle"; break;
        case XmlPlaceholderGraphic: aStr = "graphic"; break;
        case XmlPlaceholderObject: aStr = "object"; break;
        case XmlPlaceholderChart: aStr = "chart"; break;
        case XmlPlaceholderTable: aStr = "table"; break;
        case XmlPlaceholderPage: aStr = "page"; break;
        case XmlPlaceholderNotes: aStr = "notes"; break;
        case XmlPlaceholderHandout: aStr = "handout"; break;
        case XmlPlaceholderVerticalTitle: aStr = "vertical_title"; break;
        case XmlPlaceholderVerticalOutline: aStr = "vertical_outline"; break;
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
    SvXMLElementExport aPPL(*this, XML_NAMESPACE_PRESENTATION, XML_PLACEHOLDER, true, true);
}

ImpXMLEXPPageMasterInfo* SdXMLExport::ImpGetOrCreatePageMasterInfo( const Reference< XDrawPage >& xMasterPage )
{
    bool bDoesExist = false;

    ImpXMLEXPPageMasterInfo* pNewInfo = new ImpXMLEXPPageMasterInfo(*this, xMasterPage);

    // compare with prev page-master infos
    for( size_t a = 0; !bDoesExist && a < mvPageMasterInfoList.size(); a++)
    {
        if (   mvPageMasterInfoList.at(a)
           && *mvPageMasterInfoList.at(a) == *pNewInfo
           )
        {
            delete pNewInfo;
            pNewInfo = mvPageMasterInfoList.at(a).get();
            bDoesExist = true;
        }
    }
    // add entry when not found same page-master infos
    if(!bDoesExist)
        mvPageMasterInfoList.emplace_back( pNewInfo );

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
    if(!mnDocMasterPageCount)
        return;

    // look for needed page-masters, create these
    for (sal_Int32 nMPageId = 0; nMPageId < mnDocMasterPageCount; nMPageId++)
    {
        Reference< XDrawPage > xMasterPage( mxDocMasterPages->getByIndex(nMPageId), UNO_QUERY );
        ImpXMLEXPPageMasterInfo* pNewInfo = nullptr;

        if(xMasterPage.is())
            pNewInfo = ImpGetOrCreatePageMasterInfo(xMasterPage);

        mvPageMasterUsageList.push_back( pNewInfo );

        // look for page master of handout page
        if(IsImpress())
        {
            pNewInfo = nullptr;
            Reference< presentation::XPresentationPage > xPresPage(xMasterPage, UNO_QUERY);
            if(xPresPage.is())
            {
                Reference< XDrawPage > xNotesPage(xPresPage->getNotesPage());
                if(xNotesPage.is())
                {
                    pNewInfo = ImpGetOrCreatePageMasterInfo(xNotesPage);
                }
            }
            mvNotesPageMasterUsageList.push_back( pNewInfo );
        }
    }
}

void SdXMLExport::ImpWritePageMasterInfos()
{
    // write created page-masters, create names for these
    for( size_t nCnt = 0; nCnt < mvPageMasterInfoList.size(); nCnt++)
    {
        ImpXMLEXPPageMasterInfo* pInfo = mvPageMasterInfoList.at(nCnt).get();
        if(pInfo)
        {
            // create name
            OUString sNewName = "PM" + OUString::number(nCnt);
            pInfo->SetName(sNewName);

            // prepare page-master attributes
            OUString sString;
            OUStringBuffer sStringBuffer;

            sString = sNewName;
            AddAttribute(XML_NAMESPACE_STYLE, XML_NAME, sString);

            // write page-layout
            SvXMLElementExport aPME(*this, XML_NAMESPACE_STYLE, XML_PAGE_LAYOUT, true, true);

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
            SvXMLElementExport aPMF(*this, XML_NAMESPACE_STYLE, XML_PAGE_LAYOUT_PROPERTIES, true, true);
        }
    }
}

ImpXMLEXPPageMasterInfo* SdXMLExport::ImpGetPageMasterInfoByName(std::u16string_view rName)
{
    if(!rName.empty())
    {
        for(const auto & pInfo : mvPageMasterInfoList)
        {
            if(pInfo)
            {
                if(!pInfo->GetMasterPageName().isEmpty() && rName == pInfo->GetMasterPageName())
                {
                    return pInfo.get();
                }
            }
        }
    }
    return nullptr;
}

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

static OUString findOrAppendImpl( std::vector< OUString >& rVector, const OUString& rText, std::u16string_view pPrefix )
{
    // search rVector if there is already a string that equals rText
    auto aIter = std::find(rVector.begin(), rVector.end(), rText);
    sal_Int32 nIndex = std::distance(rVector.begin(), aIter) + 1;

    // if nothing is found, append the string at the end of rVector
    if( aIter == rVector.end() )
        rVector.push_back( rText );

    // create a reference string with pPrefix and the index of the
    // found or created rText
    return pPrefix + OUString::number( nIndex );
}

static OUString findOrAppendImpl( std::vector< DateTimeDeclImpl >& rVector, const OUString& rText, bool bFixed, sal_Int32 nFormat, std::u16string_view pPrefix )
{
    // search rVector if there is already a DateTimeDeclImpl with rText,bFixed and nFormat
    auto aIter = std::find_if(rVector.begin(), rVector.end(),
        [bFixed, &rText, nFormat](const DateTimeDeclImpl& rDecl) {
            return (rDecl.mbFixed == bFixed) &&
                (!bFixed || (rDecl.maStrText == rText)) &&
                (bFixed || (rDecl.mnFormat == nFormat));
        });
    sal_Int32 nIndex = std::distance(rVector.begin(), aIter) + 1;

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
    return pPrefix + OUString::number( nIndex );
}

constexpr OUStringLiteral gpStrHeaderTextPrefix = u"hdr";
constexpr OUStringLiteral gpStrFooterTextPrefix = u"ftr";
constexpr OUStringLiteral gpStrDateTimeTextPrefix = u"dtd";

HeaderFooterPageSettingsImpl SdXMLExport::ImpPrepDrawPageHeaderFooterDecls( const Reference<XDrawPage>& xDrawPage )
{
    HeaderFooterPageSettingsImpl aSettings;

    if( xDrawPage.is() ) try
    {
        Reference< XPropertySet > xSet( xDrawPage, UNO_QUERY_THROW );
        Reference< XPropertySetInfo > xInfo( xSet->getPropertySetInfo() );

        OUString aStrText;

        static const OUStringLiteral aStrHeaderTextProp( u"HeaderText" );
        if( xInfo->hasPropertyByName( aStrHeaderTextProp ) )
        {
            xSet->getPropertyValue( aStrHeaderTextProp  ) >>= aStrText;
            if( !aStrText.isEmpty() )
                aSettings.maStrHeaderDeclName = findOrAppendImpl( maHeaderDeclsVector, aStrText, gpStrHeaderTextPrefix );
        }

        static const OUStringLiteral aStrFooterTextProp( u"FooterText" );
        if( xInfo->hasPropertyByName( aStrFooterTextProp ) )
        {
            xSet->getPropertyValue( aStrFooterTextProp ) >>= aStrText;
            if( !aStrText.isEmpty() )
                aSettings.maStrFooterDeclName = findOrAppendImpl( maFooterDeclsVector, aStrText, gpStrFooterTextPrefix );
        }

        static const OUStringLiteral aStrDateTimeTextProp( u"DateTimeText" );
        if( xInfo->hasPropertyByName( aStrDateTimeTextProp ) )
        {
            bool bFixed = false;
            sal_Int32 nFormat = 0;
            xSet->getPropertyValue( aStrDateTimeTextProp ) >>= aStrText;
            xSet->getPropertyValue("IsDateTimeFixed") >>= bFixed;
            xSet->getPropertyValue("DateTimeFormat") >>= nFormat;

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
        TOOLS_WARN_EXCEPTION("xmloff.draw", "");
    }

    return aSettings;
}

void SdXMLExport::ImpWriteHeaderFooterDecls()
{
    OUStringBuffer sBuffer;

    if( !maHeaderDeclsVector.empty() )
    {
        // export header decls
        const OUString aPrefix( gpStrHeaderTextPrefix );
        sal_Int32 nIndex = 1;
        for( const auto& rDecl : maHeaderDeclsVector )
        {
            sBuffer.append( aPrefix );
            sBuffer.append( nIndex );
            AddAttribute(XML_NAMESPACE_PRESENTATION, XML_NAME, sBuffer.makeStringAndClear());

            SvXMLElementExport aElem(*this, XML_NAMESPACE_PRESENTATION, XML_HEADER_DECL, true, true);
            Characters(rDecl);
            ++nIndex;
        }
    }

    if( !maFooterDeclsVector.empty() )
    {
        // export footer decls
        const OUString aPrefix( gpStrFooterTextPrefix );
        sal_Int32 nIndex = 1;
        for( const auto& rDecl : maFooterDeclsVector )
        {
            sBuffer.append( aPrefix );
            sBuffer.append( nIndex );
            AddAttribute(XML_NAMESPACE_PRESENTATION, XML_NAME, sBuffer.makeStringAndClear());

            SvXMLElementExport aElem(*this, XML_NAMESPACE_PRESENTATION, XML_FOOTER_DECL, false, false);
            Characters(rDecl);
            ++nIndex;
        }
    }

    if( maDateTimeDeclsVector.empty() )
        return;

    // export footer decls
    const OUString aPrefix( gpStrDateTimeTextPrefix );
    sal_Int32 nIndex = 1;
    for( const auto& rDecl : maDateTimeDeclsVector )
    {
        sBuffer.append( aPrefix );
        sBuffer.append( nIndex );
        AddAttribute( XML_NAMESPACE_PRESENTATION, XML_NAME, sBuffer.makeStringAndClear());

        AddAttribute( XML_NAMESPACE_PRESENTATION, XML_SOURCE, rDecl.mbFixed ? XML_FIXED : XML_CURRENT_DATE );

        if( !rDecl.mbFixed )
            AddAttribute( XML_NAMESPACE_STYLE, XML_DATA_STYLE_NAME, getDataStyleName( rDecl.mnFormat ) );

        SvXMLElementExport aElem(*this, XML_NAMESPACE_PRESENTATION, XML_DATE_TIME_DECL, false, false);
        if( rDecl.mbFixed )
            Characters(rDecl.maStrText);

        ++nIndex;
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

OUString SdXMLExport::ImpCreatePresPageStyleName( const Reference<XDrawPage>& xDrawPage, bool bExportBackground /* = true */ )
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
            static const OUStringLiteral aBackground(u"Background");
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

        const rtl::Reference< SvXMLExportPropertyMapper > aMapperRef( GetPresPagePropsMapper() );

        std::vector<XMLPropertyState> aPropStates(aMapperRef->Filter(*this, xPropSet));

        if( !aPropStates.empty() )
        {
            // there are filtered properties -> hard attributes
            // try to find this style in AutoStylePool
            sStyleName = GetAutoStylePool()->Find(XmlStyleFamily::SD_DRAWINGPAGE_ID, sStyleName, aPropStates);

            if(sStyleName.isEmpty())
            {
                // Style did not exist, add it to AutoStalePool
                sStyleName = GetAutoStylePool()->Add(XmlStyleFamily::SD_DRAWINGPAGE_ID, sStyleName, std::move(aPropStates));
            }
        }
    }

    return sStyleName;
}

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

    if( !IsImpress() )
        return;

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

void SdXMLExport::ImpWritePresentationStyles()
{
    if(!IsImpress())
        return;

    for (sal_Int32 nCnt = 0; nCnt < mnDocMasterPageCount; nCnt++)
    {
        Any aAny(mxDocMasterPages->getByIndex(nCnt));
        Reference<container::XNamed> xNamed;

        if(aAny >>= xNamed)
        {
            // write presentation styles (ONLY if presentation)
            if(IsImpress() && mxDocStyleFamilies.is() && xNamed.is())
            {
                rtl::Reference<XMLStyleExport> aStEx(new XMLStyleExport(*this, GetAutoStylePool().get()));
                const rtl::Reference< SvXMLExportPropertyMapper > aMapperRef( GetPropertySetMapper() );

                OUString aPrefix( xNamed->getName() + "-" );

                aStEx->exportStyleFamily(xNamed->getName(),
                    OUString(XML_STYLE_FAMILY_SD_PRESENTATION_NAME),
                    aMapperRef, false,
                    XmlStyleFamily::SD_PRESENTATION_ID, &aPrefix);
            }
        }
    }
}

void SdXMLExport::ExportMeta_()
{
    uno::Sequence<beans::NamedValue> stats { { "ObjectCount", uno::makeAny(mnObjectCount) } };

    // update document statistics at the model
    uno::Reference<document::XDocumentPropertiesSupplier> xPropSup(GetModel(),
        uno::UNO_QUERY_THROW);
    uno::Reference<document::XDocumentProperties> xDocProps(
        xPropSup->getDocumentProperties());
    if (xDocProps.is()) {
        xDocProps->setDocumentStatistics(stats);
    }

    // call parent
    SvXMLExport::ExportMeta_();
}

void SdXMLExport::ExportFontDecls_()
{
    GetFontAutoStylePool(); // make sure the pool is created
    SvXMLExport::ExportFontDecls_();
}

void SdXMLExport::ExportContent_()
{
    // export <pres:header-decl>, <pres:footer-decl> and <pres:date-time-decl> elements
    ImpWriteHeaderFooterDecls();

    // page export
    for(sal_Int32 nPageInd(0); nPageInd < mnDocDrawPageCount; nPageInd++)
    {
        uno::Reference<drawing::XDrawPage> xDrawPage( mxDocDrawPages->getByIndex(nPageInd), uno::UNO_QUERY );

        // set progress view
        if(GetStatusIndicator().is())
            GetStatusIndicator()->setValue(((nPageInd + 1) * 100) / mnDocDrawPageCount);

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
                    xProps->getPropertyValue("BookmarkURL") >>= aBookmarkURL;

                    if( !aBookmarkURL.isEmpty() )
                    {
                        sal_Int32 nIndex = aBookmarkURL.lastIndexOf( '#' );
                        if( nIndex != -1 )
                        {
                            OUString aFileName( aBookmarkURL.copy( 0, nIndex ) );
                            OUString aBookmarkName( aBookmarkURL.copy( nIndex+1 ) );

                            aBookmarkURL = GetRelativeReference( aFileName ) + "#" + aBookmarkName;
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

            rtl::Reference< xmloff::AnimationsExporter >  xAnimationsExporter;
            uno::Reference< css::animations::XAnimationNodeSupplier > xAnimNodeSupplier;

            // prepare animation export
            if(IsImpress())
            {
                if( getExportFlags() & SvXMLExportFlags::OASIS )
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
                    rtl::Reference< XMLAnimationsExporter > xAnimExport = new XMLAnimationsExporter();
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
            SvXMLElementExport aDPG(*this, XML_NAMESPACE_DRAW, XML_PAGE, true, true);

            // write optional office:forms
            exportFormsElement( xDrawPage );

            // write graphic objects on this page (if any)
            if(xDrawPage.is() && xDrawPage->getCount())
                GetShapeExport()->exportShapes( xDrawPage );

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
                    rtl::Reference< XMLAnimationsExporter > xAnimExport( GetShapeExport()->getAnimationsExporter() );
                    if( xAnimExport.is() )
                        xAnimExport->exportAnimations( *this );

                    xAnimExport = nullptr;
                    GetShapeExport()->setAnimationsExporter( xAnimExport );
                }

                // presentations
                Reference< presentation::XPresentationPage > xPresPage(xDrawPage, UNO_QUERY);
                if(xPresPage.is())
                {
                    Reference< XDrawPage > xNotesPage(xPresPage->getNotesPage());
                    if(xNotesPage.is())
                    {
                        if( !maDrawNotesPagesStyleNames[nPageInd].isEmpty() )
                            AddAttribute(XML_NAMESPACE_DRAW, XML_STYLE_NAME, maDrawNotesPagesStyleNames[nPageInd]);

                        ImplExportHeaderFooterDeclAttributes( maDrawNotesPagesHeaderFooterSettings[nPageInd] );

                        // write presentation notes
                        SvXMLElementExport aPSY(*this, XML_NAMESPACE_PRESENTATION, XML_NOTES, true, true);

                        // write optional office:forms
                        exportFormsElement( xNotesPage );

                        // write shapes per se
                        GetShapeExport()->exportShapes( xNotesPage );
                    }
                }
            }

            exportAnnotations( xDrawPage );
        }
    }

    if( IsImpress() )
        exportPresentationSettings();
}

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

        bool bHasAttr = false;

        bool bTemp = false;

        // export range
        xPresProps->getPropertyValue("IsShowAll") >>= bTemp;
        if( !bTemp )
        {
            OUString aFirstPage;
            xPresProps->getPropertyValue("FirstPage") >>= aFirstPage;
            if( !aFirstPage.isEmpty() )
            {
                AddAttribute(XML_NAMESPACE_PRESENTATION, XML_START_PAGE, aFirstPage );
                bHasAttr = true;
            }
            else
            {
                OUString aCustomShow;
                xPresProps->getPropertyValue("CustomShow") >>= aCustomShow;
                if( !aCustomShow.isEmpty() )
                {
                    AddAttribute(XML_NAMESPACE_PRESENTATION, XML_SHOW, aCustomShow );
                    bHasAttr = true;
                }
            }
        }

        xPresProps->getPropertyValue("IsEndless") >>= bTemp;
        if( bTemp )
        {
            AddAttribute(XML_NAMESPACE_PRESENTATION, XML_ENDLESS, XML_TRUE );
            bHasAttr = true;

            sal_Int32 nPause = 0;
            xPresProps->getPropertyValue("Pause") >>= nPause;

            util::Duration aDuration;
            aDuration.Seconds = static_cast<sal_uInt16>(nPause);

            OUStringBuffer aOut;
            ::sax::Converter::convertDuration(aOut, aDuration);
            AddAttribute(XML_NAMESPACE_PRESENTATION, XML_PAUSE, aOut.makeStringAndClear() );
        }

        xPresProps->getPropertyValue("AllowAnimations") >>= bTemp;
        if( !bTemp )
        {
            AddAttribute(XML_NAMESPACE_PRESENTATION, XML_ANIMATIONS, XML_DISABLED );
            bHasAttr = true;
        }

        xPresProps->getPropertyValue("IsAlwaysOnTop") >>= bTemp;
        if( bTemp )
        {
            AddAttribute(XML_NAMESPACE_PRESENTATION, XML_STAY_ON_TOP, XML_TRUE );
            bHasAttr = true;
        }

        xPresProps->getPropertyValue("IsAutomatic") >>= bTemp;
        if( bTemp )
        {
            AddAttribute(XML_NAMESPACE_PRESENTATION, XML_FORCE_MANUAL, XML_TRUE );
            bHasAttr = true;
        }

        xPresProps->getPropertyValue("IsFullScreen") >>= bTemp;
        if( !bTemp )
        {
            AddAttribute(XML_NAMESPACE_PRESENTATION, XML_FULL_SCREEN, XML_FALSE );
            bHasAttr = true;
        }

        // We need to always export this attribute, because the import had the wrong default (tdf#108824)
        xPresProps->getPropertyValue("IsMouseVisible") >>= bTemp;
        AddAttribute(XML_NAMESPACE_PRESENTATION, XML_MOUSE_VISIBLE, bTemp ? XML_TRUE : XML_FALSE);
        bHasAttr = true;

        xPresProps->getPropertyValue("StartWithNavigator") >>= bTemp;
        if( bTemp )
        {
            AddAttribute(XML_NAMESPACE_PRESENTATION, XML_START_WITH_NAVIGATOR, XML_TRUE );
            bHasAttr = true;
        }

        xPresProps->getPropertyValue("UsePen") >>= bTemp;
        if( bTemp )
        {
            AddAttribute(XML_NAMESPACE_PRESENTATION, XML_MOUSE_AS_PEN, XML_TRUE );
            bHasAttr = true;
        }

        xPresProps->getPropertyValue("IsTransitionOnClick") >>= bTemp;
        if( !bTemp )
        {
            AddAttribute(XML_NAMESPACE_PRESENTATION, XML_TRANSITION_ON_CLICK, XML_DISABLED );
            bHasAttr = true;
        }

        xPresProps->getPropertyValue("IsShowLogo") >>= bTemp;
        if( bTemp )
        {
            AddAttribute(XML_NAMESPACE_PRESENTATION, XML_SHOW_LOGO, XML_TRUE );
            bHasAttr = true;
        }

        Reference< container::XNameContainer > xShows;
        Sequence< OUString > aShowNames;
        bool bHasNames = false;

        Reference< XCustomPresentationSupplier > xSup( GetModel(), UNO_QUERY );
        if( xSup.is() )
        {
            xShows = xSup->getCustomPresentations();
            if( xShows.is() )
            {
                aShowNames = xShows->getElementNames();
                bHasNames = aShowNames.hasElements();
            }
        }

        if( bHasAttr || bHasNames )
        {
            SvXMLElementExport aSettings(*this, XML_NAMESPACE_PRESENTATION, XML_SETTINGS, true, true);

            if( !bHasNames )
                return;

            Reference< XIndexContainer > xShow;
            Reference< XNamed > xPageName;

            OUStringBuffer sTmp;

            for( const auto& rShowName : std::as_const(aShowNames) )
            {
                AddAttribute(XML_NAMESPACE_PRESENTATION, XML_NAME, rShowName );

                xShows->getByName( rShowName ) >>= xShow;
                SAL_WARN_IF( !xShow.is(), "xmloff", "invalid custom show!" );
                if( !xShow.is() )
                    continue;

                const sal_Int32 nPageCount = xShow->getCount();
                for( sal_Int32 nPage = 0; nPage < nPageCount; nPage++ )
                {
                    xShow->getByIndex( nPage ) >>= xPageName;

                    if( !xPageName.is() )
                        continue;

                    if( !sTmp.isEmpty() )
                        sTmp.append(  ',' );
                    sTmp.append( xPageName->getName() );

                }

                if( !sTmp.isEmpty() )
                    AddAttribute(XML_NAMESPACE_PRESENTATION, XML_PAGES, sTmp.makeStringAndClear() );

                SvXMLElementExport aShows(*this, XML_NAMESPACE_PRESENTATION, XML_SHOW, true, true);
            }
        }
    }
    catch(const uno::Exception&)
    {
        TOOLS_WARN_EXCEPTION("xmloff.draw", "while exporting <presentation:settings>");
    }
}

void SdXMLExport::ExportStyles_(bool bUsed)
{
    GetPropertySetMapper()->SetAutoStyles( false );

    // export fill styles
    SvXMLExport::ExportStyles_( bUsed );

    // write draw:style-name for object graphic-styles
    GetShapeExport()->ExportGraphicDefaults();

    // do not export in ODF 1.1 or older
    if (getSaneDefaultVersion() >= SvtSaveOptions::ODFSVER_012)
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

        if( xInfoSetInfo->hasPropertyByName( gsPageLayoutNames ) )
        {
            xInfoSet->setPropertyValue( gsPageLayoutNames, Any(maDrawPagesAutoLayoutNames) );
        }
    }
}

void SdXMLExport::collectAutoStyles()
{
    SvXMLExport::collectAutoStyles();
    if (mbAutoStylesCollected)
        return;

    Reference< beans::XPropertySet > xInfoSet( getExportInfo() );
    if( xInfoSet.is() )
    {
        Reference< beans::XPropertySetInfo > xInfoSetInfo( xInfoSet->getPropertySetInfo() );

        if( xInfoSetInfo->hasPropertyByName( gsPageLayoutNames ) )
        {
            xInfoSet->getPropertyValue( gsPageLayoutNames ) >>= maDrawPagesAutoLayoutNames;
        }
    }

    GetPropertySetMapper()->SetAutoStyles( true );

    if( getExportFlags() & SvXMLExportFlags::STYLES )
    {
        // #80012# PageMaster export moved from _ExportStyles
        // prepare page-master infos
        ImpPrepPageMasterInfos();

        // prepare draw:style-name for master page export
        ImpPrepMasterPageInfos();
    }

    if( getExportFlags() & SvXMLExportFlags::CONTENT )
    {
        // prepare draw:style-name for page export
        ImpPrepDrawPageInfos();
    }

    if( getExportFlags() & SvXMLExportFlags::STYLES )
    {
        // create auto style infos for shapes on master handout page
        if( IsImpress() )
        {
            Reference< presentation::XHandoutMasterSupplier > xHandoutSupp( GetModel(), UNO_QUERY );
            if( xHandoutSupp.is() )
            {
                Reference< XDrawPage > xHandoutPage( xHandoutSupp->getHandoutMasterPage() );
                if( xHandoutPage.is() && xHandoutPage->getCount())
                    GetShapeExport()->collectShapesAutoStyles( xHandoutPage );
            }
        }

        // create auto style infos for objects on master pages
        for(sal_Int32 nMPageId(0); nMPageId < mnDocMasterPageCount; nMPageId++)
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
                    aMasterPageNamePrefix += "-";
                }
                GetShapeExport()->setPresentationStylePrefix( aMasterPageNamePrefix );

                if(xMasterPage.is() && xMasterPage->getCount())
                    GetShapeExport()->collectShapesAutoStyles( xMasterPage );

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

                            if(xNotesPage->getCount())
                                GetShapeExport()->collectShapesAutoStyles( xNotesPage );
                        }
                    }
                }
                collectAnnotationAutoStyles(xMasterPage);
            }
        }
    }

    if( getExportFlags() & SvXMLExportFlags::CONTENT )
    {
        // prepare animations exporter if impress
        if(IsImpress() && (!(getExportFlags() & SvXMLExportFlags::OASIS)) )
        {
            rtl::Reference< XMLAnimationsExporter > xAnimExport = new XMLAnimationsExporter();
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
                    aMasterPageNamePrefix += "-";
                }

                GetShapeExport()->setPresentationStylePrefix( aMasterPageNamePrefix );

                // prepare object infos
                if(xDrawPage.is() && xDrawPage->getCount())
                    GetShapeExport()->collectShapesAutoStyles( xDrawPage );

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

                            if(xNotesPage->getCount())
                                GetShapeExport()->collectShapesAutoStyles( xNotesPage );
                        }
                    }
                }

                collectAnnotationAutoStyles( xDrawPage );
            }
        }
        if (IsImpress())
        {
            rtl::Reference< XMLAnimationsExporter > xAnimExport;
            GetShapeExport()->setAnimationsExporter( xAnimExport );
        }
    }

    mbAutoStylesCollected = true;
}

void SdXMLExport::ExportAutoStyles_()
{
    collectAutoStyles();

    if( getExportFlags() & SvXMLExportFlags::STYLES )
    {
        // write page-master infos
        ImpWritePageMasterInfos();
    }

    // export draw-page styles
    GetAutoStylePool()->exportXML( XmlStyleFamily::SD_DRAWINGPAGE_ID );

    exportAutoDataStyles();

    GetShapeExport()->exportAutoStyles();

    SvXMLExportFlags nContentAutostyles = SvXMLExportFlags::CONTENT | SvXMLExportFlags::AUTOSTYLES;
    if ( ( getExportFlags() & nContentAutostyles ) == nContentAutostyles )
        GetFormExport()->exportAutoStyles( );

    // ...for text
    GetTextParagraphExport()->exportTextAutoStyles();
}

void SdXMLExport::ExportMasterStyles_()
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
                    const OUString& sString = pInfo->GetName();
                    AddAttribute(XML_NAMESPACE_STYLE, XML_PAGE_LAYOUT_NAME, sString );
                }

                // draw:style-name
                if( !maHandoutMasterStyleName.isEmpty() )
                    AddAttribute(XML_NAMESPACE_DRAW, XML_STYLE_NAME, maHandoutMasterStyleName);

                ImplExportHeaderFooterDeclAttributes( maHandoutPageHeaderFooterSettings );

                // write masterpage
                SvXMLElementExport aMPG(*this, XML_NAMESPACE_STYLE, XML_HANDOUT_MASTER, true, true);

                // write graphic objects on this master page (if any)
                if(xHandoutPage.is() && xHandoutPage->getCount())
                    GetShapeExport()->exportShapes( xHandoutPage );
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
            Reference < container::XNamed > xNamed(xMasterPage, UNO_QUERY);
            if(xNamed.is())
            {
                bool bEncoded = false;
                OUString sMasterPageName = xNamed->getName();
                AddAttribute(XML_NAMESPACE_STYLE, XML_NAME,
                    EncodeStyleName( sMasterPageName, &bEncoded ));
                if( bEncoded )
                    AddAttribute(
                        XML_NAMESPACE_STYLE, XML_DISPLAY_NAME,
                        sMasterPageName );
            }

            ImpXMLEXPPageMasterInfo* pInfo = mvPageMasterUsageList.at( nMPageId );
            if(pInfo)
            {
                const OUString& sString = pInfo->GetName();
                AddAttribute(XML_NAMESPACE_STYLE, XML_PAGE_LAYOUT_NAME, sString );
            }

            // draw:style-name (background attributes)
            if( !maMasterPagesStyleNames[nMPageId].isEmpty() )
                AddAttribute(XML_NAMESPACE_DRAW, XML_STYLE_NAME,
                        maMasterPagesStyleNames[nMPageId]);

            // write masterpage
            SvXMLElementExport aMPG(*this, XML_NAMESPACE_STYLE, XML_MASTER_PAGE, true, true);

            // write optional office:forms
            exportFormsElement( xMasterPage );

            // write optional loext:theme
            if (IsImpress())
            {
                ExportThemeElement(xMasterPage);
            }

            // write graphic objects on this master page (if any)
            if(xMasterPage.is() && xMasterPage->getCount())
                GetShapeExport()->exportShapes( xMasterPage );

            // write presentation notes (ONLY if presentation)
            if(IsImpress())
            {
                Reference< presentation::XPresentationPage > xPresPage(xMasterPage, UNO_QUERY);
                if(xPresPage.is())
                {
                    Reference< XDrawPage > xNotesPage(xPresPage->getNotesPage());
                    if(xNotesPage.is())
                    {
                        ImpXMLEXPPageMasterInfo* pMasterInfo = mvNotesPageMasterUsageList.at( nMPageId );
                        if(pMasterInfo)
                        {
                            const OUString& sString = pMasterInfo->GetName();
                            AddAttribute(XML_NAMESPACE_STYLE, XML_PAGE_LAYOUT_NAME, sString);
                        }

                        // write presentation notes
                        SvXMLElementExport aPSY(*this, XML_NAMESPACE_PRESENTATION, XML_NOTES, true, true);

                        // write optional office:forms
                        exportFormsElement( xNotesPage );

                        // write shapes per se
                        GetShapeExport()->exportShapes( xNotesPage );
                    }
                }
            }
            exportAnnotations( xMasterPage );
        }
    }
}

void SdXMLExport::exportFormsElement( const Reference< XDrawPage >& xDrawPage )
{
    if( !xDrawPage.is() )
        return;

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

void SdXMLExport::ExportThemeElement(const uno::Reference<drawing::XDrawPage>& xDrawPage)
{
    uno::Reference<beans::XPropertySet> xPropertySet(xDrawPage, uno::UNO_QUERY);
    if (!xPropertySet.is())
        return;

    comphelper::SequenceAsHashMap aMap(xPropertySet->getPropertyValue("Theme"));
    if (aMap.empty())
    {
        return;
    }

    if ((getSaneDefaultVersion() & SvtSaveOptions::ODFSVER_EXTENDED) == 0)
    {
        // Do not export in standard ODF 1.3 or older.
        return;
    }

    auto it = aMap.find("Name");
    if (it != aMap.end())
    {
        OUString aName;
        it->second >>= aName;
        AddAttribute(XML_NAMESPACE_LO_EXT, XML_NAME, aName);
    }
    SvXMLElementExport aTheme(*this, XML_NAMESPACE_LO_EXT, XML_THEME, true, true);

    uno::Sequence<util::Color> aColors;
    it = aMap.find("ColorScheme");
    if (it != aMap.end())
    {
        it->second >>= aColors;
    }
    if (!aColors.hasElements())
    {
        return;
    }

    it = aMap.find("ColorSchemeName");
    if (it != aMap.end())
    {
        OUString aName;
        it->second >>= aName;
        AddAttribute(XML_NAMESPACE_LO_EXT, XML_NAME, aName);
    }
    SvXMLElementExport aColorTable(*this, XML_NAMESPACE_LO_EXT, XML_COLOR_TABLE, true, true);

    static const XMLTokenEnum aColorTokens[] = {
        XML_DK1, // Background 1
        XML_LT1, // Text 1
        XML_DK2, // Background 2
        XML_LT2, // Text 2
        XML_ACCENT1,
        XML_ACCENT2,
        XML_ACCENT3,
        XML_ACCENT4,
        XML_ACCENT5,
        XML_ACCENT6,
        XML_HLINK, // Hyperlink
        XML_FOLHLINK, // Followed hyperlink
    };
    for (size_t nColor = 0; nColor < aColors.size(); ++nColor)
    {
        // Import goes via svx::Theme::FromAny(), which sanitizes user input.
        assert(nColor < SAL_N_ELEMENTS(aColorTokens));

        AddAttribute(XML_NAMESPACE_LO_EXT, XML_NAME, GetXMLToken(aColorTokens[nColor]));

        OUStringBuffer sValue;
        sax::Converter::convertColor(sValue, aColors[nColor]);
        AddAttribute(XML_NAMESPACE_LO_EXT, XML_COLOR, sValue.makeStringAndClear());

        SvXMLElementExport aColor(*this, XML_NAMESPACE_LO_EXT, XML_COLOR, true, true);
    }
}

void SdXMLExport::GetViewSettings(uno::Sequence<beans::PropertyValue>& rProps)
{
    Reference< beans::XPropertySet > xPropSet( GetModel(), UNO_QUERY );
    if( !xPropSet.is() )
        return;

    awt::Rectangle aVisArea;
    xPropSet->getPropertyValue("VisibleArea") >>= aVisArea;

    rProps.realloc(4);
    beans::PropertyValue* pProps = rProps.getArray();

    pProps[0].Name = "VisibleAreaTop";
    pProps[0].Value <<= aVisArea.Y;
    pProps[1].Name = "VisibleAreaLeft";
    pProps[1].Value <<= aVisArea.X;
    pProps[2].Name = "VisibleAreaWidth";
    pProps[2].Value <<= aVisArea.Width;
    pProps[3].Name = "VisibleAreaHeight";
    pProps[3].Value <<= aVisArea.Height;

}

void SdXMLExport::GetConfigurationSettings(uno::Sequence<beans::PropertyValue>& rProps)
{
    Reference< lang::XMultiServiceFactory > xFac( GetModel(), UNO_QUERY );
    if( !xFac.is() )
        return;

    Reference< beans::XPropertySet > xProps( xFac->createInstance("com.sun.star.document.Settings"), UNO_QUERY );
    if( xProps.is() )
        SvXMLUnitConverter::convertPropertySet( rProps, xProps );
    DocumentSettingsSerializer *pFilter(dynamic_cast<DocumentSettingsSerializer *>(xProps.get()));
    if (!pFilter)
        return;
    const uno::Reference< embed::XStorage > xStorage(GetTargetStorage());
    if (!xStorage.is())
        return;
    rProps = pFilter->filterStreamsToStorage(xStorage, rProps);
}

void SdXMLExport::addDataStyle(const sal_Int32 nNumberFormat, bool bTimeFormat )
{
    sal_Int32 nFormat = nNumberFormat;
    if( (nNumberFormat > 1) && (nNumberFormat <= 0x0f) )
        nFormat -= 2;

    if( bTimeFormat )
    {
        maUsedTimeStyles.insert( nFormat );
    }
    else
    {
        maUsedDateStyles.insert( nFormat );
    }
}

void SdXMLExport::exportDataStyles()
{
    // there are no data styles to export in draw/impress yet
}

void SdXMLExport::exportAutoDataStyles()
{
    for( const auto& rUsedDateStyle : maUsedDateStyles )
        SdXMLNumberStylesExporter::exportDateStyle( *this, rUsedDateStyle );

    for( const auto& rUsedTimeStyle : maUsedTimeStyles )
        SdXMLNumberStylesExporter::exportTimeStyle( *this, rUsedTimeStyle );

    if(HasFormExport())
        GetFormExport()->exportAutoControlNumberStyles();
}

OUString SdXMLExport::getDataStyleName(const sal_Int32 nNumberFormat, bool bTimeFormat ) const
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
        Reference< XIndexAccess > xNavOrder( xSet->getPropertyValue("NavigationOrder"), UNO_QUERY_THROW );

        Reference< XIndexAccess > xZOrderAccess = xDrawPage;

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
                    if( !sNavOrder.isEmpty() )
                        sNavOrder.append( ' ' );
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

void SdXMLExport::collectAnnotationAutoStyles( const Reference<XDrawPage>& xDrawPage )
{
    Reference< XAnnotationAccess > xAnnotationAccess( xDrawPage, UNO_QUERY );
    if( !xAnnotationAccess.is() ) return;

    try
    {
        Reference< XAnnotationEnumeration > xAnnotationEnumeration( xAnnotationAccess->createAnnotationEnumeration() );
        if( xAnnotationEnumeration.is() )
        {
            while( xAnnotationEnumeration->hasMoreElements() )
            {
                Reference< XAnnotation > xAnnotation( xAnnotationEnumeration->nextElement(), UNO_SET_THROW );
                Reference< XText > xText( xAnnotation->getTextRange() );
                if(xText.is() && !xText->getString().isEmpty())
                    GetTextParagraphExport()->collectTextAutoStyles( xText );
            }
        }
    }
    catch(const Exception&)
    {
        TOOLS_WARN_EXCEPTION("xmloff.draw",
                             "exception caught during export of annotation auto styles");
    }
}

void SdXMLExport::exportAnnotations( const Reference<XDrawPage>& xDrawPage )
{
    // do not export in standard ODF 1.3 or older
    if ((getSaneDefaultVersion() & SvtSaveOptions::ODFSVER_EXTENDED) == 0)
    {
        return;
    }

    Reference< XAnnotationAccess > xAnnotationAccess( xDrawPage, UNO_QUERY );
    if( !xAnnotationAccess.is() )
        return;

    try
    {
        Reference< XAnnotationEnumeration > xAnnotationEnumeration( xAnnotationAccess->createAnnotationEnumeration() );
        if( xAnnotationEnumeration.is() && xAnnotationEnumeration->hasMoreElements() )
        {
            bool bRemovePersonalInfo = SvtSecurityOptions::IsOptionSet(
                SvtSecurityOptions::EOption::DocWarnRemovePersonalInfo );

            OUStringBuffer sStringBuffer;
            do
            {
                Reference< XAnnotation > xAnnotation( xAnnotationEnumeration->nextElement(), UNO_SET_THROW );

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
                SvXMLElementExport aElem(*this, XML_NAMESPACE_OFFICE_EXT, XML_ANNOTATION, false, true);

                // author
                OUString aAuthor( xAnnotation->getAuthor() );
                if( !aAuthor.isEmpty() )
                {
                    SvXMLElementExport aCreatorElem( *this, XML_NAMESPACE_DC, XML_CREATOR, true, false );
                    Characters( bRemovePersonalInfo
                            ? "Author" + OUString::number( SvXMLExport::GetInfoID(aAuthor) )
                            : aAuthor );
                }

                // initials
                OUString aInitials( xAnnotation->getInitials() );
                if( !aInitials.isEmpty() )
                {
                    // OFFICE-3776 export meta:creator-initials for ODF 1.3
                    SvXMLElementExport aInitialsElem( *this,
                            (SvtSaveOptions::ODFSVER_013 <= getSaneDefaultVersion())
                                ? XML_NAMESPACE_META
                                : XML_NAMESPACE_LO_EXT,
                            (SvtSaveOptions::ODFSVER_013 <= getSaneDefaultVersion())
                                ? XML_CREATOR_INITIALS
                                : XML_SENDER_INITIALS,
                            true, false );
                    Characters( bRemovePersonalInfo
                            ? OUString::number( SvXMLExport::GetInfoID(aInitials) )
                            : aInitials );
                }

                {
                    // date time
                    css::util::DateTime aDate( bRemovePersonalInfo
                            ? css::util::DateTime(0, 0, 0, 0, 1, 1, 1970, true) // Epoch time
                            : xAnnotation->getDateTime() );
                    ::sax::Converter::convertDateTime(sStringBuffer, aDate, nullptr, true);
                    SvXMLElementExport aDateElem( *this, XML_NAMESPACE_DC, XML_DATE, true, false );
                    Characters( sStringBuffer.makeStringAndClear() );
                }

                css::uno::Reference < css::text::XText > xText( xAnnotation->getTextRange() );
                if( xText.is() )
                    GetTextParagraphExport()->exportText( xText );
            }
            while( xAnnotationEnumeration->hasMoreElements() );
        }
    }
    catch(const Exception&)
    {
        TOOLS_WARN_EXCEPTION("xmloff.draw", "exception caught during export of annotations");
    }
}

extern "C" SAL_DLLPUBLIC_EXPORT uno::XInterface*
com_sun_star_comp_Impress_XMLOasisExporter_get_implementation(
    uno::XComponentContext* pCtx, uno::Sequence<uno::Any> const& /*rSeq*/)
{
    return cppu::acquire(new SdXMLExport(
        pCtx, "XMLImpressExportOasis", false,
        SvXMLExportFlags::OASIS | SvXMLExportFlags::META | SvXMLExportFlags::STYLES
            | SvXMLExportFlags::MASTERSTYLES | SvXMLExportFlags::AUTOSTYLES
            | SvXMLExportFlags::CONTENT | SvXMLExportFlags::SCRIPTS | SvXMLExportFlags::SETTINGS
            | SvXMLExportFlags::FONTDECLS | SvXMLExportFlags::EMBEDDED));
}

extern "C" SAL_DLLPUBLIC_EXPORT uno::XInterface*
com_sun_star_comp_Impress_XMLOasisStylesExporter_get_implementation(
    uno::XComponentContext* pCtx, uno::Sequence<uno::Any> const& /*rSeq*/)
{
    return cppu::acquire(new SdXMLExport(
        pCtx, "XMLImpressStylesExportOasis", false,
        SvXMLExportFlags::OASIS | SvXMLExportFlags::STYLES | SvXMLExportFlags::MASTERSTYLES
            | SvXMLExportFlags::AUTOSTYLES | SvXMLExportFlags::FONTDECLS));
}

extern "C" SAL_DLLPUBLIC_EXPORT uno::XInterface*
com_sun_star_comp_Impress_XMLOasisContentExporter_get_implementation(
    uno::XComponentContext* pCtx, uno::Sequence<uno::Any> const& /*rSeq*/)
{
    return cppu::acquire(new SdXMLExport(pCtx, "XMLImpressContentExportOasis", false,
                                         SvXMLExportFlags::OASIS | SvXMLExportFlags::AUTOSTYLES
                                             | SvXMLExportFlags::CONTENT | SvXMLExportFlags::SCRIPTS
                                             | SvXMLExportFlags::FONTDECLS));
}

extern "C" SAL_DLLPUBLIC_EXPORT uno::XInterface*
com_sun_star_comp_Impress_XMLOasisMetaExporter_get_implementation(
    uno::XComponentContext* pCtx, uno::Sequence<uno::Any> const& /*rSeq*/)
{
    return cppu::acquire(new SdXMLExport(pCtx, "XMLImpressMetaExportOasis", false,
                                         SvXMLExportFlags::OASIS | SvXMLExportFlags::META));
}

extern "C" SAL_DLLPUBLIC_EXPORT uno::XInterface*
com_sun_star_comp_Impress_XMLOasisSettingsExporter_get_implementation(
    uno::XComponentContext* pCtx, uno::Sequence<uno::Any> const& /*rSeq*/)
{
    return cppu::acquire(new SdXMLExport(pCtx, "XMLImpressSettingsExportOasis", false,
                                         SvXMLExportFlags::OASIS | SvXMLExportFlags::SETTINGS));
}

extern "C" SAL_DLLPUBLIC_EXPORT uno::XInterface*
com_sun_star_comp_Impress_XMLExporter_get_implementation(uno::XComponentContext* pCtx,
                                                         uno::Sequence<uno::Any> const& /*rSeq*/)
{
    return cppu::acquire(new SdXMLExport(
        pCtx, "XMLImpressExportOOO", false,
        SvXMLExportFlags::META | SvXMLExportFlags::STYLES | SvXMLExportFlags::MASTERSTYLES
            | SvXMLExportFlags::AUTOSTYLES | SvXMLExportFlags::CONTENT | SvXMLExportFlags::SCRIPTS
            | SvXMLExportFlags::SETTINGS | SvXMLExportFlags::FONTDECLS
            | SvXMLExportFlags::EMBEDDED));
}

extern "C" SAL_DLLPUBLIC_EXPORT uno::XInterface*
com_sun_star_comp_Draw_XMLExporter_get_implementation(uno::XComponentContext* pCtx,
                                                      uno::Sequence<uno::Any> const& /*rSeq*/)
{
    return cppu::acquire(new SdXMLExport(
        pCtx, "XMLDrawExportOOO", true,
        SvXMLExportFlags::META | SvXMLExportFlags::STYLES | SvXMLExportFlags::MASTERSTYLES
            | SvXMLExportFlags::AUTOSTYLES | SvXMLExportFlags::CONTENT | SvXMLExportFlags::SCRIPTS
            | SvXMLExportFlags::SETTINGS | SvXMLExportFlags::FONTDECLS
            | SvXMLExportFlags::EMBEDDED));
}

extern "C" SAL_DLLPUBLIC_EXPORT uno::XInterface*
com_sun_star_comp_Draw_XMLOasisSettingsExporter_get_implementation(
    uno::XComponentContext* pCtx, uno::Sequence<uno::Any> const& /*rSeq*/)
{
    return cppu::acquire(new SdXMLExport(pCtx, "XMLDrawSettingsExportOasis", true,
                                         SvXMLExportFlags::OASIS | SvXMLExportFlags::SETTINGS));
}

extern "C" SAL_DLLPUBLIC_EXPORT uno::XInterface*
com_sun_star_comp_Draw_XMLOasisMetaExporter_get_implementation(
    uno::XComponentContext* pCtx, uno::Sequence<uno::Any> const& /*rSeq*/)
{
    return cppu::acquire(new SdXMLExport(pCtx, "XMLDrawMetaExportOasis", true,
                                         SvXMLExportFlags::OASIS | SvXMLExportFlags::META));
}

extern "C" SAL_DLLPUBLIC_EXPORT uno::XInterface*
com_sun_star_comp_Draw_XMLOasisContentExporter_get_implementation(
    uno::XComponentContext* pCtx, uno::Sequence<uno::Any> const& /*rSeq*/)
{
    return cppu::acquire(new SdXMLExport(pCtx, "XMLDrawContentExportOasis", true,
                                         SvXMLExportFlags::OASIS | SvXMLExportFlags::AUTOSTYLES
                                             | SvXMLExportFlags::CONTENT | SvXMLExportFlags::SCRIPTS
                                             | SvXMLExportFlags::FONTDECLS));
}

extern "C" SAL_DLLPUBLIC_EXPORT uno::XInterface*
com_sun_star_comp_Draw_XMLOasisStylesExporter_get_implementation(
    uno::XComponentContext* pCtx, uno::Sequence<uno::Any> const& /*rSeq*/)
{
    return cppu::acquire(new SdXMLExport(
        pCtx, "XMLDrawStylesExportOasis", true,
        SvXMLExportFlags::OASIS | SvXMLExportFlags::STYLES | SvXMLExportFlags::MASTERSTYLES
            | SvXMLExportFlags::AUTOSTYLES | SvXMLExportFlags::FONTDECLS));
}

extern "C" SAL_DLLPUBLIC_EXPORT uno::XInterface*
com_sun_star_comp_Draw_XMLOasisExporter_get_implementation(uno::XComponentContext* pCtx,
                                                           uno::Sequence<uno::Any> const& /*rSeq*/)
{
    return cppu::acquire(new SdXMLExport(
        pCtx, "XMLDrawExportOasis", true,
        SvXMLExportFlags::OASIS | SvXMLExportFlags::META | SvXMLExportFlags::STYLES
            | SvXMLExportFlags::MASTERSTYLES | SvXMLExportFlags::AUTOSTYLES
            | SvXMLExportFlags::CONTENT | SvXMLExportFlags::SCRIPTS | SvXMLExportFlags::SETTINGS
            | SvXMLExportFlags::FONTDECLS | SvXMLExportFlags::EMBEDDED));
}

extern "C" SAL_DLLPUBLIC_EXPORT uno::XInterface*
com_sun_star_comp_DrawingLayer_XMLExporter_get_implementation(
    uno::XComponentContext* pCtx, uno::Sequence<uno::Any> const& /*rSeq*/)
{
    return cppu::acquire(
        new SdXMLExport(pCtx, "XMLDrawingLayerExport", true,
                        SvXMLExportFlags::OASIS | SvXMLExportFlags::STYLES
                            | SvXMLExportFlags::AUTOSTYLES | SvXMLExportFlags::CONTENT
                            | SvXMLExportFlags::FONTDECLS | SvXMLExportFlags::EMBEDDED));
}

extern "C" SAL_DLLPUBLIC_EXPORT uno::XInterface*
com_sun_star_comp_Impress_XMLClipboardExporter_get_implementation(
    uno::XComponentContext* pCtx, uno::Sequence<uno::Any> const& /*rSeq*/)
{
    return cppu::acquire(
        new SdXMLExport(pCtx, "XMLImpressClipboardExport", /*bIsDraw=*/false,
                        SvXMLExportFlags::OASIS | SvXMLExportFlags::STYLES
                            | SvXMLExportFlags::AUTOSTYLES | SvXMLExportFlags::CONTENT
                            | SvXMLExportFlags::FONTDECLS | SvXMLExportFlags::EMBEDDED));
}

XMLFontAutoStylePool* SdXMLExport::CreateFontAutoStylePool()
{
    bool bEmbedFonts = false;
    bool bEmbedUsedOnly = false;
    bool bEmbedLatinScript = true;
    bool bEmbedAsianScript = true;
    bool bEmbedComplexScript = true;

    if (getExportFlags() & SvXMLExportFlags::CONTENT)
    {
        try
        {
            Reference<lang::XMultiServiceFactory> xFactory(GetModel(), UNO_QUERY);
            Reference<beans::XPropertySet> xProps;
            Reference<beans::XPropertySetInfo> xInfo;

            if (xFactory.is())
                xProps.set(xFactory->createInstance("com.sun.star.document.Settings"), UNO_QUERY);
            if (xProps.is())
                xInfo =  xProps->getPropertySetInfo();
            if (xInfo.is() && xProps.is())
            {
                if (xInfo->hasPropertyByName("EmbedFonts"))
                    xProps->getPropertyValue("EmbedFonts") >>= bEmbedFonts;
                if (xInfo->hasPropertyByName("EmbedOnlyUsedFonts"))
                    xProps->getPropertyValue("EmbedOnlyUsedFonts") >>= bEmbedUsedOnly;
                if (xInfo->hasPropertyByName("EmbedLatinScriptFonts"))
                    xProps->getPropertyValue("EmbedLatinScriptFonts") >>= bEmbedLatinScript;
                if (xInfo->hasPropertyByName("EmbedAsianScriptFonts"))
                    xProps->getPropertyValue("EmbedAsianScriptFonts") >>= bEmbedAsianScript;
                if (xInfo->hasPropertyByName("EmbedComplexScriptFonts"))
                    xProps->getPropertyValue("EmbedComplexScriptFonts") >>= bEmbedComplexScript;
            }
        } catch(...)
        {
            // clipboard document doesn't have shell so throws from getPropertyValue
            // gallery elements may not support com.sun.star.document.Settings so throws from createInstance
        }
    }

    XMLFontAutoStylePool *pPool = new XMLFontAutoStylePool( *this, bEmbedFonts );
    pPool->setEmbedOnlyUsedFonts(bEmbedUsedOnly);
    pPool->setEmbedFontScripts(bEmbedLatinScript, bEmbedAsianScript, bEmbedComplexScript);

    Reference< beans::XPropertySet > xProps( GetModel(), UNO_QUERY );
    if ( xProps.is() ) {
        Sequence<Any> aAnySeq;
        if( xProps->getPropertyValue("Fonts") >>= aAnySeq )
        {
            if( aAnySeq.getLength() % 5 == 0 )
            {
                int nLen = aAnySeq.getLength() / 5;
                int nSeqIndex = 0;
                for( int i = 0; i < nLen; i++ )
                {
                    OUString sFamilyName, sStyleName;
                    sal_Int16 eFamily(FAMILY_DONTKNOW),
                        ePitch(PITCH_DONTKNOW),
                        eCharSet(RTL_TEXTENCODING_DONTKNOW);

                    aAnySeq[nSeqIndex++] >>= sFamilyName;
                    aAnySeq[nSeqIndex++] >>= sStyleName;
                    aAnySeq[nSeqIndex++] >>= eFamily;
                    aAnySeq[nSeqIndex++] >>= ePitch;
                    aAnySeq[nSeqIndex++] >>= eCharSet;

                    pPool->Add( sFamilyName, sStyleName, FontFamily( eFamily ), FontPitch( ePitch ), rtl_TextEncoding( eCharSet ) );
                }
            }
        }
    }

    return pPool;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
