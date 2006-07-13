/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: MasterPageContainerFiller.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: obo $ $Date: 2006-07-13 10:30:43 $
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
    // expensive creation) of the default page.  It is replaced later one by
    // another.
    SharedMasterPageDescriptor pDescriptor (new MasterPageDescriptor(
        MasterPageContainer::DEFAULT,
        0,
        String(),
        String(),
        String(),
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
