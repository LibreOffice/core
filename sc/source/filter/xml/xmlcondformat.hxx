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

#include <array>
#include <memory>
#include <xmloff/xmlictxt.hxx>
#include "xmlimprt.hxx"
#include "importcontext.hxx"
#include <rangelst.hxx>

class ScColorScaleFormat;
class ScColorScaleEntry;
class ScDataBarFormat;
struct ScDataBarFormatData;
class ScConditionalFormat;
struct ScIconSetFormatData;

class ScXMLConditionalFormatsContext : public ScXMLImportContext
{
private:
    struct CacheEntry
    {
        ScConditionalFormat* mpFormat = nullptr;
        bool mbSingleRelativeReference;
        std::unique_ptr<const ScTokenArray> mpTokens;
        sal_Int64 mnAge = SAL_MAX_INT64;
    };

    struct CondFormatData
    {
        ScConditionalFormat* mpFormat;
        SCTAB mnTab;
    };

public:
    ScXMLConditionalFormatsContext( ScXMLImport& rImport );

    virtual css::uno::Reference< css::xml::sax::XFastContextHandler > SAL_CALL createFastChildContext(
        sal_Int32 nElement, const css::uno::Reference< css::xml::sax::XFastAttributeList >& xAttrList ) override;

    virtual void SAL_CALL endFastElement( sal_Int32 nElement ) override;

    std::array<CacheEntry, 4> maCache;

    std::vector<CondFormatData> mvCondFormatData;
};

class ScXMLConditionalFormatContext : public ScXMLImportContext
{
public:
    ScXMLConditionalFormatContext( ScXMLImport& rImport,
                                   const rtl::Reference<sax_fastparser::FastAttributeList>& rAttrList,
                                   ScXMLConditionalFormatsContext& rParent );

    virtual ~ScXMLConditionalFormatContext() override;

    virtual css::uno::Reference< css::xml::sax::XFastContextHandler > SAL_CALL createFastChildContext(
        sal_Int32 nElement, const css::uno::Reference< css::xml::sax::XFastAttributeList >& xAttrList ) override;

    virtual void SAL_CALL endFastElement( sal_Int32 nElement ) override;
private:

    std::unique_ptr<ScConditionalFormat> mxFormat;
    ScRangeList maRange;

    ScXMLConditionalFormatsContext& mrParent;
};

class ScXMLColorScaleFormatContext : public ScXMLImportContext
{
public:
    ScXMLColorScaleFormatContext( ScXMLImport& rImport,
                        ScConditionalFormat* pFormat);

    virtual css::uno::Reference< css::xml::sax::XFastContextHandler > SAL_CALL createFastChildContext(
        sal_Int32 nElement, const css::uno::Reference< css::xml::sax::XFastAttributeList >& xAttrList ) override;
private:

    ScColorScaleFormat* pColorScaleFormat;
};

class ScXMLDataBarFormatContext : public ScXMLImportContext
{
public:
    ScXMLDataBarFormatContext( ScXMLImport& rImport,
                        const rtl::Reference<sax_fastparser::FastAttributeList>& rAttrList,
                        ScConditionalFormat* pFormat);

    virtual css::uno::Reference< css::xml::sax::XFastContextHandler > SAL_CALL createFastChildContext(
        sal_Int32 nElement, const css::uno::Reference< css::xml::sax::XFastAttributeList >& xAttrList ) override;
private:

    ScDataBarFormat* mpDataBarFormat;
    ScDataBarFormatData* mpFormatData;

    sal_Int32 mnIndex;
};

class ScXMLIconSetFormatContext : public ScXMLImportContext
{
    ScIconSetFormatData* mpFormatData;
public:

    ScXMLIconSetFormatContext( ScXMLImport& rImport,
                        const rtl::Reference<sax_fastparser::FastAttributeList>& rAttrList,
                        ScConditionalFormat* pFormat);

    virtual css::uno::Reference< css::xml::sax::XFastContextHandler > SAL_CALL createFastChildContext(
        sal_Int32 nElement, const css::uno::Reference< css::xml::sax::XFastAttributeList >& xAttrList ) override;
};

class ScXMLColorScaleFormatEntryContext : public ScXMLImportContext
{
public:
    ScXMLColorScaleFormatEntryContext( ScXMLImport& rImport,
                        const rtl::Reference<sax_fastparser::FastAttributeList>& rAttrList,
                        ScColorScaleFormat* pFormat);

private:
    ScColorScaleEntry* mpFormatEntry;
};

class ScXMLFormattingEntryContext : public ScXMLImportContext
{
public:
    ScXMLFormattingEntryContext( ScXMLImport& rImport,
                        const rtl::Reference<sax_fastparser::FastAttributeList>& rAttrList,
                        ScColorScaleEntry*& pData);
};

class ScXMLCondContext : public ScXMLImportContext
{
public:
    ScXMLCondContext( ScXMLImport& rImport,
                        const rtl::Reference<sax_fastparser::FastAttributeList>& rAttrList,
                        ScConditionalFormat* pFormat);
};

class ScXMLDateContext : public ScXMLImportContext
{
public:
    ScXMLDateContext( ScXMLImport& rImport,
                        const rtl::Reference<sax_fastparser::FastAttributeList>& rAttrList,
                        ScConditionalFormat* pFormat);
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
