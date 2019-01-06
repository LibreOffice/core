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

#ifndef INCLUDED_SD_SOURCE_UI_SIDEBAR_MASTERPAGEDESCRIPTOR_HXX
#define INCLUDED_SD_SOURCE_UI_SIDEBAR_MASTERPAGEDESCRIPTOR_HXX

#include "MasterPageContainer.hxx"
#include <memory>

namespace sd { class PreviewRenderer; }
class SdDrawDocument;

namespace sd { namespace sidebar {

class PageObjectProvider;
class PreviewProvider;

class MasterPageDescriptor;
typedef std::shared_ptr<MasterPageDescriptor> SharedMasterPageDescriptor;

/** A collection of data that is stored for every master page in the
    MasterpageContainer.
*/
class MasterPageDescriptor
{
public:
    MasterPageDescriptor (
        MasterPageContainer::Origin eOrigin,
        const sal_Int32 nTemplateIndex,
        const OUString& rURL,
        const OUString& rPageName,
        const OUString& rStyleName,
        const bool bIsPrecious,
        const std::shared_ptr<PageObjectProvider>& rpPageObjectProvider,
        const std::shared_ptr<PreviewProvider>& rpPreviewProvider);

    void SetToken (MasterPageContainer::Token aToken);

    /** Update the called MasterPageDescriptor object with values from the
        given one.  Only those values are updated that have default values
        in the called object and that have non-default values in the given
        one.
        @return
            Returns a list of event types for which event notifications have
            to be sent to listeners.  The list may be empty or NULL.
    */
    ::std::unique_ptr<std::vector<MasterPageContainerChangeEvent::EventType> >
        Update (
            const MasterPageDescriptor& rDescriptor);

    /** This convenience method returns either a small or a large preview,
        depending on the given size specifier.
        Note that the previews are not created when they are not present.
        @return
            The returned preview may be empty.
    */
    const Image& GetPreview (MasterPageContainer::PreviewSize ePreviewSize) const;

    /** Use the PreviewProvider to get access to a preview of the master
        page.

        Note that this is only done, when either bForce is <TRUE/> or
        the PreviewProvider::GetCostIndex() returns 0.

        The small preview is created by scaling the large one, not by
        calling PreviewProvider::operator() a second time.

        It is the responsibility of the caller to call UpdatePageObject()
        before calling this method  when the PreviewProvider can only work
        when the master page object is present, i.e. its NeedsPageObject()
        method returns <TRUE/>.

        @param nCostThreshold
            When this is zero or positive then the preview is created only
            when the preview provider has a cost equal to or smaller than
            this threshold.  A negative value forces the preview to be
            created, regardless of the cost.
        @param rSmallSize
            Size of the small preview.
        @param rLargeSize
            Size of the large preview.
        @param rRenderer
            A PreviewRenderer object that may be used to create a preview.
        @return
            When the previews are successfully provided then <TRUE/> is
            returned.
    */
    bool UpdatePreview (
        sal_Int32 nCostThreshold,
        const Size& rSmallSize,
        const Size& rLargeSize,
        ::sd::PreviewRenderer& rRenderer);

    /** Use the PageObjectProvider to get access to the master page object.

        Note that this is only done, when either bForce is <TRUE/> or the
        PreviewProvider::GetCostIndex() returns 0.

        @param nCostThreshold
            When this is zero or positive then the page object is created
            only when the page object provider has a cost equal to or
            smaller than this threshold.  A negative value forces the
            page object be created, regardless of the cost.
        @param pDocument
            This document of the MasterPageContainer may be used to create
            a page object with or store one in.
        @return
            When the master page object is successfully provided then
            1 is returned, on no change then a 0 is provided,
            on a masterpage-error a -1 is provided.
    */
    int UpdatePageObject (
        sal_Int32 nCostThreshold,
        SdDrawDocument* pDocument);

    enum URLClassification {
        URLCLASS_USER,
        URLCLASS_LAYOUT,
        URLCLASS_PRESENTATION,
        URLCLASS_OTHER,
        URLCLASS_UNKNOWN,
        URLCLASS_UNDETERMINED
    };

    URLClassification GetURLClassification();

    /** The Token under which the MasterPageContainer gives access to the
        object.
    */
    MasterPageContainer::Token maToken;

    /** A rough specification of the origin of the master page.
    */
    MasterPageContainer::Origin meOrigin;

    /** The URL is not empty for master pages loaded from a template
        document.
    */
    OUString msURL;

    /** Taken from the title of the template file.
    */
    OUString msPageName;

    /** Taken from the master page object.
    */
    OUString msStyleName;

    const bool mbIsPrecious;

    /** The actual master page.
    */
    SdPage* mpMasterPage;

    /** A slide that uses the master page.
    */
    SdPage* mpSlide;

    /** A small (the default size) preview of the master page.  May be
        empty.  When this smaller preview is not empty then the larger one
        is not empty, too.
    */
    Image maSmallPreview;

    /** A large preview of the master page.  May be empty.  When this larger
        preview is not empty then the smaller one is not empty, too.
    */
    Image maLargePreview;

    /** The preview provider. May be empty.  May be replaced during the
        lifetime of a MasterPageDescriptor object.
    */
    std::shared_ptr<PreviewProvider> mpPreviewProvider;

    /** The master page provider.  May be empty.  May be replaced during
        the lifetime of a MasterPageDescriptor object.
    */
    std::shared_ptr<PageObjectProvider> mpPageObjectProvider;

    /** This index represents the order in which templates are provided via
        the TemplateScanner.  It defines the order in which the entries in
        the AllMasterPagesSelector are displayed.  The default value is -1.
    */
    sal_Int32 mnTemplateIndex;

    URLClassification meURLClassification;

    sal_Int32 mnUseCount;

    class URLComparator { public:
        OUString const msURL;
        explicit URLComparator (const OUString& sURL);
        bool operator() (const SharedMasterPageDescriptor& rDescriptor);
    };
    class StyleNameComparator { public:
        OUString const msStyleName;
        explicit StyleNameComparator (const OUString& sStyleName);
        bool operator() (const SharedMasterPageDescriptor& rDescriptor);
    };
    class PageObjectComparator { public:
        const SdPage* mpMasterPage;
        explicit PageObjectComparator (const SdPage* pPageObject);
        bool operator() (const SharedMasterPageDescriptor& rDescriptor);
    };
    class AllComparator { public:
        explicit AllComparator(const SharedMasterPageDescriptor& rDescriptor);
        bool operator() (const SharedMasterPageDescriptor& rDescriptor);
    private:
        SharedMasterPageDescriptor const mpDescriptor;
    };

};

} } // end of namespace sd::sidebar

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
