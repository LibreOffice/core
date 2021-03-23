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

#pragma once

#include <memory>
#include "MasterPageContainer.hxx"
#include "MasterPageDescriptor.hxx"
#include <tools/AsynchronousTask.hxx>

namespace sd
{
class TemplateScanner;
class TemplateEntry;
}

namespace sd::sidebar
{
/** Fill a MasterPageContainer with information about the available master
    pages.  These are provided by one default page and from the existing
    Impress templates.  This is done asynchronously.
*/
class MasterPageContainerFiller : public ::sd::tools::AsynchronousTask
{
public:
    class ContainerAdapter
    {
    public:
        virtual MasterPageContainer::Token
        PutMasterPage(const SharedMasterPageDescriptor& rpDescriptor)
            = 0;
        /** This method is called when all Impress templates have been added
            to the container via the PutMasterPage() method.
        */
        virtual void FillingDone() = 0;

    protected:
        ~ContainerAdapter() {}
    };

    explicit MasterPageContainerFiller(ContainerAdapter& rContainerAdapter);
    virtual ~MasterPageContainerFiller();

    /** Run the next step of the task.  After HasNextStep() returns false
        this method should ignore further calls.
    */
    virtual void RunNextStep() override;

    /** Return <TRUE/> when there is at least one more step to execute.
        When the task has been executed completely then <FALSE/> is
        returned.
    */
    virtual bool HasNextStep() override;

private:
    ContainerAdapter& mrContainerAdapter;
    // Remember what the next step has to do.
    enum State
    {
        INITIALIZE_TEMPLATE_SCANNER,
        SCAN_TEMPLATE,
        ADD_TEMPLATE,
        ERROR,
        DONE
    } meState;
    ::std::unique_ptr<TemplateScanner> mpScannerTask;
    const TemplateEntry* mpLastAddedEntry;
    int mnIndex;

    State ScanTemplate();
    State AddTemplate();
};

} // end of namespace sd::sidebar

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
