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

#ifndef COMPHELPER_ACCIMPLACCESS_HXX
#define COMPHELPER_ACCIMPLACCESS_HXX

#include <cppuhelper/implbase1.hxx>
#include <com/sun/star/lang/XUnoTunnel.hpp>
#include "comphelper/comphelperdllapi.h"

// forward declaration
namespace com { namespace sun { namespace star { namespace accessibility {
    class XAccessible;
    class XAccessibleContext;
}}}}

//.........................................................................
namespace comphelper
{
//.........................................................................

    //=====================================================================
    //= OAccessibleImplementationAccess
    //=====================================================================
    typedef ::cppu::ImplHelper1 <   ::com::sun::star::lang::XUnoTunnel
                                >   OAccImpl_Base;
    struct OAccImpl_Impl;

    /** This is a helper class which allows accessing several aspects of the the implementation
        of an AccessibleContext.

        <p>For instance, when you want to implement a context which can be re-parented, you:
            <ul><li>derive your class from <type>OAccessibleImplementationAccess</type></li>
                <li>use <code>setAccessibleParent( <em>component</em>, <em>new_parent</em> )</code>
            </ul>
        </p>

        <p>Another aspect which can be controlled from the outside are states. If you have a class which
        has only partial control over it's states, you may consider deriving from OAccessibleImplementationAccess.<br/>
        For instance, say you have an implementation (say component A) which is <em>unable</em> to know or to
        determine if the represented object is selected, but another component (say B) which uses A (and integrates
        it into a tree of accessibility components) is.<br/>
        In this case, if A is derived from OAccessibleImplementationAccess, B can manipulate this
        foreign-controlled state flag "SELECTED" by using the static helper methods on this class.</p>

        <p>Please note that the support for foreign controlled states is rather restrictive: You can't have states
        which <em>may be</em> controlled by a foreign instances. This is implied by the fact that a derived
        class can ask for states which are <em>set</em> only, not for the ones which are <em>reset</em> currently.
        </p>
    */
    class COMPHELPER_DLLPUBLIC OAccessibleImplementationAccess : public OAccImpl_Base
    {
    private:
        OAccImpl_Impl*  m_pImpl;

    protected:
        /// retrieves the parent previously set via <method>setAccessibleParent</method>
        ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible >
                implGetForeignControlledParent( ) const;

        /** retrieves the set of currently set states which are controlled by a foreign instance
        @return
            a bit mask, where a set bit 2^n means that the AccessibleStateType n has been set
        */
        sal_Int64   implGetForeignControlledStates( ) const;

        /// sets the accessible parent component
        virtual void    setAccessibleParent(
            const ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible >& _rxAccParent );

        /// sets or resets a bit of the foreign controlled states
        virtual void    setStateBit( const sal_Int16 _nState, const sal_Bool _bSet );

    protected:
        OAccessibleImplementationAccess( );
        virtual ~OAccessibleImplementationAccess( );

        // XUnoTunnel
        virtual sal_Int64 SAL_CALL getSomething( const ::com::sun::star::uno::Sequence< sal_Int8 >& _rIdentifier ) throw (::com::sun::star::uno::RuntimeException);

    public:
        /** tries to access the implementation of an OAccessibleImplementationAccess derivee which is known as
            interface only.

        @param _rxComponent
            is the component which should be examined.
        @return
            the pointer to the implementation, if successfull. The only known error condition so far
            is an invalid context (which means it is <NULL/>, or the implementation is not derived
            from <type>OAccessibleImplementationAccess</type>, or retrieving the implementation failed).
        */
        static OAccessibleImplementationAccess* getImplementation(
            const ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessibleContext >& _rxComponent
        );


        /** sets the parent for a derived implementation

        @param _rxComponent
            is the component which's new parent should be set
        @param _rxNewParent
            is the new parent of the component
        @return
            <TRUE/> in case of success, <FALSE/> otherwise. For error condition please look at
            <method>getImplementation</method>.
        */
        static sal_Bool setAccessibleParent(
            const ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessibleContext >& _rxComponent,
            const ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible >& _rxNewParent
        );

        /** sets or resets a state bit in the set of foreign-controlled states of the component.

        @param _rxComponent
            is the component which's state is to be (re)set
        @param _nState
            the state bit which should be affected. This should be one of the respective UNO constants.
        @param _bSet
            <TRUE/> if the bit should be set, <FALSE/> otherwise
        @return
            <TRUE/> in case of success, <FALSE/> otherwise. For error condition please look at
            <method>getImplementation</method>.
        */
        static sal_Bool setForeignControlledState(
            const ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessibleContext >& _rxComponent,
            const sal_Int16 _nState,
            const sal_Bool  _bSet
        );


    private:
        COMPHELPER_DLLPRIVATE static const ::com::sun::star::uno::Sequence< sal_Int8 > getUnoTunnelImplementationId();
    };

//.........................................................................
}   // namespace comphelper
//.........................................................................


#endif // COMPHELPER_ACCIMPLACCESS_HXX


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
