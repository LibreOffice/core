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

#ifndef SVX_SIDEBAR_INSERT_PROPERTY_PAGE_HXX
#define SVX_SIDEBAR_INSERT_PROPERTY_PAGE_HXX

#include <boost/scoped_ptr.hpp>

#include <vcl/ctrl.hxx>
#include <com/sun/star/frame/XFrame.hpp>
#include <com/sun/star/frame/XDispatch.hpp>
#include <com/sun/star/frame/XToolbarController.hpp>

#include <map>


namespace css = ::com::sun::star;
namespace cssu = ::com::sun::star::uno;

class ToolBox;

namespace svx { namespace sidebar {

class InsertPropertyPanel
    : public Control
{
public:
    InsertPropertyPanel (
        Window* pParent,
        const cssu::Reference<css::frame::XFrame>& rxFrame);
    virtual ~InsertPropertyPanel (void);

private:
    ::boost::scoped_ptr<Window> mpStandardShapesBackground;
    ::boost::scoped_ptr<ToolBox> mpStandardShapesToolBox;
    ::boost::scoped_ptr<Window> mpCustomShapesBackground;
    ::boost::scoped_ptr<ToolBox> mpCustomShapesToolBox;
    class ItemDescriptor
    {
    public:
        cssu::Reference<css::frame::XToolbarController> mxController;
        css::util::URL maURL;
        rtl::OUString msCurrentCommand;
        cssu::Reference<css::frame::XDispatch> mxDispatch;
    };
    typedef ::std::map<sal_uInt16, ItemDescriptor> ControllerContainer;
    ControllerContainer maControllers;
    const cssu::Reference<css::frame::XFrame> mxFrame;

    /** Add listeners to toolbox and update its size to match its
        content.
    */
    void SetupToolBox (ToolBox& rToolBox);
    cssu::Reference<css::frame::XToolbarController> GetControllerForItemId (
        const sal_uInt16 nItemId) const;
    ToolBox* GetToolBoxForItemId (const sal_uInt16 nItemId) const;
    sal_uInt16 GetItemIdForSubToolbarName (
        const ::rtl::OUString& rsCOmmandName) const;

    /** Create toolbox controller for one item.
    */
    void CreateController (
        const sal_uInt16 nItemId);
    void UpdateIcons (void);

    DECL_LINK(DropDownClickHandler, ToolBox*);
    DECL_LINK(ClickHandler, ToolBox*);
    DECL_LINK(DoubleClickHandler, ToolBox*);
    DECL_LINK(SelectHandler, ToolBox*);
    DECL_LINK(WindowEventListener, VclSimpleEvent*);
    DECL_LINK(Activate, ToolBox*);
    DECL_LINK(Deactivate, ToolBox*);
};


} } // end of namespace ::svx::sidebar

#endif
