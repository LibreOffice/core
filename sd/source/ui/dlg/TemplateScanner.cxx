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

#include <TemplateScanner.hxx>

#include <comphelper/processfactory.hxx>
#include <comphelper/documentconstants.hxx>

#include <sfx2/doctempl.hxx>
#include <com/sun/star/frame/DocumentTemplates.hpp>
#include <com/sun/star/frame/XDocumentTemplates.hpp>
#include <com/sun/star/ucb/XContentAccess.hpp>
#include <com/sun/star/sdbc/XResultSet.hpp>
#include <com/sun/star/sdbc/XRow.hpp>

#include <set>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;

namespace {

const char TITLE[] = "Title";

class FolderDescriptor
{
public:
    FolderDescriptor (
        int nPriority,
        const OUString& rsContentIdentifier,
        const Reference<css::ucb::XCommandEnvironment>& rxFolderEnvironment)
        : mnPriority(nPriority),
          msContentIdentifier(rsContentIdentifier),
          mxFolderEnvironment(rxFolderEnvironment)
    { }
    int const mnPriority;
    OUString const msContentIdentifier;
    //    Reference<sdbc::XResultSet> mxFolderResultSet;
    Reference<css::ucb::XCommandEnvironment> mxFolderEnvironment;

    class Comparator
    {
    public:
        bool operator() (const FolderDescriptor& r1, const FolderDescriptor& r2) const
            { return r1.mnPriority < r2.mnPriority; }
    };
};

/** Use a heuristic based on the URL of a top-level template folder to
    assign a priority that is used to sort the folders.
*/
int Classify (const OUString&, const OUString& rsURL)
{
    int nPriority (0);

    if (rsURL.isEmpty())
        nPriority = 100;
    else if (rsURL.indexOf("presnt")>=0)
    {
        nPriority = 30;
    }
    else if (rsURL.indexOf("layout")>=0)
    {
        nPriority = 20;
    }
    else if (rsURL.indexOf("educate")>=0)
    {
        nPriority = 40;
    }
    else if (rsURL.indexOf("finance")>=0)
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

TemplateScanner::TemplateScanner()
    : meState(INITIALIZE_SCANNING),
      maFolderContent(),
      mpFolderDescriptors(new FolderDescriptorList),
      mxTemplateRoot(),
      mxFolderEnvironment(),
      mxEntryEnvironment(),
      mxFolderResultSet(),
      mxEntryResultSet()
{
    //  empty;
}

TemplateScanner::~TemplateScanner()
{
}

TemplateScanner::State TemplateScanner::GetTemplateRoot()
{
    Reference< XComponentContext > xContext = ::comphelper::getProcessComponentContext();
    Reference<frame::XDocumentTemplates> xTemplates = frame::DocumentTemplates::create(xContext);
    mxTemplateRoot = xTemplates->getContent();

    return INITIALIZE_FOLDER_SCANNING;
}

TemplateScanner::State TemplateScanner::InitializeEntryScanning()
{
    State eNextState (SCAN_ENTRY);

    if (maFolderContent.isFolder())
    {
        mxEntryEnvironment.clear();

        //  We are interested only in three properties: the entry's name,
        //  its URL, and its content type.
        Sequence<OUString> aProps (3);
        aProps[0] = TITLE;
        aProps[1] = "TargetURL";
        aProps[2] = "TypeDescription";

        //  Create a cursor to iterate over the templates in this folders.
        mxEntryResultSet.set( maFolderContent.createCursor(aProps, ::ucbhelper::INCLUDE_DOCUMENTS_ONLY));
    }
    else
        eNextState = ERROR;

    return eNextState;
}

TemplateScanner::State TemplateScanner::ScanEntry()
{
    State eNextState (ERROR);

    Reference<css::ucb::XContentAccess> xContentAccess (mxEntryResultSet, UNO_QUERY);
    Reference<css::sdbc::XRow> xRow (mxEntryResultSet, UNO_QUERY);

    if (xContentAccess.is() && xRow.is() && mxEntryResultSet.is())
    {
        if (mxEntryResultSet->next())
        {
            OUString sTitle (xRow->getString (1));
            OUString sTargetURL (xRow->getString (2));
            OUString sContentType (xRow->getString (3));

            OUString aId = xContentAccess->queryContentIdentifierString();
            ::ucbhelper::Content  aContent = ::ucbhelper::Content (aId, mxEntryEnvironment, comphelper::getProcessComponentContext());
            if (aContent.isDocument ())
            {
                //  Check whether the entry is an impress template.  If so
                //  add a new entry to the resulting list (which is created
                //  first if necessary).
                //  These strings are used to find impress templates in the tree of
                //  template files.  Should probably be determined dynamically.
                if (    (sContentType == MIMETYPE_OASIS_OPENDOCUMENT_PRESENTATION_TEMPLATE_ASCII)
                    ||  (sContentType == MIMETYPE_OASIS_OPENDOCUMENT_PRESENTATION_ASCII)
                    ||  (sContentType == "application/vnd.stardivision.impress")
                    ||  (sContentType == MIMETYPE_VND_SUN_XML_IMPRESS_ASCII)
                        // The following id comes from the bugdoc in #i2764#.
                    ||  (sContentType == "Impress 2.0"))
                {
                    OUString sLocalisedTitle = SfxDocumentTemplates::ConvertResourceString(sTitle);
                    mpTemplateEntries.push_back(std::make_unique<TemplateEntry>(sLocalisedTitle, sTargetURL));
                }
            }

            // Continue scanning entries.
            eNextState = SCAN_ENTRY;
        }
        else
        {
            // Continue with scanning the next folder.
            eNextState = SCAN_FOLDER;
        }
    }

    return eNextState;
}

TemplateScanner::State TemplateScanner::InitializeFolderScanning()
{
    State eNextState (ERROR);

    mxFolderResultSet.clear();

    try
    {
        //  Create content for template folders.
        mxFolderEnvironment.clear();
        ::ucbhelper::Content aTemplateDir (mxTemplateRoot, mxFolderEnvironment, comphelper::getProcessComponentContext());

        //  Define the list of properties we are interested in.
        Sequence<OUString> aProps (2);
        aProps[0] = TITLE;
        aProps[1] = "TargetDirURL";

        //  Create an cursor to iterate over the template folders.
        mxFolderResultSet.set( aTemplateDir.createCursor(aProps, ::ucbhelper::INCLUDE_FOLDERS_ONLY));
        if (mxFolderResultSet.is())
            eNextState = GATHER_FOLDER_LIST;
    }
    catch (css::uno::Exception&)
    {
       eNextState = ERROR;
    }

    return eNextState;
}

TemplateScanner::State TemplateScanner::GatherFolderList()
{
    State eNextState (ERROR);

    Reference<css::ucb::XContentAccess> xContentAccess (mxFolderResultSet, UNO_QUERY);
    if (xContentAccess.is() && mxFolderResultSet.is())
    {
        while (mxFolderResultSet->next())
        {
            Reference<sdbc::XRow> xRow (mxFolderResultSet, UNO_QUERY);
            if (xRow.is())
            {
                OUString sTitle (xRow->getString (1));
                OUString sTargetDir (xRow->getString (2));
                OUString aId = xContentAccess->queryContentIdentifierString();

                mpFolderDescriptors->insert(
                    FolderDescriptor(
                        Classify(sTitle,sTargetDir),
                        aId,
                        mxFolderEnvironment));
            }
        }

        eNextState = SCAN_FOLDER;
    }

    return eNextState;
}

TemplateScanner::State TemplateScanner::ScanFolder()
{
    State eNextState (ERROR);

    if (!mpFolderDescriptors->empty())
    {
        FolderDescriptor aDescriptor (*mpFolderDescriptors->begin());
        mpFolderDescriptors->erase(mpFolderDescriptors->begin());

        OUString aId (aDescriptor.msContentIdentifier);

        maFolderContent = ::ucbhelper::Content (aId, aDescriptor.mxFolderEnvironment, comphelper::getProcessComponentContext());
        if (maFolderContent.isFolder())
        {
            // Scan the folder and insert it into the list of template
            // folders.
            // Continue with scanning all entries in the folder.
            mpTemplateEntries.clear();
            eNextState = INITIALIZE_ENTRY_SCAN;
        }
    }
    else
    {
        eNextState = DONE;
    }

    return eNextState;
}

void TemplateScanner::RunNextStep()
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
            mxFolderEnvironment.clear();
            mxEntryEnvironment.clear();
            mxFolderResultSet.clear();
            mxEntryResultSet.clear();
            break;
        default:
            break;
    }
}

bool TemplateScanner::HasNextStep()
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

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
