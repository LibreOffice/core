/*************************************************************************
 *
 *  $RCSfile: sdxmlexp.cxx,v $
 *
 *  $Revision: 1.48 $
 *
 *  last change: $Author: cl $ $Date: 2001-02-21 18:04:45 $
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
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#pragma hdrstop

#ifndef _XMLOFF_NMSPMAP_HXX
#include "nmspmap.hxx"
#endif

#ifndef _XMLOFF_XMLNMSPE_HXX
#include "xmlnmspe.hxx"
#endif

#ifndef _XMLOFF_XMLUCONV_HXX
#include "xmluconv.hxx"
#endif

#ifndef _XMLOFF_XMLKYWD_HXX
#include "xmlkywd.hxx"
#endif

#ifndef _LANG_HXX
#include <tools/lang.hxx>
#endif

#ifndef _XMLOFF_XMLMETAE_HXX
#include "xmlmetae.hxx"
#endif

#ifndef _COM_SUN_STAR_PRESENTATION_XPRESENTATIONSUPPLIER_HPP_
#include <com/sun/star/presentation/XPresentationSupplier.hpp>
#endif

#ifndef _COM_SUN_STAR_PRESENTATION_XCUSTOMPRESENTATIONSUPPLIER_HPP_
#include <com/sun/star/presentation/XCustomPresentationSupplier.hpp>
#endif

#ifndef _COM_SUN_STAR_DOCUMENT_XDOCUMENTINFOSUPPLIER_HPP_
#include <com/sun/star/document/XDocumentInfoSupplier.hpp>
#endif

#ifndef _COM_SUN_STAR_TASK_XSTATUSINDICATORSUPPLIER_HPP_
#include <com/sun/star/task/XStatusIndicatorSupplier.hpp>
#endif

#ifndef _COM_SUN_STAR_LANG_LOCALE_HPP_
#include <com/sun/star/lang/Locale.hpp>
#endif

#ifndef _COM_SUN_STAR_UNO_ANY_HXX_
#include <com/sun/star/uno/Any.hxx>
#endif

#ifndef _SDXMLEXP_HXX
#include "sdxmlexp.hxx"
#endif

#ifndef _SDXMLEXP_IMPL_HXX
#include "sdxmlexp_impl.hxx"
#endif

#ifndef _COM_SUN_STAR_DRAWING_XDRAWPAGESSUPPLIER_HPP_
#include <com/sun/star/drawing/XDrawPagesSupplier.hpp>
#endif

#ifndef _COM_SUN_STAR_DRAWING_XMASTERPAGESSUPPLIER_HPP_
#include <com/sun/star/drawing/XMasterPagesSupplier.hpp>
#endif

#ifndef _COM_SUN_STAR_VIEW_PAPERORIENTATION_HPP_
#include <com/sun/star/view/PaperOrientation.hpp>
#endif

#ifndef _COM_SUN_STAR_STYLE_XSTYLEFAMILIESSUPPLIER_HPP_
#include <com/sun/star/style/XStyleFamiliesSupplier.hpp>
#endif

#ifndef _COM_SUN_STAR_STYLE_XSTYLE_HPP_
#include <com/sun/star/style/XStyle.hpp>
#endif

#ifndef _COM_SUN_STAR_FORM_XFORMSUPPLIER_HPP_
#include <com/sun/star/form/XFormsSupplier.hpp>
#endif

#ifndef _COM_SUN_STAR_PRESENTATION_XPRESENTATIONPAGE_HPP_
#include <com/sun/star/presentation/XPresentationPage.hpp>
#endif

#ifndef _COM_SUN_STAR_DRAWING_XMASTERPAGETARGET_HPP_
#include <com/sun/star/drawing/XMasterPageTarget.hpp>
#endif

#ifndef _COM_SUN_STAR_TEXT_XTEXT_HPP_
#include <com/sun/star/text/XText.hpp>
#endif

#ifndef _COM_SUN_STAR_CHART_XCHARTDOCUMENT_HPP_
#include <com/sun/star/chart/XChartDocument.hpp>
#endif

#ifndef _COM_SUN_STAR_CONTAINER_XNAMED_HPP_
#include <com/sun/star/container/XNamed.hpp>
#endif

#ifndef _RTL_USTRBUF_HXX_
#include <rtl/ustrbuf.hxx>
#endif

#ifndef _SV_GEN_HXX
#include <tools/gen.hxx>
#endif

#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif

#ifndef _XMLOFF_XMLASTPLP_HXX
#include "xmlaustp.hxx"
#endif

#ifndef _XMLOFF_FAMILIES_HXX_
#include "families.hxx"
#endif

#ifndef _XMLOFF_STYLEEXP_HXX_
#include "styleexp.hxx"
#endif

#ifndef _SDPROPLS_HXX
#include "sdpropls.hxx"
#endif

#ifndef _XMLOFF_XMLEXPPR_HXX
#include "xmlexppr.hxx"
#endif

#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSTATE_HPP_
#include <com/sun/star/beans/XPropertyState.hpp>
#endif

#ifndef _XEXPTRANSFORM_HXX
#include "xexptran.hxx"
#endif

#ifndef _ISOLANG_HXX
#include <tools/isolang.hxx>
#endif

#ifndef _CPPUHELPER_IMPLBASE1_HXX
#include <cppuhelper/implbase1.hxx>
#endif

#ifndef _CPPUHELPER_EXTRACT_HXX_
#include <cppuhelper/extract.hxx>
#endif

#ifndef _COM_SUN_STAR_LANG_XSERVICEINFO_HPP_
#include <com/sun/star/lang/XServiceInfo.hpp>
#endif

#ifndef _XMLOFF_PROPERTYSETMERGER_HXX_
#include "PropertySetMerger.hxx"
#endif

#ifndef _XMLOFF_LAYEREXP_HXX
#include "layerexp.hxx"
#endif


#ifndef _XMLOFF_VISAREAEXPORT_HXX
#include "VisAreaExport.hxx"
#endif

using namespace ::rtl;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::drawing;
using namespace ::com::sun::star::presentation;


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
:   mnBorderBottom(0),
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

        aAny = xPropSet->getPropertyValue(OUString(RTL_CONSTASCII_USTRINGPARAM("BorderBottom")));
        aAny >>= mnBorderBottom;

        aAny = xPropSet->getPropertyValue(OUString(RTL_CONSTASCII_USTRINGPARAM("BorderLeft")));
        aAny >>= mnBorderLeft;

        aAny = xPropSet->getPropertyValue(OUString(RTL_CONSTASCII_USTRINGPARAM("BorderRight")));
        aAny >>= mnBorderRight;

        aAny = xPropSet->getPropertyValue(OUString(RTL_CONSTASCII_USTRINGPARAM("BorderTop")));
        aAny >>= mnBorderTop;

        aAny = xPropSet->getPropertyValue(OUString(RTL_CONSTASCII_USTRINGPARAM("Width")));
        aAny >>= mnWidth;

        aAny = xPropSet->getPropertyValue(OUString(RTL_CONSTASCII_USTRINGPARAM("Height")));
        aAny >>= mnHeight;

        aAny = xPropSet->getPropertyValue(OUString(RTL_CONSTASCII_USTRINGPARAM("Orientation")));
        aAny >>= meOrientation;
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

DECLARE_LIST(ImpXMLEXPPageMasterList, ImpXMLEXPPageMasterInfo*);

//////////////////////////////////////////////////////////////////////////////

#define IMP_AUTOLAYOUT_INFO_MAX         (27L)

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

DECLARE_LIST(ImpXMLAutoLayoutInfoList, ImpXMLAutoLayoutInfo*);

//////////////////////////////////////////////////////////////////////////////

SdXMLExport::SdXMLExport( sal_Bool bIsDraw)
:   SvXMLExport( MAP_CM, bIsDraw ? sXML_drawing : sXML_presentation ),
    mpPageMasterInfoList(new ImpXMLEXPPageMasterList(1, 4, 4)),
    mpPageMaterUsageList(new ImpXMLEXPPageMasterList(1, 4, 4)),
    mpAutoLayoutInfoList(new ImpXMLAutoLayoutInfoList(1, 4, 4)),
    mpPropertySetMapper(0L),
    mpPresPagePropsMapper(0L),
    mnDocMasterPageCount(0L),
    mnDocDrawPageCount(0L),
    mnShapeStyleInfoIndex(0L),
    mbIsDraw(bIsDraw),
    mbFamilyGraphicUsed(FALSE),
    mbFamilyPresentationUsed(FALSE),
    msZIndex( RTL_CONSTASCII_USTRINGPARAM(sXML_zindex) ),
    msEmptyPres( RTL_CONSTASCII_USTRINGPARAM("IsEmptyPresentationObject") ),
    msModel( RTL_CONSTASCII_USTRINGPARAM("Model") ),
    msStartShape( RTL_CONSTASCII_USTRINGPARAM("StartShape") ),
    msEndShape( RTL_CONSTASCII_USTRINGPARAM("EndShape") )
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
        UniReference < XMLPropertySetMapper > xMapper = new XMLShapePropertySetMapper( aFactoryRef);

        mpPropertySetMapper = new XMLShapeExportPropertyMapper( xMapper, (XMLTextListAutoStylePool*)&GetTextParagraphExport()->GetListAutoStylePool(), *this );
        // set lock to avoid deletion
        mpPropertySetMapper->acquire();

        // chain text attributes
        mpPropertySetMapper->ChainExportMapper(XMLTextParagraphExport::CreateCharExtPropMapper(*this));

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
            {
                maDrawPagesAutoLayoutNames.insert( maDrawPagesAutoLayoutNames.begin(), mnDocDrawPageCount, aEmpty );
            }
        }
    }

    // add namespaces
    _GetNamespaceMap().AddAtIndex(
        XML_NAMESPACE_PRESENTATION, sXML_np_presentation, sXML_n_presentation, XML_NAMESPACE_PRESENTATION);

    GetShapeExport()->enableLayerExport();
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
    if(mpPageMaterUsageList)
    {
        delete mpPageMaterUsageList;
        mpPageMaterUsageList = 0L;
    }

    // clear auto-layout infos
    if(mpAutoLayoutInfoList)
    {
        while(mpAutoLayoutInfoList->Count())
            delete mpAutoLayoutInfoList->Remove(mpAutoLayoutInfoList->Count() - 1L);
        delete mpAutoLayoutInfoList;
        mpAutoLayoutInfoList = 0L;
    }

    // stop progress view
    if(GetStatusIndicator().is())
    {
        GetStatusIndicator()->end();
        GetStatusIndicator()->reset();
    }
}

//////////////////////////////////////////////////////////////////////////////
// to get default values in XPropertySet use this wrapper class

class ImpDefaultMapper : public ::cppu::WeakAggImplHelper1< beans::XPropertySet >
{
    uno::Reference< beans::XPropertyState >     mxState;
    uno::Reference< beans::XPropertySet >       mxSet;

public:
    ImpDefaultMapper( uno::Reference< beans::XPropertyState >& rxState );

    // Methods
    virtual uno::Reference< beans::XPropertySetInfo > SAL_CALL getPropertySetInfo() throw(uno::RuntimeException);
    virtual void SAL_CALL setPropertyValue( const OUString& aPropertyName, const uno::Any& aValue ) throw(beans::UnknownPropertyException, beans::PropertyVetoException, lang::IllegalArgumentException, lang::WrappedTargetException, uno::RuntimeException);
    virtual uno::Any SAL_CALL getPropertyValue( const OUString& PropertyName ) throw(beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException);

    // empty implementations
    virtual void SAL_CALL addPropertyChangeListener( const OUString& aPropertyName, const uno::Reference< beans::XPropertyChangeListener >& xListener ) throw(beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException);
    virtual void SAL_CALL removePropertyChangeListener( const OUString& aPropertyName, const uno::Reference< beans::XPropertyChangeListener >& aListener ) throw(beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException);
    virtual void SAL_CALL addVetoableChangeListener( const OUString& PropertyName, const uno::Reference< beans::XVetoableChangeListener >& aListener ) throw(beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException);
    virtual void SAL_CALL removeVetoableChangeListener( const OUString& PropertyName, const uno::Reference< beans::XVetoableChangeListener >& aListener ) throw(beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException);
};

ImpDefaultMapper::ImpDefaultMapper( uno::Reference< beans::XPropertyState >& rxState )
:   mxState( rxState ),
    mxSet( rxState, uno::UNO_QUERY )
{
}

uno::Reference< beans::XPropertySetInfo > SAL_CALL ImpDefaultMapper::getPropertySetInfo() throw(uno::RuntimeException)
{
    return mxSet->getPropertySetInfo();
}

void SAL_CALL ImpDefaultMapper::setPropertyValue( const OUString& aPropertyName, const uno::Any& aValue ) throw(beans::UnknownPropertyException, beans::PropertyVetoException, lang::IllegalArgumentException, lang::WrappedTargetException, uno::RuntimeException)
{
    mxState->setPropertyToDefault( aPropertyName /*, aValue */ );
}

