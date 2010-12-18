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

#include "MasterPageContainerFiller.hxx"

#include "MasterPageDescriptor.hxx"
#include "MasterPageContainerProviders.hxx"
#include "TemplateScanner.hxx"

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::sd::toolpanel::controls;


namespace sd { namespace toolpanel { namespace controls {

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
        String(),
        String(),
        String(),
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
            String(),
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



} } } // end of namespace ::sd::toolpanel::controls

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
