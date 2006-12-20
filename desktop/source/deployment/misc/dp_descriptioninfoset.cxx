/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: dp_descriptioninfoset.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: ihi $ $Date: 2006-12-20 14:28:31 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2006 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_desktop.hxx"

#include "sal/config.h"

#include "boost/optional.hpp"
#include "com/sun/star/beans/Optional.hpp"
#include "com/sun/star/lang/XMultiComponentFactory.hpp"
#include "com/sun/star/uno/Reference.hxx"
#include "com/sun/star/uno/RuntimeException.hpp"
#include "com/sun/star/uno/Sequence.hxx"
#include "com/sun/star/uno/XComponentContext.hpp"
#include "com/sun/star/uno/XInterface.hpp"
#include "com/sun/star/xml/dom/DOMException.hpp"
#include "com/sun/star/xml/dom/XNode.hpp"
#include "com/sun/star/xml/dom/XNodeList.hpp"
#include "com/sun/star/xml/xpath/XXPathAPI.hpp"
#include "cppuhelper/implbase1.hxx"
#include "cppuhelper/weak.hxx"
#include "rtl/ustring.h"
#include "rtl/ustring.hxx"
#include "sal/types.h"

#include "dp_descriptioninfoset.hxx"

namespace {

namespace css = ::com::sun::star;

class EmptyNodeList: public ::cppu::WeakImplHelper1< css::xml::dom::XNodeList >
{
public:
    EmptyNodeList();

    virtual ~EmptyNodeList();

    virtual ::sal_Int32 SAL_CALL getLength() throw (css::uno::RuntimeException);

    virtual css::uno::Reference< css::xml::dom::XNode > SAL_CALL
    item(::sal_Int32 index) throw (css::uno::RuntimeException);

private:
    EmptyNodeList(EmptyNodeList &); // not defined
    void operator =(EmptyNodeList &); // not defined
};

EmptyNodeList::EmptyNodeList() {}

EmptyNodeList::~EmptyNodeList() {}

::sal_Int32 EmptyNodeList::getLength() throw (css::uno::RuntimeException) {
    return 0;
}

css::uno::Reference< css::xml::dom::XNode > EmptyNodeList::item(::sal_Int32)
    throw (css::uno::RuntimeException)
{
    throw css::uno::RuntimeException(
        ::rtl::OUString(
            RTL_CONSTASCII_USTRINGPARAM(
                "bad EmptyNodeList com.sun.star.xml.dom.XNodeList.item call")),
        static_cast< ::cppu::OWeakObject * >(this));
}

::rtl::OUString getNodeValue(
    css::uno::Reference< css::xml::dom::XNode > const & node)
{
    OSL_ASSERT(node.is());
    try {
        return node->getNodeValue();
    } catch (css::xml::dom::DOMException & e) {
        throw css::uno::RuntimeException(
            (::rtl::OUString(
                RTL_CONSTASCII_USTRINGPARAM(
                    "com.sun.star.xml.dom.DOMException: ")) +
             e.Message),
            css::uno::Reference< css::uno::XInterface >());
    }
}

}

namespace dp_misc {

DescriptionInfoset::DescriptionInfoset(
    css::uno::Reference< css::uno::XComponentContext > const & context,
    css::uno::Reference< css::xml::dom::XNode > const & element):
    m_element(element)
{
    css::uno::Reference< css::lang::XMultiComponentFactory > manager(
        context->getServiceManager(), css::uno::UNO_QUERY_THROW);
    if (m_element.is()) {
        m_xpath = css::uno::Reference< css::xml::xpath::XXPathAPI >(
            manager->createInstanceWithContext(
                ::rtl::OUString(
                    RTL_CONSTASCII_USTRINGPARAM(
                        "com.sun.star.xml.xpath.XPathAPI")),
                context),
            css::uno::UNO_QUERY_THROW);
        m_xpath->registerNS(
            ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("desc")),
            element->getNamespaceURI());
        m_xpath->registerNS(
            ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("xlink")),
            ::rtl::OUString(
                RTL_CONSTASCII_USTRINGPARAM("http://www.w3.org/1999/xlink")));
    }
}

DescriptionInfoset::~DescriptionInfoset() {}

::boost::optional< ::rtl::OUString > DescriptionInfoset::getIdentifier() const {
    return getOptionalValue(
        ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("desc:identifier/@value")));
}

::rtl::OUString DescriptionInfoset::getVersion() const {
    css::uno::Reference< css::xml::dom::XNode > n;
    if (m_element.is()) {
        n = m_xpath->selectSingleNode(
            m_element,
            ::rtl::OUString(
                RTL_CONSTASCII_USTRINGPARAM("desc:version/@value")));
    }
    return n.is() ? getNodeValue(n) : ::rtl::OUString();
}

css::uno::Reference< css::xml::dom::XNodeList >
DescriptionInfoset::getDependencies() const {
    return m_element.is()
        ? m_xpath->selectNodeList(
            m_element,
            ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("desc:dependencies/*")))
        : new EmptyNodeList;
}

css::uno::Sequence< ::rtl::OUString >
DescriptionInfoset::getUpdateInformationUrls() const {
    return getUrls(
        ::rtl::OUString(
            RTL_CONSTASCII_USTRINGPARAM(
                "desc:update-information/desc:src/@xlink:href")));
}

css::uno::Sequence< ::rtl::OUString >
DescriptionInfoset::getUpdateDownloadUrls() const
{
    return getUrls(
        ::rtl::OUString(
            RTL_CONSTASCII_USTRINGPARAM(
                "desc:update-download/desc:src/@xlink:href")));
}

::boost::optional< ::rtl::OUString > DescriptionInfoset::getUpdateWebsiteUrl()
    const
{
    return getOptionalValue(
        ::rtl::OUString(
            RTL_CONSTASCII_USTRINGPARAM(
                "desc:update-website/desc:src/@xlink:href")));
}

css::uno::Reference< css::xml::xpath::XXPathAPI > DescriptionInfoset::getXpath()
    const
{
    return m_xpath;
}

::boost::optional< ::rtl::OUString > DescriptionInfoset::getOptionalValue(
    ::rtl::OUString const & expression) const
{
    css::uno::Reference< css::xml::dom::XNode > n;
    if (m_element.is()) {
        n = m_xpath->selectSingleNode(m_element, expression);
    }
    return n.is()
        ? ::boost::optional< ::rtl::OUString >(getNodeValue(n))
        : ::boost::optional< ::rtl::OUString >();
}

css::uno::Sequence< ::rtl::OUString > DescriptionInfoset::getUrls(
    ::rtl::OUString const & expression) const
{
    css::uno::Reference< css::xml::dom::XNodeList > ns;
    if (m_element.is()) {
        ns = m_xpath->selectNodeList(m_element, expression);
    }
    css::uno::Sequence< ::rtl::OUString > urls(ns.is() ? ns->getLength() : 0);
    for (::sal_Int32 i = 0; i < urls.getLength(); ++i) {
        urls[i] = getNodeValue(ns->item(i));
    }
    return urls;
}

}
