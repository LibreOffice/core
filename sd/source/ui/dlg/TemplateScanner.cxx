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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sd.hxx"

#include "TemplateScanner.hxx"

#include <comphelper/processfactory.hxx>
#include <comphelper/documentconstants.hxx>

#include <tools/debug.hxx>
#include <vos/mutex.hxx>
#include <vcl/svapp.hxx>
#include <com/sun/star/frame/XDocumentTemplates.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/ucb/XCommandEnvironment.hpp>
#include <com/sun/star/ucb/XContentAccess.hpp>
#include <com/sun/star/sdbc/XResultSet.hpp>
#include <com/sun/star/sdbc/XRow.hpp>

#include <set>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;

namespace {

const ::rtl::OUString TITLE            = ::rtl::OUString::createFromAscii ("Title");
const ::rtl::OUString TARGET_DIR_URL   = ::rtl::OUString::createFromAscii ("TargetDirURL");
const ::rtl::OUString DESCRIPTION      = ::rtl::OUString::createFromAscii ("TypeDescription");
const ::rtl::OUString TARGET_URL       = ::rtl::OUString::createFromAscii ("TargetURL");

const ::rtl::OUString DOCTEMPLATES     = ::rtl::OUString::createFromAscii ("com.sun.star.frame.DocumentTemplates");

//  These strings are used to find impress templates in the tree of
//  template files.  Should probably be determined dynamically.
const ::rtl::OUString IMPRESS_BIN_TEMPLATE = ::rtl::OUString::createFromAscii ("application/vnd.stardivision.impress");
const ::rtl::OUString IMPRESS_XML_TEMPLATE = MIMETYPE_VND_SUN_XML_IMPRESS;
// The following id comes from the bugdoc in #i2764#.
const ::rtl::OUString IMPRESS_XML_TEMPLATE_B = ::rtl::OUString::createFromAscii ("Impress 2.0");
const ::rtl::OUString IMPRESS_XML_TEMPLATE_OASIS = MIMETYPE_OASIS_OPENDOCUMENT_PRESENTATION;


class FolderDescriptor
{
public:
    FolderDescriptor (
        int nPriority,
        const ::rtl::OUString& rsTitle,
        const ::rtl::OUString& rsTargetDir,
        const ::rtl::OUString& rsContentIdentifier,
        const Reference<com::sun::star::ucb::XCommandEnvironment>& rxFolderEnvironment)
        : mnPriority(nPriority),
          msTitle(rsTitle),
          msTargetDir(rsTargetDir),
          msContentIdentifier(rsContentIdentifier),
          mxFolderEnvironment(rxFolderEnvironment)
    { }
    int mnPriority;
    ::rtl::OUString msTitle;
    ::rtl::OUString msTargetDir;
    ::rtl::OUString msContentIdentifier;
    //    Reference<sdbc::XResultSet> mxFolderResultSet;
    Reference<com::sun::star::ucb::XCommandEnvironment> mxFolderEnvironment;

    class Comparator { public:
        bool operator() (const FolderDescriptor& r1, const FolderDescriptor& r2)
        { return r1.mnPriority < r2.mnPriority; }
    };
};

/** Use a heuristic based on the URL of a top-level template folder to
    assign a priority that is used to sort the folders.
*/
int Classify (const ::rtl::OUString&, const ::rtl::OUString& rsURL)
{
    int nPriority (0);

    if (rsURL.getLength() == 0)
        nPriority = 100;
    else if (rsURL.indexOf(::rtl::OUString::createFromAscii("presnt"))>=0)
    {
        nPriority = 30;
    }
    else if (rsURL.indexOf(::rtl::OUString::createFromAscii("layout"))>=0)
    {
        nPriority = 20;
    }
    else if (rsURL.indexOf(::rtl::OUString::createFromAscii("educate"))>=0)
    {
        nPriority = 40;
    }
    else if (rsURL.indexOf(::rtl::OUString::createFromAscii("finance"))>=0)
    {
        nPriority = 40;
    }
    else
    {
        // All other folders are taken for user supplied and have the
        // highest priority.
        nPriority = 10;
    }

    return nPriority;
}

} // end of anonymous namespace




