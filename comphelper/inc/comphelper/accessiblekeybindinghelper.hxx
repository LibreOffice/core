/*************************************************************************
 *
 *  $RCSfile: accessiblekeybindinghelper.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2003-04-24 17:25:46 $
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

#include <vector>


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
    class OAccessibleKeyBindingHelper : public OAccessibleKeyBindingHelper_Base
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
