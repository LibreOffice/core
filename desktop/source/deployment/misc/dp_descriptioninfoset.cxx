/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: dp_descriptioninfoset.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: ihi $ $Date: 2007-11-22 15:04:24 $
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

#include "dp_descriptioninfoset.hxx"

#include "dp_resource.h"
#include "sal/config.h"

#include "boost/optional.hpp"
#include "com/sun/star/beans/Optional.hpp"
#include "com/sun/star/lang/XMultiComponentFactory.hpp"
#include "com/sun/star/lang/Locale.hpp"
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

::boost::optional< ::rtl::OUString > DescriptionInfoset::getLocalizedUpdateWebsiteURL()
    const
{
    bool bParentExists = false;
    const ::rtl::OUString sURL (getLocalizedHREFAttrFromChild(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(
        "/desc:description/desc:update-website")), &bParentExists ));

    if (sURL.getLength() > 0)
        return ::boost::optional< ::rtl::OUString >(sURL);
    else
        return bParentExists ? ::boost::optional< ::rtl::OUString >(::rtl::OUString()) :
            ::boost::optional< ::rtl::OUString >();
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

::std::pair< ::rtl::OUString, ::rtl::OUString > DescriptionInfoset::getLocalizedPublisherNameAndURL() const
{
    css::uno::Reference< css::xml::dom::XNode > node =
        getLocalizedChild(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("desc:publisher")));

    ::rtl::OUString sPublisherName;
    ::rtl::OUString sURL;
    if (node.is())
    {
        const ::rtl::OUString exp1(RTL_CONSTASCII_USTRINGPARAM("text()"));
        css::uno::Reference< css::xml::dom::XNode > xPathName = m_xpath->selectSingleNode(node, exp1);
        OSL_ASSERT(xPathName.is());
        if (xPathName.is())
            sPublisherName = xPathName->getNodeValue();

        const ::rtl::OUString exp2(RTL_CONSTASCII_USTRINGPARAM("@xlink:href"));
        css::uno::Reference< css::xml::dom::XNode > xURL = m_xpath->selectSingleNode(node, exp2);
        OSL_ASSERT(xURL.is());
        if (xURL.is())
           sURL = xURL->getNodeValue();
    }
    return ::std::make_pair(sPublisherName, sURL);
}

::rtl::OUString DescriptionInfoset::getLocalizedReleaseNotesURL() const
{
    return getLocalizedHREFAttrFromChild(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(
        "/desc:description/desc:release-notes")), NULL);
}

::rtl::OUString DescriptionInfoset::getLocalizedDisplayName() const
{
    css::uno::Reference< css::xml::dom::XNode > node =
        getLocalizedChild(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("desc:display-name")));
    if (node.is())
    {
        const ::rtl::OUString exp(RTL_CONSTASCII_USTRINGPARAM("text()"));
        css::uno::Reference< css::xml::dom::XNode > xtext = m_xpath->selectSingleNode(node, exp);
        if (xtext.is())
            return xtext->getNodeValue();
    }
    return ::rtl::OUString();
}

::rtl::OUString DescriptionInfoset::getLocalizedLicenseURL() const
{
    return getLocalizedHREFAttrFromChild(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(
        "/desc:description/desc:registration/desc:simple-license")), NULL);

}

css::uno::Reference< css::xml::dom::XNode >
DescriptionInfoset::getLocalizedChild( const ::rtl::OUString & sParent) const
{
    if ( ! m_element.is() || !sParent.getLength())
        return css::uno::Reference< css::xml::dom::XNode > ();

    css::uno::Reference< css::xml::dom::XNode > xParent =
        m_xpath->selectSingleNode(m_element, sParent);
    css::uno::Reference<css::xml::dom::XNode> nodeMatch;
    if (xParent.is())
    {
        const ::rtl::OUString sLocale = getOfficeLocaleString();
        nodeMatch = matchFullLocale(xParent, sLocale);

        //office: en-DE, en, en-DE-altmark
        if (! nodeMatch.is())
        {
            const css::lang::Locale officeLocale = getOfficeLocale();
            nodeMatch = matchCountryAndLanguage(xParent, officeLocale);
            if ( ! nodeMatch.is())
            {
                nodeMatch = matchLanguage(xParent, officeLocale);
                if (! nodeMatch.is())
                    nodeMatch = getChildWithDefaultLocale(xParent);
            }
        }
    }

    return nodeMatch;
}

