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

#ifndef INCLUDED_XMLOFF_SOURCE_DRAW_SDXMLIMP_IMPL_HXX
#define INCLUDED_XMLOFF_SOURCE_DRAW_SDXMLIMP_IMPL_HXX

#include <com/sun/star/drawing/XDrawPage.hpp>
#include <com/sun/star/task/XStatusIndicator.hpp>
#include <xmloff/xmltkmap.hxx>
#include <com/sun/star/container/XNameAccess.hpp>

#include <map>
#include <memory>
#include <vector>
#include <xmloff/xmlimp.hxx>

enum SdXMLDocElemTokenMap
{
    XML_TOK_DOC_FONTDECLS,
    XML_TOK_DOC_STYLES,
    XML_TOK_DOC_AUTOSTYLES,
    XML_TOK_DOC_MASTERSTYLES,
    XML_TOK_DOC_META,
    XML_TOK_DOC_BODY,
    XML_TOK_DOC_SCRIPT,
    XML_TOK_DOC_SETTINGS,
};

enum SdXMLBodyElemTokenMap
{
    XML_TOK_BODY_PAGE,
    XML_TOK_BODY_SETTINGS,
    XML_TOK_BODY_HEADER_DECL,
    XML_TOK_BODY_FOOTER_DECL,
    XML_TOK_BODY_DATE_TIME_DECL
};

enum SdXMLStylesElemTokenMap
{
    XML_TOK_STYLES_STYLE,
    XML_TOK_STYLES_PAGE_MASTER,
    XML_TOK_STYLES_PRESENTATION_PAGE_LAYOUT
};

enum SdXMLMasterPageElemTokenMap
{
    XML_TOK_MASTERPAGE_STYLE,
    XML_TOK_MASTERPAGE_NOTES
};

enum SdXMLMasterPageAttrTokenMap
{
    XML_TOK_MASTERPAGE_NAME,
    XML_TOK_MASTERPAGE_DISPLAY_NAME,
    XML_TOK_MASTERPAGE_PAGE_MASTER_NAME,
    XML_TOK_MASTERPAGE_STYLE_NAME,
    XML_TOK_MASTERPAGE_PAGE_LAYOUT_NAME,
    XML_TOK_MASTERPAGE_USE_HEADER_NAME,
    XML_TOK_MASTERPAGE_USE_FOOTER_NAME,
    XML_TOK_MASTERPAGE_USE_DATE_TIME_NAME
};

enum SdXMLPageMasterAttrTokenMap
{
    XML_TOK_PAGEMASTER_NAME
};

enum SdXMLPageMasterStyleAttrTokenMap
{
    XML_TOK_PAGEMASTERSTYLE_MARGIN_TOP,
    XML_TOK_PAGEMASTERSTYLE_MARGIN_BOTTOM,
    XML_TOK_PAGEMASTERSTYLE_MARGIN_LEFT,
    XML_TOK_PAGEMASTERSTYLE_MARGIN_RIGHT,
    XML_TOK_PAGEMASTERSTYLE_PAGE_WIDTH,
    XML_TOK_PAGEMASTERSTYLE_PAGE_HEIGHT,
    XML_TOK_PAGEMASTERSTYLE_PAGE_ORIENTATION
};

enum SdXMLDrawPageAttrTokenMap
{
    XML_TOK_DRAWPAGE_NAME,
    XML_TOK_DRAWPAGE_STYLE_NAME,
    XML_TOK_DRAWPAGE_MASTER_PAGE_NAME,
    XML_TOK_DRAWPAGE_PAGE_LAYOUT_NAME,
    XML_TOK_DRAWPAGE_DRAWID,
    XML_TOK_DRAWPAGE_XMLID,
    XML_TOK_DRAWPAGE_HREF,
    XML_TOK_DRAWPAGE_USE_HEADER_NAME,
    XML_TOK_DRAWPAGE_USE_FOOTER_NAME,
    XML_TOK_DRAWPAGE_USE_DATE_TIME_NAME
};

