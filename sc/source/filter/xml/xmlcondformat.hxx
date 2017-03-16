/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_SC_SOURCE_FILTER_XML_XMLCONDFORMAT_HXX
#define INCLUDED_SC_SOURCE_FILTER_XML_XMLCONDFORMAT_HXX

#include <memory>
#include <xmloff/xmlictxt.hxx>
#include "xmlimprt.hxx"
#include "importcontext.hxx"
#include "rangelst.hxx"

class ScColorScaleFormat;
class ScColorScaleEntry;
class ScDataBarFormat;
struct ScDataBarFormatData;
class ScConditionalFormat;
struct ScIconSetFormatData;

class ScXMLConditionalFormatsContext : public ScXMLImportContext
{
public:
    ScXMLConditionalFormatsContext( ScXMLImport& rImport, sal_uInt16 nPrfx,
                        const OUString& rLName );

    virtual SvXMLImportContext *CreateChildContext( sal_uInt16 nPrefix,
                                     const OUString& rLocalName,
                                     const css::uno::Reference<css::xml::sax::XAttributeList>& xAttrList ) override;

    virtual void EndElement() override;
};

class ScXMLConditionalFormatContext : public ScXMLImportContext
{
public:
    ScXMLConditionalFormatContext( ScXMLImport& rImport, sal_uInt16 nPrfx,
                        const OUString& rLName,
                        const css::uno::Reference<css::xml::sax::XAttributeList>& xAttrList);

    virtual ~ScXMLConditionalFormatContext() override;

    virtual SvXMLImportContext *CreateChildContext( sal_uInt16 nPrefix,
                                     const OUString& rLocalName,
                                     const css::uno::Reference<css::xml::sax::XAttributeList>& xAttrList ) override;

    virtual void EndElement() override;
private:

    std::unique_ptr<ScConditionalFormat> mxFormat;
    ScRangeList maRange;
};

class ScXMLColorScaleFormatContext : public ScXMLImportContext
{
public:
    ScXMLColorScaleFormatContext( ScXMLImport& rImport, sal_uInt16 nPrfx,
                        const OUString& rLName, ScConditionalFormat* pFormat);

    virtual SvXMLImportContext *CreateChildContext( sal_uInt16 nPrefix,
                                     const OUString& rLocalName,
                                     const css::uno::Reference<css::xml::sax::XAttributeList>& xAttrList ) override;
private:

    ScColorScaleFormat* pColorScaleFormat;
};

class ScXMLDataBarFormatContext : public ScXMLImportContext
{
public:
    ScXMLDataBarFormatContext( ScXMLImport& rImport, sal_uInt16 nPrfx,
                        const OUString& rLName,
                        const css::uno::Reference<css::xml::sax::XAttributeList>& xAttrList,
                        ScConditionalFormat* pFormat);

    virtual SvXMLImportContext *CreateChildContext( sal_uInt16 nPrefix,
                                     const OUString& rLocalName,
                                     const css::uno::Reference<css::xml::sax::XAttributeList>& xAttrList ) override;
private:

    ScDataBarFormat* mpDataBarFormat;
    ScDataBarFormatData* mpFormatData;

    sal_Int32 mnIndex;
};

class ScXMLIconSetFormatContext : public ScXMLImportContext
{
    ScIconSetFormatData* mpFormatData;
public:

    ScXMLIconSetFormatContext( ScXMLImport& rImport, sal_uInt16 nPrfx,
                        const OUString& rLName,
                        const css::uno::Reference<css::xml::sax::XAttributeList>& xAttrList,
                        ScConditionalFormat* pFormat);

    virtual SvXMLImportContext *CreateChildContext( sal_uInt16 nPrefix,
                                     const OUString& rLocalName,
                                     const css::uno::Reference<css::xml::sax::XAttributeList>& xAttrList ) override;
};

class ScXMLColorScaleFormatEntryContext : public ScXMLImportContext
{
public:
    ScXMLColorScaleFormatEntryContext( ScXMLImport& rImport, sal_uInt16 nPrfx,
                        const OUString& rLName,
                        const css::uno::Reference<css::xml::sax::XAttributeList>& xAttrList,
                        ScColorScaleFormat* pFormat);

private:
    ScColorScaleEntry* mpFormatEntry;
};

class ScXMLFormattingEntryContext : public ScXMLImportContext
{
public:
    ScXMLFormattingEntryContext( ScXMLImport& rImport, sal_uInt16 nPrfx,
                        const OUString& rLName,
                        const css::uno::Reference<css::xml::sax::XAttributeList>& xAttrList,
                        ScColorScaleEntry*& pData);
};

class ScXMLCondContext : public ScXMLImportContext
{
public:
    ScXMLCondContext( ScXMLImport& rImport, sal_uInt16 nPrfx,
                        const OUString& rLName,
                        const css::uno::Reference<css::xml::sax::XAttributeList>& xAttrList,
                        ScConditionalFormat* pFormat);
};

class ScXMLDateContext : public ScXMLImportContext
{
public:
    ScXMLDateContext( ScXMLImport& rImport, sal_uInt16 nPrfx,
                        const OUString& rLName,
                        const css::uno::Reference<css::xml::sax::XAttributeList>& xAttrList,
                        ScConditionalFormat* pFormat);
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
