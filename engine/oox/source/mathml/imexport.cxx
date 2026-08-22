/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <oox/mathml/imexport.hxx>

#include <oox/mathml/importutils.hxx>
#include <oox/core/contexthandler.hxx>
#include <oox/token/namespaces.hxx>

#include <drawingml/textparagraph.hxx>


using namespace ::com::sun::star;

namespace oox
{

FormulaImExportBase::FormulaImExportBase()
{
}

namespace formulaimport {

namespace {

class LazyMathBufferingContext : public core::ContextHandler
{
private:
    XmlStreamBuilder & m_rBuilder;
    std::vector<sal_Int32> m_OpenElements;

public:
    LazyMathBufferingContext(core::ContextHandler const& rParent,
            drawingml::TextParagraph & rPara);

    // com.sun.star.xml.sax.XFastContextHandler interface ---------------------

    virtual void startFastElement(::sal_Int32 Element, const uno::Reference<xml::sax::XFastAttributeList>& xAttribs) override;
    virtual void endFastElement(::sal_Int32 Element) override;
    virtual uno::Reference< xml::sax::XFastContextHandler> createFastChildContext(::sal_Int32 Element, const uno::Reference<xml::sax::XFastAttributeList >& xAttribs) override;
    virtual void characters(const OUString& rChars) override;

};

}

LazyMathBufferingContext::LazyMathBufferingContext(
        core::ContextHandler const& rParent, drawingml::TextParagraph & rPara)
    : core::ContextHandler(rParent)
    , m_rBuilder(rPara.GetMathXml())
{
}

void LazyMathBufferingContext::startFastElement(
        sal_Int32 const nElement,
        uno::Reference<xml::sax::XFastAttributeList> const& xAttrs)
{
    if (0 < m_OpenElements.size()) // ignore a14:m
    {   // ignore outer oMathPara
        if (1 != m_OpenElements.size() || OOX_TOKEN(officeMath, oMathPara) != nElement)
        {
            m_rBuilder.appendOpeningTag(nElement, xAttrs);
        }
    }
    m_OpenElements.push_back(nElement);
}

void LazyMathBufferingContext::endFastElement(sal_Int32 const nElement)
{
    m_OpenElements.pop_back();
    if (0 < m_OpenElements.size()) // ignore a14:m
    {   // ignore outer oMathPara
        if (1 != m_OpenElements.size() || OOX_TOKEN(officeMath, oMathPara) != nElement)
        {
            m_rBuilder.appendClosingTag(nElement);
        }
    }
}

uno::Reference<xml::sax::XFastContextHandler>
LazyMathBufferingContext::createFastChildContext(sal_Int32 const,
        uno::Reference<xml::sax::XFastAttributeList> const&)
{
    return this;
}

void LazyMathBufferingContext::characters(OUString const& rChars)
{
    if (0 < m_OpenElements.size()) // ignore a14:m
    {
        if (m_OpenElements.back() == OOX_TOKEN(officeMath, t))
        {
            m_rBuilder.appendCharacters(rChars);
        }
    }
}

} // namespace oox::formulaimport

rtl::Reference<core::ContextHandler> CreateLazyMathBufferingContext(
        core::ContextHandler const& rParent, drawingml::TextParagraph & rPara)
{
    return new formulaimport::LazyMathBufferingContext(rParent, rPara);
}

} // namespace oox

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
