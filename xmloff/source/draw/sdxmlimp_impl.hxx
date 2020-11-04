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

    sal_Int32                   mnNewPageCount;
    sal_Int32                   mnNewMasterPageCount;

    bool                        mbIsDraw;
    bool                        mbLoadDoc;
    bool                        mbPreview;

    static constexpr OUStringLiteral gsPageLayouts = u"PageLayouts";
    static constexpr OUStringLiteral gsPreview = u"Preview";

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
    SvXMLStylesContext* CreateStylesContext();
    SvXMLStylesContext* CreateAutoStylesContext();
    SvXMLImportContext* CreateMasterStylesContext();
    SvXMLImportContext *CreateFontDeclsContext();

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

    virtual void NotifyContainsEmbeddedFont() override;
};

#endif  //  _SDXMLIMP_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
