/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/

#ifndef SD_SIDEBAR_PANELS_MASTER_PAGE_CONTAINER_FILLER_HXX
#define SD_SIDEBAR_PANELS_MASTER_PAGE_CONTAINER_FILLER_HXX

#include "MasterPageContainer.hxx"
#include "MasterPageDescriptor.hxx"
#include "tools/AsynchronousTask.hxx"

namespace sd {
class TemplateScanner;
class TemplateEntry;
}

namespace sd { namespace sidebar {

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

} } // end of namespace sd::sidebar

#endif
