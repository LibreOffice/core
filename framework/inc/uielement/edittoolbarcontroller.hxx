/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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

#ifndef __FRAMEWORK_UIELEMENT_EDITTOOLBARCONTROLLER_HXX_
#define __FRAMEWORK_UIELEMENT_EDITTOOLBARCONTROLLER_HXX_

#include <com/sun/star/beans/NamedValue.hpp>
#include <com/sun/star/frame/XDispatch.hpp>
#include <com/sun/star/frame/ControlCommand.hpp>

#include <uielement/complextoolbarcontroller.hxx>
#include <vcl/toolbox.hxx>
#include <vcl/edit.hxx>

namespace framework
{

class ToolBar;
class EditControl;

class IEditListener
{
    public:
        virtual void Modify() = 0;
        virtual void KeyInput( const KeyEvent& rKEvt ) = 0;
        virtual void GetFocus() = 0;
        virtual void LoseFocus() = 0;
        virtual long PreNotify( NotifyEvent& rNEvt ) = 0;

    protected:
        ~IEditListener() {}
};

class EditToolbarController : public IEditListener,
                              public ComplexToolbarController

{
    public:
        EditToolbarController( const com::sun::star::uno::Reference< com::sun::star::lang::XMultiServiceFactory >& rServiceManager,
                               const com::sun::star::uno::Reference< com::sun::star::frame::XFrame >& rFrame,
                               ToolBox*     pToolBar,
                               sal_uInt16       nID,
                               sal_Int32    nWidth,
                               const rtl::OUString& aCommand );
        virtual ~EditToolbarController();

        // XComponent
        virtual void SAL_CALL dispose() throw ( ::com::sun::star::uno::RuntimeException );

        // IEditListener
        virtual void Modify();
        virtual void KeyInput( const KeyEvent& rKEvt );
        virtual void GetFocus();
        virtual void LoseFocus();
        virtual long PreNotify( NotifyEvent& rNEvt );

    protected:
        virtual void executeControlCommand( const ::com::sun::star::frame::ControlCommand& rControlCommand );
        virtual ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue> getExecuteArgs(sal_Int16 KeyModifier) const;

    private:
        EditControl*    m_pEditControl;
};

}

#endif // __FRAMEWORK_UIELEMENT_EDITTOOLBARCONTROLLER_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
