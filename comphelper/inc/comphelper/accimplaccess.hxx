/*************************************************************************
 *
 *  $RCSfile: accimplaccess.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: hr $ $Date: 2003-03-19 15:58:29 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc..
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef COMPHELPER_ACCIMPLACCESS_HXX
#define COMPHELPER_ACCIMPLACCESS_HXX

#ifndef _CPPUHELPER_IMPLBASE1_HXX_
#include <cppuhelper/implbase1.hxx>
#endif

#ifndef _COM_SUN_STAR_LANG_XUNOTUNNEL_HPP_
#include <com/sun/star/lang/XUnoTunnel.hpp>
#endif

// forward declaration
namespace drafts { namespace com { namespace sun { namespace star { namespace accessibility {
    class XAccessible;
    class XAccessibleContext;
}}}}}

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
    class OAccessibleImplementationAccess : public OAccImpl_Base
    {
    private:
        OAccImpl_Impl*  m_pImpl;

    protected:
        /// retrieves the parent previously set via <method>setAccessibleParent</method>
        ::com::sun::star::uno::Reference< ::drafts::com::sun::star::accessibility::XAccessible >
                implGetForeignControlledParent( ) const;

        /** retrieves the set of currently set states which are controlled by a foreign instance
        @return
            a bit mask, where a set bit 2^n means that the AccessibleStateType n has been set
        */
        sal_Int64   implGetForeignControlledStates( ) const;

        /// sets the accessible parent component
        virtual void    setAccessibleParent(
            const ::com::sun::star::uno::Reference< ::drafts::com::sun::star::accessibility::XAccessible >& _rxAccParent );

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
            const ::com::sun::star::uno::Reference< ::drafts::com::sun::star::accessibility::XAccessibleContext >& _rxComponent
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
            const ::com::sun::star::uno::Reference< ::drafts::com::sun::star::accessibility::XAccessibleContext >& _rxComponent,
            const ::com::sun::star::uno::Reference< ::drafts::com::sun::star::accessibility::XAccessible >& _rxNewParent
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
            const ::com::sun::star::uno::Reference< ::drafts::com::sun::star::accessibility::XAccessibleContext >& _rxComponent,
            const sal_Int16 _nState,
            const sal_Bool  _bSet
        );


    private:
        static const ::com::sun::star::uno::Sequence< sal_Int8 >& getUnoTunnelImplementationId();
    };

//.........................................................................
}   // namespace comphelper
//.........................................................................


#endif // COMPHELPER_ACCIMPLACCESS_HXX


