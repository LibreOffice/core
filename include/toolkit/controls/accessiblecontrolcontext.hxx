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

#ifndef INCLUDED_TOOLKIT_CONTROLS_ACCESSIBLECONTROLCONTEXT_HXX
#define INCLUDED_TOOLKIT_CONTROLS_ACCESSIBLECONTROLCONTEXT_HXX

#include <comphelper/accessiblecomponenthelper.hxx>
#include <comphelper/accimplaccess.hxx>
#include <comphelper/uno3.hxx>
#include <com/sun/star/lang/XEventListener.hpp>
#include <vcl/vclptr.hxx>

namespace vcl { class Window; }
namespace com { namespace sun { namespace star { namespace awt { class XWindow; } } } }
namespace com { namespace sun { namespace star { namespace beans { class XPropertySet; } } } }
namespace com { namespace sun { namespace star { namespace beans { class XPropertySetInfo; } } } }

namespace toolkit
{


    //= OAccessibleControlContext


    typedef ::comphelper::OAccessibleComponentHelper    OAccessibleControlContext_Base;
    typedef ::cppu::ImplHelper1 <   css::lang::XEventListener
                                >   OAccessibleControlContext_IBase;

    /** class implementing the AccessibleContext for an UNO control - to be used in design mode of the control.
        <p><b>life time control<b/><br/>
        This control should be held weak by the creator (an UNO control), it itself holds a hard reference to the
        control model, and a weak reference to the control. The reference to the model is freed when the model
        is being disposed.</p>
    */
    class OAccessibleControlContext final
            :public ::comphelper::OAccessibleImplementationAccess
            ,public OAccessibleControlContext_Base
            ,public OAccessibleControlContext_IBase
    {
    public:
        /** creates an accessible context for an uno control
        @param _rxCreator
            the uno control's XAccessible interface. This must be an XControl, from which an XControlModel
            can be retrieved.
        */
        static OAccessibleControlContext* create(
            const css::uno::Reference< css::accessibility::XAccessible >& _rxCreator
        );

    private:
        // XInterface
        DECLARE_XINTERFACE( )
        DECLARE_XTYPEPROVIDER( )

        // XAccessibleContext
        virtual sal_Int32 SAL_CALL getAccessibleChildCount(  ) override;
        virtual css::uno::Reference< css::accessibility::XAccessible > SAL_CALL getAccessibleChild( sal_Int32 i ) override;
        virtual css::uno::Reference< css::accessibility::XAccessible > SAL_CALL getAccessibleParent(  ) override;
        virtual sal_Int16 SAL_CALL getAccessibleRole(  ) override;
        virtual OUString SAL_CALL getAccessibleDescription(  ) override;
        virtual OUString SAL_CALL getAccessibleName(  ) override;
        virtual css::uno::Reference< css::accessibility::XAccessibleRelationSet > SAL_CALL getAccessibleRelationSet(  ) override;
        virtual css::uno::Reference< css::accessibility::XAccessibleStateSet > SAL_CALL getAccessibleStateSet(  ) override;

        // XAccessibleComponent
        virtual css::uno::Reference< css::accessibility::XAccessible > SAL_CALL getAccessibleAtPoint( const css::awt::Point& aPoint ) override;
        virtual void SAL_CALL grabFocus(  ) override;
        virtual sal_Int32 SAL_CALL getForeground(  ) override;
        virtual sal_Int32 SAL_CALL getBackground(  ) override;

        // XEventListener
        using comphelper::OAccessibleContextHelper::disposing;
        virtual void SAL_CALL disposing( const css::lang::EventObject& Source ) override;

        // retrieves the value of a string property from the model, if the property is present
        OUString getModelStringProperty( const sal_Char* _pPropertyName );

        // starts listening at the control model (currently for disposal only)
        void startModelListening( );
        // stops listening at the control model
        void stopModelListening( );

        VclPtr< vcl::Window > implGetWindow( css::uno::Reference< css::awt::XWindow >* _pxUNOWindow = nullptr ) const;

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


#endif // INCLUDED_TOOLKIT_CONTROLS_ACCESSIBLECONTROLCONTEXT_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
