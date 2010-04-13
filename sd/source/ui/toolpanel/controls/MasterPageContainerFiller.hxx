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
