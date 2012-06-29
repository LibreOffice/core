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

#ifndef SD_TASKPANE_CONTROL_FACTORY_HXX
#define SD_TASKPANE_CONTROL_FACTORY_HXX

#include "taskpane/TaskPaneTreeNode.hxx"

#include <memory>

namespace sd { namespace toolpanel {
class TreeNode;
} }




namespace sd { namespace toolpanel {

/** A simple factory base class defines the interface that is used by
    some of the control containers of the task pane to create controls on
    demand when they are about to be displayed for the first time.

    It provides the InternalCreateControl() method as hook that can be
    overloaded by derived classes to provide a new control.
*/
class ControlFactory
{
public:
    ControlFactory (void);
    virtual ~ControlFactory (void);

    /** creates a tree node which acts as root of an own tree

        Derived classes should overload InternalCreateControl.
    */
    ::std::auto_ptr<TreeNode> CreateControl( ::Window& i_rParent );

protected:
    virtual TreeNode* InternalCreateControl( ::Window& i_rParent ) = 0;
};



/** A simple helper class that realizes a ControlFactory that is able to create root controls, providing
    the to-be-created control with an additional parameter.
*/
template<class ControlType, class ArgumentType>
class RootControlFactoryWithArg
    : public ControlFactory
{
public:
    RootControlFactoryWithArg (ArgumentType& rArgument)
        : mrArgument(rArgument)
    {}

protected:
    virtual TreeNode* InternalCreateControl( ::Window& i_rParent )
    {
        return new ControlType( i_rParent, mrArgument );
    }

private:
    ArgumentType& mrArgument;
};


} } // end of namespace ::sd::toolpanel

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