uno::Any SAL_CALL ImpDefaultMapper::getPropertyValue( const OUString& PropertyName ) throw(beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException)
{
    return mxState->getPropertyDefault(  PropertyName  );
}

// empty implementations
void SAL_CALL ImpDefaultMapper::addPropertyChangeListener( const OUString& aPropertyName, const uno::Reference< beans::XPropertyChangeListener >& xListener ) throw(beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException) {}
void SAL_CALL ImpDefaultMapper::removePropertyChangeListener( const OUString& aPropertyName, const uno::Reference< beans::XPropertyChangeListener >& aListener ) throw(beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException) {}
void SAL_CALL ImpDefaultMapper::addVetoableChangeListener( const OUString& PropertyName, const uno::Reference< beans::XVetoableChangeListener >& aListener ) throw(beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException) {}
void SAL_CALL ImpDefaultMapper::removeVetoableChangeListener( const OUString& PropertyName, const uno::Reference< beans::XVetoableChangeListener >& aListener ) throw(beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException) {}

//////////////////////////////////////////////////////////////////////////////

void SdXMLExport::ImpWriteDefaultStyleInfos()
{
    // create annd write pool defaults
    AddAttribute(XML_NAMESPACE_STYLE, sXML_name, OUString(RTL_CONSTASCII_USTRINGPARAM(sXML_drawpool)));
    AddAttribute(XML_NAMESPACE_STYLE, sXML_family, OUString(RTL_CONSTASCII_USTRINGPARAM(XML_STYLE_FAMILY_SD_POOL_NAME)));

    // write drawpool style
    SvXMLElementExport aPSY(*this, XML_NAMESPACE_STYLE, sXML_style, sal_True, sal_True);

    // write graphics style properites
    uno::Any aAny(mxDocStyleFamilies->getByName(OUString(RTL_CONSTASCII_USTRINGPARAM(XML_STYLE_FAMILY_SD_GRAPHICS_NAME))));
    uno::Reference< container::XIndexAccess > xGraphicStyles;

    if(aAny >>= xGraphicStyles)
    {
        BOOL bDone(FALSE);
        const sal_Int32 nNum = xGraphicStyles->getCount();

        for(sal_Int32 a(0L); !bDone && a < nNum; a++)
        {
            aAny = xGraphicStyles->getByIndex(a);
            uno::Reference< style::XStyle > xSingleStyle;

            if(aAny >>= xSingleStyle)
            {
                OUString aParentStyle = xSingleStyle->getParentStyle();
                if(!aParentStyle.getLength())
                {
                    // style without parent found
                    uno::Reference< beans::XPropertyState > xPropState(xSingleStyle, uno::UNO_QUERY);
                    if(xPropState.is())
                    {
                        uno::Reference< beans::XPropertySet > xImpDefaultMapper( new ImpDefaultMapper( xPropState ) );
                        const UniReference< SvXMLExportPropertyMapper > aMapperRef = GetPropertySetMapper();
                        std::vector< XMLPropertyState > xPropStates = aMapperRef->Filter( xImpDefaultMapper );

                        if(xPropStates.size())
                        {
                            aMapperRef->exportXML(GetDocHandler(), xPropStates,
                                GetMM100UnitConverter(), GetNamespaceMap());
                            bDone = TRUE;
                        }
                    }
                }
            }
        }
    }
}

