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

#ifndef _SDXMLEXP_IMPL_HXX
#define _SDXMLEXP_IMPL_HXX

#include <xmloff/xmlexp.hxx>

#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/task/XStatusIndicator.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/drawing/XDrawPage.hpp>
#include <comphelper/stl_types.hxx>

#include <vector>

class Rectangle;

class ImpXMLEXPPageMasterInfo;
class ImpXMLAutoLayoutInfo;
class XMLSdPropHdlFactory;
class XMLShapeExportPropertyMapper;
class XMLPageExportPropertyMapper;

typedef ::std::vector< ImpXMLEXPPageMasterInfo* > ImpXMLEXPPageMasterList;
typedef ::std::vector< ImpXMLAutoLayoutInfo*    > ImpXMLAutoLayoutInfoList;

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
    XmlPlaceholderHandout,
    XmlPlaceholderVerticalTitle,
    XmlPlaceholderVerticalOutline
};

DECLARE_STL_STDKEY_SET( sal_Int32, SdXMLFormatMap );

struct HeaderFooterPageSettingsImpl
{
    OUString maStrHeaderDeclName;
    OUString maStrFooterDeclName;
    OUString maStrDateTimeDeclName;
};

struct DateTimeDeclImpl
{
    OUString maStrText;
    sal_Bool mbFixed;
    sal_Int32 mnFormat;
};

class SdXMLExport : public SvXMLExport
{
    com::sun::star::uno::Reference< com::sun::star::container::XNameAccess > mxDocStyleFamilies;
    com::sun::star::uno::Reference< com::sun::star::container::XIndexAccess > mxDocMasterPages;
    com::sun::star::uno::Reference< com::sun::star::container::XIndexAccess > mxDocDrawPages;
    sal_Int32                   mnDocMasterPageCount;
    sal_Int32                   mnDocDrawPageCount;
    sal_uInt32                  mnShapeStyleInfoIndex;
    sal_uInt32                  mnObjectCount;

    // temporary infos
    ImpXMLEXPPageMasterList*    mpPageMasterInfoList;
    ImpXMLEXPPageMasterList*    mpPageMasterUsageList;
    ImpXMLEXPPageMasterList*    mpNotesPageMasterUsageList;
    ImpXMLEXPPageMasterInfo*    mpHandoutPageMaster;
    ImpXMLAutoLayoutInfoList*   mpAutoLayoutInfoList;

    com::sun::star::uno::Sequence< OUString > maDrawPagesAutoLayoutNames;

    ::std::vector< OUString >        maDrawPagesStyleNames;
    ::std::vector< OUString >        maDrawNotesPagesStyleNames;
    ::std::vector< OUString >        maMasterPagesStyleNames;
    OUString                         maHandoutMasterStyleName;
    ::std::vector< HeaderFooterPageSettingsImpl >   maDrawPagesHeaderFooterSettings;
    ::std::vector< HeaderFooterPageSettingsImpl >   maDrawNotesPagesHeaderFooterSettings;

    ::std::vector< OUString >        maHeaderDeclsVector;
    ::std::vector< OUString >        maFooterDeclsVector;
    ::std::vector< DateTimeDeclImpl >       maDateTimeDeclsVector;

    HeaderFooterPageSettingsImpl            maHandoutPageHeaderFooterSettings;

    XMLSdPropHdlFactory*                mpSdPropHdlFactory;
    XMLShapeExportPropertyMapper*       mpPropertySetMapper;
    XMLPageExportPropertyMapper*        mpPresPagePropsMapper;

    SdXMLFormatMap  maUsedDateStyles;           // this is a vector with the used formatings for date fields
    SdXMLFormatMap  maUsedTimeStyles;           // this is a vector with the used formatings for time fields

    sal_Bool                    mbIsDraw;
    sal_Bool                    mbFamilyGraphicUsed;
    sal_Bool                    mbFamilyPresentationUsed;

    const OUString         msZIndex;
    const OUString         msEmptyPres;
    const OUString         msModel;
    const OUString         msStartShape;
    const OUString         msEndShape;
    const OUString         msPageLayoutNames;

    virtual void _ExportStyles(sal_Bool bUsed);
    virtual void _ExportAutoStyles();
    virtual void _ExportFontDecls();
    virtual void _ExportMasterStyles();
    virtual void _ExportContent();
    // #82003#
    virtual void _ExportMeta();

