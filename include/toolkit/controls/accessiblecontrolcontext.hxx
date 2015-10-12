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
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/awt/XWindow.hpp>
#include <vcl/vclptr.hxx>

namespace vcl { class Window; }

namespace toolkit
{



    //= OAccessibleControlContext


    typedef ::comphelper::OAccessibleComponentHelper    OAccessibleControlContext_Base;
    typedef ::cppu::ImplHelper1 <   ::com::sun::star::lang::XEventListener
                                >   OAccessibleControlContext_IBase;

    /** class implementing the AccessibleContext for an UNO control - to be used in design mode of the control.
        <p><b>life time control<b/><br/>
        This control should be held weak by the creator (an UNO control), it itself holds a hard reference to the
        control model, and a weak reference to the control. The reference to the model is freed when the model
        is being disposed.</p>
    */
    class OAccessibleControlContext
            :public ::comphelper::OAccessibleImplementationAccess
            ,public OAccessibleControlContext_Base
            ,public OAccessibleControlContext_IBase
    {
    private:
        ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >
                    m_xControlModel;        // the model of the control which's context we implement
        ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo >
                    m_xModelPropsInfo;      // the cached property set info of the model

    protected:
        /// ctor. @see Init
        OAccessibleControlContext();
        virtual ~OAccessibleControlContext();

        /** late ctor
        */
        void Init(
            const ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible >& _rxCreator
        );

        // OCommonAccessibleComponent overridables
        virtual ::com::sun::star::awt::Rectangle implGetBounds(  ) throw (::com::sun::star::uno::RuntimeException) override;

    public:
        /** creates an accessible context for an uno control
        @param _rxCreator
            the uno control's XAccessible interface. This must be an XControl, from which an XControlModel
            can be retrieved.
        */
        static OAccessibleControlContext* create(
            const ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible >& _rxCreator
        );

    protected:
        // XInterface
        DECLARE_XINTERFACE( )
        DECLARE_XTYPEPROVIDER( )

        // XAccessibleContext
        virtual sal_Int32 SAL_CALL getAccessibleChildCount(  ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible > SAL_CALL getAccessibleChild( sal_Int32 i ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException, std::exception) override;
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible > SAL_CALL getAccessibleParent(  ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;
        virtual sal_Int16 SAL_CALL getAccessibleRole(  ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;
        virtual OUString SAL_CALL getAccessibleDescription(  ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;
        virtual OUString SAL_CALL getAccessibleName(  ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessibleRelationSet > SAL_CALL getAccessibleRelationSet(  ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessibleStateSet > SAL_CALL getAccessibleStateSet(  ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;

        // XAccessibleComponent
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible > SAL_CALL getAccessibleAtPoint( const ::com::sun::star::awt::Point& aPoint ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL grabFocus(  ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;
        virtual sal_Int32 SAL_CALL getForeground(  ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;
        virtual sal_Int32 SAL_CALL getBackground(  ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;

        // XEventListener
        using comphelper::OAccessibleContextHelper::disposing;
        virtual void SAL_CALL disposing( const ::com::sun::star::lang::EventObject& Source ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;

    private:
        // retrieves the value of a string property from the model, if the property is present
        OUString getModelStringProperty( const sal_Char* _pPropertyName );

        // starts listening at the control model (currently for disposal only)
        void startModelListening( );
        // stops listening at the control model
        void stopModelListening( );

        VclPtr< vcl::Window > implGetWindow( ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindow >* _pxUNOWindow = NULL ) const;
    };


}   // namespace toolkit


#endif // INCLUDED_TOOLKIT_CONTROLS_ACCESSIBLECONTROLCONTEXT_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
