/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <array>
#include <memory>
#include <tools/link.hxx>
#include "importcontext.hxx"
#include <tokenarray.hxx>

namespace sax_fastparser { class FastAttributeList; }

class ScColorScaleFormat;
class ScColorScaleEntry;
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

    DECL_LINK(FormatDeletedHdl, ScConditionalFormat*, void);

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

    ScDataBarFormatData* mpFormatData;
    ScConditionalFormat* mpParent;

    sal_Int32 mnIndex;
};

class ScXMLIconSetFormatContext : public ScXMLImportContext
{
    ScIconSetFormatData* mpFormatData;
    ScConditionalFormat* mpParent;
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
