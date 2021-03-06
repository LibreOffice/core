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

#pragma once

#include <xmloff/xmlexp.hxx>

#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/drawing/XDrawPage.hpp>

#include <o3tl/sorted_vector.hxx>
#include <vector>

namespace tools { class Rectangle; }

class ImpXMLEXPPageMasterInfo;
class ImpXMLAutoLayoutInfo;
class XMLSdPropHdlFactory;
class XMLShapeExportPropertyMapper;
class XMLPageExportPropertyMapper;

typedef ::std::vector< ImpXMLEXPPageMasterInfo* > ImpXMLEXPPageMasterList;

enum XmlPlaceholder
{
    XmlPlaceholderTitle,
    XmlPlaceholderOutline,
    XmlPlaceholderSubtitle,
    XmlPlaceholderGraphic,
    XmlPlaceholderObject,
    XmlPlaceholderChart,
    XmlPlaceholderTable,
    XmlPlaceholderPage,
    XmlPlaceholderNotes,
    XmlPlaceholderHandout,
    XmlPlaceholderVerticalTitle,
    XmlPlaceholderVerticalOutline
};

typedef o3tl::sorted_vector<sal_Int32> SdXMLFormatMap;

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
    css::uno::Reference< css::container::XNameAccess > mxDocStyleFamilies;
    css::uno::Reference< css::container::XIndexAccess > mxDocMasterPages;
    css::uno::Reference< css::container::XIndexAccess > mxDocDrawPages;
    sal_Int32                   mnDocMasterPageCount;
    sal_Int32                   mnDocDrawPageCount;
    sal_uInt32                  mnObjectCount;

    // temporary infos
    std::vector< std::unique_ptr<ImpXMLEXPPageMasterInfo> > mvPageMasterInfoList;
    ImpXMLEXPPageMasterList     mvPageMasterUsageList;
    ImpXMLEXPPageMasterList     mvNotesPageMasterUsageList;
    ImpXMLEXPPageMasterInfo*    mpHandoutPageMaster;
    std::vector< std::unique_ptr<ImpXMLAutoLayoutInfo> >    mvAutoLayoutInfoList;

    css::uno::Sequence< OUString > maDrawPagesAutoLayoutNames;

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

    rtl::Reference<XMLSdPropHdlFactory>          mpSdPropHdlFactory;
    rtl::Reference<XMLShapeExportPropertyMapper> mpPropertySetMapper;
    rtl::Reference<XMLPageExportPropertyMapper>  mpPresPagePropsMapper;

    SdXMLFormatMap  maUsedDateStyles;           // this is a vector with the used formattings for date fields
    SdXMLFormatMap  maUsedTimeStyles;           // this is a vector with the used formattings for time fields

    bool            mbIsDraw;

    virtual void ExportStyles_(bool bUsed) override;
    virtual void ExportAutoStyles_() override;
    virtual void ExportFontDecls_() override;
    virtual void ExportMasterStyles_() override;
    virtual void ExportContent_() override;
    virtual void ExportMeta_() override;

    ImpXMLEXPPageMasterInfo* ImpGetOrCreatePageMasterInfo( const css::uno::Reference< css::drawing::XDrawPage >& xMasterPage );
    void ImpPrepPageMasterInfos();
    void ImpWritePageMasterInfos();
    void ImpPrepAutoLayoutInfos();
    HeaderFooterPageSettingsImpl ImpPrepDrawPageHeaderFooterDecls( const css::uno::Reference< css::drawing::XDrawPage >& xDrawPage );
    ImpXMLEXPPageMasterInfo* ImpGetPageMasterInfoByName(std::u16string_view rName);

    void ImpPrepDrawPageInfos();
    void ImpPrepMasterPageInfos();
    void ImpWritePresentationStyles();
    OUString ImpCreatePresPageStyleName( const css::uno::Reference<css::drawing::XDrawPage>& xDrawPage, bool bExportBackground = true );

    bool ImpPrepAutoLayoutInfo(const css::uno::Reference< css::drawing::XDrawPage >& xPage, OUString& rName);
    void ImpWriteAutoLayoutInfos();
    void ImpWriteAutoLayoutPlaceholder(XmlPlaceholder ePl, const tools::Rectangle& rRect);
    void ImpWriteHeaderFooterDecls();
    void ImplExportHeaderFooterDeclAttributes( const HeaderFooterPageSettingsImpl& aSettings );

    void exportFormsElement( const css::uno::Reference< css::drawing::XDrawPage >& xDrawPage );
    void exportPresentationSettings();

    // #82003# helper function for recursive object count
    sal_uInt32 ImpRecursiveObjectCount( const css::uno::Reference< css::drawing::XShapes >& xShapes);

    OUString getNavigationOrder( const css::uno::Reference< css::drawing::XDrawPage >& xDrawPage );

    void collectAnnotationAutoStyles( const css::uno::Reference< css::drawing::XDrawPage >& xDrawPage );
    void exportAnnotations( const css::uno::Reference< css::drawing::XDrawPage >& xDrawPage );

protected:
    virtual void GetViewSettings(css::uno::Sequence<css::beans::PropertyValue>& aProps) override;
    virtual void GetConfigurationSettings(css::uno::Sequence<css::beans::PropertyValue>& aProps) override;
    virtual XMLFontAutoStylePool* CreateFontAutoStylePool() override;

public:
    SdXMLExport(
        const css::uno::Reference< css::uno::XComponentContext >& xContext,
        OUString const & implementationName,
        bool bIsDraw, SvXMLExportFlags nExportFlags );
    virtual ~SdXMLExport() override;

    void collectAutoStyles() override;

    // XExporter
    virtual void SAL_CALL setSourceDocument( const css::uno::Reference< css::lang::XComponent >& xDoc ) override;

    // get factories and mappers
    XMLShapeExportPropertyMapper* GetPropertySetMapper() const { return mpPropertySetMapper.get(); }
    XMLPageExportPropertyMapper* GetPresPagePropsMapper() const { return mpPresPagePropsMapper.get(); }

    bool IsDraw() const { return mbIsDraw; }
    bool IsImpress() const { return !mbIsDraw; }

    virtual void addDataStyle(const sal_Int32 nNumberFormat, bool bTimeFormat = false ) override;
    virtual void exportDataStyles() override;
    virtual void exportAutoDataStyles() override;
    virtual OUString getDataStyleName(const sal_Int32 nNumberFormat, bool bTimeFormat = false ) const override;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
