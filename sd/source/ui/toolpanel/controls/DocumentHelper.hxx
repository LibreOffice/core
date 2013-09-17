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

#ifndef SD_TOOLPANEL_CONTROLS_DCUMENT_HELPER_HXX
#define SD_TOOLPANEL_CONTROLS_DCUMENT_HELPER_HXX

#include <rtl/ustring.hxx>
#include <tools/solar.h>
#include <boost/shared_ptr.hpp>
#include <vector>

class SdDrawDocument;
class SdPage;

namespace sd { namespace toolpanel { namespace controls {

/** A collection of methods supporting the handling of master pages.
*/
class DocumentHelper
{
public:
    /** Return a copy of the given master page in the given document.
    */
    static SdPage* CopyMasterPageToLocalDocument (
        SdDrawDocument& rTargetDocument,
        SdPage* pMasterPage);

    /** Return and, when not yet present, create a slide that uses the given
        masster page.
    */
    static SdPage* GetSlideForMasterPage (SdPage* pMasterPage);

    /** Copy the styles used by the given page from the source document to
        the target document.
    */
    static void ProvideStyles (
        SdDrawDocument& rSourceDocument,
        SdDrawDocument& rTargetDocument,
        SdPage* pPage);

    /** Assign the given master page to the list of pages.
        @param rTargetDocument
            The document that is the owner of the pages in rPageList.
        @param pMasterPage
            This master page will usually be a member of the list of all
            available master pages as provided by the MasterPageContainer.
        @param rPageList
            The pages to which to assign the master page.  These pages may
            be slides or master pages themselves.
    */
    static void AssignMasterPageToPageList (
        SdDrawDocument& rTargetDocument,
        SdPage* pMasterPage,
        const ::boost::shared_ptr<std::vector<SdPage*> >& rPageList);

private:
    static SdPage* AddMasterPage (
        SdDrawDocument& rTargetDocument,
        SdPage* pMasterPage);
    static SdPage* AddMasterPage (
        SdDrawDocument& rTargetDocument,
        SdPage* pMasterPage,
        sal_uInt16 nInsertionIndex);
    static SdPage* ProvideMasterPage (
        SdDrawDocument& rTargetDocument,
        SdPage* pMasterPage,
        const ::boost::shared_ptr<std::vector<SdPage*> >& rpPageList);

    /** Assign the given master page to the given page.
        @param pMasterPage
            In contrast to AssignMasterPageToPageList() this page is assumed
            to be in the target document, i.e. the same document that pPage
            is in.  The caller will usually call AddMasterPage() to create a
            clone of a master page in a another document to create it.
        @param rsBaseLayoutName
            The layout name of the given master page.  It is given so that
            it has not to be created on every call.  It could be generated
            from the given master page, though.
        @param pPage
            The page to which to assign the master page.  It can be a slide
            or a master page itself.
    */
    static void AssignMasterPageToPage (
        SdPage* pMasterPage,
        const OUString& rsBaseLayoutName,
        SdPage* pPage);
};


} } } // end of namespace ::sd::toolpanel::controls

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
