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

#include "MasterPageContainerFiller.hxx"

#include "MasterPageDescriptor.hxx"
#include "MasterPageContainerProviders.hxx"
#include "TemplateScanner.hxx"

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;


namespace sd { namespace sidebar {

MasterPageContainerFiller::MasterPageContainerFiller (ContainerAdapter& rpAdapter)
    : mrContainerAdapter(rpAdapter),
      meState(INITIALIZE_TEMPLATE_SCANNER),
      mpScannerTask(),
      mpLastAddedEntry(NULL),
      mnIndex(1)
{
    // Add one entry for the default master page.  We use temporarily the
    // DefaultPagePreviewProvider to prevent the rendering (and the
    // expensive creation) of the default page.  It is replaced later on by
    // another.
    SharedMasterPageDescriptor pDescriptor (new MasterPageDescriptor(
        MasterPageContainer::DEFAULT,
        0,
        OUString(),
        OUString(),
        OUString(),
        false,
        ::boost::shared_ptr<PageObjectProvider>(new DefaultPageObjectProvider()),
        ::boost::shared_ptr<PreviewProvider>(new PagePreviewProvider())));
    mrContainerAdapter.PutMasterPage(pDescriptor);
}




MasterPageContainerFiller::~MasterPageContainerFiller (void)
{
}




void MasterPageContainerFiller::RunNextStep (void)
{
    switch (meState)
    {
        case INITIALIZE_TEMPLATE_SCANNER:
            mpScannerTask.reset(new TemplateScanner());
            meState = SCAN_TEMPLATE;
            break;

        case SCAN_TEMPLATE:
            meState = ScanTemplate();
            break;

        case ADD_TEMPLATE:
            meState = AddTemplate();
            break;

        case DONE:
        case ERROR:
        default:
            break;
    }

    // When the state has just been set to DONE or ERROR then tell the
    // container that no more templates will be coming and stop the
    // scanning.
    switch (meState)
    {
        case DONE:
        case ERROR:
            if (mpScannerTask.get() != NULL)
            {
                mrContainerAdapter.FillingDone();
                mpScannerTask.reset();
            }
        default:
            break;
    }
}




bool MasterPageContainerFiller::HasNextStep (void)
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




MasterPageContainerFiller::State MasterPageContainerFiller::ScanTemplate (void)
{
    State eState (ERROR);

    if (mpScannerTask.get() != NULL)
    {
        if (mpScannerTask->HasNextStep())
        {
            mpScannerTask->RunNextStep();
            if (mpScannerTask->GetLastAddedEntry() != mpLastAddedEntry)
            {
                mpLastAddedEntry = mpScannerTask->GetLastAddedEntry();
                if (mpLastAddedEntry != NULL)
                    eState = ADD_TEMPLATE;
                else
                    eState = SCAN_TEMPLATE;
            }
            else
                eState = SCAN_TEMPLATE;
        }
        else
            eState = DONE;
    }

    return eState;
}




MasterPageContainerFiller::State MasterPageContainerFiller::AddTemplate (void)
{
    if (mpLastAddedEntry != NULL)
    {
        SharedMasterPageDescriptor pDescriptor (new MasterPageDescriptor(
            MasterPageContainer::TEMPLATE,
            mnIndex,
            mpLastAddedEntry->msPath,
            mpLastAddedEntry->msTitle,
            OUString(),
            false,
            ::boost::shared_ptr<PageObjectProvider>(
                new TemplatePageObjectProvider(mpLastAddedEntry->msPath)),
            ::boost::shared_ptr<PreviewProvider>(
                new TemplatePreviewProvider(mpLastAddedEntry->msPath))));
        // For user supplied templates we use a different preview provider:
        // The preview in the document shows not only shapes on the master
        // page but also shapes on the foreground.  This is misleading and
        // therefore these previews are discarded and created directly from
        // the page objects.
        if (pDescriptor->GetURLClassification() == MasterPageDescriptor::URLCLASS_USER)
            pDescriptor->mpPreviewProvider = ::boost::shared_ptr<PreviewProvider>(
                new PagePreviewProvider());

        mrContainerAdapter.PutMasterPage(pDescriptor);
        ++mnIndex;
    }

    return SCAN_TEMPLATE;
}



} } // end of namespace sd::sidebar

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
