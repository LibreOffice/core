/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <uinums.hxx>

#include <cppuhelper/implbase.hxx>

#include <com/sun/star/container/XIndexReplace.hpp>
#include <com/sun/star/container/XNamed.hpp>
#include <com/sun/star/lang/IndexOutOfBoundsException.hpp>
#include <com/sun/star/util/MeasureUnit.hpp>
#include <com/sun/star/xml/sax/Writer.hpp>

#include <comphelper/processfactory.hxx>

#include <unotools/streamwrap.hxx>

#include <xmloff/xmlnamespace.hxx>
#include <xmloff/xmltoken.hxx>
#include <xmloff/namespacemap.hxx>
#include <xmloff/xmlexp.hxx>
#include <xmloff/xmlnume.hxx>
#include <xmloff/xmlimp.hxx>
#include <xmloff/xmlictxt.hxx>
#include <xmloff/xmlnumi.hxx>

#include <vcl/svapp.hxx>
#include <tools/diagnose_ex.h>

#include <unosett.hxx>


using namespace ::com::sun::star;
using namespace ::xmloff::token;

namespace sw {

class StoredChapterNumberingRules
    : public ::cppu::WeakImplHelper<container::XNamed,container::XIndexReplace>
{
private:
    // TODO in case this ever becomes accessible via API need an invalidate
    SwChapterNumRules & m_rNumRules;
    sal_uInt16 const m_nIndex;

    SwNumRulesWithName * GetOrCreateRules()
    {
        SwNumRulesWithName const* pRules(m_rNumRules.GetRules(m_nIndex));
        if (!pRules)
        {
            m_rNumRules.CreateEmptyNumRule(m_nIndex);
            pRules = m_rNumRules.GetRules(m_nIndex);
            assert(pRules);
        }
        return const_cast<SwNumRulesWithName*>(pRules);
    }

public:
    StoredChapterNumberingRules(
            SwChapterNumRules & rNumRules, sal_uInt16 const nIndex)
        : m_rNumRules(rNumRules)
        , m_nIndex(nIndex)
    {
        assert(m_nIndex < SwChapterNumRules::nMaxRules);
    }

    // XNamed
    virtual OUString SAL_CALL getName() override
    {
        SolarMutexGuard g;
        SwNumRulesWithName const* pRules(m_rNumRules.GetRules(m_nIndex));
        if (!pRules)
        {
            return OUString();
        }
        return pRules->GetName();
    }

    virtual void SAL_CALL setName(OUString const& rName) override
    {
        SolarMutexGuard g;
        SwNumRulesWithName *const pRules(GetOrCreateRules());
        pRules->SetName(rName);
    }

    // XElementAccess
    virtual uno::Type SAL_CALL getElementType() override
    {
        return ::cppu::UnoType<uno::Sequence<beans::PropertyValue>>::get();
    }

    virtual ::sal_Bool SAL_CALL hasElements() override
    {
        return true;
    }

    // XIndexAccess
    virtual sal_Int32 SAL_CALL getCount() override
    {
        return MAXLEVEL;
    }

    virtual uno::Any SAL_CALL getByIndex(sal_Int32 nIndex) override
    {
        if (nIndex < 0 || MAXLEVEL <= nIndex)
            throw lang::IndexOutOfBoundsException();

        SolarMutexGuard g;
        SwNumRulesWithName const* pRules(m_rNumRules.GetRules(m_nIndex));
        if (!pRules)
        {
            return uno::Any();
        }
        SwNumFormat const* pNumFormat(nullptr);
        OUString const* pCharStyleName(nullptr);
        pRules->GetNumFormat(nIndex, pNumFormat, pCharStyleName);
        if (!pNumFormat)
        {   // the dialog only fills in those levels that are non-default
            return uno::Any(); // the export will ignore this level, yay
        }
        assert(pCharStyleName);
        OUString dummy; // pass in empty HeadingStyleName - can't import anyway
        uno::Sequence<beans::PropertyValue> const ret(
            SwXNumberingRules::GetPropertiesForNumFormat(
                *pNumFormat, *pCharStyleName, &dummy, ""));
        return uno::makeAny(ret);
    }

    // XIndexReplace
    virtual void SAL_CALL replaceByIndex(
            sal_Int32 nIndex, uno::Any const& rElement) override
    {
        if (nIndex < 0 || MAXLEVEL <= nIndex)
            throw lang::IndexOutOfBoundsException();
        uno::Sequence<beans::PropertyValue> props;
        if (!(rElement >>= props))
            throw lang::IllegalArgumentException("invalid type",
                    static_cast< ::cppu::OWeakObject*>(this), 1);

        SolarMutexGuard g;
        SwNumFormat aNumberFormat;
        OUString charStyleName;
        SwXNumberingRules::SetPropertiesToNumFormat(
            aNumberFormat,
            charStyleName,
            nullptr, nullptr, nullptr, nullptr,
            props);
        SwNumRulesWithName *const pRules(GetOrCreateRules());
        pRules->SetNumFormat(nIndex, aNumberFormat, charStyleName);
    }
};

namespace {

class StoredChapterNumberingExport
    : public SvXMLExport
{
public:
    StoredChapterNumberingExport(
            uno::Reference<uno::XComponentContext> const& xContext,
            OUString const& rFileName,
            uno::Reference<xml::sax::XDocumentHandler> const& xHandler)
        : SvXMLExport(xContext, "sw::StoredChapterNumberingExport", rFileName,
            util::MeasureUnit::CM, xHandler)
    {
        GetNamespaceMap_().Add(GetXMLToken(XML_NP_OFFICE),
                               GetXMLToken(XML_N_OFFICE), XML_NAMESPACE_OFFICE);
        GetNamespaceMap_().Add(GetXMLToken(XML_NP_TEXT),
                               GetXMLToken(XML_N_TEXT), XML_NAMESPACE_TEXT);
        GetNamespaceMap_().Add(GetXMLToken(XML_NP_STYLE),
                               GetXMLToken(XML_N_STYLE), XML_NAMESPACE_STYLE);
        GetNamespaceMap_().Add(GetXMLToken(XML_NP_FO),
                               GetXMLToken(XML_N_FO), XML_NAMESPACE_FO);
        GetNamespaceMap_().Add(GetXMLToken(XML_NP_SVG),
                               GetXMLToken(XML_N_SVG), XML_NAMESPACE_SVG);
    }

    virtual void ExportAutoStyles_() override {}
    virtual void ExportMasterStyles_() override {}
    virtual void ExportContent_() override {}

    void ExportRule(SvxXMLNumRuleExport & rExport,
            uno::Reference<container::XIndexReplace> const& xRule)
    {
        uno::Reference<container::XNamed> const xNamed(xRule, uno::UNO_QUERY);
        OUString const name(xNamed->getName());
        bool bEncoded(false);
        AddAttribute( XML_NAMESPACE_STYLE, XML_NAME,
                      EncodeStyleName(name, &bEncoded) );
        if (bEncoded)
        {
            AddAttribute(XML_NAMESPACE_STYLE, XML_DISPLAY_NAME, name);
        }

        SvXMLElementExport aElem( *this, XML_NAMESPACE_TEXT,
                                  XML_OUTLINE_STYLE, true, true );
        rExport.exportLevelStyles(xRule, true);
    }

    void ExportRules(
            std::set<OUString> const& rCharStyles,
            std::vector<uno::Reference<container::XIndexReplace>> const& rRules)
    {
        GetDocHandler()->startDocument();

        AddAttribute(XML_NAMESPACE_NONE,
                      GetNamespaceMap_().GetAttrNameByKey(XML_NAMESPACE_OFFICE),
                      GetNamespaceMap_().GetNameByKey(XML_NAMESPACE_OFFICE));
        AddAttribute(XML_NAMESPACE_NONE,
                      GetNamespaceMap_().GetAttrNameByKey (XML_NAMESPACE_TEXT),
                      GetNamespaceMap_().GetNameByKey(XML_NAMESPACE_TEXT));
        AddAttribute(XML_NAMESPACE_NONE,
                      GetNamespaceMap_().GetAttrNameByKey(XML_NAMESPACE_STYLE),
                      GetNamespaceMap_().GetNameByKey(XML_NAMESPACE_STYLE));
        AddAttribute(XML_NAMESPACE_NONE,
                      GetNamespaceMap_().GetAttrNameByKey(XML_NAMESPACE_FO),
                      GetNamespaceMap_().GetNameByKey(XML_NAMESPACE_FO));
        AddAttribute(XML_NAMESPACE_NONE,
                      GetNamespaceMap_().GetAttrNameByKey(XML_NAMESPACE_SVG),
                      GetNamespaceMap_().GetNameByKey(XML_NAMESPACE_SVG));

        {
            // let's just have an office:styles as a dummy root
            SvXMLElementExport styles(*this,
                    XML_NAMESPACE_OFFICE, XML_STYLES, true, true);

            // horrible hack for char styles to get display-name mapping
            for (const auto& rCharStyle : rCharStyles)
            {
                AddAttribute( XML_NAMESPACE_STYLE, XML_FAMILY, XML_TEXT );
                bool bEncoded(false);
                AddAttribute( XML_NAMESPACE_STYLE, XML_NAME,
                              EncodeStyleName(rCharStyle, &bEncoded) );
                if (bEncoded)
                {
                    AddAttribute(XML_NAMESPACE_STYLE, XML_DISPLAY_NAME, rCharStyle);
                }

                SvXMLElementExport style(*this,
                        XML_NAMESPACE_STYLE, XML_STYLE, true, true);
            }

            SvxXMLNumRuleExport numRuleExport(*this);

            for (const auto& rRule : rRules)
            {
                ExportRule(numRuleExport, rRule);
            }
        }

        GetDocHandler()->endDocument();
    }
};

/** Dummy import context for style:style element that can just read the
    attributes needed to map name to display-name.
    Unfortunately the "real" context for this depends on some other things.
    The mapping is necessary to import the text:style-name attribute
    of the text:outline-level-style element.
 */
class StoredChapterNumberingDummyStyleContext
    : public SvXMLImportContext
{
public:
    StoredChapterNumberingDummyStyleContext(
            SvXMLImport & rImport,
            uno::Reference<xml::sax::XFastAttributeList> const& xAttrList)
        : SvXMLImportContext(rImport)
    {
        OUString name;
        OUString displayName;
        XmlStyleFamily nFamily(XmlStyleFamily::DATA_STYLE);

        for (auto &aIter : sax_fastparser::castToFastAttributeList( xAttrList ))
            if (aIter.getToken() == (XML_NAMESPACE_STYLE | XML_FAMILY))
            {
                if (IsXMLToken(aIter, XML_TEXT))
                    nFamily = XmlStyleFamily::TEXT_TEXT;
                else if (IsXMLToken(aIter, XML_NAME))
                    name = aIter.toString();
                else if (IsXMLToken(aIter, XML_DISPLAY_NAME))
                    displayName = aIter.toString();
                else
                    SAL_WARN("xmloff", "unknown value for style:family=" << aIter.toString());
            }
            else
                SAL_WARN("xmloff", "unknown attribute " << SvXMLImport::getPrefixAndNameFromToken(aIter.getToken()) << "=" << aIter.toString());

        if (nFamily != XmlStyleFamily::DATA_STYLE && !name.isEmpty() && !displayName.isEmpty())
        {
            rImport.AddStyleDisplayName(nFamily, name, displayName);
        }
    }
};

class StoredChapterNumberingImport;

class StoredChapterNumberingRootContext
    : public SvXMLImportContext
{
private:
    SwChapterNumRules & m_rNumRules;
    size_t m_nCounter;
    std::vector<rtl::Reference<SvxXMLListStyleContext>> m_Contexts;

public:
    StoredChapterNumberingRootContext(
            SwChapterNumRules & rNumRules, SvXMLImport & rImport)
        : SvXMLImportContext(rImport)
        , m_rNumRules(rNumRules)
        , m_nCounter(0)
    {
    }

    virtual void SAL_CALL startFastElement(
            sal_Int32 /*nElement*/,
            const css::uno::Reference< css::xml::sax::XFastAttributeList >& /*xAttrList*/ ) override {}

    virtual void SAL_CALL endFastElement(sal_Int32 /*Element*/) override
    {
        assert(m_Contexts.size() < SwChapterNumRules::nMaxRules);
        for (auto iter = m_Contexts.begin(); iter != m_Contexts.end(); ++iter)
        {
            uno::Reference<container::XIndexReplace> const xRule(
                new sw::StoredChapterNumberingRules(m_rNumRules,
                    iter - m_Contexts.begin()));
            (*iter)->FillUnoNumRule(xRule);
            // TODO: xmloff's outline-style import seems to ignore this???
            uno::Reference<container::XNamed> const xNamed(xRule, uno::UNO_QUERY);
            xNamed->setName((*iter)->GetDisplayName());
        }
    }

    virtual SvXMLImportContextRef CreateChildContext(
            sal_uInt16 nPrefix,
            const OUString& rLocalName,
            const css::uno::Reference< css::xml::sax::XAttributeList >& xAttrList ) override
    {
        if (XML_NAMESPACE_TEXT == nPrefix && IsXMLToken(rLocalName, XML_OUTLINE_STYLE))
        {
            ++m_nCounter;
            if (m_nCounter <= SwChapterNumRules::nMaxRules)
            {
                SvxXMLListStyleContext *const pContext(
                    new SvxXMLListStyleContext(GetImport(),
                                nPrefix, rLocalName, xAttrList, true));
                m_Contexts.emplace_back(pContext);
                return pContext;
            }
        }
        return nullptr;
    }

    virtual css::uno::Reference<XFastContextHandler> SAL_CALL createFastChildContext(
                sal_Int32 Element,
                const css::uno::Reference< css::xml::sax::XFastAttributeList > & xAttrList ) override
    {
        if (Element == XML_ELEMENT(TEXT, XML_OUTLINE_STYLE))
        {
            // handled in CreateChildContext
        }
        else if (Element == XML_ELEMENT(STYLE, XML_STYLE))
        {
            return new StoredChapterNumberingDummyStyleContext(GetImport(), xAttrList);
        }

        return nullptr;
    }
};

class StoredChapterNumberingImport
    : public SvXMLImport
{
private:
    SwChapterNumRules & m_rNumRules;

public:
    StoredChapterNumberingImport(
            uno::Reference<uno::XComponentContext> const& xContext,
            SwChapterNumRules & rNumRules)
        : SvXMLImport(xContext, "sw::StoredChapterNumberingImport", SvXMLImportFlags::ALL)
        , m_rNumRules(rNumRules)
    {
    }

    virtual SvXMLImportContext *CreateFastContext( sal_Int32 Element,
        const css::uno::Reference< css::xml::sax::XFastAttributeList > & /*xAttrList*/ ) override
    {
        if (Element == XML_ELEMENT(OFFICE, XML_STYLES))
            return new StoredChapterNumberingRootContext(m_rNumRules, *this);
        return nullptr;
    }
};

}

void ExportStoredChapterNumberingRules(SwChapterNumRules & rRules,
        SvStream & rStream, OUString const& rFileName)
{
    uno::Reference<uno::XComponentContext> const xContext(
            ::comphelper::getProcessComponentContext());

    uno::Reference<io::XOutputStream> const xOutStream(
            new ::utl::OOutputStreamWrapper(rStream));

    uno::Reference<xml::sax::XWriter> const xWriter(
            xml::sax::Writer::create(xContext));

    xWriter->setOutputStream(xOutStream);

    rtl::Reference<StoredChapterNumberingExport> exp(new StoredChapterNumberingExport(xContext, rFileName, xWriter));

    // if style name contains a space then name != display-name
    // ... and the import needs to map from name to display-name then!
    std::set<OUString> charStyles;
    std::vector<uno::Reference<container::XIndexReplace>> numRules;
    for (size_t i = 0; i < SwChapterNumRules::nMaxRules; ++i)
    {
        if (SwNumRulesWithName const* pRule = rRules.GetRules(i))
        {
            for (size_t j = 0; j < MAXLEVEL; ++j)
            {
                SwNumFormat const* pDummy(nullptr);
                OUString const* pCharStyleName(nullptr);
                pRule->GetNumFormat(j, pDummy, pCharStyleName);
                if (pCharStyleName && !pCharStyleName->isEmpty())
                {
                    charStyles.insert(*pCharStyleName);
                }
            }
            numRules.push_back(new StoredChapterNumberingRules(rRules, i));
        }
    }

    try
    {
        exp->ExportRules(charStyles, numRules);
    }
    catch (uno::Exception const&)
    {
        TOOLS_WARN_EXCEPTION("sw.ui", "ExportStoredChapterNumberingRules");
    }
}

void ImportStoredChapterNumberingRules(SwChapterNumRules & rRules,
        SvStream & rStream, OUString const& rFileName)
{
    uno::Reference<uno::XComponentContext> const xContext(
            ::comphelper::getProcessComponentContext());

    uno::Reference<io::XInputStream> const xInStream(
            new ::utl::OInputStreamWrapper(rStream));

    rtl::Reference<StoredChapterNumberingImport> const xImport(new StoredChapterNumberingImport(xContext, rRules));

    xml::sax::InputSource const source(xInStream, "", "", rFileName);

    try
    {
        xImport->parseStream(source);
    }
    catch (uno::Exception const&)
    {
        TOOLS_WARN_EXCEPTION("sw.ui", "ImportStoredChapterNumberingRules");
    }
}

} // namespace sw

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
