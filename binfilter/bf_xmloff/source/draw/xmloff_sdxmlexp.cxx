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

#ifdef _MSC_VER
#pragma hdrstop
#endif

#include "nmspmap.hxx"

#include "xmlnmspe.hxx"

#include "xmluconv.hxx"



#include "xmlmetae.hxx"


#include <com/sun/star/presentation/XPresentationSupplier.hpp>

#include <com/sun/star/presentation/XCustomPresentationSupplier.hpp>





#include "sdxmlexp.hxx"

#include "sdxmlexp_impl.hxx"

#include <com/sun/star/drawing/XDrawPagesSupplier.hpp>

#include <com/sun/star/drawing/XMasterPagesSupplier.hpp>

#include <com/sun/star/presentation/XHandoutMasterSupplier.hpp>

#include <com/sun/star/view/PaperOrientation.hpp>

#include <com/sun/star/style/XStyleFamiliesSupplier.hpp>


#include <com/sun/star/form/XFormsSupplier.hpp>

#include <com/sun/star/presentation/XPresentationPage.hpp>

#include <com/sun/star/drawing/XMasterPageTarget.hpp>






#include <tools/debug.hxx>




#include "sdpropls.hxx"


#include <com/sun/star/beans/XPropertyState.hpp>

#include "xexptran.hxx"


#include <cppuhelper/implbase1.hxx>



#include "PropertySetMerger.hxx"

#include "layerexp.hxx"



