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

#ifndef SD_TOOLPANEL_CONTROLS_DCUMENT_HELPER_HXX
#define SD_TOOLPANEL_CONTROLS_DCUMENT_HELPER_HXX

#include <tools/solar.h>
#include <boost/shared_ptr.hpp>
#include <vector>

class SdDrawDocument;
class SdPage;
class String;

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
        const String& rsBaseLayoutName,
        SdPage* pPage);
};


} } } // end of namespace ::sd::toolpanel::controls

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