//////////////////////////////////////////////////////////////////////////////

void SdXMLExport::ImpWriteObjGraphicStyleInfos()
{
    XMLStyleExport aStEx(*this,
        OUString(RTL_CONSTASCII_USTRINGPARAM(sXML_drawpool)), GetAutoStylePool().get());
    const UniReference< SvXMLExportPropertyMapper > aMapperRef = GetPropertySetMapper();

    aStEx.exportStyleFamily(XML_STYLE_FAMILY_SD_GRAPHICS_NAME, XML_STYLE_FAMILY_SD_GRAPHICS_NAME,
        aMapperRef, FALSE, XML_STYLE_FAMILY_SD_GRAPHICS_ID);
}

//////////////////////////////////////////////////////////////////////////////

void SdXMLExport::ImpPrepAutoLayoutInfos()
{
    if(IsImpress())
    {
        // prepare name creation
        for(sal_Int32 nCnt = 0L; nCnt < mnDocDrawPageCount; nCnt++)
        {
            uno::Any aAny(mxDocDrawPages->getByIndex(nCnt));
            uno::Reference<drawing::XDrawPage> xDrawPage;

            if((aAny >>= xDrawPage) && xDrawPage.is())
            {
                OUString aStr;
                if(ImpPrepAutoLayoutInfo(xDrawPage, aStr))
                    maDrawPagesAutoLayoutNames[nCnt] = aStr;
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
                AddAttribute(XML_NAMESPACE_STYLE, sXML_name, pInfo->GetLayoutName());

                // write draw-style attributes
                SvXMLElementExport aDSE(*this, XML_NAMESPACE_STYLE, sXML_presentation_page_layout, sal_True, sal_True);

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
    }

    AddAttribute(XML_NAMESPACE_PRESENTATION, sXML_object, aStr);

    // svg:x,y,width,height
    GetMM100UnitConverter().convertMeasure(sStringBuffer, rRect.Left());
    aStr = sStringBuffer.makeStringAndClear();
    AddAttribute(XML_NAMESPACE_SVG, sXML_x, aStr);

    GetMM100UnitConverter().convertMeasure(sStringBuffer, rRect.Top());
    aStr = sStringBuffer.makeStringAndClear();
    AddAttribute(XML_NAMESPACE_SVG, sXML_y, aStr);

    GetMM100UnitConverter().convertMeasure(sStringBuffer, rRect.GetWidth());
    aStr = sStringBuffer.makeStringAndClear();
    AddAttribute(XML_NAMESPACE_SVG, sXML_width, aStr);

    GetMM100UnitConverter().convertMeasure(sStringBuffer, rRect.GetHeight());
    aStr = sStringBuffer.makeStringAndClear();
    AddAttribute(XML_NAMESPACE_SVG, sXML_height, aStr);

    // write presentation-placeholder
    SvXMLElementExport aPPL(*this, XML_NAMESPACE_PRESENTATION, sXML_placeholder, sal_True, sal_True);
}

//////////////////////////////////////////////////////////////////////////////

void SdXMLExport::ImpPrepPageMasterInfos()
{
    if(mnDocMasterPageCount)
    {
        // look for needed page-masters, create these
        for(sal_Int32 nMPageId = 0L; nMPageId < mnDocMasterPageCount; nMPageId++)
        {
            uno::Any aAny(mxDocMasterPages->getByIndex(nMPageId));
            uno::Reference< drawing::XDrawPage > xMasterPage;
            ImpXMLEXPPageMasterInfo* pNewInfo = 0L;
            BOOL bDoesExist(FALSE);

            if(aAny >>= xMasterPage)
            {
                pNewInfo = new ImpXMLEXPPageMasterInfo(*this, xMasterPage);

                // compare with prev page-master infos
                for(sal_uInt32 a = 0; !bDoesExist && a < mpPageMasterInfoList->Count(); a++)
                {
                    if(mpPageMasterInfoList->GetObject(a)
                        && *mpPageMasterInfoList->GetObject(a) == *pNewInfo)
                    {
                        delete pNewInfo;
                        pNewInfo = mpPageMasterInfoList->GetObject(a);
                        bDoesExist = TRUE;
                    }
                }
            }

            // add entry when not found same page-master infos
            if(!bDoesExist)
                mpPageMasterInfoList->Insert(pNewInfo, LIST_APPEND);
            mpPageMaterUsageList->Insert(pNewInfo, LIST_APPEND);
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
            AddAttribute(XML_NAMESPACE_STYLE, sXML_name, sString);

            // write page-master
            SvXMLElementExport aPME(*this, XML_NAMESPACE_STYLE, sXML_page_master, sal_True, sal_True);

            // prepare style:properties inside page-master
            GetMM100UnitConverter().convertMeasure(sStringBuffer, pInfo->GetBorderTop());
            sString = sStringBuffer.makeStringAndClear();
            AddAttribute(XML_NAMESPACE_FO, sXML_margin_top, sString);

            GetMM100UnitConverter().convertMeasure(sStringBuffer, pInfo->GetBorderBottom());
            sString = sStringBuffer.makeStringAndClear();
            AddAttribute(XML_NAMESPACE_FO, sXML_margin_bottom, sString);

            GetMM100UnitConverter().convertMeasure(sStringBuffer, pInfo->GetBorderLeft());
            sString = sStringBuffer.makeStringAndClear();
            AddAttribute(XML_NAMESPACE_FO, sXML_margin_left, sString);

            GetMM100UnitConverter().convertMeasure(sStringBuffer, pInfo->GetBorderRight());
            sString = sStringBuffer.makeStringAndClear();
            AddAttribute(XML_NAMESPACE_FO, sXML_margin_right, sString);

            GetMM100UnitConverter().convertMeasure(sStringBuffer, pInfo->GetWidth());
            sString = sStringBuffer.makeStringAndClear();
            AddAttribute(XML_NAMESPACE_FO, sXML_page_width, sString);

            GetMM100UnitConverter().convertMeasure(sStringBuffer, pInfo->GetHeight());
            sString = sStringBuffer.makeStringAndClear();
            AddAttribute(XML_NAMESPACE_FO, sXML_page_height, sString);

            if(pInfo->GetOrientation() == view::PaperOrientation_PORTRAIT)
                AddAttributeASCII(XML_NAMESPACE_STYLE, sXML_print_orientation, sXML_portrait);
            else
                AddAttributeASCII(XML_NAMESPACE_STYLE, sXML_print_orientation, sXML_landscape);

            // write style:properties
            SvXMLElementExport aPMF(*this, XML_NAMESPACE_STYLE, sXML_properties, sal_True, sal_True);
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
                    XMLStyleExport aStEx(*this,
                        OUString(RTL_CONSTASCII_USTRINGPARAM(sXML_drawpool)), GetAutoStylePool().get());
                    const UniReference< SvXMLExportPropertyMapper > aMapperRef( GetPropertySetMapper() );

                    OUString aPrefix = xNamed->getName();
                    aPrefix += OUString(RTL_CONSTASCII_USTRINGPARAM("-"));
                    aStEx.exportStyleFamily(xNamed->getName(),
                        XML_STYLE_FAMILY_SD_PRESENTATION_NAME, aMapperRef, FALSE,
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

void SdXMLExport::_ExportViewSettings()
{
    SvXMLElementExport aViewSettingsElem(*this, XML_NAMESPACE_DRAW, sXML_view_settings, sal_True, sal_True);

    uno::Reference< beans::XPropertySet > xPropSet( GetModel(), uno::UNO_QUERY );
    if( !xPropSet.is() )
        return;

    awt::Rectangle aVisArea;
    xPropSet->getPropertyValue( OUString( RTL_CONSTASCII_USTRINGPARAM( "VisibleArea" ) ) ) >>= aVisArea;

    sal_Int16 nMapUnit;
    xPropSet->getPropertyValue( OUString( RTL_CONSTASCII_USTRINGPARAM( "MapUnit" ) ) ) >>= nMapUnit;

    XMLVisAreaExport aVisAreaExport(*this, sXML_embedded_visible_area, aVisArea, nMapUnit );
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
                AddAttribute(XML_NAMESPACE_DRAW, sXML_name, xNamed->getName());

            // draw:style-name (presentation page attributes AND background attributes)
            if( maDrawPagesStyleNames[nPageInd].getLength() )
                AddAttribute(XML_NAMESPACE_DRAW, sXML_style_name, maDrawPagesStyleNames[nPageInd]);

            if( IsImpress() )
                AddAttribute(XML_NAMESPACE_DRAW, sXML_id, OUString::valueOf( sal_Int32( nPageInd + 1 ) ) );

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
                        AddAttribute(XML_NAMESPACE_DRAW, sXML_master_page_name, xMasterNamed->getName());
                    }
                }
            }

            // presentation:page-layout-name
            if( IsImpress() && maDrawPagesAutoLayoutNames[nPageInd].getLength())
            {
                AddAttribute(XML_NAMESPACE_PRESENTATION, sXML_presentation_page_layout_name, maDrawPagesAutoLayoutNames[nPageInd]);
            }

            // write page
            SvXMLElementExport aDPG(*this, XML_NAMESPACE_DRAW, sXML_page, sal_True, sal_True);

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
                            SvXMLElementExport aPSY(*this, XML_NAMESPACE_PRESENTATION, sXML_notes, sal_True, sal_True);

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
                AddAttribute(XML_NAMESPACE_PRESENTATION, sXML_start_page, aFirstPage );
                bHasAttr = sal_True;
            }
            else
            {
                OUString aCustomShow;
                xPresProps->getPropertyValue( OUString( RTL_CONSTASCII_USTRINGPARAM( "CustomShow" ) ) ) >>= aCustomShow;
                if( aCustomShow.getLength() )
                {
                    AddAttribute(XML_NAMESPACE_PRESENTATION, sXML_show, aCustomShow );
                    bHasAttr = sal_True;
                }
            }
        }

        xPresProps->getPropertyValue( OUString( RTL_CONSTASCII_USTRINGPARAM( "IsEndless" ) ) ) >>= bTemp;
        if( bTemp )
        {
            AddAttributeASCII(XML_NAMESPACE_PRESENTATION, sXML_endless, sXML_true );
            bHasAttr = sal_True;

            sal_Int32 nPause;
            xPresProps->getPropertyValue( OUString( RTL_CONSTASCII_USTRINGPARAM( "Pause" ) ) ) >>= nPause;
            if( nPause )
            {
                util::DateTime aTime( 0, (sal_uInt16)nPause, 0, 0, 0, 0, 0 );

                OUStringBuffer aOut;
                SvXMLUnitConverter::convertTime( aOut, aTime );
                AddAttribute(XML_NAMESPACE_PRESENTATION, sXML_pause, aOut.makeStringAndClear() );
            }
        }

        xPresProps->getPropertyValue( OUString( RTL_CONSTASCII_USTRINGPARAM( "AllowAnimations" ) ) ) >>= bTemp;
        if( !bTemp )
        {
            AddAttributeASCII(XML_NAMESPACE_PRESENTATION, sXML_animations, sXML_disabled );
            bHasAttr = sal_True;
        }

        xPresProps->getPropertyValue( OUString( RTL_CONSTASCII_USTRINGPARAM( "IsAlwaysOnTop" ) ) ) >>= bTemp;
        if( bTemp )
        {
            AddAttributeASCII(XML_NAMESPACE_PRESENTATION, sXML_stay_on_top, sXML_true );
            bHasAttr = sal_True;
        }

        xPresProps->getPropertyValue( OUString( RTL_CONSTASCII_USTRINGPARAM( "IsAutomatic" ) ) ) >>= bTemp;
        if( bTemp )
        {
            AddAttributeASCII(XML_NAMESPACE_PRESENTATION, sXML_force_manual, sXML_true );
            bHasAttr = sal_True;
        }

        xPresProps->getPropertyValue( OUString( RTL_CONSTASCII_USTRINGPARAM( "IsFullScreen" ) ) ) >>= bTemp;
        if( !bTemp )
        {
            AddAttributeASCII(XML_NAMESPACE_PRESENTATION, sXML_full_screen, sXML_false );
            bHasAttr = sal_True;
        }

        xPresProps->getPropertyValue( OUString( RTL_CONSTASCII_USTRINGPARAM( "IsMouseVisible" ) ) ) >>= bTemp;
        if( !bTemp )
        {
            AddAttributeASCII(XML_NAMESPACE_PRESENTATION, sXML_mouse_visible, sXML_false );
            bHasAttr = sal_True;
        }

        xPresProps->getPropertyValue( OUString( RTL_CONSTASCII_USTRINGPARAM( "StartWithNavigator" ) ) ) >>= bTemp;
        if( bTemp )
        {
            AddAttributeASCII(XML_NAMESPACE_PRESENTATION, sXML_start_with_navigator, sXML_true );
            bHasAttr = sal_True;
        }

        xPresProps->getPropertyValue( OUString( RTL_CONSTASCII_USTRINGPARAM( "UsePen" ) ) ) >>= bTemp;
        if( bTemp )
        {
            AddAttributeASCII(XML_NAMESPACE_PRESENTATION, sXML_mouse_as_pen, sXML_true );
            bHasAttr = sal_True;
        }

        xPresProps->getPropertyValue( OUString( RTL_CONSTASCII_USTRINGPARAM( "IsTransitionOnClick" ) ) ) >>= bTemp;
        if( !bTemp )
        {
            AddAttributeASCII(XML_NAMESPACE_PRESENTATION, sXML_transition_on_click, sXML_disabled );
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
            SvXMLElementExport aSettings(*this, XML_NAMESPACE_PRESENTATION, sXML_settings, sal_True, sal_True);

            if( nShowCount == 0 )
                return;

            Reference< XIndexContainer > xShow;
            Reference< XNamed > xPageName;

            OUStringBuffer sTmp;

            for( sal_Int32 nIndex = 0; nIndex < nShowCount; nIndex++, pShowNames++ )
            {
                AddAttribute(XML_NAMESPACE_PRESENTATION, sXML_name, *pShowNames );

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
                    AddAttribute(XML_NAMESPACE_PRESENTATION, sXML_pages, sTmp.makeStringAndClear() );

                SvXMLElementExport aShows(*this, XML_NAMESPACE_PRESENTATION, sXML_show, sal_True, sal_True);
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

    // prepare and write default-draw-style-pool
    ImpWriteDefaultStyleInfos();

    // write draw:style-name for object graphic-styles
    ImpWriteObjGraphicStyleInfos();

    // write presentation styles
    ImpWritePresentationStyles();

    // prepare draw:auto-layout-name for page export
    ImpPrepAutoLayoutInfos();

    // write draw:auto-layout-name for page export
    ImpWriteAutoLayoutInfos();
}

//////////////////////////////////////////////////////////////////////////////

void SdXMLExport::_ExportAutoStyles()
{
    GetPropertySetMapper()->SetAutoStyles( sal_True );

    // #80012# PageMaster export moved from _ExportStyles
    // prepare page-master infos
    ImpPrepPageMasterInfos();

    // write page-master infos
    ImpWritePageMasterInfos();

    // prepare draw:style-name for master page export
    ImpPrepMasterPageInfos();

    // prepare draw:style-name for page export
    ImpPrepDrawPageInfos();

    // export draw-page styles
    GetAutoStylePool()->exportXML(
        XML_STYLE_FAMILY_SD_DRAWINGPAGE_ID,
        GetDocHandler(),
        GetMM100UnitConverter(),
        GetNamespaceMap());

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

    GetShapeExport()->exportAutoStyles();

    // ...for text
    GetTextParagraphExport()->exportTextAutoStyles();

    // ...for chart
    GetChartExport()->exportAutoStyles();
}

//////////////////////////////////////////////////////////////////////////////

void SdXMLExport::_ExportMasterStyles()
{
    // export layer
    SdXMLayerExporter::exportLayer( *this );

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
                AddAttribute(XML_NAMESPACE_STYLE, sXML_name, sMasterPageName);
            }

            ImpXMLEXPPageMasterInfo* pInfo = mpPageMaterUsageList->GetObject(nMPageId);
            if(pInfo)
            {
                OUString sString = pInfo->GetName();
                AddAttribute(XML_NAMESPACE_STYLE, sXML_page_master_name, sString);
            }

            // draw:style-name (background attributes)
            if( maMasterPagesStyleNames[nMPageId].getLength() )
                AddAttribute(XML_NAMESPACE_DRAW, sXML_style_name, maMasterPagesStyleNames[nMPageId]);

            // write masterpage
            SvXMLElementExport aMPG(*this, XML_NAMESPACE_STYLE, sXML_master_page, sal_True, sal_True);

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
                            // write presentation notes
                            SvXMLElementExport aPSY(*this, XML_NAMESPACE_PRESENTATION, sXML_notes, sal_True, sal_True);

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
                SvXMLElementExport aForms(*this, XML_NAMESPACE_OFFICE, sXML_forms, sal_True, sal_True);
                GetFormExport()->exportForms( xDrawPage );
            }
        }

        sal_Bool bRet = GetFormExport()->seekPage( xDrawPage );
        DBG_ASSERT( bRet, "OFormLayerXMLExport::seekPage failed!" );
    }
}
//////////////////////////////////////////////////////////////////////////////

