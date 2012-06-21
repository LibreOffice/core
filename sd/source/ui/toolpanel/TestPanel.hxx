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

#ifndef SD_TASKPANE_TEST_PANEL_HXX
#define SD_TASKPANE_TEST_PANEL_HXX

#include "taskpane/SubToolPanel.hxx"


namespace sd { namespace toolpanel {

class ControlFactory;
class TreeNode;

#ifdef SHOW_TEST_PANEL

/** This panel demonstrates how to create a panel for the task pane.
*/
class TestPanel
    : public SubToolPanel
{
public:
    TestPanel (::Window& i_rParent);
    virtual ~TestPanel (void);

    static std::auto_ptr<ControlFactory> CreateControlFactory (void);
};

#endif

} } // end of namespace ::sd::toolpanel

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
