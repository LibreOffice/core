/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: accessiblekeybindinghelper.hxx,v $
 * $Revision: 1.5 $
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

#ifndef COMPHELPER_ACCESSIBLE_KEYBINDING_HELPER_HXX
#define COMPHELPER_ACCESSIBLE_KEYBINDING_HELPER_HXX

#include <com/sun/star/accessibility/XAccessibleKeyBinding.hpp>
#include <cppuhelper/implbase1.hxx>
#include <osl/mutex.hxx>

#ifndef INCLUDED_VECTOR
#include <vector>
#define INCLUDED_VECTOR
#endif
#include "comphelper/comphelperdllapi.h"

//..............................................................................
namespace comphelper
{
//..............................................................................

    //==============================================================================
    // OAccessibleKeyBindingHelper
    //==============================================================================

    typedef ::cppu::WeakImplHelper1 <   ::com::sun::star::accessibility::XAccessibleKeyBinding
                                    >   OAccessibleKeyBindingHelper_Base;

    /** a helper class for implementing an accessible keybinding
     */
    class COMPHELPER_DLLPUBLIC OAccessibleKeyBindingHelper : public OAccessibleKeyBindingHelper_Base
    {
    private:
        typedef ::std::vector< ::com::sun::star::uno::Sequence< ::com::sun::star::awt::KeyStroke > > KeyBindings;

        KeyBindings     m_aKeyBindings;

    protected:
        ::osl::Mutex    m_aMutex;

        virtual ~OAccessibleKeyBindingHelper();

    public:
        OAccessibleKeyBindingHelper();
        OAccessibleKeyBindingHelper( const OAccessibleKeyBindingHelper& rHelper );

        void AddKeyBinding( const ::com::sun::star::uno::Sequence< ::com::sun::star::awt::KeyStroke >& rKeyBinding ) throw (::com::sun::star::uno::RuntimeException);
        void AddKeyBinding( const ::com::sun::star::awt::KeyStroke& rKeyStroke ) throw (::com::sun::star::uno::RuntimeException);

        // XAccessibleKeyBinding
        virtual sal_Int32 SAL_CALL getAccessibleKeyBindingCount() throw (::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Sequence< ::com::sun::star::awt::KeyStroke > SAL_CALL getAccessibleKeyBinding( sal_Int32 nIndex ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException);
    };

//..............................................................................
}   // namespace comphelper
//..............................................................................

#endif // COMPHELPER_ACCESSIBLE_KEYBINDING_HELPER_HXX
