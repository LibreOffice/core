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

#ifndef SD_TASKPANE_CONTROL_FACTORY_HXX
#define SD_TASKPANE_CONTROL_FACTORY_HXX

#include "taskpane/TaskPaneTreeNode.hxx"

#include <memory>

namespace sd {
class ViewShellBase;
}

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