uno::Sequence< OUString > SAL_CALL SdImpressXMLExport_getSupportedServiceNames() throw()
{
    const OUString aServiceName( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.office.sax.exporter.Impress" ) );
    const uno::Sequence< OUString > aSeq( &aServiceName, 1 );
    return aSeq;
}

OUString SAL_CALL SdImpressXMLExport_getImplementationName() throw()
{
    return OUString( RTL_CONSTASCII_USTRINGPARAM( "SdXMLExport.Impress" ) );
}

uno::Reference< uno::XInterface > SAL_CALL SdImpressXMLExport_createInstance(const uno::Reference< lang::XMultiServiceFactory > & rSMgr) throw( uno::Exception )
{
    return (cppu::OWeakObject*)new SdXMLExport( sal_False );
}

uno::Sequence< OUString > SAL_CALL SdDrawXMLExport_getSupportedServiceNames() throw()
{
    const OUString aServiceName( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.office.sax.exporter.Draw" ) );
    const uno::Sequence< OUString > aSeq( &aServiceName, 1 );
    return aSeq;
}

OUString SAL_CALL SdDrawXMLExport_getImplementationName() throw()
{
    return OUString( RTL_CONSTASCII_USTRINGPARAM( "SdXMLExport.Draw" ) );
}

uno::Reference< uno::XInterface > SAL_CALL SdDrawXMLExport_createInstance(const uno::Reference< lang::XMultiServiceFactory > & rSMgr) throw( uno::Exception )
{
    return (cppu::OWeakObject*)new SdXMLExport( sal_True );
}
