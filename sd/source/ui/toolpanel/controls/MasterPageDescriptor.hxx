/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#ifndef SD_TOOLPANEL_CONTROLS_MASTER_PAGE_DESCRIPTOR_HXX
#define SD_TOOLPANEL_CONTROLS_MASTER_PAGE_DESCRIPTOR_HXX

#include "MasterPageContainer.hxx"
#include <boost/shared_ptr.hpp>

namespace sd { namespace toolpanel { namespace controls {

class PageObjectProvider;
class PreviewProvider;

class MasterPageDescriptor;
typedef ::boost::shared_ptr<MasterPageDescriptor> SharedMasterPageDescriptor;

/** A collection of data that is stored for every master page in the
    MasterpageContainer.
*/
class MasterPageDescriptor
{
public:
    MasterPageDescriptor (
        MasterPageContainer::Origin eOrigin,
        const sal_Int32 nTemplateIndex,
        const String& rURL,
        const String& rPageName,
        const String& rStyleName,
        const bool bIsPrecious,
        const ::boost::shared_ptr<PageObjectProvider>& rpPageObjectProvider,
        const ::boost::shared_ptr<PreviewProvider>& rpPreviewProvider);
    MasterPageDescriptor (const MasterPageDescriptor& rDescriptor);
    ~MasterPageDescriptor (void);

    void SetToken (MasterPageContainer::Token aToken);

    /** Update the called MasterPageDescriptor object with values from the
        given one.  Only those values are updated that have default values
        in the called object and that have non-default values in the given
        one.
        @return
            Returns a list of event types for which event notifications have
            to be sent to listeners.  The list may be empty or NULL.
    */
    ::std::auto_ptr<std::vector<MasterPageContainerChangeEvent::EventType> >
        Update (
            const MasterPageDescriptor& rDescriptor);

    /** This convenience method returns either a small or a large preview,
        depending on the given size specifier.
        Note that the previews are not created when they are not present.
        @return
            The returned preview may be empty.
    */
    Image GetPreview (MasterPageContainer::PreviewSize ePreviewSize) const;

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
            <TRUE/> is returned.
    */
    bool UpdatePageObject (
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

    URLClassification GetURLClassification (void);

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
    ::rtl::OUString msURL;

    /** Taken from the title of the template file.
    */
    ::rtl::OUString msPageName;

    /** Taken from the master page object.
    */
    ::rtl::OUString msStyleName;

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

    /** The prewview provider. May be empty.  May be replaced during the
        lifetime of a MasterPageDescriptor object.
    */
    ::boost::shared_ptr<PreviewProvider> mpPreviewProvider;

    /** The master page provider.  May be empty.  May be replaced during
        the lifetime of a MasterPageDescriptor object.
    */
    ::boost::shared_ptr<PageObjectProvider> mpPageObjectProvider;

    /** This index represents the order in which templates are provided via
        the TemplateScanner.  It defines the order in which the entries in
        the AllMasterPagesSelector are displayed.  The default value is -1.
    */
    sal_Int32 mnTemplateIndex;

    URLClassification meURLClassification;

    sal_Int32 mnUseCount;

    class URLComparator { public:
        ::rtl::OUString msURL;
        URLComparator (const ::rtl::OUString& sURL);
        bool operator() (const SharedMasterPageDescriptor& rDescriptor);
    };
    class StyleNameComparator { public:
        ::rtl::OUString msStyleName;
        StyleNameComparator (const ::rtl::OUString& sStyleName);
        bool operator() (const SharedMasterPageDescriptor& rDescriptor);
    };
    class PageObjectComparator { public:
        const SdPage* mpMasterPage;
        PageObjectComparator (const SdPage* pPageObject);
        bool operator() (const SharedMasterPageDescriptor& rDescriptor);
    };
    class AllComparator { public:
        AllComparator(const SharedMasterPageDescriptor& rDescriptor);
        bool operator() (const SharedMasterPageDescriptor& rDescriptor);
    private:
        SharedMasterPageDescriptor mpDescriptor;
    };


};


} } } // end of namespace ::sd::toolpanel::controls

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
