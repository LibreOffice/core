/*************************************************************************
 *
 *  $RCSfile: sdxmlexp_impl.hxx,v $
 *
 *  $Revision: 1.12 $
 *
 *  last change: $Author: cl $ $Date: 2001-03-04 23:07:53 $
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

#ifndef _SDXMLEXP_IMPL_HXX
#define _SDXMLEXP_IMPL_HXX

#ifndef _XMLOFF_XMLEXP_HXX
#include "xmlexp.hxx"
#endif

#ifndef _XMLOFF_XMLITMAP_HXX
#include "xmlitmap.hxx"
#endif

#ifndef _COM_SUN_STAR_FRAME_XMODEL_HPP_
#include <com/sun/star/frame/XModel.hpp>
#endif

#ifndef _COM_SUN_STAR_TASK_XSTATUSINDICATOR_HPP_
#include <com/sun/star/task/XStatusIndicator.hpp>
#endif

#ifndef _COM_SUN_STAR_CONTAINER_XNAMEACCESS_HPP_
#include <com/sun/star/container/XNameAccess.hpp>
#endif

#ifndef _COM_SUN_STAR_DRAWING_XDRAWPAGE_HPP_
#include <com/sun/star/drawing/XDrawPage.hpp>
#endif

//////////////////////////////////////////////////////////////////////////////

class SvXMLUnitConverter;
class SvXMLExportItemMapper;
class SfxPoolItem;
class SfxItemSet;
class OUStrings_Impl;
class OUStringsSort_Impl;
class Rectangle;

class ImpPresPageDrawStylePropMapper;
class ImpXMLEXPPageMasterList;
class ImpXMLEXPPageMasterInfo;
class ImpXMLDrawPageInfoList;
class ImpXMLAutoLayoutInfoList;
class SvXMLAutoStylePoolP;
class XMLSdPropHdlFactory;
class ImpXMLShapeStyleInfo;
class XMLShapeExportPropertyMapper;
class XMLPageExportPropertyMapper;

//////////////////////////////////////////////////////////////////////////////

enum XmlPlaceholder
{
    XmlPlaceholderTitle,
    XmlPlaceholderOutline,
    XmlPlaceholderSubtitle,
    XmlPlaceholderText,
    XmlPlaceholderGraphic,
    XmlPlaceholderObject,
    XmlPlaceholderChart,
    XmlPlaceholderOrgchart,
    XmlPlaceholderTable,
    XmlPlaceholderPage,
    XmlPlaceholderNotes,
    XmlPlaceholderHandout
};

//////////////////////////////////////////////////////////////////////////////

class SdXMLExport : public SvXMLExport
{
    com::sun::star::uno::Reference< com::sun::star::container::XNameAccess > mxDocStyleFamilies;
    com::sun::star::uno::Reference< com::sun::star::container::XIndexAccess > mxDocMasterPages;
    com::sun::star::uno::Reference< com::sun::star::container::XIndexAccess > mxDocDrawPages;
    sal_Int32                   mnDocMasterPageCount;
    sal_Int32                   mnDocDrawPageCount;
    sal_uInt32                  mnShapeStyleInfoIndex;

    // temporary infos
    ImpXMLEXPPageMasterList*    mpPageMasterInfoList;
    ImpXMLEXPPageMasterList*    mpPageMaterUsageList;
    ImpXMLAutoLayoutInfoList*   mpAutoLayoutInfoList;

    ::std::vector< ::rtl::OUString >        maDrawPagesAutoLayoutNames;
    ::std::vector< ::rtl::OUString >        maDrawPagesStyleNames;
    ::std::vector< ::rtl::OUString >        maMasterPagesStyleNames;

    XMLSdPropHdlFactory*                mpSdPropHdlFactory;
    XMLShapeExportPropertyMapper*       mpPropertySetMapper;
    XMLPageExportPropertyMapper*        mpPresPagePropsMapper;

    sal_Bool                    mbIsDraw;
    sal_Bool                    mbFamilyGraphicUsed;
    sal_Bool                    mbFamilyPresentationUsed;

    const rtl::OUString         msZIndex;
    const rtl::OUString         msEmptyPres;
    const rtl::OUString         msModel;
    const rtl::OUString         msStartShape;
    const rtl::OUString         msEndShape;

    virtual void _ExportViewSettings();
    virtual void _ExportStyles(BOOL bUsed);
    virtual void _ExportAutoStyles();
    virtual void _ExportMasterStyles();
    virtual void _ExportContent();

    void ImpPrepPageMasterInfos();
    void ImpPrepDrawMasterInfos();
    void ImpWritePageMasterInfos();
    void ImpPrepAutoLayoutInfos();

    ImpXMLEXPPageMasterInfo* ImpGetPageMasterInfoByName(const rtl::OUString& rName);

    void ImpPrepDrawPageInfos();
    void ImpPrepMasterPageInfos();
    void ImpWritePresentationStyles();

    BOOL ImpPrepAutoLayoutInfo(const com::sun::star::uno::Reference< com::sun::star::drawing::XDrawPage >& xPage, rtl::OUString& rName);
    void ImpWriteAutoLayoutInfos();
    void ImpWriteAutoLayoutPlaceholder(XmlPlaceholder ePl, const Rectangle& rRect);

    void ImpWriteObjGraphicStyleInfos();

    void exportFormsElement( com::sun::star::uno::Reference< com::sun::star::drawing::XDrawPage > xDrawPage );
    void exportPresentationSettings();

public:
    SdXMLExport( sal_Bool bIsDraw, sal_uInt16 nExportFlags = EXPORT_ALL );
    virtual ~SdXMLExport();

    void SetProgress(sal_Int32 nProg);

    // XExporter
    virtual void SAL_CALL setSourceDocument( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XComponent >& xDoc ) throw(::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException);

    // get factories and mappers
    XMLSdPropHdlFactory* GetSdPropHdlFactory() const { return mpSdPropHdlFactory; }
    XMLShapeExportPropertyMapper* GetPropertySetMapper() const { return mpPropertySetMapper; }
    XMLPageExportPropertyMapper* GetPresPagePropsMapper() const { return mpPresPagePropsMapper; }

    BOOL IsDraw() const { return mbIsDraw; }
    BOOL IsImpress() const { return !mbIsDraw; }

    BOOL IsFamilyGraphicUsed() const { return mbFamilyGraphicUsed; }
    void SetFamilyGraphicUsed() { mbFamilyGraphicUsed = TRUE; }
    BOOL IsFamilyPresentationUsed() const { return mbFamilyPresentationUsed; }
    void SetFamilyPresentationUsed() { mbFamilyPresentationUsed = TRUE; }
};

#endif  //  _SDXMLEXP_HXX

