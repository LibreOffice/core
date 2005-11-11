/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: itemholder1.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2005-11-11 08:50:22 $
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

#ifndef INCLUDED_SVTOOLS_ITEMHOLDER1_HXX_
#define INCLUDED_SVTOOLS_ITEMHOLDER1_HXX_

//-----------------------------------------------
// includes

#include "itemholderbase.hxx"

#ifndef _CPPUHELPER_IMPLBASE1_HXX_
#include <cppuhelper/implbase1.hxx>
#endif

#ifndef _COM_SUN_STAR_LANG_XEVENTLISTENER_HPP_
#include <com/sun/star/lang/XEventListener.hpp>
#endif

//-----------------------------------------------
// namespaces

#ifdef css
#error "Cant use css as namespace alias."
#else
#define css ::com::sun::star
#endif

//-----------------------------------------------
// definitions

class ItemHolder1 : private ItemHolderMutexBase
                  , public  ::cppu::WeakImplHelper1< css::lang::XEventListener >
{
    //...........................................
    // member
    private:

        TItems m_lItems;

    //...........................................
    // c++ interface
    public:

        ItemHolder1();
        virtual ~ItemHolder1();
        static void holdConfigItem(EItem eItem);

    //...........................................
    // uno interface
    public:

        virtual void SAL_CALL disposing(const css::lang::EventObject& aEvent)
            throw(css::uno::RuntimeException);

    //...........................................
    // helper
    private:

        void impl_addItem(EItem eItem);
        void impl_releaseAllItems();
        void impl_newItem(TItemInfo& rItem);
        void impl_deleteItem(TItemInfo& rItem);
};

//-----------------------------------------------
// namespaces

#undef css

#endif // INCLUDED_SVTOOLS_ITEMHOLDER1_HXX_
