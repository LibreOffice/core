/*************************************************************************
 *
 *  $RCSfile: accessiblestatesethelper.hxx,v $
 *
 *  $Revision: 1.10 $
 *
 *  last change: $Author: vg $ $Date: 2003-04-24 16:47:34 $
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
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/


#ifndef _UTL_ACCESSIBLESTATESETHELPER_HXX_
#define _UTL_ACCESSIBLESTATESETHELPER_HXX_

#ifndef _COM_SUN_STAR_ACCESSIBILITY_XACCESSIBLESSTATESET_HPP_
#include <com/sun/star/accessibility/XAccessibleStateSet.hpp>
#endif

#ifndef _COM_SUN_STAR_UNO_REFERENCE_HXX_
#include <com/sun/star/uno/Reference.hxx>
#endif
#ifndef _CPPUHELPER_WEAK_HXX_
#include <cppuhelper/weak.hxx>
#endif
#ifndef _COM_SUN_STAR_LANG_XSERVICEINFO_HPP_
#include <com/sun/star/lang/XServiceInfo.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XTYPEPROVIDER_HPP_
#include <com/sun/star/lang/XTypeProvider.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XSERVICENAME_HPP_
#include <com/sun/star/lang/XServiceName.hpp>
#endif

#ifndef _VOS_MUTEX_HXX_
#include <vos/mutex.hxx>
#endif
#include <cppuhelper/implbase1.hxx>
#include <unotools/servicehelper.hxx>

class AccessibleStateSetHelperImpl;

//=========================================================================
//= XAccessibleStateSet helper classes
//=========================================================================

//... namespace utl .......................................................
namespace utl
{
//.........................................................................

/** @descr
        This base class provides an implementation of the
        <code>AccessibleStateSet</code> service.
*/
class AccessibleStateSetHelper
    :   public cppu::WeakImplHelper1<
        ::com::sun::star::accessibility::XAccessibleStateSet
        >
{
public:
    //=====  internal  ========================================================

    AccessibleStateSetHelper ();
    /** constructs an object with some states initially set

        <p>This ctor is compatible with
        <method scope="comphelper">OAccessibleImplementationAccess::implGetForeignControlledStates</method></p>

        @param _nInitialStates
            is a bit mask. Every bit 2^n means that the state number n (as got from the
            AccessibleStateType constants) should be set initially.
    */
    AccessibleStateSetHelper ( const sal_Int64 _nInitialStates );

    AccessibleStateSetHelper ( const AccessibleStateSetHelper& rHelper );
protected:
    virtual ~AccessibleStateSetHelper   (void);
public:

    //=====  XAccessibleStateSet  ==============================================

    /** Checks whether the current state set is empty.

        @return
            Returns <TRUE/> if there is no state in this state set and
            <FALSE/> if there is at least one state set in it.
    */
    virtual sal_Bool SAL_CALL isEmpty ()
        throw (::com::sun::star::uno::RuntimeException);

    /** Checks if the given state is a member of the state set of this
        object.

        @param aState
            The state for which to check membership.  This has to be one of
            the constants of <type>AccessibleStateType</type>.

        @return
            Returns <TRUE/> if the given state is a memeber of this object's
            state set and <FALSE/> otherwise.
    */
    virtual sal_Bool SAL_CALL contains (sal_Int16 aState)
        throw (::com::sun::star::uno::RuntimeException);

    /** Checks if all of the given states are in this object's state
        set.

        @param aStateSet
            This sequence of states is interpreted as set and every of its
            members, duplicates are ignored, is checked for membership in
            this object's state set.  Each state has to be one of the
            constants of <type>AccessibleStateType</type>.

        @return
            Returns <TRUE/> if all states of the given state set are members
            of this object's state set.  <FALSE/> is returned if at least
            one of the states in the given state is not a member of this
            object's state set.
    */
    virtual sal_Bool SAL_CALL containsAll (
        const ::com::sun::star::uno::Sequence<sal_Int16>& rStateSet)
        throw (::com::sun::star::uno::RuntimeException);

    /** Returns a sequence of all states.
    */
    virtual com::sun::star::uno::Sequence<sal_Int16> SAL_CALL getStates()
        throw (::com::sun::star::uno::RuntimeException);

    /** Adds a state to the set.
    */
    void    AddState(sal_Int16 aState)
        throw (::com::sun::star::uno::RuntimeException);

    /** Removes a state from the set if the set contains the state, otherwise nothing is done.
    */
    void    RemoveState(sal_Int16 aState)
        throw (::com::sun::star::uno::RuntimeException);

    /** Compares the set with the set given by rComparativeValue and puts the results
        into rOldStates and rNewStates.

        rOldStates contains after call all states which are in the own set and
        not in the comparative set.

        rNewStates contains after call all states which are in the comparative
         set and not in the own set.
    */
    sal_Bool Compare(const AccessibleStateSetHelper& rComparativeValue,
                        AccessibleStateSetHelper& rOldStates,
                        AccessibleStateSetHelper& rNewStates)
        throw (::com::sun::star::uno::RuntimeException);

    //=====  XTypeProvider  ===================================================

    /** Returns a sequence of all supported interfaces.
    */
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type> SAL_CALL
        getTypes (void)
        throw (::com::sun::star::uno::RuntimeException);

    /** Returns a implementation id.
    */
    virtual ::com::sun::star::uno::Sequence<sal_Int8> SAL_CALL
        getImplementationId (void)
        throw (::com::sun::star::uno::RuntimeException);

protected:
    /// Mutex guarding this object.
    ::vos::OMutex maMutex;

private:
    /// The implementation of this helper interface.
    AccessibleStateSetHelperImpl*   mpHelperImpl;
};

//.........................................................................
}
//... namespace utl .......................................................
#endif
