/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: AccessibleEditableTextPara.cxx,v $
 * $Revision: 1.53 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_editeng.hxx"

#include <com/sun/star/uno/Any.hxx>
#include <com/sun/star/uno/Reference.hxx>
#include <comphelper/accessiblekeybindinghelper.hxx>

#include "AccessibleHyperlink.hxx"
#include "editeng/unoedprx.hxx"
#include <editeng/flditem.hxx>
#include <vcl/keycodes.hxx>

using namespace ::com::sun::star;


//------------------------------------------------------------------------
//
// AccessibleHyperlink implementation
//
//------------------------------------------------------------------------

namespace accessibility
{

    AccessibleHyperlink::AccessibleHyperlink( SvxAccessibleTextAdapter& r, SvxFieldItem* p, sal_uInt16 nP, sal_uInt16 nR, sal_Int32 nStt, sal_Int32 nEnd, const ::rtl::OUString& rD )
    : rTA( r )
    {
        pFld = p;
        nPara = nP;
        nRealIdx = nR;
        nStartIdx = nStt;
        nEndIdx = nEnd;
        aDescription = rD;
    }

    AccessibleHyperlink::~AccessibleHyperlink()
    {
        delete pFld;
    }

    // XAccessibleAction
    sal_Int32 SAL_CALL AccessibleHyperlink::getAccessibleActionCount() throw (uno::RuntimeException)
    {
         return isValid() ? 1 : 0;
    }

    sal_Bool SAL_CALL AccessibleHyperlink::doAccessibleAction( sal_Int32 nIndex  ) throw (lang::IndexOutOfBoundsException, uno::RuntimeException)
    {
        sal_Bool bRet = sal_False;
        if ( isValid() && ( nIndex == 0 ) )
        {
            rTA.FieldClicked( *pFld, nPara, nRealIdx );
            bRet = sal_True;
        }
        return bRet;
    }

    ::rtl::OUString  SAL_CALL AccessibleHyperlink::getAccessibleActionDescription( sal_Int32 nIndex ) throw (lang::IndexOutOfBoundsException, uno::RuntimeException)
    {
        ::rtl::OUString aDesc;

        if ( isValid() && ( nIndex == 0 ) )
            aDesc = aDescription;

        return aDesc;
    }

    uno::Reference< ::com::sun::star::accessibility::XAccessibleKeyBinding > SAL_CALL AccessibleHyperlink::getAccessibleActionKeyBinding( sal_Int32 nIndex ) throw (lang::IndexOutOfBoundsException, uno::RuntimeException)
    {
        uno::Reference< ::com::sun::star::accessibility::XAccessibleKeyBinding > xKeyBinding;

        if( isValid() && ( nIndex == 0 ) )
        {
            ::comphelper::OAccessibleKeyBindingHelper* pKeyBindingHelper = new ::comphelper::OAccessibleKeyBindingHelper();
            xKeyBinding = pKeyBindingHelper;

            awt::KeyStroke aKeyStroke;
            aKeyStroke.Modifiers = 0;
            aKeyStroke.KeyCode = KEY_RETURN;
            aKeyStroke.KeyChar = 0;
            aKeyStroke.KeyFunc = 0;
            pKeyBindingHelper->AddKeyBinding( aKeyStroke );
        }

        return xKeyBinding;
    }

    // XAccessibleHyperlink
    uno::Any SAL_CALL AccessibleHyperlink::getAccessibleActionAnchor( sal_Int32 /*nIndex*/ ) throw (lang::IndexOutOfBoundsException, uno::RuntimeException)
    {
        return uno::Any();
    }

    uno::Any SAL_CALL AccessibleHyperlink::getAccessibleActionObject( sal_Int32 /*nIndex*/ ) throw (lang::IndexOutOfBoundsException, uno::RuntimeException)
    {
        return uno::Any();
    }

    sal_Int32 SAL_CALL AccessibleHyperlink::getStartIndex() throw (uno::RuntimeException)
    {
        return nStartIdx;
    }

    sal_Int32 SAL_CALL AccessibleHyperlink::getEndIndex() throw (uno::RuntimeException)
    {
        return nEndIdx;
    }

    sal_Bool SAL_CALL AccessibleHyperlink::isValid(  ) throw (uno::RuntimeException)
    {
        return rTA.IsValid();
    }

}  // end of namespace accessibility

//------------------------------------------------------------------------