namespace sd
{

class TemplateScanner::FolderDescriptorList
    : public ::std::multiset<FolderDescriptor,FolderDescriptor::Comparator>
{
};

TemplateScanner::TemplateScanner (void)
    : meState(INITIALIZE_SCANNING),
      maFolderContent(),
      mpTemplateDirectory(NULL),
      maFolderList(),
      mpLastAddedEntry(NULL),
      mpFolderDescriptors(new FolderDescriptorList()),
      mxTemplateRoot(),
      mxFolderEnvironment(),
      mxEntryEnvironment(),
      mxFolderResultSet(),
      mxEntryResultSet()
{
    //  empty;
}




TemplateScanner::~TemplateScanner (void)
{
    mpFolderDescriptors.reset();

    // Delete all entries of the template list that have not been
    // transferred to another object.
    std::vector<TemplateDir*>::iterator I;
    for (I=maFolderList.begin(); I!=maFolderList.end(); I++)
        if (*I != NULL)
            delete *I;
}




TemplateScanner::State TemplateScanner::GetTemplateRoot (void)
{
    State eNextState (INITIALIZE_FOLDER_SCANNING);

    Reference<lang::XMultiServiceFactory> xFactory = ::comphelper::getProcessServiceFactory ();
    DBG_ASSERT (xFactory.is(), "TemplateScanner::GetTemplateRoot: xFactory is NULL");

    if (xFactory.is())
    {
        Reference<frame::XDocumentTemplates> xTemplates (
            xFactory->createInstance (DOCTEMPLATES), UNO_QUERY);
        DBG_ASSERT (xTemplates.is(), "TemplateScanner::GetTemplateRoot: xTemplates is NULL");

        if (xTemplates.is())
            mxTemplateRoot = xTemplates->getContent();
        else
            eNextState = ERROR;
    }
    else
        eNextState = ERROR;

    return eNextState;
}




TemplateScanner::State TemplateScanner::InitializeEntryScanning (void)
{
    State eNextState (SCAN_ENTRY);

    if (maFolderContent.isFolder())
    {
        mxEntryEnvironment = Reference<com::sun::star::ucb::XCommandEnvironment>();

        //  We are interested only in three properties: the entry's name,
        //  its URL, and its content type.
        Sequence<rtl::OUString> aProps (3);
        aProps[0] = TITLE;
        aProps[1] = TARGET_URL;
        aProps[2] = DESCRIPTION;

        //  Create a cursor to iterate over the templates in this folders.
        ::ucbhelper::ResultSetInclude eInclude = ::ucbhelper::INCLUDE_DOCUMENTS_ONLY;
        mxEntryResultSet = Reference<com::sun::star::sdbc::XResultSet>(
            maFolderContent.createCursor(aProps, eInclude));
    }
    else
        eNextState = ERROR;

    return eNextState;
}




TemplateScanner::State TemplateScanner::ScanEntry (void)
{
    State eNextState (ERROR);

    Reference<com::sun::star::ucb::XContentAccess> xContentAccess (mxEntryResultSet, UNO_QUERY);
    Reference<com::sun::star::sdbc::XRow> xRow (mxEntryResultSet, UNO_QUERY);

    if (xContentAccess.is() && xRow.is() && mxEntryResultSet.is())
    {
        if (mxEntryResultSet->next())
        {
            ::rtl::OUString sTitle (xRow->getString (1));
            ::rtl::OUString sTargetURL (xRow->getString (2));
            ::rtl::OUString sContentType (xRow->getString (3));

            ::rtl::OUString aId = xContentAccess->queryContentIdentifierString();
            ::ucbhelper::Content  aContent = ::ucbhelper::Content (aId, mxEntryEnvironment);
            if (aContent.isDocument ())
            {
                //  Check wether the entry is an impress template.  If so
                //  add a new entry to the resulting list (which is created
                //  first if necessary).
                if (    (sContentType == MIMETYPE_OASIS_OPENDOCUMENT_PRESENTATION_TEMPLATE)
                    ||  (sContentType == IMPRESS_XML_TEMPLATE_OASIS)
                    ||  (sContentType == IMPRESS_BIN_TEMPLATE)
                    ||  (sContentType == IMPRESS_XML_TEMPLATE)
                    ||  (sContentType == IMPRESS_XML_TEMPLATE_B))
                {
                    mpLastAddedEntry = new TemplateEntry(sTitle, sTargetURL);
                    mpTemplateDirectory->maEntries.push_back(mpLastAddedEntry);
                }
            }

            // Continue scanning entries.
            eNextState = SCAN_ENTRY;
        }
        else
        {
            if (mpTemplateDirectory->maEntries.empty())
            {
                delete mpTemplateDirectory;
                mpTemplateDirectory = NULL;
            }
            else
            {
                SolarMutexGuard aGuard;
                maFolderList.push_back(mpTemplateDirectory);
            }

            // Continue with scanning the next folder.
            eNextState = SCAN_FOLDER;
        }
    }

    return eNextState;
}




TemplateScanner::State TemplateScanner::InitializeFolderScanning (void)
{
    State eNextState (ERROR);

    mxFolderResultSet = Reference<sdbc::XResultSet>();

    try
    {
        //  Create content for template folders.
        mxFolderEnvironment = Reference<com::sun::star::ucb::XCommandEnvironment>();
        ::ucbhelper::Content aTemplateDir (mxTemplateRoot, mxFolderEnvironment);

        //  Define the list of properties we are interested in.
        Sequence<rtl::OUString> aProps (2);
        aProps[0] = TITLE;
        aProps[1] = TARGET_DIR_URL;

        //  Create an cursor to iterate over the template folders.
        ::ucbhelper::ResultSetInclude eInclude = ::ucbhelper::INCLUDE_FOLDERS_ONLY;
        mxFolderResultSet = Reference<sdbc::XResultSet>(
            aTemplateDir.createCursor(aProps, eInclude));
        if (mxFolderResultSet.is())
            eNextState = GATHER_FOLDER_LIST;
    }
    catch (::com::sun::star::uno::Exception&)
    {
       eNextState = ERROR;
    }

    return eNextState;
}




TemplateScanner::State TemplateScanner::GatherFolderList (void)
{
    State eNextState (ERROR);

    Reference<com::sun::star::ucb::XContentAccess> xContentAccess (mxFolderResultSet, UNO_QUERY);
    if (xContentAccess.is() && mxFolderResultSet.is())
    {
        while (mxFolderResultSet->next())
        {
            Reference<sdbc::XRow> xRow (mxFolderResultSet, UNO_QUERY);
            if (xRow.is())
            {
                ::rtl::OUString sTitle (xRow->getString (1));
                ::rtl::OUString sTargetDir (xRow->getString (2));
                ::rtl::OUString aId = xContentAccess->queryContentIdentifierString();

                mpFolderDescriptors->insert(
                    FolderDescriptor(
                        Classify(sTitle,sTargetDir),
                        sTitle,
                        sTargetDir,
                        aId,
                        mxFolderEnvironment));
            }
        }

        eNextState = SCAN_FOLDER;
    }

    return eNextState;
}




TemplateScanner::State TemplateScanner::ScanFolder (void)
{
    State eNextState (ERROR);

    if (mpFolderDescriptors->size() > 0)
    {
        FolderDescriptor aDescriptor (*mpFolderDescriptors->begin());
        mpFolderDescriptors->erase(mpFolderDescriptors->begin());

        ::rtl::OUString sTitle (aDescriptor.msTitle);
        ::rtl::OUString sTargetDir (aDescriptor.msTargetDir);
        ::rtl::OUString aId (aDescriptor.msContentIdentifier);

        maFolderContent = ::ucbhelper::Content (aId, aDescriptor.mxFolderEnvironment);
        if (maFolderContent.isFolder())
        {
            // Scan the folder and insert it into the list of template
            // folders.
            mpTemplateDirectory = new TemplateDir (sTitle, sTargetDir);
            if (mpTemplateDirectory != NULL)
            {
                // Continue with scanning all entries in the folder.
                eNextState = INITIALIZE_ENTRY_SCAN;
            }
        }
    }
    else
    {
        eNextState = DONE;
    }

    return eNextState;
}




void TemplateScanner::Scan (void)
{
    while (HasNextStep())
        RunNextStep();
}




std::vector<TemplateDir*>& TemplateScanner::GetFolderList (void)
{
    return maFolderList;
}




void TemplateScanner::RunNextStep (void)
{
    switch (meState)
    {
        case INITIALIZE_SCANNING:
            meState = GetTemplateRoot();
            break;

        case INITIALIZE_FOLDER_SCANNING:
            meState = InitializeFolderScanning();
            break;

        case SCAN_FOLDER:
            meState = ScanFolder();
            break;

        case GATHER_FOLDER_LIST:
            meState = GatherFolderList();
            break;

        case INITIALIZE_ENTRY_SCAN:
            meState = InitializeEntryScanning();
            break;

        case SCAN_ENTRY:
            meState = ScanEntry();
            break;
        default:
            break;
    }

    switch (meState)
    {
        case DONE:
        case ERROR:
            mxTemplateRoot.clear();
            mxTemplateRoot.clear();
            mxFolderEnvironment.clear();
            mxEntryEnvironment.clear();
            mxFolderResultSet.clear();
            mxEntryResultSet.clear();
            mpLastAddedEntry = NULL;
            break;
        default:
            break;
    }
}




bool TemplateScanner::HasNextStep (void)
{
    switch (meState)
    {
        case DONE:
        case ERROR:
            return false;

        default:
            return true;
    }
}




const TemplateEntry* TemplateScanner::GetLastAddedEntry (void) const
{
    return mpLastAddedEntry;
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
