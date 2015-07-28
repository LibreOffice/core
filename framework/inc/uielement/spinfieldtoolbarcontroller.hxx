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

#ifndef INCLUDED_FRAMEWORK_INC_UIELEMENT_SPINFIELDTOOLBARCONTROLLER_HXX
#define INCLUDED_FRAMEWORK_INC_UIELEMENT_SPINFIELDTOOLBARCONTROLLER_HXX

#include <com/sun/star/beans/NamedValue.hpp>
#include <com/sun/star/frame/XDispatch.hpp>
#include <com/sun/star/frame/ControlCommand.hpp>

#include <uielement/complextoolbarcontroller.hxx>
#include <vcl/spinfld.hxx>
#include <vcl/vclptr.hxx>

class ToolBox;

namespace framework
{

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
        virtual void StateChanged( StateChangedType nType ) = 0;
        virtual void DataChanged( const DataChangedEvent& rDCEvt ) = 0;
        virtual bool PreNotify( NotifyEvent& rNEvt ) = 0;

    protected:
        ~ISpinfieldListener() {}
};

class SpinfieldToolbarController : public ISpinfieldListener,
                                   public ComplexToolbarController

{
    public:
        SpinfieldToolbarController( const com::sun::star::uno::Reference< com::sun::star::uno::XComponentContext >& rxContext,
                                    const com::sun::star::uno::Reference< com::sun::star::frame::XFrame >& rFrame,
                                    ToolBox* pToolBar,
                                    sal_uInt16 nID,
                                    sal_Int32 nWidth,
                                    const OUString& aCommand );
        virtual ~SpinfieldToolbarController();

        // XComponent
        virtual void SAL_CALL dispose() throw ( ::com::sun::star::uno::RuntimeException, std::exception ) SAL_OVERRIDE;

        // ISpinfieldListener
        virtual void Up() SAL_OVERRIDE;
        virtual void Down() SAL_OVERRIDE;
        virtual void First() SAL_OVERRIDE;
        virtual void Last() SAL_OVERRIDE;
        virtual void KeyInput( const KeyEvent& rKEvt ) SAL_OVERRIDE;
        virtual void Modify() SAL_OVERRIDE;
        virtual void GetFocus() SAL_OVERRIDE;
        virtual void StateChanged( StateChangedType nType ) SAL_OVERRIDE;
        virtual void DataChanged( const DataChangedEvent& rDCEvt ) SAL_OVERRIDE;
        virtual bool PreNotify( NotifyEvent& rNEvt ) SAL_OVERRIDE;

    protected:
        virtual void executeControlCommand( const ::com::sun::star::frame::ControlCommand& rControlCommand ) SAL_OVERRIDE;
        virtual ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue> getExecuteArgs(sal_Int16 KeyModifier) const SAL_OVERRIDE;

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
        VclPtr<SpinfieldControl> m_pSpinfieldControl;
        OUString     m_aOutFormat;
};

}

#endif // INCLUDED_FRAMEWORK_INC_UIELEMENT_SPINFIELDTOOLBARCONTROLLER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
