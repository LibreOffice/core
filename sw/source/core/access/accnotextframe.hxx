/*************************************************************************
 *
 *  $RCSfile: accnotextframe.hxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: vg $ $Date: 2003-04-24 16:12:15 $
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
#ifndef _ACCNOTEXTFRAME_HXX
#define _ACCNOTEXTFRAME_HXX

#ifndef _ACCFRAMEBASE_HXX
#include "accframebase.hxx"
#endif

#ifndef _COM_SUN_STAR_UNO_RUNTIMEEXCEPTION_HPP_
#include <com/sun/star/uno/RuntimeException.hpp>
#endif

#ifndef _COM_SUN_STAR_ACCESSIBILITY_XACCESSIBLEIMAGE_HPP_
#include <com/sun/star/accessibility/XAccessibleImage.hpp>
#endif

class SwFlyFrm;
class SwNoTxtNode;

class SwAccessibleNoTextFrame : public  SwAccessibleFrameBase,
                                public ::com::sun::star::accessibility::XAccessibleImage
{
    SwDepend        aDepend;
    ::rtl::OUString sDesc;

protected:

    virtual ~SwAccessibleNoTextFrame();

    const SwNoTxtNode *GetNoTxtNode() const;

public:

    SwAccessibleNoTextFrame( SwAccessibleMap *pMap, sal_Int16 nRole,
                             const SwFlyFrm *pFlyFrm );

    virtual void Modify( SfxPoolItem *pOld, SfxPoolItem *pNew);

    //=====  XAccessibleContext  ==============================================

    /// Return this object's description.
    virtual ::rtl::OUString SAL_CALL
        getAccessibleDescription (void)
        throw (com::sun::star::uno::RuntimeException);

    //=====  XInterface  ======================================================

    // XInterface methods need to be implemented to disambiguate
    // between those inherited through SwAcessibleContext and
    // XAccessibleImage.

    virtual ::com::sun::star::uno::Any SAL_CALL queryInterface(
        const ::com::sun::star::uno::Type& aType )
        throw (::com::sun::star::uno::RuntimeException);

    virtual void SAL_CALL acquire(  ) throw ()
        { SwAccessibleContext::acquire(); };

    virtual void SAL_CALL release(  ) throw ()
        { SwAccessibleContext::release(); };

    //====== XTypeProvider ====================================================
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type > SAL_CALL getTypes(  ) throw(::com::sun::star::uno::RuntimeException);

    //=====  XAccessibleImage  ================================================

    virtual ::rtl::OUString SAL_CALL
        getAccessibleImageDescription(  )
        throw ( ::com::sun::star::uno::RuntimeException );

    virtual sal_Int32 SAL_CALL
        getAccessibleImageHeight(  )
        throw ( ::com::sun::star::uno::RuntimeException );

    virtual sal_Int32 SAL_CALL
        getAccessibleImageWidth(  )
        throw ( ::com::sun::star::uno::RuntimeException );

    // The object is not visible an longer and should be destroyed
    virtual void Dispose( sal_Bool bRecursive = sal_False );
};


#endif
