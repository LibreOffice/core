/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_SC_SOURCE_FILTER_XML_CELLTEXTPARACONTEXT_HXX
#define INCLUDED_SC_SOURCE_FILTER_XML_CELLTEXTPARACONTEXT_HXX

#include "importcontext.hxx"

class ScXMLImport;
class ScXMLTableRowCellContext;

/**
 * This context handles <text:p> element inside <table:table-cell>.
 */
class ScXMLCellTextParaContext : public ScXMLImportContext
{
    ScXMLTableRowCellContext& mrParentCxt;
    OUString maContent;
public:
    ScXMLCellTextParaContext(ScXMLImport& rImport, ScXMLTableRowCellContext& rParent);

    virtual void SAL_CALL endFastElement( sal_Int32 nElement ) override;
    virtual void SAL_CALL characters( const OUString& aChars ) override;
    virtual css::uno::Reference< css::xml::sax::XFastContextHandler > SAL_CALL createFastChildContext(
        sal_Int32 nElement, const css::uno::Reference< css::xml::sax::XFastAttributeList >& xAttrList ) override;

    void PushSpan(const OUString& rSpan, const OUString& rStyleName);
    void PushFieldSheetName(const OUString& rStyleName);
    void PushFieldDate(const OUString& rStyleName);
    void PushFieldTitle(const OUString& rStyleName);
    void PushFieldURL(const OUString& rURL, const OUString& rRep, const OUString& rStyleName, const OUString& rTargetFrame);
};

/**
 * This context handles <text:span> element inside <text:p>.
 */
class ScXMLCellTextSpanContext : public ScXMLImportContext
{
    ScXMLCellTextParaContext& mrParentCxt;
    OUString maStyleName;
    OUString maContent;
public:
    ScXMLCellTextSpanContext(ScXMLImport& rImport, ScXMLCellTextParaContext& rParent);

    virtual void SAL_CALL startFastElement( sal_Int32 nElement,
        const css::uno::Reference< css::xml::sax::XFastAttributeList >& xAttrList ) override;
    virtual void SAL_CALL endFastElement( sal_Int32 nElement ) override;
    virtual void SAL_CALL characters( const OUString& aChars ) override;
    virtual css::uno::Reference< css::xml::sax::XFastContextHandler > SAL_CALL createFastChildContext(
        sal_Int32 nElement, const css::uno::Reference< css::xml::sax::XFastAttributeList >& xAttrList ) override;
    void submitContentAndClear();
};

/**
 * This context handles <text:sheet-name> element inside <text:p>.
 */
class ScXMLCellFieldSheetNameContext : public ScXMLImportContext
{
    ScXMLCellTextParaContext& mrParentCxt;
    OUString maStyleName;
public:
    ScXMLCellFieldSheetNameContext(ScXMLImport& rImport, ScXMLCellTextParaContext& rParent);

    void SetStyleName(const OUString& rStyleName);

    virtual void SAL_CALL endFastElement( sal_Int32 nElement ) override;
};

/**
 * This context handles <text:date> element inside <text:p>.
 */
class ScXMLCellFieldDateContext : public ScXMLImportContext
{
    ScXMLCellTextParaContext& mrParentCxt;
    OUString maStyleName;
public:
    ScXMLCellFieldDateContext(ScXMLImport& rImport, ScXMLCellTextParaContext& rParent);

    void SetStyleName(const OUString& rStyleName);

    virtual void SAL_CALL endFastElement( sal_Int32 nElement ) override;
};

/**
 * This context handles <text:title> element inside <text:p>.
 */
class ScXMLCellFieldTitleContext : public ScXMLImportContext
{
    ScXMLCellTextParaContext& mrParentCxt;
    OUString maStyleName;
public:
    ScXMLCellFieldTitleContext(ScXMLImport& rImport, ScXMLCellTextParaContext& rParent);

    void SetStyleName(const OUString& rStyleName);

    virtual void SAL_CALL endFastElement( sal_Int32 nElement ) override;
};

