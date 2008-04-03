/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: DocumentHelper.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: kz $ $Date: 2008-04-03 14:49:11 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
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
        USHORT nInsertionIndex);
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