css::uno::Reference<css::xml::dom::XNode>
DescriptionInfoset::matchFullLocale(css::uno::Reference< css::xml::dom::XNode >
                                    const & xParent, ::rtl::OUString const & sLocale) const
{
    OSL_ASSERT(xParent.is());
    const ::rtl::OUString exp1(
        ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("*[@lang=\""))
        + sLocale +
        ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("\"]")));
    return m_xpath->selectSingleNode(xParent, exp1);
}

css::uno::Reference<css::xml::dom::XNode>
DescriptionInfoset::matchCountryAndLanguage(
    css::uno::Reference< css::xml::dom::XNode > const & xParent, css::lang::Locale const & officeLocale) const
{
    OSL_ASSERT(xParent.is());
    css::uno::Reference<css::xml::dom::XNode> nodeMatch;

    if (officeLocale.Country.getLength())
    {
        const ::rtl::OUString sLangCountry(officeLocale.Language +
            ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("-")) +
            officeLocale.Country);
        //first try exact match for lang-country
        const ::rtl::OUString exp1(
            ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("*[@lang=\""))
            + sLangCountry +
            ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("\"]")));
        nodeMatch = m_xpath->selectSingleNode(xParent, exp1);

        //try to match in strings that also have a variant, for example en-US matches in
        //en-US-montana
        if (!nodeMatch.is())
        {
            const ::rtl::OUString exp2(
                ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("*[starts-with(@lang,\""))
                + sLangCountry +
                ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("-\")]")));
            nodeMatch = m_xpath->selectSingleNode(xParent, exp2);
        }
    }

    return nodeMatch;
}


css::uno::Reference<css::xml::dom::XNode>
DescriptionInfoset::matchLanguage(
    css::uno::Reference< css::xml::dom::XNode > const & xParent, css::lang::Locale const & officeLocale) const
{
    OSL_ASSERT(xParent.is());
    css::uno::Reference<css::xml::dom::XNode> nodeMatch;

    //first try exact match for lang
    const ::rtl::OUString exp1(
        ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("*[@lang=\""))
        + officeLocale.Language
        + ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("\"]")));
    nodeMatch = m_xpath->selectSingleNode(xParent, exp1);

    //try to match in strings that also have a country and/orvariant, for example en  matches in
    //en-US-montana, en-US, en-montana
    if (!nodeMatch.is())
    {
        const ::rtl::OUString exp2(
            ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("*[starts-with(@lang,\""))
            + officeLocale.Language
            + ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("-\")]")));
        nodeMatch = m_xpath->selectSingleNode(xParent, exp2);
    }
    return nodeMatch;
}

css::uno::Reference<css::xml::dom::XNode>
DescriptionInfoset::getChildWithDefaultLocale(css::uno::Reference< css::xml::dom::XNode >
                                    const & xParent) const
{
    OSL_ASSERT(xParent.is());
    if (xParent->getNodeName().equals(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("simple-license"))))
    {
        css::uno::Reference<css::xml::dom::XNode> nodeDefault =
            m_xpath->selectSingleNode(xParent, ::rtl::OUString(
                RTL_CONSTASCII_USTRINGPARAM("@default-license-id")));
        if (nodeDefault.is())
        {
            //The old way
            const ::rtl::OUString exp1(
                ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("desc:license-text[@license-id = \""))
                + nodeDefault->getNodeValue()
                + ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("\"]")));
            return m_xpath->selectSingleNode(xParent, exp1);
        }
    }

    const ::rtl::OUString exp2(RTL_CONSTASCII_USTRINGPARAM("*[1]"));
    return m_xpath->selectSingleNode(xParent, exp2);
}

::rtl::OUString DescriptionInfoset::getLocalizedHREFAttrFromChild(
    ::rtl::OUString const & sXPathParent, bool * out_bParentExists)
    const
{
    css::uno::Reference< css::xml::dom::XNode > node =
        getLocalizedChild(sXPathParent);

    ::rtl::OUString sURL;
    if (node.is())
    {
        if (out_bParentExists)
            *out_bParentExists = true;
        const ::rtl::OUString exp(RTL_CONSTASCII_USTRINGPARAM("@xlink:href"));
        css::uno::Reference< css::xml::dom::XNode > xURL = m_xpath->selectSingleNode(node, exp);
        OSL_ASSERT(xURL.is());
        if (xURL.is())
            sURL = xURL->getNodeValue();
    }
    else
    {
        if (out_bParentExists)
            *out_bParentExists = false;
    }
    return sURL;
}

}
