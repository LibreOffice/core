/*************************************************************************
 *
 *  $RCSfile: accessiblerelationsethelper.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: vg $ $Date: 2003-04-24 16:47:24 $
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


#ifndef _UTL_ACCESSIBLERELATIONSETHELPER_HXX_
#define _UTL_ACCESSIBLERELATIONSETHELPER_HXX_

#ifndef _COM_SUN_STAR_ACCESSIBILITY_XACCESSIBLESSTATESET_HPP_
#include <com/sun/star/accessibility/XAccessibleRelationSet.hpp>
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

class AccessibleRelationSetHelperImpl;

//=========================================================================
//= XAccessibleRelationSet helper classes
//=========================================================================

//... namespace utl .......................................................
namespace utl
{
//.........................................................................

/** @descr
        This base class provides an implementation of the
        <code>AccessibleRelationSet</code> service.
*/
class AccessibleRelationSetHelper
    :   public cppu::WeakImplHelper1<
        ::com::sun::star::accessibility::XAccessibleRelationSet
        >
{
public:
    //=====  internal  ========================================================
    AccessibleRelationSetHelper ();
    AccessibleRelationSetHelper (const AccessibleRelationSetHelper& rHelper);
protected:
    virtual ~AccessibleRelationSetHelper    (void);
public:

    //=====  XAccessibleRelationSet  ==========================================

    /** Returns the number of relations in this relation set.

        @return
            Returns the number of relations or zero if there are none.
    */
    virtual sal_Int32 SAL_CALL getRelationCount(  )
        throw (::com::sun::star::uno::RuntimeException);

    /** Returns the relation of this relation set that is specified by
        the given index.

        @param nIndex
            This index specifies the relatio to return.

        @return
            For a valid index, i.e. inside the range 0 to the number of
            relations minus one, the returned value is the requested
            relation.  If the index is invalid then the returned relation
            has the type INVALID.

    */
    virtual ::com::sun::star::accessibility::AccessibleRelation SAL_CALL
        getRelation( sal_Int32 nIndex )
            throw (::com::sun::star::lang::IndexOutOfBoundsException,
                    ::com::sun::star::uno::RuntimeException);

    /** Tests whether the relation set contains a relation matching the
        specified key.

        @param aRelationType
            The type of relation to look for in this set of relations.  This
            has to be one of the constants of
            <type>AccessibleRelationType</type>.

        @return
            Returns <TRUE/> if there is a (at least one) relation of the
            given type and <FALSE/> if there is no such relation in the set.
    */
    virtual sal_Bool SAL_CALL containsRelation( sal_Int16 aRelationType )
        throw (::com::sun::star::uno::RuntimeException);

    /** Retrieve and return the relation with the given relation type.

        @param aRelationType
            The type of the relation to return.  This has to be one of the
            constants of <type>AccessibleRelationType</type>.

        @return
            If a relation with the given type could be found than (a copy
            of) this relation is returned.  Otherwise a relation with the
            type INVALID is returned.
    */
    virtual ::com::sun::star::accessibility::AccessibleRelation SAL_CALL
        getRelationByType( sal_Int16 aRelationType )
            throw (::com::sun::star::uno::RuntimeException);

    void AddRelation(
        const ::com::sun::star::accessibility::AccessibleRelation& rRelation)
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
    AccessibleRelationSetHelperImpl*    mpHelperImpl;
};

//.........................................................................
}
//... namespace utl .......................................................
#endif
