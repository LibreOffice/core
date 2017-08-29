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

#ifndef INCLUDED_SD_SOURCE_UI_INC_TEMPLATESCANNER_HXX
#define INCLUDED_SD_SOURCE_UI_INC_TEMPLATESCANNER_HXX

#include "tools/AsynchronousTask.hxx"
#include "sddllapi.h"
#include <ucbhelper/content.hxx>
#include "com/sun/star/uno/Reference.hxx"
#include <o3tl/make_unique.hxx>

#include <memory>
#include <vector>

namespace com { namespace sun { namespace star { namespace ucb {
class XContent;
class XCommandEnvironment;
} } } }

namespace com { namespace sun { namespace star { namespace sdbc {
class XResultSet;
} } } }

namespace comphelper { namespace string {
class NaturalStringSorter;
} }

namespace sd {

/** Representation of a template or layout file.
*/
class TemplateEntry
{
public:
    TemplateEntry   (const OUString& rsTitle, const OUString& rsPath)
        :   msTitle(rsTitle), msPath(rsPath) {}

    OUString msTitle;
    OUString msPath;
};

/** This class scans the template folders for impress templates.  There are
    two ways to use this class.
    1. The old and deprecated way is to call Scan() to scan all templates
    and collect the supported ones in a tree structure.  This structure is
    returned by GetFolderList().
    2. The new way implements the AsynchronousTask interface.  Call
    RunNextStep() as long HasNextStep() returns <TRUE/>.  After every step
    GetLastAddedEntry() returns the template that was scanned (and has a
    supported format) last.  When a step does not add a new template then
    the value of the previous step is returned.
*/
class SD_DLLPUBLIC TemplateScanner
    : public ::sd::tools::AsynchronousTask
{
public:
    /** Create a new template scanner and prepare but do not execute the scanning.
    */
    TemplateScanner();

    /** The destructor deletes any remaining entries of the local list of
        templates.
    */
    virtual ~TemplateScanner();

    /** Implementation of the AsynchronousTask interface method.
    */
    virtual void RunNextStep() override;

    /** Implementation of the AsynchronousTask interface method.
    */
    virtual bool HasNextStep() override;

    /** Return the TemplateDir object that was last added to
        mpTemplateEntries.
        @return
            <nullptr/> is returned either before the template scanning is
            started or after it has ended.
    */
    const TemplateEntry* GetLastAddedEntry() const { return mpTemplateEntries.empty()?nullptr:mpTemplateEntries.back().get();}

private:
    /** The current state determines which step will be executed next by
        RunNextStep().
    */
    enum State {
        INITIALIZE_SCANNING,
        INITIALIZE_FOLDER_SCANNING,
        GATHER_FOLDER_LIST,
        SCAN_FOLDER,
        INITIALIZE_ENTRY_SCAN,
        SCAN_ENTRY,
        DONE,
        ERROR
    };
    State meState;

    ::ucbhelper::Content maFolderContent;
    ::std::vector< std::unique_ptr<TemplateEntry> > mpTemplateEntries;

    /** The folders that are collected by GatherFolderList().
    */
    class FolderDescriptorList;
    std::unique_ptr<FolderDescriptorList> mpFolderDescriptors;

    /** Set of state variables used by the methods
        InitializeFolderScanning(), GatherFolderList(), ScanFolder(),
        InitializeEntryScanning(), and ScanEntry().
    */
    css::uno::Reference<css::ucb::XContent> mxTemplateRoot;
    css::uno::Reference<css::ucb::XCommandEnvironment> mxFolderEnvironment;
    css::uno::Reference<css::ucb::XCommandEnvironment> mxEntryEnvironment;
    css::uno::Reference<css::sdbc::XResultSet> mxFolderResultSet;
    css::uno::Reference<css::sdbc::XResultSet> mxEntryResultSet;

    /** Obtain the root folder of the template folder hierarchy.  The result
        is stored in mxTemplateRoot for later use.
    */
    State GetTemplateRoot();

    /** Initialize the scanning of folders.  This is called exactly once.
        @return
            Returns one of the two states ERROR or GATHER_FOLDER_LIST.
    */
    State InitializeFolderScanning();

    /** Collect all available top-level folders in an ordered list which can
        then be processed by ScanFolder().
        @return
            Returns one of the two states ERROR or SCAN_FOLDER.
    */
    State GatherFolderList();

    /** From the list of top-level folders collected by GatherFolderList()
        the one with highest priority is processed.
        @return
            Returns one of the states ERROR, DONE, or INITIALIZE_ENTRY_SCAN.
    */
    State ScanFolder();

    /** Initialize the scanning of entries of a top-level folder.
        @return
            Returns one of the states ERROR or SCAN_ENTRY.
    */
    State InitializeEntryScanning();

    /** Scan one entry.  When this entry matches the recognized template
        types it is appended to the result set.
        @return
            Returns one of the states ERROR, SCAN_ENTRY, or SCAN_FOLDER.
    */
    State ScanEntry();
};

} // end of namespace sd

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
