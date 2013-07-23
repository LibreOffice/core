/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef __SC_XML_CELLTEXTPARACONTEXT_HXX__
#define __SC_XML_CELLTEXTPARACONTEXT_HXX__

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
    ScXMLCellTextParaContext(ScXMLImport& rImport, sal_uInt16 nPrefix, const OUString& rLName, ScXMLTableRowCellContext& rParent);

    virtual void StartElement(const com::sun::star::uno::Reference<com::sun::star::xml::sax::XAttributeList>& xAttrList);
    virtual void EndElement();
    virtual void Characters(const OUString& rChars);
    virtual SvXMLImportContext* CreateChildContext(
        sal_uInt16 nPrefix, const OUString& rLocalName, const com::sun::star::uno::Reference<com::sun::star::xml::sax::XAttributeList>& xAttrList);

    void PushSpan(const OUString& rSpan, const OUString& rStyleName);
    void PushFieldSheetName(const OUString& rStyleName);
    void PushFieldDate(const OUString& rStyleName);
    void PushFieldTitle(const OUString& rStyleName);
    void PushFieldURL(const OUString& rURL, const OUString& rRep, const OUString& rStyleName);
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
    ScXMLCellTextSpanContext(ScXMLImport& rImport, sal_uInt16 nPrefix, const OUString& rLName, ScXMLCellTextParaContext& rParent);

    virtual void StartElement(const com::sun::star::uno::Reference<com::sun::star::xml::sax::XAttributeList>& xAttrList);
    virtual void EndElement();
    virtual void Characters(const OUString& rChars);
    virtual SvXMLImportContext* CreateChildContext(
        sal_uInt16 nPrefix, const OUString& rLocalName, const com::sun::star::uno::Reference<com::sun::star::xml::sax::XAttributeList>& xAttrList);
};

/**
 * This context handles <text:sheet-name> element inside <text:p>.
 */
class ScXMLCellFieldSheetNameContext : public ScXMLImportContext
{
    ScXMLCellTextParaContext& mrParentCxt;
    OUString maStyleName;
public:
    ScXMLCellFieldSheetNameContext(ScXMLImport& rImport, sal_uInt16 nPrefix, const OUString& rLName, ScXMLCellTextParaContext& rParent);

    void SetStyleName(const OUString& rStyleName);

    virtual void StartElement(const com::sun::star::uno::Reference<com::sun::star::xml::sax::XAttributeList>& xAttrList);
    virtual void EndElement();
    virtual void Characters(const OUString& rChars);
    virtual SvXMLImportContext* CreateChildContext(
        sal_uInt16 nPrefix, const OUString& rLocalName, const com::sun::star::uno::Reference<com::sun::star::xml::sax::XAttributeList>& xAttrList);
};

/**
 * This context handles <text:date> element inside <text:p>.
 */
class ScXMLCellFieldDateContext : public ScXMLImportContext
{
    ScXMLCellTextParaContext& mrParentCxt;
    OUString maStyleName;
public:
    ScXMLCellFieldDateContext(ScXMLImport& rImport, sal_uInt16 nPrefix, const OUString& rLName, ScXMLCellTextParaContext& rParent);

    void SetStyleName(const OUString& rStyleName);

    virtual void StartElement(const com::sun::star::uno::Reference<com::sun::star::xml::sax::XAttributeList>& xAttrList);
    virtual void EndElement();
    virtual void Characters(const OUString& rChars);
    virtual SvXMLImportContext* CreateChildContext(
        sal_uInt16 nPrefix, const OUString& rLocalName, const com::sun::star::uno::Reference<com::sun::star::xml::sax::XAttributeList>& xAttrList);
};

/**
 * This context handles <text:title> element inside <text:p>.
 */
class ScXMLCellFieldTitleContext : public ScXMLImportContext
{
    ScXMLCellTextParaContext& mrParentCxt;
    OUString maStyleName;
public:
    ScXMLCellFieldTitleContext(ScXMLImport& rImport, sal_uInt16 nPrefix, const OUString& rLName, ScXMLCellTextParaContext& rParent);

    void SetStyleName(const OUString& rStyleName);

    virtual void StartElement(const com::sun::star::uno::Reference<com::sun::star::xml::sax::XAttributeList>& xAttrList);
    virtual void EndElement();
    virtual void Characters(const OUString& rChars);
    virtual SvXMLImportContext* CreateChildContext(
        sal_uInt16 nPrefix, const OUString& rLocalName, const com::sun::star::uno::Reference<com::sun::star::xml::sax::XAttributeList>& xAttrList);
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
public:
    ScXMLCellFieldURLContext(ScXMLImport& rImport, sal_uInt16 nPrefix, const OUString& rLName, ScXMLCellTextParaContext& rParent);

    void SetStyleName(const OUString& rStyleName);

    virtual void StartElement(const com::sun::star::uno::Reference<com::sun::star::xml::sax::XAttributeList>& xAttrList);
    virtual void EndElement();
    virtual void Characters(const OUString& rChars);
    virtual SvXMLImportContext* CreateChildContext(
        sal_uInt16 nPrefix, const OUString& rLocalName, const com::sun::star::uno::Reference<com::sun::star::xml::sax::XAttributeList>& xAttrList);
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
    ScXMLCellFieldSContext(ScXMLImport& rImport, sal_uInt16 nPrefix, const OUString& rLName, ScXMLCellTextParaContext& rParent);

    void SetStyleName(const OUString& rStyleName);

    virtual void StartElement(const com::sun::star::uno::Reference<com::sun::star::xml::sax::XAttributeList>& xAttrList);
    virtual void EndElement();
    virtual SvXMLImportContext* CreateChildContext(
        sal_uInt16 nPrefix, const OUString& rLocalName, const com::sun::star::uno::Reference<com::sun::star::xml::sax::XAttributeList>& xAttrList);
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