    ImpXMLEXPPageMasterInfo* ImpGetOrCreatePageMasterInfo( com::sun::star::uno::Reference< com::sun::star::drawing::XDrawPage > xMasterPage );
    void ImpPrepPageMasterInfos();
    void ImpPrepDrawMasterInfos();
    void ImpWritePageMasterInfos();
    void ImpPrepAutoLayoutInfos();
    HeaderFooterPageSettingsImpl ImpPrepDrawPageHeaderFooterDecls( const com::sun::star::uno::Reference< com::sun::star::drawing::XDrawPage >& xDrawPage );
    ImpXMLEXPPageMasterInfo* ImpGetPageMasterInfoByName(const OUString& rName);

    void ImpPrepDrawPageInfos();
    void ImpPrepMasterPageInfos();
    void ImpWritePresentationStyles();
    OUString ImpCreatePresPageStyleName( com::sun::star::uno::Reference<com::sun::star::drawing::XDrawPage> xDrawPage, bool bExportBackground = true );

    sal_Bool ImpPrepAutoLayoutInfo(const com::sun::star::uno::Reference< com::sun::star::drawing::XDrawPage >& xPage, OUString& rName);
    void ImpWriteAutoLayoutInfos();
    void ImpWriteAutoLayoutPlaceholder(XmlPlaceholder ePl, const Rectangle& rRect);
    void ImpWriteHeaderFooterDecls();
    void ImplExportHeaderFooterDeclAttributes( const HeaderFooterPageSettingsImpl& aSettings );

    void exportFormsElement( com::sun::star::uno::Reference< com::sun::star::drawing::XDrawPage > xDrawPage );
    void exportPresentationSettings();

    // #82003# helper function for recursive object count
    sal_uInt32 ImpRecursiveObjectCount( com::sun::star::uno::Reference< com::sun::star::drawing::XShapes > xShapes);

    OUString getNavigationOrder( const ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XDrawPage >& xDrawPage );

    void collectAnnotationAutoStyles( const ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XDrawPage >& xDrawPage );
    void exportAnnotations( const ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XDrawPage >& xDrawPage );

protected:
    virtual void GetViewSettings(com::sun::star::uno::Sequence<com::sun::star::beans::PropertyValue>& aProps);
    virtual void GetConfigurationSettings(com::sun::star::uno::Sequence<com::sun::star::beans::PropertyValue>& aProps);
    virtual XMLFontAutoStylePool* CreateFontAutoStylePool();

public:
    SdXMLExport(
        const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >& xContext,
        sal_Bool bIsDraw, sal_uInt16 nExportFlags = EXPORT_ALL );
    virtual ~SdXMLExport();

    void SetProgress(sal_Int32 nProg);

    // XExporter
    virtual void SAL_CALL setSourceDocument( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XComponent >& xDoc ) throw(::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException);

    // get factories and mappers
    XMLSdPropHdlFactory* GetSdPropHdlFactory() const { return mpSdPropHdlFactory; }
    XMLShapeExportPropertyMapper* GetPropertySetMapper() const { return mpPropertySetMapper; }
    XMLPageExportPropertyMapper* GetPresPagePropsMapper() const { return mpPresPagePropsMapper; }

    sal_Bool IsDraw() const { return mbIsDraw; }
    sal_Bool IsImpress() const { return !mbIsDraw; }

    sal_Bool IsFamilyGraphicUsed() const { return mbFamilyGraphicUsed; }
    void SetFamilyGraphicUsed() { mbFamilyGraphicUsed = sal_True; }
    sal_Bool IsFamilyPresentationUsed() const { return mbFamilyPresentationUsed; }
    void SetFamilyPresentationUsed() { mbFamilyPresentationUsed = sal_True; }

    virtual void addDataStyle(const sal_Int32 nNumberFormat, sal_Bool bTimeFormat = sal_False );
    virtual void exportDataStyles();
    virtual void exportAutoDataStyles();
    virtual OUString getDataStyleName(const sal_Int32 nNumberFormat, sal_Bool bTimeFormat = sal_False ) const;

    // XServiceInfo ( : SvXMLExport )
    virtual OUString SAL_CALL getImplementationName() throw( ::com::sun::star::uno::RuntimeException );
};

#endif  //  _SDXMLEXP_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