enum SdXMLDrawPageElemTokenMap
{
    XML_TOK_DRAWPAGE_NOTES,
    XML_TOK_DRAWPAGE_PAR,
    XML_TOK_DRAWPAGE_SEQ,
    XML_TOK_DRAWPAGE_LAYER_SET
};

enum SdXMLPresentationPlaceholderAttrTokenMap
{
    XML_TOK_PRESENTATIONPLACEHOLDER_OBJECTNAME,
    XML_TOK_PRESENTATIONPLACEHOLDER_X,
    XML_TOK_PRESENTATIONPLACEHOLDER_Y,
    XML_TOK_PRESENTATIONPLACEHOLDER_WIDTH,
    XML_TOK_PRESENTATIONPLACEHOLDER_HEIGHT
};

class SvXMLUnitConverter;
class SvXMLTokenMap;
class SdXMLMasterStylesContext;

struct DateTimeDeclContextImpl
{
    OUString maStrText;
    bool mbFixed;
    OUString maStrDateTimeFormat;

    DateTimeDeclContextImpl() : mbFixed(true) {}
};

typedef std::map<OUString, OUString> HeaderFooterDeclMap;
typedef std::map<OUString, DateTimeDeclContextImpl> DateTimeDeclMap;

class SdXMLImport: public SvXMLImport
{
    css::uno::Reference< css::container::XNameAccess > mxDocStyleFamilies;
    css::uno::Reference< css::container::XIndexAccess > mxDocMasterPages;
    css::uno::Reference< css::container::XIndexAccess > mxDocDrawPages;
    css::uno::Reference< css::container::XNameAccess > mxPageLayouts;

    // contexts for Style and AutoStyle import
    rtl::Reference<SdXMLMasterStylesContext> mxMasterStylesContext;

    // token map lists
    std::unique_ptr<SvXMLTokenMap>              mpDocElemTokenMap;
    std::unique_ptr<SvXMLTokenMap>              mpBodyElemTokenMap;
    std::unique_ptr<SvXMLTokenMap>              mpStylesElemTokenMap;
    std::unique_ptr<SvXMLTokenMap>              mpMasterPageElemTokenMap;
    std::unique_ptr<SvXMLTokenMap>              mpMasterPageAttrTokenMap;
    std::unique_ptr<SvXMLTokenMap>              mpPageMasterAttrTokenMap;
    std::unique_ptr<SvXMLTokenMap>              mpPageMasterStyleAttrTokenMap;
    std::unique_ptr<SvXMLTokenMap>              mpDrawPageAttrTokenMap;
    std::unique_ptr<SvXMLTokenMap>              mpDrawPageElemTokenMap;
    std::unique_ptr<SvXMLTokenMap>              mpPresentationPlaceholderAttrTokenMap;

    sal_Int32                   mnNewPageCount;
    sal_Int32                   mnNewMasterPageCount;

    bool                        mbIsDraw;
    bool                        mbLoadDoc;
    bool                        mbPreview;

    static constexpr OUStringLiteral gsPageLayouts = "PageLayouts";
    static constexpr OUStringLiteral gsPreview = "Preview";

    HeaderFooterDeclMap         maHeaderDeclsMap;
    HeaderFooterDeclMap         maFooterDeclsMap;
    DateTimeDeclMap             maDateTimeDeclsMap;

protected:

    // This method is called after the namespace map has been updated, but
    // before a context for the current element has been pushed.
    virtual SvXMLImportContext *CreateFastContext( sal_Int32 nElement,
        const ::css::uno::Reference< ::css::xml::sax::XFastAttributeList >& xAttrList ) override;

public:
    SdXMLImport(
        const css::uno::Reference< css::uno::XComponentContext >& xContext,
        OUString const & implementationName,
        bool bIsDraw, SvXMLImportFlags nImportFlags );

    // XImporter
    virtual void SAL_CALL setTargetDocument( const css::uno::Reference< css::lang::XComponent >& xDoc ) override;

    // XInitialization
    virtual void SAL_CALL initialize( const css::uno::Sequence< css::uno::Any >& aArguments ) override;

