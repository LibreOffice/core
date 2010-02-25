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
#ifndef _ACCCELL_HXX
#define _ACCCELL_HXX

#include "acccontext.hxx"
#include <com/sun/star/accessibility/XAccessibleValue.hpp>

class SwCellFrm;
class SwFrmFmt;

class SwAccessibleCell : public SwAccessibleContext,
                  ::com::sun::star::accessibility::XAccessibleValue

{
    sal_Bool    bIsSelected;    // protected by base class mutex

    sal_Bool    IsSelected();

    sal_Bool _InvalidateMyCursorPos();
    sal_Bool _InvalidateChildrenCursorPos( const SwFrm *pFrm );

protected:

    // Set states for getAccessibleStateSet.
    // This drived class additionaly sets SELECTABLE(1) and SELECTED(+)
    virtual void GetStates( ::utl::AccessibleStateSetHelper& rStateSet );

    virtual void _InvalidateCursorPos();

    virtual ~SwAccessibleCell();

public:

    SwAccessibleCell( SwAccessibleMap* pInitMap, const SwCellFrm *pCellFrm );

    virtual sal_Bool HasCursor();   // required by map to remember that object

    //=====  XAccessibleContext  ==============================================

    /// Return this object's description.
    virtual ::rtl::OUString SAL_CALL
        getAccessibleDescription (void)
        throw (com::sun::star::uno::RuntimeException);

    //=====  XServiceInfo  ====================================================

    /** Returns an identifier for the implementation of this object.
    */
    virtual ::rtl::OUString SAL_CALL
        getImplementationName (void)
        throw (::com::sun::star::uno::RuntimeException);

    /** Return whether the specified service is supported by this class.
    */
    virtual sal_Bool SAL_CALL
        supportsService (const ::rtl::OUString& sServiceName)
        throw (::com::sun::star::uno::RuntimeException);

    /** Returns a list of all supported services.  In this case that is just
        the AccessibleContext service.
    */
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString> SAL_CALL
        getSupportedServiceNames (void)
        throw (::com::sun::star::uno::RuntimeException);

    virtual void Dispose( sal_Bool bRecursive = sal_False );

    virtual void InvalidatePosOrSize( const SwRect& rFrm );

    //=====  XInterface  ======================================================

    // (XInterface methods need to be implemented to disambiguate
    // between those inherited through SwAcessibleContext and
    // XAccessibleValue).

    virtual ::com::sun::star::uno::Any SAL_CALL queryInterface(
        const ::com::sun::star::uno::Type& aType )
        throw (::com::sun::star::uno::RuntimeException);

    virtual void SAL_CALL acquire(  ) throw ()
        { SwAccessibleContext::acquire(); };

    virtual void SAL_CALL release(  ) throw ()
        { SwAccessibleContext::release(); };

    //====== XTypeProvider ====================================================
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type > SAL_CALL getTypes(  ) throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< sal_Int8 > SAL_CALL getImplementationId(  ) throw(::com::sun::star::uno::RuntimeException);

    //=====  XAccessibleValue  ================================================

private:
    SwFrmFmt* GetTblBoxFormat() const;

public:
    virtual ::com::sun::star::uno::Any SAL_CALL getCurrentValue( )
        throw (::com::sun::star::uno::RuntimeException);

    virtual sal_Bool SAL_CALL setCurrentValue(
        const ::com::sun::star::uno::Any& aNumber )
        throw (::com::sun::star::uno::RuntimeException);

    virtual ::com::sun::star::uno::Any SAL_CALL getMaximumValue(  )
        throw (::com::sun::star::uno::RuntimeException);

    virtual ::com::sun::star::uno::Any SAL_CALL getMinimumValue(  )
        throw (::com::sun::star::uno::RuntimeException);
};


#endif

