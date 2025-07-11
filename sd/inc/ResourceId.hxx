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
#include "sddllapi.h"
#include <com/sun/star/util/URL.hpp>
#include <com/sun/star/drawing/framework/AnchorBindingMode.hpp>
#include <cppuhelper/implbase.hxx>
#include <rtl/ref.hxx>
#include <vector>
#include <memory>

namespace com::sun::star::util { class XURLTransformer; }
namespace com::sun::star::uno { template <class interface_type> class WeakReference; }

namespace sd::framework {

// avoid LNK2005 duplicate WeakImplHelper definitions with MSVC
class SAL_DLLPUBLIC_TEMPLATE ResourceId_Base : public cppu::WeakImplHelper<>
{
};

/** A resource id uses a set of URLs to unambiguously specify a resource of
    the drawing framework.
    <p>Resources of the drawing framework are panes, views, tool bars, and
    command groups.  One URL describes the type of the actual resource.  A
    sequence of URLs (typically one, sometimes two) specifies its anchor,
    the resource it is bound to.  The anchor typically is a pane (for
    views), or it is empty (for panes).</p>
    <p>The resource URL may be empty.  In this case the anchor is empty,
    too.  Such an empty resource id does not describe a resource but rather
    the absence of one.  Instead of an empty ResourceId object
    an empty reference can be used in many places.</p>
    <p>The resource URL may have arguments that are passed to the factory
    method on its creation.  Arguments are only available through the
    getFullResourceURL().  The getResourceURL() method strips them away.</p>
*/
class SD_DLLPUBLIC ResourceId final : public ResourceId_Base
{
public:
    /** Create a new, empty resource id.
    */
    ResourceId();

    /** Create a new resource id that has an empty anchor.
        @param rsResourceURL
            When this resource URL is empty then the resulting ResourceId
            object is identical to when the ResourceId() default constructor
            had been called.
    */
    ResourceId (
        const OUString& rsResourceURL);

    /** Create a resource id for an anchor that is given as
        ResourceId object.  This is the most general of the
        constructor variants.
    */
    ResourceId(const OUString& sResourceURL, const rtl::Reference<ResourceId>& xAnchor);

    /** Create a new resource id for the given resource type and an anchor
        that is specified by a single URL.  This constructor can be used for
        example for views that are bound to panes.
        @param rsResourceURL
            The URL of the actual resource.
        @param rsAnchorURL
            The single URL of the anchor.
    */
    ResourceId (
        const OUString& rsResourceURL,
        const OUString& rsAnchorURL);

    virtual ~ResourceId() override;

    /** Return the URL of the resource.  Arguments supplied on creation are
        stripped away.  Use getFullResourceURL() to access them.
    */
    OUString getResourceURL() const;

    /** Return a URL object of the resource URL that may contain arguments.
    */
    css::util::URL getFullResourceURL();

    /** Return whether there is a non-empty anchor URL.  When this method
        returns `FALSE` then getAnchorURLs() will return an empty list.
    */
    bool hasAnchor() const;

    /** Return a new ResourceId that represents the anchor resource.
    */
    rtl::Reference<ResourceId> getAnchor() const;

    /** Return the, possibly empty, list of anchor URLs.  The URLs are
        ordered so that the one in position 0 is the direct anchor of the
        resource, while the one in position i+1 is the direct anchor of the
        one in position i.
    */
    std::vector<OUString> getAnchorURLs() const;

    /** Return the type prefix of the resource URL.  This includes all up to
        and including the second slash.
    */
    OUString getResourceTypePrefix() const;

    /** Compare the called ResourceId object with the given
        one.
        <p>The two resource ids A and B are compared so that if A<B (return
        value is -1) then either A and B are unrelated or A is a direct or
        indirect anchor of B.</p>
        <p>The algorithm for this comparison is quite simple. It uses a
        double lexicographic ordering.  On the lower level individual URLs
        are compared via the lexicographic order defined on strings.  On the
        higher level two resource ids are compared via a lexicographic order
        defined on the URLS.  So when there are two resource ids A1.A2
        (A1 being the anchor of A2) and B1.B2 then A1.A2<B1.B2 when A1<B1 or
        A1==B1 and A2<B2.  Resource ids may have different lengths: A1 <
        B1.B2 when A1<B1 or A1==B1 (anchors first then resources linked to them.</p>
        @param xId
            The resource id to which the called resource id is compared.
        @return
            Returns 0 when the called resource id is
            equivalent to the given resource id. Returns <code>-1</code> or
            <code>+1</code> when the two compared resource ids differ.
    */
    sal_Int16 compareTo(const rtl::Reference<ResourceId>& rxResourceId) const;

    /** Return whether the anchor of the called resource id object
        represents the same resource as the given object.
        <p>Note that not only the anchor of the given object is taken into
        account. The whole object, including the resource URL, is
        interpreted as anchor resource.</p>
        @param xAnchorId
            The resource id of the anchor.
        @param eMode
            This mode specifies how the called resource has to be bound to
            the given anchor in order to have this function return `TRUE`.
            <p>If eMode is DIRECT then the anchor of the called resource id
            has to be identical to the given anchor. If eMode is
            INDIRECT then the given anchor has to be a part
            of the anchor of the called resource.
    */
    bool
        isBoundTo (
            const rtl::Reference<ResourceId>& rxResourceId,
            css::drawing::framework::AnchorBindingMode eMode) const;

    /** Return whether the anchor of the called resource id object
        represents the same resource as the given anchor URL. This is a
        convenience variant of the isBoundTo() function
        that can also be seen as an optimization for the case that the
        anchor consists of exactly one URL.
        @param AnchorURL
            The resource URL of the anchor.
        @param eMode
            This mode specifies how the called resource has to be bound to
            the given anchor in order to have this function return. See the
            description of isBoundTo() for more
            information.
    */
    bool
        isBoundToURL (
            const OUString& rsAnchorURL,
            css::drawing::framework::AnchorBindingMode eMode) const;

private:
    /** The set of URLs that consist of the resource URL at index 0 and the
        anchor URLs and indices 1 and above.
    */
    std::vector<OUString> maResourceURLs;

    std::unique_ptr<css::util::URL> mpURL;

    static css::uno::WeakReference<css::util::XURLTransformer> mxURLTransformerWeak;

    /** Compare the called ResourceId object to the given ResourceId object.
        This uses the implementation of both objects to speed up the
        comparison.
    */
    sal_Int16 CompareToLocalImplementation (const ResourceId& rId) const;

    /** Return whether the called ResourceId object is bound to the anchor
        consisting of the URLs given by psFirstAnchorURL.
        @param psFirstAnchorURL
            Optional first URL of the anchor. This can be missing or present
            independently of paAnchorURLs.
        @param eMode
            This specifies whether the called resource has to be directly
            bound to the given anchor in order to return <TRUE/> or whether
            it can be bound indirectly, too.
    */
    bool IsBoundToAnchor (
        const OUString* psFirstAnchorURL,
        css::drawing::framework::AnchorBindingMode eMode) const;

    /** Return whether the called ResourceId object is bound to the anchor
        consisting of the URLs in rResourceURLs.
        @param rResourceURLs
            A possibly empty list of anchor URLs.
        @param eMode
            This specifies whether the called resource has to be directly
            bound to the given anchor in order to return <TRUE/> or whether
            it can be bound indirectly, too.
    */
    bool IsBoundToAnchor (
        const ::std::vector<OUString>& rResourceURLs,
        css::drawing::framework::AnchorBindingMode eMode) const;

    void ParseResourceURL();
};

} // end of namespace sd::framework

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
