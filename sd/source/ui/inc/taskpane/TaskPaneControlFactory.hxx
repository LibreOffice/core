/*************************************************************************
 *
 *  $RCSfile: TaskPaneControlFactory.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: kz $ $Date: 2005-03-18 16:49:35 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
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
