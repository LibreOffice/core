/*************************************************************************
 *
 *  $RCSfile: AccessibleCellBase.hxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: sab $ $Date: 2002-02-20 13:50:58 $
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


#ifndef _SC_ACCESSIBLECELLBASE_HXX
#define _SC_ACCESSIBLECELLBASE_HXX

#ifndef _SC_ACCESSIBLECONTEXTBASE_HXX
#include "AccessibleContextBase.hxx"
#endif
#ifndef SC_SCGLOB_HXX
#include "global.hxx"
#endif

#ifndef _DRAFTS_COM_SUN_STAR_ACCESSIBILITY_XACCESSIBLEVALUE_HPP_
#include <drafts/com/sun/star/accessibility/XAccessibleValue.hpp>
#endif

class ScTabViewShell;

class ScAccessibleCellBase
    :   public  ::drafts::com::sun::star::accessibility::XAccessibleValue,
        public  ScAccessibleContextBase
{
public:
    //=====  internal  ========================================================
    ScAccessibleCellBase (
        const ::com::sun::star::uno::Reference<
        ::drafts::com::sun::star::accessibility::XAccessible>& rxParent,
        ScDocument* pDoc,
        ScAddress& rCellAddress,
        sal_Int32 nIndex);
protected:
    virtual ~ScAccessibleCellBase ();
public:

    //=====  XInterface  ======================================================

    virtual ::com::sun::star::uno::Any SAL_CALL queryInterface(
                    const ::com::sun::star::uno::Type & rType )
                    throw(::com::sun::star::uno::RuntimeException);

    /** Increase the reference count.
    */
    virtual void SAL_CALL
                acquire (void)
                    throw ();

    /** Decrease the reference count.
    */
    virtual void SAL_CALL
                release (void)
                    throw ();

    //=====  XAccessibleComponent  ============================================

    /** Determines if the object is visible.

        <p>If an object and all of its parents are visible then the object
        is also called showing.  If an object is showing then it has also
        set the <const>AccessibleStateType::VISIBLE</const> state set in its
        <type>AccessibleStateSet</type>.</p>

        @return
            Returns <TRUE/> if the object is visible and <FALSE/> otherwise.

        @see isShowing
    */
    virtual sal_Bool SAL_CALL isVisible(  )
        throw (::com::sun::star::uno::RuntimeException);

    //=====  XAccessibleContext  ==============================================

    /// Return this objects index among the parents children.
    virtual sal_Int32 SAL_CALL
        getAccessibleIndexInParent (void)
        throw (::com::sun::star::uno::RuntimeException);

protected:
    /// Return this object's description.
    virtual ::rtl::OUString SAL_CALL
        createAccessibleDescription (void)
        throw (::com::sun::star::uno::RuntimeException);

    /// Return the object's current name.
    virtual ::rtl::OUString SAL_CALL
        createAccessibleName (void)
        throw (::com::sun::star::uno::RuntimeException);

    //=====  XAccessibleValue  ================================================

    /** Returns the value of this object as a number.

        <p>The exact return type is implementation dependent.  Typical types
        are long and double.</p>

        @return
            Returns the current value represented by this object.
    */
    virtual ::com::sun::star::uno::Any SAL_CALL
        getCurrentValue(  )
        throw (::com::sun::star::uno::RuntimeException);

    /** Sets the value of this object to the given number.

        <p>The argument is clipped to the valid interval whose upper and
        lower bounds are returned by the methods
        <member>getMaximumAccessibleValue</member> and
        <member>getMinimumAccessibleValue</member>, i.e. if it is lower than
        the minimum value the new value will be the minimum and if it is
        greater than the maximum then the new value will be the maximum.</p>

        @param aNumber
            The new value represented by this object.  The set of admissible
            types for this argument is implementation dependent.

        @return
            Returns <TRUE/> if the new value could successfully be set and
            <FALSE/> otherwise.
    */
    virtual sal_Bool SAL_CALL
        setCurrentValue( const ::com::sun::star::uno::Any& aNumber )
        throw (::com::sun::star::uno::RuntimeException);

    /** Returns the maximal value that can be represented by this object.

        <p>The type of the returned value is implementation dependent.  It
        does not have to be the same type as that returned by
        <member>getCurrentAccessibleValue</member>.</p>

        @return
            Returns the maximal value in an implementation dependent type.
            If this object has no upper bound then an empty object is
            returned.
    */
    virtual ::com::sun::star::uno::Any SAL_CALL
        getMaximumValue(  )
        throw (::com::sun::star::uno::RuntimeException);

    /** Returns the minimal value that can be represented by this object.

        <p>The type of the returned value is implementation dependent.  It
        does not have to be the same type as that returned by
        <member>getCurrentAccessibleValue</member>.</p>

        @return
            Returns the minimal value in an implementation dependent type.
            If this object has no upper bound then an empty object is
            returned.
    */
    virtual ::com::sun::star::uno::Any SAL_CALL
        getMinimumValue(  )
        throw (::com::sun::star::uno::RuntimeException);

    //=====  XServiceInfo  ====================================================

    /** Returns an identifier for the implementation of this object.
    */
    virtual ::rtl::OUString SAL_CALL
        getImplementationName (void)
        throw (::com::sun::star::uno::RuntimeException);

    //=====  XTypeProvider  ===================================================

    /** Returns a sequence of all supported interfaces.
    */
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type> SAL_CALL
        getTypes (void)
        throw (::com::sun::star::uno::RuntimeException);

protected:
    ScAddress maCellAddress;

    ScDocument* mpDoc;

    sal_Int32 mnIndex;
};


#endif
