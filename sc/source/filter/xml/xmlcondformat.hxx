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

#include <xmloff/xmlictxt.hxx>
#include "xmlimprt.hxx"
#include "rangelst.hxx"

class ScColorScaleFormat;
class ScColorScaleEntry;
class ScDataBarFormat;
struct ScDataBarFormatData;
class ScConditionalFormat;
struct ScIconSetFormatData;

class ScXMLConditionalFormatsContext : public SvXMLImportContext
{
    const ScXMLImport& GetScImport() const { return static_cast<const ScXMLImport&>(GetImport()); }
    ScXMLImport& GetScImport() { return static_cast<ScXMLImport&>(GetImport()); }
public:
    ScXMLConditionalFormatsContext( ScXMLImport& rImport, sal_uInt16 nPrfx,
                        const OUString& rLName );

    virtual ~ScXMLConditionalFormatsContext() {}

    virtual SvXMLImportContext *CreateChildContext( sal_uInt16 nPrefix,
                                     const OUString& rLocalName,
                                     const css::uno::Reference<css::xml::sax::XAttributeList>& xAttrList ) override;

    virtual void EndElement() override;
};

class ScXMLConditionalFormatContext : public SvXMLImportContext
{
    const ScXMLImport& GetScImport() const { return static_cast<const ScXMLImport&>(GetImport()); }
    ScXMLImport& GetScImport() { return static_cast<ScXMLImport&>(GetImport()); }
public:
    ScXMLConditionalFormatContext( ScXMLImport& rImport, sal_uInt16 nPrfx,
                        const OUString& rLName,
                        const css::uno::Reference<css::xml::sax::XAttributeList>& xAttrList);

    virtual ~ScXMLConditionalFormatContext();

    virtual SvXMLImportContext *CreateChildContext( sal_uInt16 nPrefix,
                                     const OUString& rLocalName,
                                     const css::uno::Reference<css::xml::sax::XAttributeList>& xAttrList ) override;

    virtual void EndElement() override;
private:

    std::unique_ptr<ScConditionalFormat> mxFormat;
    ScRangeList maRange;
};

class ScXMLColorScaleFormatContext : public SvXMLImportContext
{
private:
    const ScXMLImport& GetScImport() const { return static_cast<const ScXMLImport&>(GetImport()); }
    ScXMLImport& GetScImport() { return static_cast<ScXMLImport&>(GetImport()); }

public:
    ScXMLColorScaleFormatContext( ScXMLImport& rImport, sal_uInt16 nPrfx,
                        const OUString& rLName, ScConditionalFormat* pFormat);

    virtual ~ScXMLColorScaleFormatContext() {}

    virtual SvXMLImportContext *CreateChildContext( sal_uInt16 nPrefix,
                                     const OUString& rLocalName,
                                     const css::uno::Reference<css::xml::sax::XAttributeList>& xAttrList ) override;
private:

    ScColorScaleFormat* pColorScaleFormat;
};

class ScXMLDataBarFormatContext : public SvXMLImportContext
{
    const ScXMLImport& GetScImport() const { return static_cast<const ScXMLImport&>(GetImport()); }
    ScXMLImport& GetScImport() { return static_cast<ScXMLImport&>(GetImport()); }
public:
    ScXMLDataBarFormatContext( ScXMLImport& rImport, sal_uInt16 nPrfx,
                        const OUString& rLName,
                        const css::uno::Reference<css::xml::sax::XAttributeList>& xAttrList,
                        ScConditionalFormat* pFormat);

    virtual ~ScXMLDataBarFormatContext() {}

    virtual SvXMLImportContext *CreateChildContext( sal_uInt16 nPrefix,
                                     const OUString& rLocalName,
                                     const css::uno::Reference<css::xml::sax::XAttributeList>& xAttrList ) override;
private:

    ScDataBarFormat* mpDataBarFormat;
    ScDataBarFormatData* mpFormatData;

};

class ScXMLIconSetFormatContext : public SvXMLImportContext
{
    const ScXMLImport& GetScImport() const { return static_cast<const ScXMLImport&>(GetImport()); }
    ScXMLImport& GetScImport() { return static_cast<ScXMLImport&>(GetImport()); }

    ScIconSetFormatData* mpFormatData;
public:

    ScXMLIconSetFormatContext( ScXMLImport& rImport, sal_uInt16 nPrfx,
                        const OUString& rLName,
                        const css::uno::Reference<css::xml::sax::XAttributeList>& xAttrList,
                        ScConditionalFormat* pFormat);

    virtual ~ScXMLIconSetFormatContext() {}

    virtual SvXMLImportContext *CreateChildContext( sal_uInt16 nPrefix,
                                     const OUString& rLocalName,
                                     const css::uno::Reference<css::xml::sax::XAttributeList>& xAttrList ) override;
};

class ScXMLColorScaleFormatEntryContext : public SvXMLImportContext
{
    const ScXMLImport& GetScImport() const { return static_cast<const ScXMLImport&>(GetImport()); }
    ScXMLImport& GetScImport() { return static_cast<ScXMLImport&>(GetImport()); }
public:
    ScXMLColorScaleFormatEntryContext( ScXMLImport& rImport, sal_uInt16 nPrfx,
                        const OUString& rLName,
                        const css::uno::Reference<css::xml::sax::XAttributeList>& xAttrList,
                        ScColorScaleFormat* pFormat);

    virtual ~ScXMLColorScaleFormatEntryContext() {}
private:

    ScColorScaleEntry* mpFormatEntry;
};

class ScXMLFormattingEntryContext : public SvXMLImportContext
{
    const ScXMLImport& GetScImport() const { return static_cast<const ScXMLImport&>(GetImport()); }
    ScXMLImport& GetScImport() { return static_cast<ScXMLImport&>(GetImport()); }
public:
    ScXMLFormattingEntryContext( ScXMLImport& rImport, sal_uInt16 nPrfx,
                        const OUString& rLName,
                        const css::uno::Reference<css::xml::sax::XAttributeList>& xAttrList,
                        ScColorScaleEntry*& pData);

    virtual ~ScXMLFormattingEntryContext() {}
};

class ScXMLCondContext : public SvXMLImportContext
{
    const ScXMLImport& GetScImport() const { return static_cast<const ScXMLImport&>(GetImport()); }
    ScXMLImport& GetScImport() { return static_cast<ScXMLImport&>(GetImport()); }
public:
    ScXMLCondContext( ScXMLImport& rImport, sal_uInt16 nPrfx,
                        const OUString& rLName,
                        const css::uno::Reference<css::xml::sax::XAttributeList>& xAttrList,
                        ScConditionalFormat* pFormat);

    virtual ~ScXMLCondContext() {}
};

class ScXMLDateContext : public SvXMLImportContext
{
    const ScXMLImport& GetScImport() const { return static_cast<const ScXMLImport&>(GetImport()); }
    ScXMLImport& GetScImport() { return static_cast<ScXMLImport&>(GetImport()); }
public:
    ScXMLDateContext( ScXMLImport& rImport, sal_uInt16 nPrfx,
                        const OUString& rLName,
                        const css::uno::Reference<css::xml::sax::XAttributeList>& xAttrList,
                        ScConditionalFormat* pFormat);

    virtual ~ScXMLDateContext() {}
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
