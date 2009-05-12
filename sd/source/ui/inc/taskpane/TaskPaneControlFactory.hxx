/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: TaskPaneControlFactory.hxx,v $
 * $Revision: 1.4 $
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

    /** Derived classes should overload InternalCreateControl(), not this
        method.
    */
    ::std::auto_ptr<TreeNode> CreateControl (TreeNode* pTreeNode);

protected:
    /** This is the internal hook for derived classes to overload in order
        to provide a new control instance.
    */
    virtual TreeNode* InternalCreateControl (TreeNode* pTreeNode) = 0;
};



/** A simple helper class that realizes a ControlFactory that provides its
    newly created controls with one additional argument (additional to the
    parent TreeNode).
*/
template<class ControlType, class ArgumentType>
class ControlFactoryWithArgs1
    : public ControlFactory
{
public:
    ControlFactoryWithArgs1 (ArgumentType& rArgument)
        : mrArgument(rArgument)
    {}

protected:
    virtual TreeNode* InternalCreateControl (TreeNode* pTreeNode)
    {
        return new ControlType(pTreeNode, mrArgument);
    }

private:
    ArgumentType& mrArgument;
};


} } // end of namespace ::sd::toolpanel

#endif
