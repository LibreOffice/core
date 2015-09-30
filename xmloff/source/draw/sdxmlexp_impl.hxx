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

#ifndef INCLUDED_XMLOFF_SOURCE_DRAW_SDXMLEXP_IMPL_HXX
#define INCLUDED_XMLOFF_SOURCE_DRAW_SDXMLEXP_IMPL_HXX

#include <xmloff/xmlexp.hxx>

#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/task/XStatusIndicator.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/drawing/XDrawPage.hpp>

#include <set>
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

typedef std::set<sal_Int32> SdXMLFormatMap;

struct HeaderFooterPageSettingsImpl
{
    OUString maStrHeaderDeclName;
    OUString maStrFooterDeclName;
    OUString maStrDateTimeDeclName;
};

struct DateTimeDeclImpl
{
    OUString maStrText;
    bool mbFixed;
    sal_Int32 mnFormat;
};

class SdXMLExport : public SvXMLExport
{
    com::sun::star::uno::Reference< com::sun::star::container::XNameAccess > mxDocStyleFamilies;
    com::sun::star::uno::Reference< com::sun::star::container::XIndexAccess > mxDocMasterPages;
    com::sun::star::uno::Reference< com::sun::star::container::XIndexAccess > mxDocDrawPages;
    sal_Int32                   mnDocMasterPageCount;
    sal_Int32                   mnDocDrawPageCount;
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

    bool                    mbIsDraw;

    const OUString         msPageLayoutNames;

    virtual void _ExportStyles(bool bUsed) SAL_OVERRIDE;
    virtual void _ExportAutoStyles() SAL_OVERRIDE;
    virtual void _ExportFontDecls() SAL_OVERRIDE;
    virtual void _ExportMasterStyles() SAL_OVERRIDE;
    virtual void _ExportContent() SAL_OVERRIDE;
    virtual void _ExportMeta() SAL_OVERRIDE;

    ImpXMLEXPPageMasterInfo* ImpGetOrCreatePageMasterInfo( com::sun::star::uno::Reference< com::sun::star::drawing::XDrawPage > xMasterPage );
    void ImpPrepPageMasterInfos();
    void ImpWritePageMasterInfos();
    void ImpPrepAutoLayoutInfos();
    HeaderFooterPageSettingsImpl ImpPrepDrawPageHeaderFooterDecls( const com::sun::star::uno::Reference< com::sun::star::drawing::XDrawPage >& xDrawPage );
    ImpXMLEXPPageMasterInfo* ImpGetPageMasterInfoByName(const OUString& rName);

    void ImpPrepDrawPageInfos();
    void ImpPrepMasterPageInfos();
    void ImpWritePresentationStyles();
    OUString ImpCreatePresPageStyleName( com::sun::star::uno::Reference<com::sun::star::drawing::XDrawPage> xDrawPage, bool bExportBackground = true );

    bool ImpPrepAutoLayoutInfo(const com::sun::star::uno::Reference< com::sun::star::drawing::XDrawPage >& xPage, OUString& rName);
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
    virtual void GetViewSettings(com::sun::star::uno::Sequence<com::sun::star::beans::PropertyValue>& aProps) SAL_OVERRIDE;
    virtual void GetConfigurationSettings(com::sun::star::uno::Sequence<com::sun::star::beans::PropertyValue>& aProps) SAL_OVERRIDE;
    virtual XMLFontAutoStylePool* CreateFontAutoStylePool() SAL_OVERRIDE;

public:
    SdXMLExport(
        const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >& xContext,
        OUString const & implementationName,
        bool bIsDraw, SvXMLExportFlags nExportFlags = SvXMLExportFlags::ALL );
    virtual ~SdXMLExport();

    void SetProgress(sal_Int32 nProg);

    // XExporter
    virtual void SAL_CALL setSourceDocument( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XComponent >& xDoc ) throw(::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // get factories and mappers
    XMLShapeExportPropertyMapper* GetPropertySetMapper() const { return mpPropertySetMapper; }
    XMLPageExportPropertyMapper* GetPresPagePropsMapper() const { return mpPresPagePropsMapper; }

    bool IsDraw() const { return mbIsDraw; }
    bool IsImpress() const { return !mbIsDraw; }

    virtual void addDataStyle(const sal_Int32 nNumberFormat, bool bTimeFormat = false ) SAL_OVERRIDE;
    virtual void exportDataStyles() SAL_OVERRIDE;
    virtual void exportAutoDataStyles() SAL_OVERRIDE;
    virtual OUString getDataStyleName(const sal_Int32 nNumberFormat, bool bTimeFormat = false ) const SAL_OVERRIDE;
};

#endif  //  _SDXMLEXP_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
