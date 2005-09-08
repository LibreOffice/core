/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: accessiblekeybindinghelper.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 02:25:33 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#ifndef COMPHELPER_ACCESSIBLE_KEYBINDING_HELPER_HXX
#define COMPHELPER_ACCESSIBLE_KEYBINDING_HELPER_HXX

#ifndef _COM_SUN_STAR_ACCESSIBILITY_XACCESSIBLEKEYBINDING_HPP_
#include <com/sun/star/accessibility/XAccessibleKeyBinding.hpp>
#endif
#ifndef _CPPUHELPER_IMPLBASE1_HXX_
#include <cppuhelper/implbase1.hxx>
#endif
#ifndef _OSL_MUTEX_HXX_
#include <osl/mutex.hxx>
#endif

#ifndef INCLUDED_VECTOR
#include <vector>
#define INCLUDED_VECTOR
#endif

#ifndef INCLUDED_COMPHELPERDLLAPI_H
#include "comphelper/comphelperdllapi.h"
#endif

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
