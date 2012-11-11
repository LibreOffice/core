/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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


#include "itemholder2.hxx"

#include <comphelper/processfactory.hxx>
#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/configuration/theDefaultProvider.hpp>

#include <svtools/accessibilityoptions.hxx>
#include <svtools/apearcfg.hxx>
#include <svtools/menuoptions.hxx>
#include <svtools/colorcfg.hxx>
#include <svtools/fontsubstconfig.hxx>
#include <svtools/helpopt.hxx>
#include <svtools/printoptions.hxx>
#include <unotools/options.hxx>
#include <svtools/miscopt.hxx>


#include <tools/debug.hxx>

//-----------------------------------------------
// namespaces

namespace css = ::com::sun::star;


namespace svtools {
//-----------------------------------------------
ItemHolder2::ItemHolder2()
    : ItemHolderMutexBase()
{
    try
    {
        css::uno::Reference< css::uno::XComponentContext > xContext = ::comphelper::getProcessComponentContext();
        css::uno::Reference< css::lang::XComponent > xCfg(
            css::configuration::theDefaultProvider::get( xContext ),
            css::uno::UNO_QUERY_THROW );
        xCfg->addEventListener(static_cast< css::lang::XEventListener* >(this));
    }
    catch(const css::uno::RuntimeException&)
    {
        throw;
    }
#ifdef DBG_UTIL
    catch(const css::uno::Exception& rEx)
    {
        static sal_Bool bMessage = sal_True;
        if(bMessage)
        {
            bMessage = sal_False;
            ::rtl::OString sMsg("CreateInstance with arguments exception: ");
            sMsg += ::rtl::OString(rEx.Message.getStr(),
                        rEx.Message.getLength(),
                        RTL_TEXTENCODING_ASCII_US);
            OSL_FAIL(sMsg.getStr());
        }
    }
#else
    catch(css::uno::Exception&){}
#endif
}

//-----------------------------------------------
ItemHolder2::~ItemHolder2()
{
    impl_releaseAllItems();
}

//-----------------------------------------------
void ItemHolder2::holdConfigItem(EItem eItem)
{
    static ItemHolder2* pHolder = new ItemHolder2();
    pHolder->impl_addItem(eItem);
}

//-----------------------------------------------
void SAL_CALL ItemHolder2::disposing(const css::lang::EventObject&)
    throw(css::uno::RuntimeException)
{
    impl_releaseAllItems();
}

//-----------------------------------------------
void ItemHolder2::impl_addItem(EItem eItem)
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
        case E_ACCESSIBILITYOPTIONS :
            rItem.pItem = new SvtAccessibilityOptions();
            break;

        case E_APEARCFG :
// no ref count            rItem.pItem = new SvtTabAppearanceCfg();
            break;

        case E_COLORCFG :
            rItem.pItem = new ::svtools::ColorConfig();
            break;

        case E_FONTSUBSTCONFIG :
// no ref count            rItem.pItem = new SvtFontSubstConfig();
            break;

        case E_HELPOPTIONS :
            rItem.pItem = new SvtHelpOptions();
            break;

        case E_MENUOPTIONS :
            rItem.pItem = new SvtMenuOptions();
            break;

        case E_PRINTOPTIONS :
            rItem.pItem = new SvtPrinterOptions();
            break;

        case E_PRINTFILEOPTIONS :
            rItem.pItem = new SvtPrintFileOptions();
            break;

        case E_MISCOPTIONS :
            rItem.pItem = new SvtMiscOptions();
            break;

        default:
            OSL_ASSERT(false);
            break;
    }
}

//-----------------------------------------------
void ItemHolder2::impl_deleteItem(TItemInfo& rItem)
{
    if (rItem.pItem)
    {
        delete rItem.pItem;
        rItem.pItem = 0;
    }
}

} // namespace svtools

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
