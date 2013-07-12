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

#include <vcl/ctrl.hxx>
#include <com/sun/star/frame/XFrame.hpp>
#include <com/sun/star/frame/XDispatch.hpp>
#include <com/sun/star/frame/XToolbarController.hpp>

#include <map>
#include <svx/sidebar/PanelLayout.hxx>

namespace css = ::com::sun::star;
namespace cssu = ::com::sun::star::uno;

class ToolBox;

namespace svx { namespace sidebar {

/** This panel provides buttons for inserting shapes into a document.
*/
class InsertPropertyPanel
    : public PanelLayout
{
public:
    InsertPropertyPanel (
        Window* pParent,
        const cssu::Reference<css::frame::XFrame>& rxFrame);
    virtual ~InsertPropertyPanel (void);

private:
    ToolBox*        mpStandardShapesToolBox;
    ToolBox*        mpCustomShapesToolBox;
    const cssu::Reference<css::frame::XFrame> mxFrame;

    DECL_LINK(WindowEventListener, VclSimpleEvent*);
};


} } // end of namespace ::svx::sidebar

#endif
