/*************************************************************************
 *
 *  $RCSfile: accpage.hxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: obo $ $Date: 2004-08-12 12:11:15 $
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
#ifndef _ACCPAGE_HXX
#define _ACCPAGE_HXX

#ifndef _ACCCONTEXT_HXX
#include "acccontext.hxx"
#endif


/**
 * accessibility implementation for the page (SwPageFrm)
 * The page is _only_ visible in the page preview. For the regular
 * document view, it doesn't make sense to add this additional element
 * into the hierarchy. For the page preview, however, the page is the
 * important.
 */
class SwAccessiblePage : public SwAccessibleContext
{
    sal_Bool    bIsSelected;    // protected by base class mutex

    sal_Bool    IsSelected();

protected:

    // return the bounding box for the page in page preview mode
    SwRect GetBounds( /* const SwFrm *pFrm =0 */ );

    // Set states for getAccessibleStateSet.
    // This drived class additionaly sets
    // FOCUSABLE(1) and FOCUSED(+)
    virtual void GetStates( ::utl::AccessibleStateSetHelper& rStateSet );

    virtual void _InvalidateCursorPos();
    virtual void _InvalidateFocus();

    virtual ~SwAccessiblePage();

public:
    // convenience constructor to avoid typecast;
    // may only be called with SwPageFrm argument
    SwAccessiblePage( SwAccessibleMap* pMap, const SwFrm *pFrame );



    //
    // XAccessibleContext methods that need to be overridden
    //

    virtual ::rtl::OUString SAL_CALL getAccessibleDescription (void)
        throw (::com::sun::star::uno::RuntimeException);

    //
    // XServiceInfo
    //

    virtual ::rtl::OUString SAL_CALL getImplementationName (void)
        throw (::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL supportsService (
        const ::rtl::OUString& sServiceName)
        throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString> SAL_CALL
        getSupportedServiceNames (void)
        throw (::com::sun::star::uno::RuntimeException);

    //=====  XTypeProvider  ====================================================
    virtual ::com::sun::star::uno::Sequence< sal_Int8 > SAL_CALL getImplementationId(  ) throw(::com::sun::star::uno::RuntimeException);

    virtual sal_Bool HasCursor();   // required by map to remember that object
};

#endif

