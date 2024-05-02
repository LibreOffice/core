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

#pragma once

#include <comphelper/accessiblecomponenthelper.hxx>
#include <com/sun/star/lang/XEventListener.hpp>
#include <cppuhelper/implbase.hxx>
#include <rtl/ref.hxx>

namespace vcl { class Window; }
namespace com::sun::star::awt { class XWindow; }
namespace com::sun::star::beans { class XPropertySet; }
namespace com::sun::star::beans { class XPropertySetInfo; }

namespace toolkit
{


    //= OAccessibleControlContext


    typedef ::comphelper::OAccessibleComponentHelper    OAccessibleControlContext_Base;

    /** class implementing the AccessibleContext for a UNO control - to be used in design mode of the control.
        <p><b>life time control<b/><br/>
        This control should be held weak by the creator (a UNO control), it itself holds a hard reference to the
        control model, and a weak reference to the control. The reference to the model is freed when the model
        is being disposed.</p>
    */
    class OAccessibleControlContext final
            :public cppu::ImplInheritanceHelper<
                OAccessibleControlContext_Base, css::lang::XEventListener>
    {
    public:
        /** creates an accessible context for a uno control
        @param _rxCreator
            the uno control's XAccessible interface. This must be an XControl, from which an XControlModel
            can be retrieved.
        */
        static rtl::Reference<OAccessibleControlContext> create(
            const css::uno::Reference< css::accessibility::XAccessible >& _rxCreator
        );

    private:

        // XAccessibleContext
        virtual sal_Int64 SAL_CALL getAccessibleChildCount(  ) override;
        virtual css::uno::Reference< css::accessibility::XAccessible > SAL_CALL getAccessibleChild( sal_Int64 i ) override;
        virtual css::uno::Reference< css::accessibility::XAccessible > SAL_CALL getAccessibleParent(  ) override;
        virtual sal_Int16 SAL_CALL getAccessibleRole(  ) override;
        virtual OUString SAL_CALL getAccessibleDescription(  ) override;
        virtual OUString SAL_CALL getAccessibleName(  ) override;
        virtual css::uno::Reference< css::accessibility::XAccessibleRelationSet > SAL_CALL getAccessibleRelationSet(  ) override;
        virtual sal_Int64 SAL_CALL getAccessibleStateSet(  ) override;

        // XAccessibleComponent
        virtual css::uno::Reference< css::accessibility::XAccessible > SAL_CALL getAccessibleAtPoint( const css::awt::Point& aPoint ) override;
        virtual void SAL_CALL grabFocus(  ) override;
        virtual sal_Int32 SAL_CALL getForeground(  ) override;
        virtual sal_Int32 SAL_CALL getBackground(  ) override;

        // XEventListener
        using comphelper::OCommonAccessibleComponent::disposing;
        virtual void SAL_CALL disposing( const css::lang::EventObject& Source ) override;

        // retrieves the value of a string property from the model, if the property is present
        OUString getModelStringProperty( const OUString& _pPropertyName );

        // starts listening at the control model (currently for disposal only)
        void startModelListening( );
        // stops listening at the control model
        void stopModelListening( );

        vcl::Window* implGetWindow( css::uno::Reference< css::awt::XWindow >* _pxUNOWindow = nullptr ) const;

        /// ctor. @see Init
        OAccessibleControlContext();
        virtual ~OAccessibleControlContext() override;

        /** late ctor
        */
        void Init(
            const css::uno::Reference< css::accessibility::XAccessible >& _rxCreator
        );

        // OCommonAccessibleComponent overridables
        virtual css::awt::Rectangle implGetBounds(  ) override;

        css::uno::Reference< css::beans::XPropertySet >
                    m_xControlModel;        // the model of the control which's context we implement
        css::uno::Reference< css::beans::XPropertySetInfo >
                    m_xModelPropsInfo;      // the cached property set info of the model
    };


}   // namespace toolkit


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
