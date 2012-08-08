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

#ifndef __FRAMEWORK_UIELEMENT_SPINFIELDTOOLBARCONTROLLER_HXX_
#define __FRAMEWORK_UIELEMENT_SPINFIELDTOOLBARCONTROLLER_HXX_

#include <com/sun/star/beans/NamedValue.hpp>
#include <com/sun/star/frame/XDispatch.hpp>
#include <com/sun/star/frame/ControlCommand.hpp>

#include <uielement/complextoolbarcontroller.hxx>
#include <vcl/toolbox.hxx>
#include <vcl/spinfld.hxx>

namespace framework
{

class ToolBar;
class SpinfieldControl;

class ISpinfieldListener
{
    public:
        virtual void Up() = 0;
        virtual void Down() = 0;
        virtual void First() = 0;
        virtual void Last() = 0;
        virtual void KeyInput( const KeyEvent& rKEvt ) = 0;
        virtual void Modify() = 0;
        virtual void GetFocus() = 0;
        virtual void LoseFocus() = 0;
        virtual void StateChanged( StateChangedType nType ) = 0;
        virtual void DataChanged( const DataChangedEvent& rDCEvt ) = 0;
        virtual long PreNotify( NotifyEvent& rNEvt ) = 0;

    protected:
        ~ISpinfieldListener() {}
};

class SpinfieldToolbarController : public ISpinfieldListener,
                                   public ComplexToolbarController

{
    public:
        SpinfieldToolbarController( const com::sun::star::uno::Reference< com::sun::star::lang::XMultiServiceFactory >& rServiceManager,
                                    const com::sun::star::uno::Reference< com::sun::star::frame::XFrame >& rFrame,
                                    ToolBox*             pToolBar,
                                    sal_uInt16               nID,
                                    sal_Int32            nWidth,
                                    const rtl::OUString& aCommand );
        virtual ~SpinfieldToolbarController();

        // XComponent
        virtual void SAL_CALL dispose() throw ( ::com::sun::star::uno::RuntimeException );

        // ISpinfieldListener
        virtual void Up();
        virtual void Down();
        virtual void First();
        virtual void Last();
        virtual void KeyInput( const KeyEvent& rKEvt );
        virtual void Modify();
        virtual void GetFocus();
        virtual void LoseFocus();
        virtual void StateChanged( StateChangedType nType );
        virtual void DataChanged( const DataChangedEvent& rDCEvt );
        virtual long PreNotify( NotifyEvent& rNEvt );

    protected:
        virtual void executeControlCommand( const ::com::sun::star::frame::ControlCommand& rControlCommand );
        virtual ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue> getExecuteArgs(sal_Int16 KeyModifier) const;

    private:
        bool impl_getValue( const ::com::sun::star::uno::Any& rAny, sal_Int32& nValue, double& fValue, bool& bFloat );
        rtl::OUString impl_formatOutputString( double fValue );

        bool              m_bFloat,
                          m_bMaxSet,
                          m_bMinSet;
        double            m_nMax;
        double            m_nMin;
        double            m_nValue;
        double            m_nStep;
        SpinfieldControl* m_pSpinfieldControl;
        rtl::OUString     m_aOutFormat;
};

}

#endif // __FRAMEWORK_UIELEMENT_SPINFIELDTOOLBARCONTROLLER_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
