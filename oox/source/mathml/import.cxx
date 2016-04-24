/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "oox/mathml/import.hxx"

#include <oox/mathml/importutils.hxx>
#include <oox/core/contexthandler.hxx>
#include <oox/token/namespaces.hxx>
#include <oox/token/tokens.hxx>

#include <drawingml/textparagraph.hxx>


using namespace ::com::sun::star;

namespace oox
{

FormulaImportBase::FormulaImportBase()
{
}

namespace formulaimport {

class LazyMathBufferingContext : public core::ContextHandler
{
private:
    XmlStreamBuilder & m_rBuilder;
    long m_Counter;

public:
    LazyMathBufferingContext(core::ContextHandler const& rParent,
            drawingml::TextParagraph & rPara);

    // com.sun.star.xml.sax.XFastContextHandler interface ---------------------

    virtual void SAL_CALL startFastElement(::sal_Int32 Element, const uno::Reference<xml::sax::XFastAttributeList>& xAttribs) throw (xml::sax::SAXException, uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL endFastElement(::sal_Int32 Element) throw (xml::sax::SAXException, uno::RuntimeException, std::exception) override;
    virtual uno::Reference< xml::sax::XFastContextHandler> SAL_CALL createFastChildContext(::sal_Int32 Element, const uno::Reference<xml::sax::XFastAttributeList >& xAttribs) throw (xml::sax::SAXException, uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL characters(const OUString& rChars) throw (xml::sax::SAXException, uno::RuntimeException, std::exception) override;

};

LazyMathBufferingContext::LazyMathBufferingContext(
        core::ContextHandler const& rParent, drawingml::TextParagraph & rPara)
    : core::ContextHandler(rParent)
    , m_rBuilder(rPara.GetMathXml())
    , m_Counter(0)
{
}

void SAL_CALL LazyMathBufferingContext::startFastElement(
        sal_Int32 const nElement,
        uno::Reference<xml::sax::XFastAttributeList> const& xAttrs)
    throw (xml::sax::SAXException, uno::RuntimeException, std::exception)
{
    if (0 < m_Counter) // ignore a14:m
    {   // ignore outer oMathPara
        if (1 != m_Counter || OOX_TOKEN(officeMath, oMathPara) != nElement)
        {
            m_rBuilder.appendOpeningTag(nElement, xAttrs);
        }
    }
    ++m_Counter;
}

void SAL_CALL LazyMathBufferingContext::endFastElement(sal_Int32 const nElement)
    throw (xml::sax::SAXException, uno::RuntimeException, std::exception)
{
    --m_Counter;
    if (0 < m_Counter) // ignore a14:m
    {   // ignore outer oMathPara
        if (1 != m_Counter || OOX_TOKEN(officeMath, oMathPara) != nElement)
        {
            m_rBuilder.appendClosingTag(nElement);
        }
    }
}

uno::Reference<xml::sax::XFastContextHandler> SAL_CALL
LazyMathBufferingContext::createFastChildContext(sal_Int32 const,
        uno::Reference<xml::sax::XFastAttributeList> const&)
    throw (xml::sax::SAXException, uno::RuntimeException, std::exception)
{
    return this;
}

void SAL_CALL LazyMathBufferingContext::characters(OUString const& rChars)
    throw (xml::sax::SAXException, uno::RuntimeException, std::exception)
{
    if (0 < m_Counter) // ignore a14:m
    {
        m_rBuilder.appendCharacters(rChars);
    }
}

} // namespace formulaimport

core::ContextHandlerRef CreateLazyMathBufferingContext(
        core::ContextHandler const& rParent, drawingml::TextParagraph & rPara)
{
    return new formulaimport::LazyMathBufferingContext(rParent, rPara);
}

} // namespace oox

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
