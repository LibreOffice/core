/*************************************************************************
 *
 *  $RCSfile: itemholder2.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: obo $ $Date: 2004-11-15 17:21:32 $
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

#ifndef INCLUDED_SVTOOLS_ITEMHOLDER2_HXX_
#define INCLUDED_SVTOOLS_ITEMHOLDER2_HXX_

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

class ItemHolder2 : private ItemHolderMutexBase
                  , public  ::cppu::WeakImplHelper1< css::lang::XEventListener >
{
    //...........................................
    // member
    private:

        TItems m_lItems;

    //...........................................
    // c++ interface
    public:

        ItemHolder2();
        virtual ~ItemHolder2();
        virtual void holdConfigItem(EItem eItem);
        static ItemHolder2* getGlobalItemHolder();

    //...........................................
    // uno interface
    public:

        virtual void SAL_CALL disposing(const css::lang::EventObject& aEvent)
            throw(css::uno::RuntimeException);

    //...........................................
    // helper
    private:

        void impl_releaseAllItems();
        void impl_newItem(TItemInfo& rItem);
        void impl_deleteItem(TItemInfo& rItem);
};

//-----------------------------------------------
// namespaces

#undef css

#endif // INCLUDED_SVTOOLS_ITEMHOLDER2_HXX_
