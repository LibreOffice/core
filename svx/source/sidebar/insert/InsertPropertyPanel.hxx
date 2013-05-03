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
    DECL_LINK(ActivateToolBox, ToolBox*);
    DECL_LINK(DeactivateToolBox, ToolBox*);
};


} } // end of namespace ::svx::sidebar

#endif
