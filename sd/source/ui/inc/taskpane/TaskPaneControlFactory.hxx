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
