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
        SpinfieldToolbarController( const com::sun::star::uno::Reference< com::sun::star::uno::XComponentContext >& rxContext,
                                    const com::sun::star::uno::Reference< com::sun::star::frame::XFrame >& rFrame,
                                    ToolBox*             pToolBar,
                                    sal_uInt16               nID,
                                    sal_Int32            nWidth,
                                    const OUString& aCommand );
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
        OUString impl_formatOutputString( double fValue );

        bool              m_bFloat,
                          m_bMaxSet,
                          m_bMinSet;
        double            m_nMax;
        double            m_nMin;
        double            m_nValue;
        double            m_nStep;
        SpinfieldControl* m_pSpinfieldControl;
        OUString     m_aOutFormat;
};

}

#endif // __FRAMEWORK_UIELEMENT_SPINFIELDTOOLBARCONTROLLER_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
