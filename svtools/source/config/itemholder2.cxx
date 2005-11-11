/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: itemholder2.cxx,v $
 *
 *  $Revision: 1.8 $
 *
 *  last change: $Author: rt $ $Date: 2005-11-11 08:50:34 $
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

#ifdef  SVL_DLLIMPLEMENTATION
#undef  SVL_DLLIMPLEMENTATION
#endif
#define SVT_DLLIMPLEMENTATION

#include "itemholder2.hxx"

//-----------------------------------------------
// includes

#ifndef _COMPHELPER_PROCESSFACTORY_HXX_
#include <comphelper/processfactory.hxx>
#endif

#ifndef _COM_SUN_STAR_LANG_XCOMPONENT_HPP_
#include <com/sun/star/lang/XComponent.hpp>
#endif

#include <accessibilityoptions.hxx>
#include <apearcfg.hxx>
#include <cjkoptions.hxx>
#include <colorcfg.hxx>
#include <ctloptions.hxx>
#include <fontsubstconfig.hxx>
#include <helpopt.hxx>
#include <languageoptions.hxx>
#include <misccfg.hxx>
#include <printoptions.hxx>
#include <syslocaleoptions.hxx>
#include <undoopt.hxx>
#include <useroptions.hxx>

#include <tools/debug.hxx>

//-----------------------------------------------
// namespaces

namespace css = ::com::sun::star;

//-----------------------------------------------
// declarations

//-----------------------------------------------
ItemHolder2::ItemHolder2()
    : ItemHolderMutexBase()
{
    try
    {
        css::uno::Reference< css::lang::XMultiServiceFactory > xSMGR = ::comphelper::getProcessServiceFactory();
        css::uno::Reference< css::lang::XComponent > xCfg(
            xSMGR->createInstance(::rtl::OUString::createFromAscii("com.sun.star.configuration.ConfigurationProvider")),
            css::uno::UNO_QUERY);
        if (xCfg.is())
            xCfg->addEventListener(static_cast< css::lang::XEventListener* >(this));
    }
// #i37892  got errorhandling from   ConfigManager::GetConfigurationProvider()
    catch(css::uno::RuntimeException& rREx)
    {
        throw rREx;
    }
#ifdef DBG_UTIL
    catch(css::uno::Exception& rEx)
    {
        static sal_Bool bMessage = sal_True;
        if(bMessage)
        {
            bMessage = sal_False;
            ::rtl::OString sMsg("CreateInstance with arguments exception: ");
            sMsg += ::rtl::OString(rEx.Message.getStr(),
                        rEx.Message.getLength(),
                        RTL_TEXTENCODING_ASCII_US);
            DBG_ERROR(sMsg.getStr());
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
void SAL_CALL ItemHolder2::disposing(const css::lang::EventObject& aEvent)
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

        case E_CJKOPTIONS :
            rItem.pItem = new SvtCJKOptions();
            break;

        case E_COLORCFG :
            rItem.pItem = new ::svtools::ColorConfig();
            break;

        case E_CTLOPTIONS :
            rItem.pItem = new SvtCTLOptions();
            break;

        case E_FONTSUBSTCONFIG :
// no ref count            rItem.pItem = new SvtFontSubstConfig();
            break;

        case E_HELPOPTIONS :
            rItem.pItem = new SvtHelpOptions();
            break;

        case E_LANGUAGEOPTIONS :
// capsulate CTL and CJL options !            rItem.pItem = new SvtLanguageOptions();
            break;

        case E_MISCCFG :
// no ref count            rItem.pItem = new SfxMiscCfg();
            break;

        case E_PRINTOPTIONS :
            rItem.pItem = new SvtPrinterOptions();
            break;

        case E_PRINTFILEOPTIONS :
            rItem.pItem = new SvtPrintFileOptions();
            break;

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
    if (rItem.pItem)
    {
        delete rItem.pItem;
        rItem.pItem = 0;
    }
/*
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

        case E_HELPOPTIONS :
            delete (SvtHelpOptions*)rItem.pItem;
            break;
    }
*/
}
