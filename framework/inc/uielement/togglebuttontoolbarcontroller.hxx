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

#ifndef __FRAMEWORK_UIELEMENT_TOGGLEBUTTONTOOLBARCONTROLLER_HXX_
#define __FRAMEWORK_UIELEMENT_TOGGLEBUTTONTOOLBARCONTROLLER_HXX_

#include <com/sun/star/beans/NamedValue.hpp>
#include <com/sun/star/frame/XDispatch.hpp>
#include <com/sun/star/frame/ControlCommand.hpp>

#include <uielement/complextoolbarcontroller.hxx>
#include <vcl/toolbox.hxx>
#include <vcl/image.hxx>

namespace framework
{

class ToolBar;
class ToggleButtonToolbarController : public ComplexToolbarController

{
    public:
        enum Style
        {
            STYLE_TOGGLEBUTTON,
            STYLE_DROPDOWNBUTTON,
            STYLE_TOGGLE_DROPDOWNBUTTON
        };

        ToggleButtonToolbarController( const com::sun::star::uno::Reference< com::sun::star::lang::XMultiServiceFactory >& rServiceManager,
                                       const com::sun::star::uno::Reference< com::sun::star::frame::XFrame >& rFrame,
                                       ToolBox*             pToolBar,
                                       sal_uInt16               nID,
                                       Style                eStyle,
                                       const rtl::OUString& aCommand );
        virtual ~ToggleButtonToolbarController();

        // XComponent
        virtual void SAL_CALL dispose() throw ( ::com::sun::star::uno::RuntimeException );

        // XToolbarController
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindow > SAL_CALL createPopupWindow() throw (::com::sun::star::uno::RuntimeException);

    protected:
        virtual void executeControlCommand( const ::com::sun::star::frame::ControlCommand& rControlCommand );
        virtual ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue> getExecuteArgs(sal_Int16 KeyModifier) const;

    private:
        DECL_LINK( MenuSelectHdl, Menu *);

        Style                        m_eStyle;
        rtl::OUString                m_aCurrentSelection;
        std::vector< rtl::OUString > m_aDropdownMenuList;
};

}

#endif // __FRAMEWORK_UIELEMENT_TOGGLEBUTTONTOOLBARCONTROLLER_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