/**
 * This context handles <text:a> element inside <text:p> or <text:span>.
 */
class ScXMLCellFieldURLContext : public ScXMLImportContext
{
    ScXMLCellTextParaContext& mrParentCxt;
    OUString maStyleName;
    OUString maURL;
    OUString maRep;
    OUString maTargetFrame;
public:
    ScXMLCellFieldURLContext(ScXMLImport& rImport, ScXMLCellTextParaContext& rParent);

    void SetStyleName(const OUString& rStyleName);

    virtual void SAL_CALL startFastElement( sal_Int32 nElement,
        const css::uno::Reference< css::xml::sax::XFastAttributeList >& xAttrList ) override;
    virtual void SAL_CALL endFastElement( sal_Int32 nElement ) override;
    virtual void SAL_CALL characters( const OUString& aChars ) override;
};

/**
 * This context handles <text:s> element inside <text:p> or <text:span>.
 */
class ScXMLCellFieldSContext : public ScXMLImportContext
{
    ScXMLCellTextParaContext& mrParentCxt;
    OUString  maStyleName;
    sal_Int32 mnCount;

    void PushSpaces();
public:
    ScXMLCellFieldSContext(ScXMLImport& rImport, ScXMLCellTextParaContext& rParent);

    void SetStyleName(const OUString& rStyleName);

    virtual void SAL_CALL startFastElement( sal_Int32 nElement,
        const css::uno::Reference< css::xml::sax::XFastAttributeList >& xAttrList ) override;
    virtual void SAL_CALL endFastElement( sal_Int32 nElement ) override;
    virtual css::uno::Reference< css::xml::sax::XFastContextHandler > SAL_CALL createFastChildContext(
        sal_Int32 nElement, const css::uno::Reference< css::xml::sax::XFastAttributeList >& xAttrList ) override;
};

/**
 * This context handles <text:ruby> element inside <text:p>.
 */
class ScXMLCellTextRubyContext : public ScXMLImportContext
{
    ScXMLCellTextParaContext& mrParentCxt;
    OUString maRubyStyleName;
    OUString maRubyTextStyle;
    OUString maRubyText;
public:
    ScXMLCellTextRubyContext(ScXMLImport& rImport, ScXMLCellTextParaContext& rParent);

    virtual void SAL_CALL startFastElement( sal_Int32 nElement,
        const css::uno::Reference< css::xml::sax::XFastAttributeList >& xAttrList ) override;
    virtual css::uno::Reference< css::xml::sax::XFastContextHandler > SAL_CALL createFastChildContext(
        sal_Int32 nElement, const css::uno::Reference< css::xml::sax::XFastAttributeList >& xAttrList ) override;
};

/**
 * This context handles <text:ruby-base> element inside <text:ruby>.
 */
class ScXMLCellRubyBaseContext : public ScXMLCellTextSpanContext
{
    ScXMLCellTextParaContext& mrParentCxt;
public:
    ScXMLCellRubyBaseContext(ScXMLImport& rImport, ScXMLCellTextParaContext& rParent);
    virtual css::uno::Reference< css::xml::sax::XFastContextHandler > SAL_CALL createFastChildContext(
        sal_Int32 nElement, const css::uno::Reference< css::xml::sax::XFastAttributeList >& xAttrList ) override;
};

/**
 * This context handles <text:ruby-text> element inside <text:ruby>.
 */
class ScXMLCellRubyTextContext : public ScXMLImportContext
{
    OUString& mrRubyText;
    OUString& mrRubyTextStyle;
public:
    ScXMLCellRubyTextContext(ScXMLImport& rImport, OUString& rRubyText, OUString& rRubyTextStyle);

    virtual void SAL_CALL startFastElement( sal_Int32 nElement,
        const css::uno::Reference< css::xml::sax::XFastAttributeList >& xAttrList ) override;
    virtual void SAL_CALL characters( const OUString& aChars ) override;
};
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
