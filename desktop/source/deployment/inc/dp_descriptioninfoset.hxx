/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#pragma once

#include <sal/config.h>

#include <optional>
#include <string_view>

#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/uno/Sequence.hxx>
#include <sal/types.h>
#include "dp_misc_api.hxx"

/// @HTML

namespace com::sun::star {
    namespace uno { class XComponentContext; }
    namespace xml {
        namespace dom {
            class XNode;
            class XNodeList;
        }
        namespace xpath { class XXPathAPI; }
    }
}

namespace dp_misc {

struct DESKTOP_DEPLOYMENTMISC_DLLPUBLIC SimpleLicenseAttributes
{
    OUString acceptBy;
    //Attribute suppress-on-update. Default is false.
    bool suppressOnUpdate;
    //Attribute suppress-if-required. Default is false.
    bool suppressIfRequired;
};


/**
   Access to the content of an XML <code>description</code> element.

   <p>This works for <code>description</code> elements in both the
   <code>description.xml</code> file and online update information formats.</p>
*/
class DESKTOP_DEPLOYMENTMISC_DLLPUBLIC DescriptionInfoset {
public:
    /**
       Create an instance.

       @param context
       a non-null component context

       @param element
       a <code>description</code> element; may be null (equivalent to an element
       with no content)
    */
    DescriptionInfoset(
        css::uno::Reference< css::uno::XComponentContext > const & context,
        css::uno::Reference< css::xml::dom::XNode > const & element);

    ~DescriptionInfoset();

    /**
       Return the identifier.

       @return
       the identifier, or an empty <code>optional</code> if none is specified
    */
    ::std::optional< OUString > getIdentifier() const;

    /**
       Return the textual version representation.

       @return
       textual version representation
    */
    OUString getVersion() const;

    /**
        Returns a list of supported platforms.

        If the extension does not specify a platform by leaving out the platform element
        then we assume that the extension supports all platforms. In this case the returned
        sequence will have one element, which is &quot;all&quot;.
        If the platform element is present but does not specify a platform then an empty
        sequence is returned. Examples for invalid platform elements:
        <pre>
            <platform />, <platform value="" />, <platform value=",">
        </pre>

        The value attribute can contain various platform tokens. They must be separated by
        commas.Each token will be stripped from leading and trailing white space (trim()).
    */
    css::uno::Sequence< OUString > getSupportedPlatforms() const;

    /**
        Returns the localized publisher name and the corresponding URL.

        In case there is no publisher element then a pair of two empty strings is returned.
    */
    std::pair< OUString, OUString > getLocalizedPublisherNameAndURL() const;

    /**
        Returns the URL for the release notes corresponding to the office's locale.

        In case there is no release-notes element then an empty string is returned.
    */
    OUString getLocalizedReleaseNotesURL() const;

    /** returns the relative path to the license file.

        In case there is no simple-license element then an empty string is returned.
    */
    OUString getLocalizedLicenseURL() const;

    /** returns the attributes of the simple-license element

        As long as there is a simple-license element, the function will return
        the structure. If it does not exist, then the optional object is uninitialized.
    */
    ::std::optional<SimpleLicenseAttributes> getSimpleLicenseAttributes() const;

    /** returns the localized display name of the extensions.

        In case there is no localized display-name then an empty string is returned.
    */
    OUString getLocalizedDisplayName() const;

    /**
        returns the download website URL from the update information.

        There can be multiple URLs where each is assigned to a particular locale.
        The function returns the URL which locale matches best the one used in the office.

        The return value is an optional because it may be necessary to find out if there
        was a value provided or not. This is necessary to flag the extension in the update dialog
        properly as "browser based update". The return value will only then not be initialized
        if there is no <code>&lt;update-website&gt;</code>. If the element exists, then it must
        have at least one child element containing a URL.

        The <code>&lt;update-website&gt;</code> and <code>&lt;update-download&gt;</code>
        elements are mutually exclusive.

        @return
        the download website URL, or an empty <code>optional</code> if none is
        specified
    */
    ::std::optional< OUString > getLocalizedUpdateWebsiteURL() const;