    virtual void SetViewSettings(const css::uno::Sequence<css::beans::PropertyValue>& aViewProps) override;
    virtual void SetConfigurationSettings(const css::uno::Sequence<css::beans::PropertyValue>& aConfigProps) override;

    // namespace office
    // NB: in contrast to other CreateFooContexts, this particular one handles
    //     the root element (i.e. office:document-meta)
    SvXMLImportContext* CreateMetaContext(const sal_Int32 nElement,
        const css::uno::Reference< css::xml::sax::XFastAttributeList >& xAttrList);
    SvXMLImportContext* CreateScriptContext( const OUString& rLocalName );
    SvXMLImportContext* CreateBodyContext(const OUString& rLocalName,
        const css::uno::Reference< css::xml::sax::XAttributeList >& xAttrList);
    SvXMLStylesContext* CreateStylesContext(const OUString& rLocalName,
        const css::uno::Reference< css::xml::sax::XAttributeList >& xAttrList);
    SvXMLStylesContext* CreateAutoStylesContext(const OUString& rLocalName,
        const css::uno::Reference< css::xml::sax::XAttributeList >& xAttrList);
    SvXMLImportContext* CreateMasterStylesContext(const OUString& rLocalName,
        const css::uno::Reference< css::xml::sax::XAttributeList >& xAttrList);
    SvXMLImportContext *CreateFontDeclsContext(const OUString& rLocalName,
        const css::uno::Reference< css::xml::sax::XAttributeList > & xAttrList );

    // Styles and AutoStyles contexts

    const SvXMLTokenMap& GetDocElemTokenMap();
    const SvXMLTokenMap& GetBodyElemTokenMap();
    const SvXMLTokenMap& GetStylesElemTokenMap();
    const SvXMLTokenMap& GetMasterPageElemTokenMap();
    const SvXMLTokenMap& GetMasterPageAttrTokenMap();
    const SvXMLTokenMap& GetPageMasterAttrTokenMap();
    const SvXMLTokenMap& GetPageMasterStyleAttrTokenMap();
    const SvXMLTokenMap& GetDrawPageAttrTokenMap();
    const SvXMLTokenMap& GetDrawPageElemTokenMap();
    const SvXMLTokenMap& GetPresentationPlaceholderAttrTokenMap();

    // export local parameters concerning page access and similar
    const css::uno::Reference< css::container::XNameAccess >& GetLocalDocStyleFamilies() const { return mxDocStyleFamilies; }
    const css::uno::Reference< css::container::XIndexAccess >& GetLocalMasterPages() const { return mxDocMasterPages; }
    const css::uno::Reference< css::container::XIndexAccess >& GetLocalDrawPages() const { return mxDocDrawPages; }

    sal_Int32 GetNewPageCount() const { return mnNewPageCount; }
    void IncrementNewPageCount() { mnNewPageCount++; }
    sal_Int32 GetNewMasterPageCount() const { return mnNewMasterPageCount; }
    void IncrementNewMasterPageCount() { mnNewMasterPageCount++; }

    const css::uno::Reference< css::container::XNameAccess >& getPageLayouts() const { return mxPageLayouts; }

    bool IsDraw() const { return mbIsDraw; }
    bool IsImpress() const { return !mbIsDraw; }

    virtual void SetStatistics(
        const css::uno::Sequence< css::beans::NamedValue> & i_rStats) override;

    bool IsPreview() const { return mbPreview; }

    void AddHeaderDecl( const OUString& rName, const OUString& rText );
    void AddFooterDecl( const OUString& rName, const OUString& rText );
    void AddDateTimeDecl( const OUString& rName, const OUString& rText, bool bFixed, const OUString& rDateTimeFormat );

    OUString GetHeaderDecl( const OUString& rName ) const;
    OUString GetFooterDecl( const OUString& rName ) const;
    OUString GetDateTimeDecl( const OUString& rName, bool& rbFixed, OUString& rDateTimeFormat );

    virtual void NotifyEmbeddedFontRead() override;
};

#endif  //  _SDXMLIMP_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
