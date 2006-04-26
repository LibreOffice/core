/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: MasterPageContainerFiller.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: kz $ $Date: 2006-04-26 20:49:19 $
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

#ifndef SD_TOOLPANEL_CONTROLS_MASTER_PAGE_CONTAINER_FILLER_HXX
#define SD_TOOLPANEL_CONTROLS_MASTER_PAGE_CONTAINER_FILLER_HXX

#include "MasterPageContainer.hxx"
#include "MasterPageDescriptor.hxx"
#include "tools/AsynchronousTask.hxx"

namespace sd {
class TemplateScanner;
class TemplateEntry;
}

namespace sd { namespace toolpanel { namespace controls {

/** Fill a MasterPageContainer with information about the available master
    pages.  These are provided by one default page and from the existing
    Impress templates.  This is done asynchronously.
*/
class MasterPageContainerFiller
    : public ::sd::tools::AsynchronousTask
{
public:
    class ContainerAdapter
    {
    public:
        virtual MasterPageContainer::Token PutMasterPage (
            const SharedMasterPageDescriptor& rpDescriptor) = 0;
        /** This method is called when all Impress templates have been added
            to the container via the PutMasterPage() method.
        */
        virtual void FillingDone (void) = 0;
    };

    MasterPageContainerFiller (ContainerAdapter& rContainerAdapter);
    virtual ~MasterPageContainerFiller (void);

    /** Run the next step of the task.  After HasNextStep() returns false
        this method should ignore further calls.
    */
    virtual void RunNextStep (void);

    /** Return <TRUE/> when there is at least one more step to execute.
        When the task has been executed completely then <FALSE/> is
        returned.
    */
    virtual bool HasNextStep (void);

private:
    ContainerAdapter& mrContainerAdapter;
    // Remember what the next step has to do.
    enum State {
        INITIALIZE_TEMPLATE_SCANNER,
        SCAN_TEMPLATE,
        ADD_TEMPLATE,
        ERROR,
        DONE
    } meState;
    ::std::auto_ptr<TemplateScanner> mpScannerTask;
    const TemplateEntry* mpLastAddedEntry;
    int mnIndex;

    State ScanTemplate (void);
    State AddTemplate (void);
};

} } } // end of namespace ::sd::toolpanel::controls

#endif
