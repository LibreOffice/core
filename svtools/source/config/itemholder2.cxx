/*************************************************************************
 *
 *  $RCSfile: itemholder2.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: obo $ $Date: 2004-11-15 17:21:22 $
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

#include "itemholder2.hxx"

//-----------------------------------------------
// includes

#ifndef _COMPHELPER_PROCESSFACTORY_HXX_
#include <comphelper/processfactory.hxx>
#endif

#ifndef _COM_SUN_STAR_LANG_XCOMPONENT_HPP_
#include <com/sun/star/lang/XComponent.hpp>
#endif

#include <svtools/syslocaleoptions.hxx>
#include <svtools/undoopt.hxx>
#include <svtools/useroptions.hxx>

//-----------------------------------------------
// namespaces

namespace css = ::com::sun::star;

//-----------------------------------------------
// declarations

//-----------------------------------------------
ItemHolder2::ItemHolder2()
    : ItemHolderMutexBase()
{
    css::uno::Reference< css::lang::XMultiServiceFactory > xSMGR = ::comphelper::getProcessServiceFactory();
    css::uno::Reference< css::lang::XComponent > xCfg(
        xSMGR->createInstance(::rtl::OUString::createFromAscii("com.sun.star.configuration.ConfigurationProvider")),
        css::uno::UNO_QUERY);
    if (xCfg.is())
        xCfg->addEventListener(static_cast< css::lang::XEventListener* >(this));
}

//-----------------------------------------------
ItemHolder2::~ItemHolder2()
{
    impl_releaseAllItems();
}

//-----------------------------------------------
ItemHolder2* ItemHolder2::getGlobalItemHolder()
{
    static ItemHolder2* pHolder = new ItemHolder2();
    return pHolder;
}

//-----------------------------------------------
void ItemHolder2::holdConfigItem(EItem eItem)
{
    ::osl::ResettableMutexGuard aLock(m_aLock);

    TItems::const_iterator pIt;
    for (  pIt  = m_lItems.begin();
           pIt != m_lItems.end()  ;
         ++pIt                    )
    {
        const TItemInfo& rInfo = *pIt;
        if (rInfo.eItem == eItem)
            return;
    }

    TItemInfo aNewItem;
    aNewItem.eItem = eItem;
    impl_newItem(aNewItem);
    if (aNewItem.pItem)
        m_lItems.push_back(aNewItem);
}

//-----------------------------------------------
void SAL_CALL ItemHolder2::disposing(const css::lang::EventObject& aEvent)
    throw(css::uno::RuntimeException)
{
    impl_releaseAllItems();
}

//-----------------------------------------------
void ItemHolder2::impl_releaseAllItems()
{
    ::osl::ResettableMutexGuard aLock(m_aLock);

    TItems::iterator pIt;
    for (  pIt  = m_lItems.begin();
           pIt != m_lItems.end()  ;
         ++pIt                    )
    {
        TItemInfo& rInfo = *pIt;
        impl_deleteItem(rInfo);
    }
    m_lItems.clear();
}

//-----------------------------------------------
void ItemHolder2::impl_newItem(TItemInfo& rItem)
{
    switch(rItem.eItem)
    {
        case E_SYSLOCALEOPTIONS :
            rItem.pItem = new SvtSysLocaleOptions();
            break;

        case E_UNDOOPTIONS :
            rItem.pItem = new SvtUndoOptions();
            break;

        case E_USEROPTIONS :
            rItem.pItem = new SvtUserOptions();
            break;
    }
}

//-----------------------------------------------
void ItemHolder2::impl_deleteItem(TItemInfo& rItem)
{
    if (!rItem.pItem)
        return;

    switch(rItem.eItem)
    {
        case E_SYSLOCALEOPTIONS :
            delete (SvtSysLocaleOptions*)rItem.pItem;
            break;

        case E_UNDOOPTIONS :
            delete (SvtUndoOptions*)rItem.pItem;
            break;

        case E_USEROPTIONS :
            delete (SvtUserOptions*)rItem.pItem;
            break;
    }

    rItem.pItem = 0;
}