#include "XMLNumberStylesExport.hxx"
namespace binfilter {

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::drawing;
using namespace ::com::sun::star::presentation;
using namespace ::binfilter::xmloff::token;

using rtl::OUString;
using rtl::OUStringBuffer;


//////////////////////////////////////////////////////////////////////////////

class ImpXMLEXPPageMasterInfo
{
    sal_Int32					mnBorderBottom;
    sal_Int32					mnBorderLeft;
    sal_Int32					mnBorderRight;
    sal_Int32					mnBorderTop;
    sal_Int32					mnWidth;
    sal_Int32					mnHeight;
    view::PaperOrientation		meOrientation;
    OUString					msName;
    OUString					msMasterPageName;

public:
    ImpXMLEXPPageMasterInfo(const SdXMLExport& rExp, const uno::Reference<drawing::XDrawPage>& xPage);
    BOOL operator==(const ImpXMLEXPPageMasterInfo& rInfo) const;
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
    const uno::Reference<drawing::XDrawPage>& xPage)
:	mnBorderBottom(0),
    mnBorderLeft(0),
    mnBorderRight(0),
    mnBorderTop(0),
    mnWidth(0),
    mnHeight(0),
    meOrientation(rExp.IsDraw() ? view::PaperOrientation_PORTRAIT : view::PaperOrientation_LANDSCAPE)
{
    uno::Reference <beans::XPropertySet> xPropSet(xPage, uno::UNO_QUERY);
    if(xPropSet.is())
    {
        uno::Any aAny;

        uno::Reference< beans::XPropertySetInfo > xPropsInfo( xPropSet->getPropertySetInfo() );
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

    uno::Reference <container::XNamed> xMasterNamed(xPage, uno::UNO_QUERY);
    if(xMasterNamed.is())
    {
        msMasterPageName = xMasterNamed->getName();
    }
}

BOOL ImpXMLEXPPageMasterInfo::operator==(const ImpXMLEXPPageMasterInfo& rInfo) const
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

DECLARE_LIST(ImpXMLEXPPageMasterList, ImpXMLEXPPageMasterInfo*)//STRIP007;

//////////////////////////////////////////////////////////////////////////////

#define	IMP_AUTOLAYOUT_INFO_MAX			(31L)

class ImpXMLAutoLayoutInfo
{
    sal_uInt16					mnType;
    ImpXMLEXPPageMasterInfo*	mpPageMasterInfo;
    OUString					msLayoutName;
    Rectangle					maTitleRect;
    Rectangle					maPresRect;
    sal_Int32					mnGapX;
    sal_Int32					mnGapY;

public:
    ImpXMLAutoLayoutInfo(sal_uInt16 nTyp, ImpXMLEXPPageMasterInfo* pInf);

    BOOL operator==(const ImpXMLAutoLayoutInfo& rInfo) const;

    sal_uInt16 GetLayoutType() const { return mnType; }
    sal_Int32 GetGapX() const { return mnGapX; }
    sal_Int32 GetGapY() const { return mnGapY; }

    const OUString& GetLayoutName() const { return msLayoutName; }
    void SetLayoutName(const OUString& rNew) { msLayoutName = rNew; }

    const Rectangle& GetTitleRectangle() const { return maTitleRect; }
    const Rectangle& GetPresRectangle() const { return maPresRect; }

    static BOOL IsCreateNecessary(sal_uInt16 nTyp);
};

BOOL ImpXMLAutoLayoutInfo::IsCreateNecessary(sal_uInt16 nTyp)
{
    if(nTyp == 5 /* AUTOLAYOUT_ORG */
        || nTyp == 20 /* AUTOLAYOUT_NONE */
        || nTyp >= IMP_AUTOLAYOUT_INFO_MAX)
        return FALSE;
    return TRUE;
}

BOOL ImpXMLAutoLayoutInfo::operator==(const ImpXMLAutoLayoutInfo& rInfo) const
{
    return ((mnType == rInfo.mnType
        && mpPageMasterInfo == rInfo.mpPageMasterInfo));
}

ImpXMLAutoLayoutInfo::ImpXMLAutoLayoutInfo(sal_uInt16 nTyp, ImpXMLEXPPageMasterInfo* pInf)
:	mnType(nTyp),
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

        // tatsaechliche Seitengroesse in das Handout-Rechteck skalieren
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
    else if(mnType >= 22 && mnType <= 26) // AUTOLAYOUT_HANDOUT
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

DECLARE_LIST(ImpXMLAutoLayoutInfoList, ImpXMLAutoLayoutInfo*)//STRIP007;

//////////////////////////////////////////////////////////////////////////////

// #110680#
SdXMLExport::SdXMLExport( 
    const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& xServiceFactory,
    sal_Bool bIsDraw, sal_uInt16 nExportFlags )
:	SvXMLExport( xServiceFactory, MAP_CM, bIsDraw ? XML_DRAWING : XML_PRESENTATION, nExportFlags ),
    mpPageMasterInfoList(new ImpXMLEXPPageMasterList(1, 4, 4)),
    mpPageMasterUsageList(new ImpXMLEXPPageMasterList(1, 4, 4)),
    mpNotesPageMasterUsageList(new ImpXMLEXPPageMasterList(1, 4, 4)),
    mpHandoutPageMaster(NULL),
    mpAutoLayoutInfoList(new ImpXMLAutoLayoutInfoList(1, 4, 4)),
    mpSdPropHdlFactory(0L),
    mpPropertySetMapper(0L),
    mpPresPagePropsMapper(0L),
    mnDocMasterPageCount(0L),
    mnDocDrawPageCount(0L),
    mnShapeStyleInfoIndex(0L),
    mnObjectCount(0L),
    mbIsDraw(bIsDraw),
    mbFamilyGraphicUsed(FALSE),
    mbFamilyPresentationUsed(FALSE),
    msZIndex( GetXMLToken(XML_ZINDEX) ),
    msEmptyPres( RTL_CONSTASCII_USTRINGPARAM("IsEmptyPresentationObject") ),
    msModel( RTL_CONSTASCII_USTRINGPARAM("Model") ),
    msStartShape( RTL_CONSTASCII_USTRINGPARAM("StartShape") ),
    msEndShape( RTL_CONSTASCII_USTRINGPARAM("EndShape") ),
    msPageLayoutNames( RTL_CONSTASCII_USTRINGPARAM("PageLayoutNames") ),
    mnUsedDateStyles( 0 ),
    mnUsedTimeStyles( 0 )
{


}

// XExporter
void SAL_CALL SdXMLExport::setSourceDocument( const uno::Reference< lang::XComponent >& xDoc )
    throw(lang::IllegalArgumentException, uno::RuntimeException)
{
    SvXMLExport::setSourceDocument( xDoc );

    const OUString aEmpty;

    // prepare factory parts
    mpSdPropHdlFactory = new XMLSdPropHdlFactory( GetModel() );
    if(mpSdPropHdlFactory)
    {
        // set lock to avoid deletion
        mpSdPropHdlFactory->acquire();

        // build one ref
        const UniReference< XMLPropertyHandlerFactory > aFactoryRef = mpSdPropHdlFactory;

        // construct PropertySetMapper
        UniReference < XMLPropertySetMapper > xMapper =	new XMLShapePropertySetMapper( aFactoryRef);

        mpPropertySetMapper = new XMLShapeExportPropertyMapper( xMapper, (XMLTextListAutoStylePool*)&GetTextParagraphExport()->GetListAutoStylePool(), *this );
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
    uno::Reference< style::XStyleFamiliesSupplier > xFamSup( GetModel(), uno::UNO_QUERY );
    if(xFamSup.is())
    {
        mxDocStyleFamilies = xFamSup->getStyleFamilies();
    }

    // prepare access to master pages
    uno::Reference < drawing::XMasterPagesSupplier > xMasterPagesSupplier(GetModel(), uno::UNO_QUERY);
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
    uno::Reference <drawing::XDrawPagesSupplier> xDrawPagesSupplier(GetModel(), uno::UNO_QUERY);
    if(xDrawPagesSupplier.is())
    {
        mxDocDrawPages = mxDocDrawPages.query( xDrawPagesSupplier->getDrawPages() );
        if(mxDocDrawPages.is())
        {
            mnDocDrawPageCount = mxDocDrawPages->getCount();
            maDrawPagesStyleNames.insert( maDrawPagesStyleNames.begin(), mnDocDrawPageCount, aEmpty );
            if( !mbIsDraw )
                maDrawPagesAutoLayoutNames.realloc( mnDocDrawPageCount + 1 );
        }
    }

    // #82003# count all draw objects for use with progress bar.
    // #88245# init mnObjectCount once, use counter itself as flag. It
    // is initialized to 0.
    if(!mnObjectCount)
    {
        // #91587# add handout master count
        uno::Reference<presentation::XHandoutMasterSupplier> xHandoutSupp(GetModel(), uno::UNO_QUERY);
        if(xHandoutSupp.is())
        {
            uno::Reference<drawing::XDrawPage> xHandoutPage(xHandoutSupp->getHandoutMasterPage());
            if(xHandoutPage.is())
            {
                uno::Reference<drawing::XShapes> xShapes(xHandoutPage, uno::UNO_QUERY);
                if(xShapes.is() && xShapes->getCount())
                {
                    mnObjectCount += ImpRecursiveObjectCount(xShapes);
                }
            }
        }

        if(mxDocMasterPages.is())
        {
            for(sal_Int32 a(0); a < mnDocMasterPageCount; a++)
            {
                uno::Any aAny(mxDocMasterPages->getByIndex(a));
                uno::Reference< drawing::XShapes > xMasterPage;

                if((aAny >>= xMasterPage) && xMasterPage.is())
                {
                    mnObjectCount += ImpRecursiveObjectCount(xMasterPage);
                }

                // #91587# take notes pages from master pages into account
                uno::Reference<presentation::XPresentationPage> xPresPage;
                if((aAny >>= xPresPage) && xPresPage.is())
                {
                    uno::Reference<drawing::XDrawPage> xNotesPage(xPresPage->getNotesPage());
                    if(xNotesPage.is())
                    {
                        uno::Reference<drawing::XShapes> xShapes(xNotesPage, uno::UNO_QUERY);
                        if(xShapes.is() && xShapes->getCount())
                        {
                            mnObjectCount += ImpRecursiveObjectCount(xShapes);
                        }
                    }
                }
            }
        }

        if(mxDocDrawPages.is())
        {
            for(sal_Int32 a(0); a < mnDocDrawPageCount; a++)
            {
                uno::Any aAny(mxDocDrawPages->getByIndex(a));
                uno::Reference< drawing::XShapes > xPage;

                if((aAny >>= xPage) && xPage.is())
                {
                    mnObjectCount += ImpRecursiveObjectCount(xPage);
                }

                // #91587# take notes pages from draw pages into account
                uno::Reference<presentation::XPresentationPage> xPresPage;
                if((aAny >>= xPresPage) && xPresPage.is())
                {
                    uno::Reference<drawing::XDrawPage> xNotesPage(xPresPage->getNotesPage());
                    if(xNotesPage.is())
                    {
                        uno::Reference<drawing::XShapes> xShapes(xNotesPage, uno::UNO_QUERY);
                        if(xShapes.is() && xShapes->getCount())
                        {
                            mnObjectCount += ImpRecursiveObjectCount(xShapes);
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

    GetShapeExport()->enableLayerExport();

    // #88546# enable progress bar increments
    GetShapeExport()->enableHandleProgressBar();
}

//////////////////////////////////////////////////////////////////////////////
// #82003# helper function for recursive object count
sal_uInt32 SdXMLExport::ImpRecursiveObjectCount(uno::Reference< drawing::XShapes > xShapes)
{
    sal_uInt32 nRetval(0L);

    if(xShapes.is())
    {
        sal_Int32 nCount = xShapes->getCount();

        for(sal_Int32 a(0L); a < nCount; a++)
        {
            uno::Any aAny(xShapes->getByIndex(a));
            uno::Reference< drawing::XShapes > xGroup;

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

__EXPORT SdXMLExport::~SdXMLExport()
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
    if(mpPageMasterInfoList)
    {
        while(mpPageMasterInfoList->Count())
            delete mpPageMasterInfoList->Remove(mpPageMasterInfoList->Count() - 1L);
        delete mpPageMasterInfoList;
        mpPageMasterInfoList = 0L;
    }
    if(mpPageMasterUsageList)
    {
        delete mpPageMasterUsageList;
        mpPageMasterUsageList = 0L;
    }
    if(mpNotesPageMasterUsageList)
    {
        delete mpNotesPageMasterUsageList;
        mpNotesPageMasterUsageList = 0L;
    }

    // clear auto-layout infos
    if(mpAutoLayoutInfoList)
    {
        while(mpAutoLayoutInfoList->Count())
            delete mpAutoLayoutInfoList->Remove(mpAutoLayoutInfoList->Count() - 1L);
        delete mpAutoLayoutInfoList;
        mpAutoLayoutInfoList = 0L;
    }

// #82003# status indicator stop is called exclusively 
// from SdXMLFilter::Export() now.
//
// stop progress view
//	if(GetStatusIndicator().is())
//	{
//		GetStatusIndicator()->end();
//		GetStatusIndicator()->reset();
//	}
}


//////////////////////////////////////////////////////////////////////////////

void SdXMLExport::ImpPrepAutoLayoutInfos()
{
    if(IsImpress())
    {
        OUString aStr;

        uno::Reference< presentation::XHandoutMasterSupplier > xHandoutSupp( GetModel(), uno::UNO_QUERY );
        if( xHandoutSupp.is() )
        {
            uno::Reference< drawing::XDrawPage > xHandoutPage( xHandoutSupp->getHandoutMasterPage() );
            if( xHandoutPage.is() )
            {
                if(ImpPrepAutoLayoutInfo(xHandoutPage, aStr))
                    maDrawPagesAutoLayoutNames[0] = aStr;
            }
        }

        // prepare name creation
        for(sal_Int32 nCnt = 0L; nCnt < mnDocDrawPageCount; nCnt++)
        {
            uno::Any aAny(mxDocDrawPages->getByIndex(nCnt));
            uno::Reference<drawing::XDrawPage> xDrawPage;

            if((aAny >>= xDrawPage) && xDrawPage.is())
            {
                if(ImpPrepAutoLayoutInfo(xDrawPage, aStr))
                    maDrawPagesAutoLayoutNames[nCnt+1] = aStr;
            }
        }
    }
}

BOOL SdXMLExport::ImpPrepAutoLayoutInfo(const uno::Reference<drawing::XDrawPage>& xPage, OUString& rName)
{
    rName = OUString();
    BOOL bRetval(FALSE);

    uno::Reference <beans::XPropertySet> xPropSet(xPage, uno::UNO_QUERY);
    if(xPropSet.is())
    {
        sal_uInt16 nType;
        uno::Any aAny;

        aAny = xPropSet->getPropertyValue(OUString(RTL_CONSTASCII_USTRINGPARAM("Layout")));
        if(aAny >>= nType)
        {
            if(ImpXMLAutoLayoutInfo::IsCreateNecessary(nType))
            {
                ImpXMLEXPPageMasterInfo* pInfo = 0L;

                // get master-page info
                uno::Reference < drawing::XMasterPageTarget > xMasterPageInt(xPage, uno::UNO_QUERY);
                if(xMasterPageInt.is())
                {
                    uno::Reference<drawing::XDrawPage> xUsedMasterPage(xMasterPageInt->getMasterPage());
                    if(xUsedMasterPage.is())
                    {
                        uno::Reference < container::XNamed > xMasterNamed(xUsedMasterPage, uno::UNO_QUERY);
                        if(xMasterNamed.is())
                        {
                            OUString sMasterPageName = xMasterNamed->getName();
                            pInfo = ImpGetPageMasterInfoByName(sMasterPageName);
                        }
                    }
                }

                // create entry and look for existance
                ImpXMLAutoLayoutInfo* pNew = new ImpXMLAutoLayoutInfo(nType, pInfo);
                BOOL bDidExist(FALSE);

                for(sal_uInt32 nCnt = 0L; !bDidExist && nCnt < mpAutoLayoutInfoList->Count(); nCnt++)
                {
                    if(*mpAutoLayoutInfoList->GetObject(nCnt) == *pNew)
                    {
                        delete pNew;
                        pNew = mpAutoLayoutInfoList->GetObject(nCnt);
                        bDidExist = TRUE;
                    }
                }

                if(!bDidExist)
                {
                    mpAutoLayoutInfoList->Insert(pNew, LIST_APPEND);
                    OUString sNewName = OUString(RTL_CONSTASCII_USTRINGPARAM("AL"));
                    sNewName += OUString::valueOf(sal_Int32(mpAutoLayoutInfoList->Count() - 1));
                    sNewName += OUString(RTL_CONSTASCII_USTRINGPARAM("T"));
                    sNewName += OUString::valueOf(sal_Int32(nType));
                    pNew->SetLayoutName(sNewName);
                }

                rName = pNew->GetLayoutName();
                bRetval = TRUE;
            }
        }
    }

    return bRetval;
}

//////////////////////////////////////////////////////////////////////////////

void SdXMLExport::ImpWriteAutoLayoutInfos()
{
    if(mpAutoLayoutInfoList->Count())
    {
        for(sal_uInt32 nCnt = 0L; nCnt < mpAutoLayoutInfoList->Count(); nCnt++)
        {
            ImpXMLAutoLayoutInfo* pInfo = mpAutoLayoutInfoList->GetObject(nCnt);
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
                            case 26 : nColCnt = 3; nRowCnt = 3; break;
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

                        for(sal_Int32 a = 0L; a < nRowCnt; a++)
                        {
                            aTmpPos.X() = aPartPos.X();

                            for(sal_Int32 b = 0L; b < nColCnt; b++)
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
                    default:
                    {
                        DBG_ERROR("XMLEXP: unknown autolayout export");
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
    GetMM100UnitConverter().convertMeasure(sStringBuffer, rRect.Left());
    aStr = sStringBuffer.makeStringAndClear();
    AddAttribute(XML_NAMESPACE_SVG, XML_X, aStr);

    GetMM100UnitConverter().convertMeasure(sStringBuffer, rRect.Top());
    aStr = sStringBuffer.makeStringAndClear();
    AddAttribute(XML_NAMESPACE_SVG, XML_Y, aStr);

    GetMM100UnitConverter().convertMeasure(sStringBuffer, rRect.GetWidth());
    aStr = sStringBuffer.makeStringAndClear();
    AddAttribute(XML_NAMESPACE_SVG, XML_WIDTH, aStr);

    GetMM100UnitConverter().convertMeasure(sStringBuffer, rRect.GetHeight());
    aStr = sStringBuffer.makeStringAndClear();
    AddAttribute(XML_NAMESPACE_SVG, XML_HEIGHT, aStr);

    // write presentation-placeholder
    SvXMLElementExport aPPL(*this, XML_NAMESPACE_PRESENTATION, XML_PLACEHOLDER, sal_True, sal_True);
}

//////////////////////////////////////////////////////////////////////////////

ImpXMLEXPPageMasterInfo* SdXMLExport::ImpGetOrCreatePageMasterInfo( uno::Reference< drawing::XDrawPage > xMasterPage )
{
    bool bDoesExist = false;

    ImpXMLEXPPageMasterInfo* pNewInfo = new ImpXMLEXPPageMasterInfo(*this, xMasterPage);

    // compare with prev page-master infos
    for(sal_uInt32 a = 0; !bDoesExist && a < mpPageMasterInfoList->Count(); a++)
    {
        if(mpPageMasterInfoList->GetObject(a)
            && *mpPageMasterInfoList->GetObject(a) == *pNewInfo)
        {
            delete pNewInfo;
            pNewInfo = mpPageMasterInfoList->GetObject(a);
            bDoesExist = true;
        }
    }
    // add entry when not found same page-master infos
    if(!bDoesExist)
        mpPageMasterInfoList->Insert(pNewInfo, LIST_APPEND);

    return pNewInfo;
}

void SdXMLExport::ImpPrepPageMasterInfos()
{
    // create page master info for handout master page
    uno::Reference< drawing::XDrawPage > xMasterPage;

    uno::Reference< XHandoutMasterSupplier > xHMS( GetModel(), uno::UNO_QUERY );
    if( xHMS.is() )
        xMasterPage = xHMS->getHandoutMasterPage();

    if( xMasterPage.is() )
        mpHandoutPageMaster = ImpGetOrCreatePageMasterInfo(xMasterPage);

    // create page master infos for master pages
    if(mnDocMasterPageCount)
    {
        // look for needed page-masters, create these
        for(sal_Int32 nMPageId = 0L; nMPageId < mnDocMasterPageCount; nMPageId++)
        {
            mxDocMasterPages->getByIndex(nMPageId) >>= xMasterPage;
            ImpXMLEXPPageMasterInfo* pNewInfo = 0L;

            if(xMasterPage.is())
                pNewInfo = ImpGetOrCreatePageMasterInfo(xMasterPage);

            mpPageMasterUsageList->Insert(pNewInfo, LIST_APPEND);

            // look for page master of handout page
            if(IsImpress())
            {
                pNewInfo = NULL;
                uno::Reference< presentation::XPresentationPage > xPresPage(xMasterPage, uno::UNO_QUERY);
                if(xPresPage.is())
                {
                    uno::Reference< drawing::XDrawPage > xNotesPage(xPresPage->getNotesPage());
                    if(xNotesPage.is())
                    {
                        pNewInfo = ImpGetOrCreatePageMasterInfo(xNotesPage);
                    }
                }
                mpNotesPageMasterUsageList->Insert( pNewInfo, LIST_APPEND );
            }
        }
    }
}

//////////////////////////////////////////////////////////////////////////////

void SdXMLExport::ImpWritePageMasterInfos()
{
    // write created page-masters, create names for these
    for(sal_uInt32 nCnt = 0L; nCnt < mpPageMasterInfoList->Count(); nCnt++)
    {
        ImpXMLEXPPageMasterInfo* pInfo = mpPageMasterInfoList->GetObject(nCnt);
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

            // write page-master
            SvXMLElementExport aPME(*this, XML_NAMESPACE_STYLE, XML_PAGE_MASTER, sal_True, sal_True);

            // prepare style:properties inside page-master
            GetMM100UnitConverter().convertMeasure(sStringBuffer, pInfo->GetBorderTop());
            sString = sStringBuffer.makeStringAndClear();
            AddAttribute(XML_NAMESPACE_FO, XML_MARGIN_TOP, sString);

            GetMM100UnitConverter().convertMeasure(sStringBuffer, pInfo->GetBorderBottom());
            sString = sStringBuffer.makeStringAndClear();
            AddAttribute(XML_NAMESPACE_FO, XML_MARGIN_BOTTOM, sString);

            GetMM100UnitConverter().convertMeasure(sStringBuffer, pInfo->GetBorderLeft());
            sString = sStringBuffer.makeStringAndClear();
            AddAttribute(XML_NAMESPACE_FO, XML_MARGIN_LEFT, sString);

            GetMM100UnitConverter().convertMeasure(sStringBuffer, pInfo->GetBorderRight());
            sString = sStringBuffer.makeStringAndClear();
            AddAttribute(XML_NAMESPACE_FO, XML_MARGIN_RIGHT, sString);

            GetMM100UnitConverter().convertMeasure(sStringBuffer, pInfo->GetWidth());
            sString = sStringBuffer.makeStringAndClear();
            AddAttribute(XML_NAMESPACE_FO, XML_PAGE_WIDTH, sString);

            GetMM100UnitConverter().convertMeasure(sStringBuffer, pInfo->GetHeight());
            sString = sStringBuffer.makeStringAndClear();
            AddAttribute(XML_NAMESPACE_FO, XML_PAGE_HEIGHT, sString);

            if(pInfo->GetOrientation() == view::PaperOrientation_PORTRAIT)
                AddAttribute(XML_NAMESPACE_STYLE, XML_PRINT_ORIENTATION, XML_PORTRAIT);
            else
                AddAttribute(XML_NAMESPACE_STYLE, XML_PRINT_ORIENTATION, XML_LANDSCAPE);

            // write style:properties
            SvXMLElementExport aPMF(*this, XML_NAMESPACE_STYLE, XML_PROPERTIES, sal_True, sal_True);
        }
    }
}

//////////////////////////////////////////////////////////////////////////////

ImpXMLEXPPageMasterInfo* SdXMLExport::ImpGetPageMasterInfoByName(const OUString& rName)
{
    if(rName.getLength() && mpPageMasterInfoList->Count())
    {
        for(sal_uInt32 nCnt = 0L; nCnt < mpPageMasterInfoList->Count(); nCnt++)
        {
            ImpXMLEXPPageMasterInfo* pInfo = mpPageMasterInfoList->GetObject(nCnt);
            if(pInfo)
            {
                if(pInfo->GetMasterPageName().getLength() && rName.equals(pInfo->GetMasterPageName()))
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
    if(mnDocDrawPageCount)
    {
        // prepare name creation
        for(sal_Int32 nCnt = 0L; nCnt < mnDocDrawPageCount; nCnt++)
        {
            uno::Any aAny(mxDocDrawPages->getByIndex(nCnt));
            uno::Reference<drawing::XDrawPage> xDrawPage;

            if(aAny >>= xDrawPage)
            {
                // create name
                OUString sStyleName;

                // create style for this page and add to auto style pool

                uno::Reference< beans::XPropertySet > xPropSet1(xDrawPage, uno::UNO_QUERY);
                if(xPropSet1.is())
                {
                    // since the background items are in a different propertyset
                    // which itself is a property of the pages property set
                    // we now merge these two propertysets if possible to simulate
                    // a single propertyset with all draw page properties
                    const OUString aBackground(RTL_CONSTASCII_USTRINGPARAM("Background"));
                    uno::Reference< beans::XPropertySet > xPropSet2;
                    uno::Reference< beans::XPropertySetInfo > xInfo( xPropSet1->getPropertySetInfo() );
                    if( xInfo.is() && xInfo->hasPropertyByName( aBackground ) )
                    {
                        uno::Any aAny( xPropSet1->getPropertyValue( aBackground ) );
                        aAny >>= xPropSet2;
                    }

                    uno::Reference< beans::XPropertySet > xPropSet;
                    if( xPropSet2.is() )
                        xPropSet = PropertySetMerger_CreateInstance( xPropSet1, xPropSet2 );
                    else
                        xPropSet = xPropSet1;

                    const UniReference< SvXMLExportPropertyMapper > aMapperRef( GetPresPagePropsMapper() );
                    std::vector< XMLPropertyState > xPropStates( aMapperRef->Filter( xPropSet ) );

                    if( !xPropStates.empty() )
                    {
                        // there are filtered properties -> hard attributes
                        // try to find this style in AutoStylePool
                        sStyleName = GetAutoStylePool()->Find(XML_STYLE_FAMILY_SD_DRAWINGPAGE_ID, sStyleName, xPropStates);

                        if(!sStyleName.getLength())
                        {
                            // Style did not exist, add it to AutoStalePool
                            sStyleName = GetAutoStylePool()->Add(XML_STYLE_FAMILY_SD_DRAWINGPAGE_ID, sStyleName, xPropStates);
                        }

                        maDrawPagesStyleNames[nCnt] = sStyleName;
                    }
                }
            }
        }
    }
}

//////////////////////////////////////////////////////////////////////////////

void SdXMLExport::ImpPrepMasterPageInfos()
{
    // create draw:style-name entries for master page export
    // containing only background attributes
    // fixed family for page-styles is "drawing-page" (XML_STYLE_FAMILY_SD_DRAWINGPAGE_NAME)
    if(mnDocMasterPageCount)
    {
        // prepare name creation
        for(sal_Int32 nCnt = 0L; nCnt < mnDocMasterPageCount; nCnt++)
        {
            uno::Any aAny(mxDocMasterPages->getByIndex(nCnt));
            uno::Reference<drawing::XDrawPage> xDrawPage;

            if(aAny >>= xDrawPage)
            {
                // create name
                OUString sStyleName;

                // create style for this page and add to auto style pool
                uno::Reference< beans::XPropertySet > xPropSet1(xDrawPage, uno::UNO_QUERY);
                if(xPropSet1.is())
                {
                    // since the background items are in a different propertyset
                    // which itself is a property of the pages property set
                    // we now merge these two propertysets if possible to simulate
                    // a single propertyset with all draw page properties
                    const OUString aBackground(RTL_CONSTASCII_USTRINGPARAM("Background"));
                    uno::Reference< beans::XPropertySet > xPropSet2;
                    uno::Reference< beans::XPropertySetInfo > xInfo( xPropSet1->getPropertySetInfo() );
                    if( xInfo.is() && xInfo->hasPropertyByName( aBackground ) )
                    {
                        uno::Any aAny( xPropSet1->getPropertyValue( aBackground ) );
                        aAny >>= xPropSet2;
                    }

                    uno::Reference< beans::XPropertySet > xPropSet;
                    if( xPropSet2.is() )
                        xPropSet = PropertySetMerger_CreateInstance( xPropSet1, xPropSet2 );
                    else
                        xPropSet = xPropSet1;

                    if( xPropSet.is() )
                    {
                        const UniReference< SvXMLExportPropertyMapper > aMapperRef( GetPresPagePropsMapper() );
                        std::vector< XMLPropertyState > xPropStates( aMapperRef->Filter( xPropSet ) );

                        if( !xPropStates.empty() )
                        {
                            // there are filtered properties -> hard attributes
                            // try to find this style in AutoStylePool
                            sStyleName = GetAutoStylePool()->Find(XML_STYLE_FAMILY_SD_DRAWINGPAGE_ID, sStyleName, xPropStates);

                            if(!sStyleName.getLength())
                            {
                                // Style did not exist, add it to AutoStalePool
                                sStyleName = GetAutoStylePool()->Add(XML_STYLE_FAMILY_SD_DRAWINGPAGE_ID, sStyleName, xPropStates);
                            }

                            maMasterPagesStyleNames[nCnt] = sStyleName;
                        }
                    }
                }
            }
        }
    }
}

//////////////////////////////////////////////////////////////////////////////
void SdXMLExport::ImpWritePresentationStyles()
{
    if(IsImpress())
    {
        for(sal_Int32 nCnt = 0L; nCnt < mnDocMasterPageCount; nCnt++)
        {
            uno::Any aAny(mxDocMasterPages->getByIndex(nCnt));
            uno::Reference<container::XNamed> xNamed;

            if(aAny >>= xNamed)
            {
                // write presentation styles (ONLY if presentation)
                if(IsImpress() && mxDocStyleFamilies.is() && xNamed.is())
                {
                    XMLStyleExport aStEx(*this, OUString(), GetAutoStylePool().get());
                    const UniReference< SvXMLExportPropertyMapper > aMapperRef( GetPropertySetMapper() );

                    OUString aPrefix = xNamed->getName();
                    aPrefix += OUString(RTL_CONSTASCII_USTRINGPARAM("-"));
                    aStEx.exportStyleFamily(xNamed->getName(),
                        OUString(RTL_CONSTASCII_USTRINGPARAM(XML_STYLE_FAMILY_SD_PRESENTATION_NAME)),
                        aMapperRef, FALSE,
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
    // call parent
    SvXMLExport::_ExportMeta();

    // prepare export statistic info (mainly for progress bar at reload)
    sal_Bool bContentUsed(FALSE);
    ::rtl::OUStringBuffer sBuffer;

    // export shape count info
    if(mnObjectCount)
    {
        GetMM100UnitConverter().convertNumber(sBuffer, mnObjectCount);
        AddAttribute(XML_NAMESPACE_META, XML_OBJECT_COUNT, sBuffer.makeStringAndClear());
        bContentUsed = TRUE;
    }

    // when there is data, export it
    if(bContentUsed)
        SvXMLElementExport aElemStat(*this, XML_NAMESPACE_META, XML_DOCUMENT_STATISTIC, sal_True, sal_True);
}

//////////////////////////////////////////////////////////////////////////////

void SdXMLExport::_ExportContent()
{
    // page export
    for(sal_Int32 nPageInd(0); nPageInd < mnDocDrawPageCount; nPageInd++)
    {
        uno::Any aAny(mxDocDrawPages->getByIndex(nPageInd));
        uno::Reference<drawing::XDrawPage> xDrawPage;

        SetProgress(((nPageInd + 1) * 100) / mnDocDrawPageCount);

        if(aAny >>= xDrawPage)
        {
            // prepare page attributes, name of page
            uno::Reference < container::XNamed > xNamed(xDrawPage, uno::UNO_QUERY);
            if(xNamed.is())
                AddAttribute(XML_NAMESPACE_DRAW, XML_NAME, xNamed->getName());

            // draw:style-name (presentation page attributes AND background attributes)
            if( maDrawPagesStyleNames[nPageInd].getLength() )
                AddAttribute(XML_NAMESPACE_DRAW, XML_STYLE_NAME, maDrawPagesStyleNames[nPageInd]);

            if( IsImpress() )
                AddAttribute(XML_NAMESPACE_DRAW, XML_ID, OUString::valueOf( sal_Int32( nPageInd + 1 ) ) );

            // draw:master-page-name
            uno::Reference < drawing::XMasterPageTarget > xMasterPageInt(xDrawPage, uno::UNO_QUERY);
            if(xMasterPageInt.is())
            {
                uno::Reference<drawing::XDrawPage> xUsedMasterPage(xMasterPageInt->getMasterPage());
                if(xUsedMasterPage.is())
                {
                    uno::Reference < container::XNamed > xMasterNamed(xUsedMasterPage, uno::UNO_QUERY);
                    if(xMasterNamed.is())
                    {
                        AddAttribute(XML_NAMESPACE_DRAW, XML_MASTER_PAGE_NAME, xMasterNamed->getName());
                    }
                }
            }

            // presentation:page-layout-name
            if( IsImpress() && maDrawPagesAutoLayoutNames[nPageInd+1].getLength())
            {
                AddAttribute(XML_NAMESPACE_PRESENTATION, XML_PRESENTATION_PAGE_LAYOUT_NAME, maDrawPagesAutoLayoutNames[nPageInd+1]);
            }

            uno::Reference< beans::XPropertySet > xProps( xDrawPage, uno::UNO_QUERY );
            if( xProps.is() )
            {
                OUString aBookmarkURL;
                xProps->getPropertyValue( OUString( RTL_CONSTASCII_USTRINGPARAM( "BookmarkURL" ) ) ) >>= aBookmarkURL;

                if( aBookmarkURL.getLength() )
                {
                    sal_Int32 nIndex = aBookmarkURL.lastIndexOf( (sal_Unicode)'#' );
                    if( nIndex != -1 )
                    {
                        OUString aFileName( aBookmarkURL.copy( 0, nIndex ) );
                        OUString aBookmarkName( aBookmarkURL.copy( nIndex+1 ) );

                        aBookmarkURL = GetRelativeReference( aFileName );
                        aBookmarkURL += OUString( (sal_Unicode)'#' );
                        aBookmarkURL += aBookmarkName;
                    }

                    AddAttribute ( XML_NAMESPACE_XLINK, XML_HREF, aBookmarkURL);
                    AddAttribute ( XML_NAMESPACE_XLINK, XML_TYPE, XML_SIMPLE );
                    AddAttribute ( XML_NAMESPACE_XLINK, XML_SHOW, XML_REPLACE );
                    AddAttribute ( XML_NAMESPACE_XLINK, XML_ACTUATE, XML_ONREQUEST );
                }
            }

            // write page
            SvXMLElementExport aDPG(*this, XML_NAMESPACE_DRAW, XML_PAGE, sal_True, sal_True);

            // write optional office:forms
            exportFormsElement( xDrawPage );

            // prepare animations exporter if impress
            if(IsImpress())
            {
                UniReference< XMLAnimationsExporter > xAnimExport = new XMLAnimationsExporter( GetShapeExport().get() );
                GetShapeExport()->setAnimationsExporter( xAnimExport );
            }

            // write graphic objects on this page (if any)
            uno::Reference< drawing::XShapes > xShapes(xDrawPage, uno::UNO_QUERY);
            if(xShapes.is() && xShapes->getCount())
                GetShapeExport()->exportShapes( xShapes );

            // write animations and presentation notes (ONLY if presentation)
            if(IsImpress())
            {
                // animations
                UniReference< XMLAnimationsExporter > xAnimExport( GetShapeExport()->getAnimationsExporter() );
                if( xAnimExport.is() )
                    xAnimExport->exportAnimations( *this );

                xAnimExport = NULL;
                GetShapeExport()->setAnimationsExporter( xAnimExport );

                // presentations
                uno::Reference< presentation::XPresentationPage > xPresPage(xDrawPage, uno::UNO_QUERY);
                if(xPresPage.is())
                {
                    uno::Reference< drawing::XDrawPage > xNotesPage(xPresPage->getNotesPage());
                    if(xNotesPage.is())
                    {
                        uno::Reference< drawing::XShapes > xShapes(xNotesPage, uno::UNO_QUERY);
                        if(xShapes.is() && xShapes->getCount())
                        {
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

        sal_Bool bTemp;

        // export range
        xPresProps->getPropertyValue( OUString( RTL_CONSTASCII_USTRINGPARAM( "IsShowAll" ) ) ) >>= bTemp;
        if( !bTemp )
        {
            OUString aFirstPage;
            xPresProps->getPropertyValue( OUString( RTL_CONSTASCII_USTRINGPARAM( "FirstPage" ) ) ) >>= aFirstPage;
            if( aFirstPage.getLength() )
            {
                AddAttribute(XML_NAMESPACE_PRESENTATION, XML_START_PAGE, aFirstPage );
                bHasAttr = sal_True;
            }
            else
            {
                OUString aCustomShow;
                xPresProps->getPropertyValue( OUString( RTL_CONSTASCII_USTRINGPARAM( "CustomShow" ) ) ) >>= aCustomShow;
                if( aCustomShow.getLength() )
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

            sal_Int32 nPause;
            xPresProps->getPropertyValue( OUString( RTL_CONSTASCII_USTRINGPARAM( "Pause" ) ) ) >>= nPause;

            util::DateTime aTime( 0, (sal_uInt16)nPause, 0, 0, 0, 0, 0 );

            OUStringBuffer aOut;
            SvXMLUnitConverter::convertTime( aOut, aTime );
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
    catch( uno::Exception )
    {
        DBG_ERROR( "uno::Exception while exporting <presentation:settings>" );
    }
}

//////////////////////////////////////////////////////////////////////////////

void SdXMLExport::_ExportStyles(BOOL bUsed)
{
    GetPropertySetMapper()->SetAutoStyles( sal_False );

    // export fill styles
    SvXMLExport::_ExportStyles( bUsed );

    // write draw:style-name for object graphic-styles
    GetShapeExport()->ExportGraphicDefaults();

    // write presentation styles
    ImpWritePresentationStyles();

    // prepare draw:auto-layout-name for page export
    ImpPrepAutoLayoutInfos();

    // write draw:auto-layout-name for page export
    ImpWriteAutoLayoutInfos();

    uno::Reference< beans::XPropertySet > xInfoSet( getExportInfo() );
    if( xInfoSet.is() )
    {
        uno::Reference< beans::XPropertySetInfo > xInfoSetInfo( xInfoSet->getPropertySetInfo() );

        uno::Any aAny;

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
    uno::Reference< beans::XPropertySet > xInfoSet( getExportInfo() );
    if( xInfoSet.is() )
    {
        uno::Reference< beans::XPropertySetInfo > xInfoSetInfo( xInfoSet->getPropertySetInfo() );

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
            uno::Reference< presentation::XHandoutMasterSupplier > xHandoutSupp( GetModel(), uno::UNO_QUERY );
            if( xHandoutSupp.is() )
            {
                uno::Reference< drawing::XDrawPage > xHandoutPage( xHandoutSupp->getHandoutMasterPage() );
                if( xHandoutPage.is() )
                {
                    uno::Reference< drawing::XShapes > xShapes(xHandoutPage, uno::UNO_QUERY);
                    if(xShapes.is() && xShapes->getCount())
                        GetShapeExport()->collectShapesAutoStyles( xShapes );
                }
            }
        }

        // create auto style infos for objects on master pages
        for(sal_Int32 nMPageId(0L); nMPageId < mnDocMasterPageCount; nMPageId++)
        {
            uno::Any aAny(mxDocMasterPages->getByIndex(nMPageId));
            uno::Reference< drawing::XDrawPage > xMasterPage;

            if((aAny >>= xMasterPage) && xMasterPage.is() )
            {
                // collect layer information
                GetFormExport()->examineForms( xMasterPage );

                // get MasterPage Name
                OUString aMasterPageNamePrefix;
                uno::Reference < container::XNamed > xNamed(xMasterPage, uno::UNO_QUERY);
                if(xNamed.is())
                {
                    aMasterPageNamePrefix = xNamed->getName();
                }
                if(aMasterPageNamePrefix.getLength())
                {
                    aMasterPageNamePrefix += OUString(RTL_CONSTASCII_USTRINGPARAM("-"));
                }
                GetShapeExport()->setPresentationStylePrefix( aMasterPageNamePrefix );

                uno::Reference< drawing::XShapes > xShapes(xMasterPage, uno::UNO_QUERY);
                if(xShapes.is() && xShapes->getCount())
                    GetShapeExport()->collectShapesAutoStyles( xShapes );

                if(IsImpress())
                {
                    uno::Reference< presentation::XPresentationPage > xPresPage(xMasterPage, uno::UNO_QUERY);
                    if(xPresPage.is())
                    {
                        uno::Reference< drawing::XDrawPage > xNotesPage(xPresPage->getNotesPage());
                        if(xNotesPage.is())
                        {
                            // collect layer information
                            GetFormExport()->examineForms( xNotesPage );

                            uno::Reference< drawing::XShapes > xShapes(xNotesPage, uno::UNO_QUERY);
                            if(xShapes.is() && xShapes->getCount())
                                GetShapeExport()->collectShapesAutoStyles( xShapes );
                        }
                    }
                }
            }
        }
    }

    if( getExportFlags() & EXPORT_CONTENT )
    {
        // prepare animations exporter if impress
        if(IsImpress())
        {
            UniReference< XMLAnimationsExporter > xAnimExport = new XMLAnimationsExporter( GetShapeExport().get() );
            GetShapeExport()->setAnimationsExporter( xAnimExport );
        }

        // create auto style infos for objects on pages
        for(sal_Int32 nPageInd(0); nPageInd < mnDocDrawPageCount; nPageInd++)
        {
            uno::Any aAny(mxDocDrawPages->getByIndex(nPageInd));
            uno::Reference<drawing::XDrawPage> xDrawPage;

            if((aAny >>= xDrawPage) && xDrawPage.is() )
            {
                // collect layer information
                GetFormExport()->examineForms( xDrawPage );

                // get MasterPage Name
                OUString aMasterPageNamePrefix;
                uno::Reference < drawing::XMasterPageTarget > xMasterPageInt(xDrawPage, uno::UNO_QUERY);
                if(xMasterPageInt.is())
                {
                    uno::Reference<drawing::XDrawPage> xUsedMasterPage(xMasterPageInt->getMasterPage());
                    if(xUsedMasterPage.is())
                    {
                        uno::Reference < container::XNamed > xMasterNamed(xUsedMasterPage, uno::UNO_QUERY);
                        if(xMasterNamed.is())
                        {
                            aMasterPageNamePrefix = xMasterNamed->getName();
                        }
                    }
                }
                if(aMasterPageNamePrefix.getLength())
                {
                    aMasterPageNamePrefix += OUString(RTL_CONSTASCII_USTRINGPARAM("-"));
                }

                GetShapeExport()->setPresentationStylePrefix( aMasterPageNamePrefix );

                // prepare object infos
                uno::Reference< drawing::XShapes > xShapes(xDrawPage, uno::UNO_QUERY);
                if(xShapes.is() && xShapes->getCount())
                    GetShapeExport()->collectShapesAutoStyles( xShapes );

                // prepare presentation notes page object infos (ONLY if presentation)
                if(IsImpress())
                {
                    uno::Reference< presentation::XPresentationPage > xPresPage(xDrawPage, uno::UNO_QUERY);
                    if(xPresPage.is())
                    {
                        uno::Reference< drawing::XDrawPage > xNotesPage(xPresPage->getNotesPage());
                        if(xNotesPage.is())
                        {
                            // collect layer information
                            GetFormExport()->examineForms( xNotesPage );

                            uno::Reference< drawing::XShapes > xShapes(xNotesPage, uno::UNO_QUERY);
                            if(xShapes.is() && xShapes->getCount())
                                GetShapeExport()->collectShapesAutoStyles( xShapes );
                        }
                    }
                }
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
        uno::Reference< presentation::XHandoutMasterSupplier > xHandoutSupp( GetModel(), uno::UNO_QUERY );
        if( xHandoutSupp.is() )
        {
            uno::Reference< drawing::XDrawPage > xHandoutPage( xHandoutSupp->getHandoutMasterPage() );
            if( xHandoutPage.is() )
            {
                // presentation:page-layout-name
                if( IsImpress() && maDrawPagesAutoLayoutNames[0].getLength())
                {
                    AddAttribute(XML_NAMESPACE_PRESENTATION, XML_PRESENTATION_PAGE_LAYOUT_NAME, maDrawPagesAutoLayoutNames[0]);
                }

                ImpXMLEXPPageMasterInfo* pInfo = mpHandoutPageMaster;
                if(pInfo)
                {
                    OUString sString = pInfo->GetName();
                    AddAttribute(XML_NAMESPACE_STYLE, XML_PAGE_MASTER_NAME, sString);
                }

                // write masterpage
                SvXMLElementExport aMPG(*this, XML_NAMESPACE_STYLE, XML_HANDOUT_MASTER, sal_True, sal_True);

                // write graphic objects on this master page (if any)
                uno::Reference< drawing::XShapes > xShapes(xHandoutPage, uno::UNO_QUERY);
                if(xShapes.is() && xShapes->getCount())
                    GetShapeExport()->exportShapes( xShapes );
            }
        }
    }

    // export MasterPages in master-styles section
    for(sal_Int32 nMPageId = 0L; nMPageId < mnDocMasterPageCount; nMPageId++)
    {
        uno::Any aAny(mxDocMasterPages->getByIndex(nMPageId));
        uno::Reference< drawing::XDrawPage > xMasterPage;

        if((aAny >>= xMasterPage) && xMasterPage.is())
        {
            // prepare masterpage attributes
            OUString sMasterPageName;
            uno::Reference < container::XNamed > xNamed(xMasterPage, uno::UNO_QUERY);
            if(xNamed.is())
            {
                sMasterPageName = xNamed->getName();
                AddAttribute(XML_NAMESPACE_STYLE, XML_NAME, sMasterPageName);
            }

            ImpXMLEXPPageMasterInfo* pInfo = mpPageMasterUsageList->GetObject(nMPageId);
            if(pInfo)
            {
                OUString sString = pInfo->GetName();
                AddAttribute(XML_NAMESPACE_STYLE, XML_PAGE_MASTER_NAME, sString);
            }

            // draw:style-name (background attributes)
            if( maMasterPagesStyleNames[nMPageId].getLength() )
                AddAttribute(XML_NAMESPACE_DRAW, XML_STYLE_NAME, maMasterPagesStyleNames[nMPageId]);

            // write masterpage
            SvXMLElementExport aMPG(*this, XML_NAMESPACE_STYLE, XML_MASTER_PAGE, sal_True, sal_True);

            // write optional office:forms
            exportFormsElement( xMasterPage );

            // write graphic objects on this master page (if any)
            uno::Reference< drawing::XShapes > xShapes(xMasterPage, uno::UNO_QUERY);
            if(xShapes.is() && xShapes->getCount())
                GetShapeExport()->exportShapes( xShapes );

            // write presentation notes (ONLY if presentation)
            if(IsImpress())
            {
                uno::Reference< presentation::XPresentationPage > xPresPage(xMasterPage, uno::UNO_QUERY);
                if(xPresPage.is())
                {
                    uno::Reference< drawing::XDrawPage > xNotesPage(xPresPage->getNotesPage());
                    if(xNotesPage.is())
                    {
                        uno::Reference< drawing::XShapes > xShapes(xNotesPage, uno::UNO_QUERY);
                        if(xShapes.is() && xShapes->getCount())
                        {
                            ImpXMLEXPPageMasterInfo* pInfo = mpNotesPageMasterUsageList->GetObject(nMPageId);
                            if(pInfo)
                            {
                                OUString sString = pInfo->GetName();
                                AddAttribute(XML_NAMESPACE_STYLE, XML_PAGE_MASTER_NAME, sString);
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
        }
    }
}

void SdXMLExport::exportFormsElement( uno::Reference< drawing::XDrawPage > xDrawPage )
{
    if( xDrawPage.is() )
    {
        uno::Reference< form::XFormsSupplier > xFormsSupplier( xDrawPage, uno::UNO_QUERY );
        if( xFormsSupplier.is() )
        {
            uno::Reference< container::XNameContainer > xForms( xFormsSupplier->getForms() );
            if( xForms.is() && xForms->hasElements() )
            {
                // write masterpage
                ::binfilter::xmloff::OOfficeFormsExport aForms(*this);
                GetFormExport()->exportForms( xDrawPage );
            }
        }

        sal_Bool bRet = GetFormExport()->seekPage( xDrawPage );
        DBG_ASSERT( bRet, "OFormLayerXMLExport::seekPage failed!" );
    }
}

void SdXMLExport::GetViewSettings(uno::Sequence<beans::PropertyValue>& rProps)
{
    rProps.realloc(4);	
    beans::PropertyValue* pProps = rProps.getArray();
    if(pProps)
    {
//		SvXMLElementExport aViewSettingsElem(*this, XML_NAMESPACE_DRAW, XML_VIEW_SETTINGS, sal_True, sal_True);

        uno::Reference< beans::XPropertySet > xPropSet( GetModel(), uno::UNO_QUERY );
        if( !xPropSet.is() )
            return;

        awt::Rectangle aVisArea;
        xPropSet->getPropertyValue( OUString( RTL_CONSTASCII_USTRINGPARAM( "VisibleArea" ) ) ) >>= aVisArea;
/*
        sal_Int16 nMapUnit;
        xPropSet->getPropertyValue( OUString( RTL_CONSTASCII_USTRINGPARAM( "MapUnit" ) ) ) >>= nMapUnit;
*/

        sal_uInt16 i = 0;
        pProps[i].Name = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("VisibleAreaTop"));
        pProps[i++].Value <<= aVisArea.Y;
        pProps[i].Name = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("VisibleAreaLeft"));
        pProps[i++].Value <<= aVisArea.X;
        pProps[i].Name = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("VisibleAreaWidth"));
        pProps[i++].Value <<= aVisArea.Width;
        pProps[i].Name = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("VisibleAreaHeight"));
        pProps[i++].Value <<= aVisArea.Height;
    }
}

void SdXMLExport::GetConfigurationSettings(uno::Sequence<beans::PropertyValue>& rProps)
{
    uno::Reference< lang::XMultiServiceFactory > xFac( GetModel(), uno::UNO_QUERY );
    if( xFac.is() )
    {
        uno::Reference< beans::XPropertySet > xProps( xFac->createInstance( OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.document.Settings" ) ) ), uno::UNO_QUERY );
        if( xProps.is() )
            SvXMLUnitConverter::convertPropertySet( rProps, xProps );
    }
}

//////////////////////////////////////////////////////////////////////////////

void SdXMLExport::addDataStyle(const sal_Int32 nNumberFormat, sal_Bool bTimeFormat )
{
    sal_Int32 nFormat = nNumberFormat;
    if( nNumberFormat > 1 )
        nFormat -= 2;

    const sal_uInt32 nIndex = 1 << nFormat;

    if( bTimeFormat )
    {
        mnUsedTimeStyles |= nIndex;
    }
    else
    {
        mnUsedDateStyles |= nIndex;
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
    for( sal_Int16 nDateFormat = 0; nDateFormat < SdXMLNumberStylesExporter::getDateStyleCount(); nDateFormat++ )
    {
        const sal_uInt32 nIndex = 1 << nDateFormat;
        if( (mnUsedDateStyles & nIndex) != 0 )
            SdXMLNumberStylesExporter::exportDateStyle( *this, nDateFormat );
    }
            
    for( sal_Int16 nTimeFormat = 0; nTimeFormat < SdXMLNumberStylesExporter::getTimeStyleCount(); nTimeFormat++ )
    {
        const sal_uInt32 nIndex = 1 << nTimeFormat;
        if( (mnUsedTimeStyles & nIndex) != 0 )
            SdXMLNumberStylesExporter::exportTimeStyle( *this, nTimeFormat );
    }

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

//////////////////////////////////////////////////////////////////////////////

uno::Sequence< OUString > SAL_CALL SdImpressXMLExport_getSupportedServiceNames() throw()
{
    const OUString aServiceName( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.comp.Impress.XMLExporter" ) );
    const uno::Sequence< OUString > aSeq( &aServiceName, 1 );
    return aSeq;
}

OUString SAL_CALL SdImpressXMLExport_getImplementationName() throw()
{
    return OUString( RTL_CONSTASCII_USTRINGPARAM( "SdXMLExport.Impress" ) );
}

uno::Reference< uno::XInterface > SAL_CALL SdImpressXMLExport_createInstance(const uno::Reference< lang::XMultiServiceFactory > & rSMgr) throw( uno::Exception )
{
    // #110680#
    // return (cppu::OWeakObject*)new SdXMLExport( sal_False, EXPORT_META|EXPORT_STYLES|EXPORT_MASTERSTYLES|EXPORT_AUTOSTYLES|EXPORT_CONTENT|EXPORT_SCRIPTS|EXPORT_SETTINGS|EXPORT_FONTDECLS|EXPORT_EMBEDDED );
    return (cppu::OWeakObject*)new SdXMLExport( rSMgr, sal_False, EXPORT_META|EXPORT_STYLES|EXPORT_MASTERSTYLES|EXPORT_AUTOSTYLES|EXPORT_CONTENT|EXPORT_SCRIPTS|EXPORT_SETTINGS|EXPORT_FONTDECLS|EXPORT_EMBEDDED);
}

uno::Sequence< OUString > SAL_CALL SdDrawXMLExport_getSupportedServiceNames() throw()
{
    const OUString aServiceName( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.comp.Draw.XMLExporter" ) );
    const uno::Sequence< OUString > aSeq( &aServiceName, 1 );
    return aSeq;
}

OUString SAL_CALL SdDrawXMLExport_getImplementationName() throw()
{
    return OUString( RTL_CONSTASCII_USTRINGPARAM( "SdXMLExport.Draw" ) );
}

uno::Reference< uno::XInterface > SAL_CALL SdDrawXMLExport_createInstance(const uno::Reference< lang::XMultiServiceFactory > & rSMgr) throw( uno::Exception )
{
    // #110680#
    // return (cppu::OWeakObject*)new SdXMLExport( sal_True, EXPORT_META|EXPORT_STYLES|EXPORT_MASTERSTYLES|EXPORT_AUTOSTYLES|EXPORT_CONTENT|EXPORT_SCRIPTS|EXPORT_SETTINGS|EXPORT_FONTDECLS|EXPORT_EMBEDDED );
    return (cppu::OWeakObject*)new SdXMLExport( rSMgr, sal_True, EXPORT_META|EXPORT_STYLES|EXPORT_MASTERSTYLES|EXPORT_AUTOSTYLES|EXPORT_CONTENT|EXPORT_SCRIPTS|EXPORT_SETTINGS|EXPORT_FONTDECLS|EXPORT_EMBEDDED);
}

//////////////////////////////////////////////////////////////////////////////

uno::Sequence< OUString > SAL_CALL DrawingLayerXMLExport_getSupportedServiceNames() throw()
{
    const OUString aServiceName( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.comp.DrawingLayer.XMLExporter" ) );
    const uno::Sequence< OUString > aSeq( &aServiceName, 1 );
    return aSeq;
}

OUString SAL_CALL DrawingLayerXMLExport_getImplementationName() throw()
{
    return OUString( RTL_CONSTASCII_USTRINGPARAM( "DrawingLayerXMLExport" ) );
}

uno::Reference< uno::XInterface > SAL_CALL DrawingLayerXMLExport_createInstance(const uno::Reference< lang::XMultiServiceFactory > & rSMgr) throw( uno::Exception )
{
    // #110680#
    // return (cppu::OWeakObject*)new SdXMLExport( sal_True, EXPORT_STYLES|EXPORT_AUTOSTYLES|EXPORT_CONTENT|EXPORT_FONTDECLS|EXPORT_EMBEDDED );
    return (cppu::OWeakObject*)new SdXMLExport( rSMgr, sal_True, EXPORT_STYLES|EXPORT_AUTOSTYLES|EXPORT_CONTENT|EXPORT_FONTDECLS|EXPORT_EMBEDDED );
}

//////////////////////////////////////////////////////////////////////////////

uno::Sequence< OUString > SAL_CALL ImpressXMLClipboardExport_getSupportedServiceNames() throw()
{
    const OUString aServiceName( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.comp.Impress.XMLClipboardExporter" ) );
    const uno::Sequence< OUString > aSeq( &aServiceName, 1 );
    return aSeq;
}

OUString SAL_CALL ImpressXMLClipboardExport_getImplementationName() throw()
{
    return OUString( RTL_CONSTASCII_USTRINGPARAM( "ImpressXMLClipboardExport" ) );
}

uno::Reference< uno::XInterface > SAL_CALL ImpressXMLClipboardExport_createInstance(const uno::Reference< lang::XMultiServiceFactory > & rSMgr) throw( uno::Exception )
{
    // #110680#
    // return (cppu::OWeakObject*)new SdXMLExport( sal_False, EXPORT_STYLES|EXPORT_AUTOSTYLES|EXPORT_CONTENT|EXPORT_FONTDECLS|EXPORT_EMBEDDED );
    return (cppu::OWeakObject*)new SdXMLExport( rSMgr, sal_False, EXPORT_STYLES|EXPORT_AUTOSTYLES|EXPORT_CONTENT|EXPORT_FONTDECLS|EXPORT_EMBEDDED );
}
//////////////////////////////////////////////////////////////////////////////

uno::Sequence< OUString > SAL_CALL SdImpressXMLExport_Style_getSupportedServiceNames() throw()
{
    const OUString aServiceName( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.comp.Impress.XMLStylesExporter" ) );
    const uno::Sequence< OUString > aSeq( &aServiceName, 1 );
    return aSeq;
}

OUString SAL_CALL SdImpressXMLExport_Style_getImplementationName() throw()
{
    return OUString( RTL_CONSTASCII_USTRINGPARAM( "SdXMLExport.Impress.Styles" ) );
}

uno::Reference< uno::XInterface > SAL_CALL SdImpressXMLExport_Style_createInstance(const uno::Reference< lang::XMultiServiceFactory > & rSMgr) throw( uno::Exception )
{
    // #110680#
    // return (cppu::OWeakObject*)new SdXMLExport( sal_False, EXPORT_STYLES|EXPORT_MASTERSTYLES|EXPORT_AUTOSTYLES );
    return (cppu::OWeakObject*)new SdXMLExport( rSMgr, sal_False, EXPORT_STYLES|EXPORT_MASTERSTYLES|EXPORT_AUTOSTYLES );
}

uno::Sequence< OUString > SAL_CALL SdDrawXMLExport_Style_getSupportedServiceNames() throw()
{
    const OUString aServiceName( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.comp.Draw.XMLStylesExporter" ) );
    const uno::Sequence< OUString > aSeq( &aServiceName, 1 );
    return aSeq;
}

OUString SAL_CALL SdDrawXMLExport_Style_getImplementationName() throw()
{
    return OUString( RTL_CONSTASCII_USTRINGPARAM( "SdXMLExport.Draw.Styles" ) );
}

uno::Reference< uno::XInterface > SAL_CALL SdDrawXMLExport_Style_createInstance(const uno::Reference< lang::XMultiServiceFactory > & rSMgr) throw( uno::Exception )
{
    // #110680#
    // return (cppu::OWeakObject*)new SdXMLExport( sal_True, EXPORT_STYLES|EXPORT_MASTERSTYLES|EXPORT_AUTOSTYLES );
    return (cppu::OWeakObject*)new SdXMLExport( rSMgr, sal_True, EXPORT_STYLES|EXPORT_MASTERSTYLES|EXPORT_AUTOSTYLES );
}

//////////////////////////////////////////////////////////////////////////////

uno::Sequence< OUString > SAL_CALL SdImpressXMLExport_Meta_getSupportedServiceNames() throw()
{
    const OUString aServiceName( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.comp.Impress.XMLMetaExporter" ) );
    const uno::Sequence< OUString > aSeq( &aServiceName, 1 );
    return aSeq;
}

OUString SAL_CALL SdImpressXMLExport_Meta_getImplementationName() throw()
{
    return OUString( RTL_CONSTASCII_USTRINGPARAM( "SdXMLExport.Impress.Meta" ) );
}

uno::Reference< uno::XInterface > SAL_CALL SdImpressXMLExport_Meta_createInstance(const uno::Reference< lang::XMultiServiceFactory > & rSMgr) throw( uno::Exception )
{
    // #110680#
    // return (cppu::OWeakObject*)new SdXMLExport( sal_False, EXPORT_META );
    return (cppu::OWeakObject*)new SdXMLExport( rSMgr, sal_False, EXPORT_META );
}

uno::Sequence< OUString > SAL_CALL SdDrawXMLExport_Meta_getSupportedServiceNames() throw()
{
    const OUString aServiceName( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.comp.Draw.XMLMetaExporter" ) );
    const uno::Sequence< OUString > aSeq( &aServiceName, 1 );
    return aSeq;
}

OUString SAL_CALL SdDrawXMLExport_Meta_getImplementationName() throw()
{
    return OUString( RTL_CONSTASCII_USTRINGPARAM( "SdXMLExport.Draw.Meta" ) );
}

uno::Reference< uno::XInterface > SAL_CALL SdDrawXMLExport_Meta_createInstance(const uno::Reference< lang::XMultiServiceFactory > & rSMgr) throw( uno::Exception )
{
    // #110680#
    // return (cppu::OWeakObject*)new SdXMLExport( sal_True, EXPORT_META );
    return (cppu::OWeakObject*)new SdXMLExport( rSMgr, sal_True, EXPORT_META );
}

//////////////////////////////////////////////////////////////////////////////

uno::Sequence< OUString > SAL_CALL SdImpressXMLExport_Settings_getSupportedServiceNames() throw()
{
    const OUString aServiceName( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.comp.Impress.XMLSettingsExporter" ) );
    const uno::Sequence< OUString > aSeq( &aServiceName, 1 );
    return aSeq;
}

OUString SAL_CALL SdImpressXMLExport_Settings_getImplementationName() throw()
{
    return OUString( RTL_CONSTASCII_USTRINGPARAM( "SdXMLExport.Impress.Settings" ) );
}

uno::Reference< uno::XInterface > SAL_CALL SdImpressXMLExport_Settings_createInstance(const uno::Reference< lang::XMultiServiceFactory > & rSMgr) throw( uno::Exception )
{
    // #110680#
    // return (cppu::OWeakObject*)new SdXMLExport( sal_False, EXPORT_SETTINGS );
    return (cppu::OWeakObject*)new SdXMLExport( rSMgr, sal_False, EXPORT_SETTINGS );
}

uno::Sequence< OUString > SAL_CALL SdDrawXMLExport_Settings_getSupportedServiceNames() throw()
{
    const OUString aServiceName( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.comp.Draw.XMLSettingsExporter" ) );
    const uno::Sequence< OUString > aSeq( &aServiceName, 1 );
    return aSeq;
}

OUString SAL_CALL SdDrawXMLExport_Settings_getImplementationName() throw()
{
    return OUString( RTL_CONSTASCII_USTRINGPARAM( "SdXMLExport.Draw.Settings" ) );
}

uno::Reference< uno::XInterface > SAL_CALL SdDrawXMLExport_Settings_createInstance(const uno::Reference< lang::XMultiServiceFactory > & rSMgr) throw( uno::Exception )
{
    // #110680#
    // return (cppu::OWeakObject*)new SdXMLExport( sal_True, EXPORT_SETTINGS );
    return (cppu::OWeakObject*)new SdXMLExport( rSMgr, sal_True, EXPORT_SETTINGS );
}

//////////////////////////////////////////////////////////////////////////////

uno::Sequence< OUString > SAL_CALL SdImpressXMLExport_Content_getSupportedServiceNames() throw()
{
    const OUString aServiceName( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.comp.Impress.XMLContentExporter" ) );
    const uno::Sequence< OUString > aSeq( &aServiceName, 1 );
    return aSeq;
}

OUString SAL_CALL SdImpressXMLExport_Content_getImplementationName() throw()
{
    return OUString( RTL_CONSTASCII_USTRINGPARAM( "SdXMLExport.Impress.Content" ) );
}

uno::Reference< uno::XInterface > SAL_CALL SdImpressXMLExport_Content_createInstance(const uno::Reference< lang::XMultiServiceFactory > & rSMgr) throw( uno::Exception )
{
    // #110680#
    // return (cppu::OWeakObject*)new SdXMLExport( sal_False, EXPORT_AUTOSTYLES|EXPORT_CONTENT|EXPORT_SCRIPTS|EXPORT_FONTDECLS );
    return (cppu::OWeakObject*)new SdXMLExport( rSMgr, sal_False, EXPORT_AUTOSTYLES|EXPORT_CONTENT|EXPORT_SCRIPTS|EXPORT_FONTDECLS );
}

uno::Sequence< OUString > SAL_CALL SdDrawXMLExport_Content_getSupportedServiceNames() throw()
{
    const OUString aServiceName( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.comp.Draw.XMLContentExporter" ) );
    const uno::Sequence< OUString > aSeq( &aServiceName, 1 );
    return aSeq;
}

OUString SAL_CALL SdDrawXMLExport_Content_getImplementationName() throw()
{
    return OUString( RTL_CONSTASCII_USTRINGPARAM( "SdXMLExport.Draw.Content" ) );
}

uno::Reference< uno::XInterface > SAL_CALL SdDrawXMLExport_Content_createInstance(const uno::Reference< lang::XMultiServiceFactory > & rSMgr) throw( uno::Exception )
{
    // #110680#
    // return (cppu::OWeakObject*)new SdXMLExport( sal_True, EXPORT_AUTOSTYLES|EXPORT_CONTENT|EXPORT_SCRIPTS|EXPORT_FONTDECLS );
    return (cppu::OWeakObject*)new SdXMLExport( rSMgr, sal_True, EXPORT_AUTOSTYLES|EXPORT_CONTENT|EXPORT_SCRIPTS|EXPORT_FONTDECLS );
}

// XServiceInfo
OUString SAL_CALL SdXMLExport::getImplementationName() throw( uno::RuntimeException )
{
    if( IsDraw())
    {
        // Draw

        switch( getExportFlags())
        {
            case EXPORT_ALL:
                return SdDrawXMLExport_getImplementationName();
            case (EXPORT_STYLES|EXPORT_MASTERSTYLES|EXPORT_AUTOSTYLES):
                return SdDrawXMLExport_Style_getImplementationName();
            case (EXPORT_AUTOSTYLES|EXPORT_CONTENT|EXPORT_SCRIPTS|EXPORT_FONTDECLS):
                return SdDrawXMLExport_Content_getImplementationName();
            case EXPORT_META:
                return SdDrawXMLExport_Meta_getImplementationName();
            case EXPORT_SETTINGS:
                return SdDrawXMLExport_Settings_getImplementationName();
            default:
                return OUString::createFromAscii( "SdXMLExport.Draw" );
        }
    }
    else
    {
        // Impress

        switch( getExportFlags())
        {
            case EXPORT_ALL:
                return SdImpressXMLExport_getImplementationName();
            case (EXPORT_STYLES|EXPORT_MASTERSTYLES|EXPORT_AUTOSTYLES):
                return SdImpressXMLExport_Style_getImplementationName();
            case (EXPORT_AUTOSTYLES|EXPORT_CONTENT|EXPORT_SCRIPTS|EXPORT_FONTDECLS):
                return SdImpressXMLExport_Content_getImplementationName();
            case EXPORT_META:
                return SdImpressXMLExport_Meta_getImplementationName();
            case EXPORT_SETTINGS:
                return SdImpressXMLExport_Settings_getImplementationName();
            default:
                return OUString::createFromAscii( "SdXMLExport.Impress" );
        }
    }
}
}//end of namespace binfilter

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