    /** returns the relative URL to the description.

       The URL is relative to the root directory of the extensions.
    */
    OUString getLocalizedDescriptionURL() const;
    /**
       Return the dependencies.

       @return
       dependencies; will never be null
    */
    css::uno::Reference< css::xml::dom::XNodeList >
    getDependencies() const;

    /**
       Return the update information URLs.

       @return
       update information URLs
    */
    css::uno::Sequence< OUString > getUpdateInformationUrls() const;

     /**
        Return the download URLs from the update information.

        Because the <code>&lt;update-download&gt;</code> and the <code>&lt;update-website&gt;</code>
        elements are mutually exclusive one may need to determine exactly if the element
        was provided.

        @return
        download URLs
     */
    css::uno::Sequence< OUString > getUpdateDownloadUrls() const;

    /**
        Returns the URL for the icon image.
    */
    OUString getIconURL( bool bHighContrast ) const;

    bool hasDescription() const;

private:
    SAL_DLLPRIVATE ::std::optional< OUString > getOptionalValue(
        OUString const & expression) const;

    SAL_DLLPRIVATE css::uno::Sequence< OUString > getUrls(
        OUString const & expression) const;

    /** Retrieves a child element which as lang attribute which matches the office locale.

        Only top-level children are taken into account. It is also assumed that they are all
        of the same element type and have a lang attribute. The matching algorithm is according
        to RFC 3066, with the exception that only one variant is allowed.
        @param parent
        the expression used to obtain the parent of the localized children. It can be null.
        Then a null reference is returned.
    */
    SAL_DLLPRIVATE css::uno::Reference< css::xml::dom::XNode >
        getLocalizedChild( OUString const & sParent) const;
    SAL_DLLPRIVATE  css::uno::Reference< css::xml::dom::XNode>
        matchLanguageTag(
        css::uno::Reference< css::xml::dom::XNode > const & xParent,
        std::u16string_view rTag) const;

    /** If there is no child element with a locale matching the office locale, then we use
        the first child. In the case of the simple-license we also use the former default locale, which
        was determined by the default-license-id (/description/registration/simple-license/@default-license-id)
        and the license-id attributes (/description/registration/simple-license/license-text/@license-id).
        However, since OOo 2.4 we use also the first child as default for the license
        unless the two attributes are present.
    */
    SAL_DLLPRIVATE  css::uno::Reference< css::xml::dom::XNode>
        getChildWithDefaultLocale(
        css::uno::Reference< css::xml::dom::XNode > const & xParent) const;
    /**
        @param out_bParentExists
            indicates if the element node specified in sXPathParent exists.
    */
    SAL_DLLPRIVATE OUString getLocalizedHREFAttrFromChild(
        OUString const & sXPathParent, bool * out_bParentExists) const;

    /** Gets the node value for a given expression. The expression is used in
        m_xpath-selectSingleNode. The value of the returned node is return value
        of this function.
    */
    SAL_DLLPRIVATE OUString
        getNodeValueFromExpression(OUString const & expression) const;

    /** Check the extensions denylist if additional extension meta data (e.g. dependencies)
        are defined for this extension and have to be taken into account.
    */
    SAL_DLLPRIVATE void
        checkDenylist() const;

    /** Helper method to compare the versions with the current version
     */
    SAL_DLLPRIVATE static bool
        checkDenylistVersion(std::u16string_view currentversion,
                              css::uno::Sequence< OUString > const & versions);

    css::uno::Reference< css::uno::XComponentContext > m_context;
    css::uno::Reference< css::xml::dom::XNode >        m_element;
    css::uno::Reference< css::xml::xpath::XXPathAPI >  m_xpath;
};

inline  bool DescriptionInfoset::hasDescription() const
{
    return m_element.is();
}

/** creates a DescriptionInfoset object.

    The argument sExtensionFolderURL is a file URL to extension folder containing
    the description.xml.
 */
DESKTOP_DEPLOYMENTMISC_DLLPUBLIC
DescriptionInfoset getDescriptionInfoset(std::u16string_view sExtensionFolderURL);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
