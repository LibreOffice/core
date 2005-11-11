/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: itemholder1.cxx,v $
 *
 *  $Revision: 1.8 $
 *
 *  last change: $Author: rt $ $Date: 2005-11-11 13:50:28 $
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

#include "itemholder1.hxx"

//-----------------------------------------------
// includes

#ifndef _COMPHELPER_PROCESSFACTORY_HXX_
#include <comphelper/processfactory.hxx>
#endif

#ifndef _COM_SUN_STAR_LANG_XCOMPONENT_HPP_
#include <com/sun/star/lang/XComponent.hpp>
#endif

#include <accelcfg.hxx>
#include <addxmltostorageoptions.hxx>
#include <cacheoptions.hxx>
#include <cmdoptions.hxx>
#include <compatibility.hxx>
#include <defaultoptions.hxx>
#include <dynamicmenuoptions.hxx>
#include <eventcfg.hxx>
#include <extendedsecurityoptions.hxx>
#include <fltrcfg.hxx>
#include <fontoptions.hxx>
#include <historyoptions.hxx>
#include <inetoptions.hxx>
#include <internaloptions.hxx>
#include <javaoptions.hxx>
#include <lingucfg.hxx>
#include <localisationoptions.hxx>
#include <menuoptions.hxx>
#include <miscopt.hxx>
#include <moduleoptions.hxx>
#include <options3d.hxx>
#include <pathoptions.hxx>
#include <printwarningoptions.hxx>
#include <regoptions.hxx>
#include <optionsdlg.hxx>
#include <saveopt.hxx>
#include <searchopt.hxx>
#include <securityoptions.hxx>
#include <sourceviewconfig.hxx>
#include <startoptions.hxx>
#include <viewoptions.hxx>
#include <workingsetoptions.hxx>
#include <xmlaccelcfg.hxx>

//-----------------------------------------------
// namespaces

namespace css = ::com::sun::star;

//-----------------------------------------------
// declarations

//-----------------------------------------------
ItemHolder1::ItemHolder1()
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
            OSL_ENSURE(sal_False, sMsg.getStr());
        }
    }
#else
    catch(css::uno::Exception&){}
#endif
}

//-----------------------------------------------
ItemHolder1::~ItemHolder1()
{
    impl_releaseAllItems();
}

//-----------------------------------------------
void ItemHolder1::holdConfigItem(EItem eItem)
{
    static ItemHolder1* pHolder = new ItemHolder1();
    pHolder->impl_addItem(eItem);
}

//-----------------------------------------------
void SAL_CALL ItemHolder1::disposing(const css::lang::EventObject& aEvent)
    throw(css::uno::RuntimeException)
{
    css::uno::Reference< css::uno::XInterface > xSelfHold(static_cast< css::lang::XEventListener* >(this), css::uno::UNO_QUERY);
    impl_releaseAllItems();
}

//-----------------------------------------------
void ItemHolder1::impl_addItem(EItem eItem)
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
void ItemHolder1::impl_releaseAllItems()
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
void ItemHolder1::impl_newItem(TItemInfo& rItem)
{
    switch(rItem.eItem)
    {
        case E_ACCELCFG :
            rItem.pItem = new SvtAcceleratorConfiguration();
            break;

        case E_ADDXMLTOSTORAGEOPTIONS :
            rItem.pItem = new SvtAddXMLToStorageOptions();
            break;

        case E_CMDOPTIONS :
            rItem.pItem = new SvtCommandOptions();
            break;

        case E_COMPATIBILITY :
            rItem.pItem = new SvtCompatibilityOptions();
            break;

        case E_DEFAULTOPTIONS :
            rItem.pItem = new SvtDefaultOptions();
            break;

        case E_DYNAMICMENUOPTIONS :
            rItem.pItem = new SvtDynamicMenuOptions();
            break;

        case E_EVENTCFG :
            //rItem.pItem = new GlobalEventConfig();
            break;

        case E_EXTENDEDSECURITYOPTIONS :
            rItem.pItem = new SvtExtendedSecurityOptions();
            break;

        case E_FLTRCFG :
// no ref count            rItem.pItem = new SvtFilterOptions();
            break;

        case E_FONTOPTIONS :
            rItem.pItem = new SvtFontOptions();
            break;

        case E_HISTORYOPTIONS :
            rItem.pItem = new SvtHistoryOptions();
            break;

        case E_INETOPTIONS :
            rItem.pItem = new SvtInetOptions();
            break;

        case E_INTERNALOPTIONS :
            rItem.pItem = new SvtInternalOptions();
            break;

        case E_JAVAOPTIONS :
// no ref count            rItem.pItem = new SvtJavaOptions();
            break;

        case E_LINGUCFG :
            rItem.pItem = new SvtLinguConfig();
            break;

        case E_LOCALISATIONOPTIONS :
            rItem.pItem = new SvtLocalisationOptions();
            break;

        case E_MENUOPTIONS :
            rItem.pItem = new SvtMenuOptions();
            break;

        case E_MISCOPTIONS :
            rItem.pItem = new SvtMiscOptions();
            break;

        case E_MODULEOPTIONS :
            rItem.pItem = new SvtModuleOptions();
            break;

        case E_OPTIONSDLGOPTIONS :
            rItem.pItem = new SvtOptionsDialogOptions();
            break;

        case E_OPTIONS3D :
            rItem.pItem = new SvtOptions3D();
            break;

        case E_PATHOPTIONS :
            rItem.pItem = new SvtPathOptions();
            break;

        case E_PRINTWARNINGOPTIONS :
            rItem.pItem = new SvtPrintWarningOptions();
            break;

        case E_REGOPTIONS :
// no ref count            rItem.pItem = new ::svt::RegOptions();
            break;

        case E_SAVEOPTIONS :
            rItem.pItem = new SvtSaveOptions();
            break;

        case E_SEARCHOPT :
// no ref count            rItem.pItem = new SvtSearchOptions();
            break;

        case E_SECURITYOPTIONS :
            rItem.pItem = new SvtSecurityOptions();
            break;

        case E_SOURCEVIEWCONFIG :
            rItem.pItem = new ::svt::SourceViewConfig();
            break;

        case E_STARTOPTIONS :
            rItem.pItem = new SvtStartOptions();
            break;

        case E_VIEWOPTIONS_DIALOG :
            rItem.pItem = new SvtViewOptions(E_DIALOG, ::rtl::OUString());
            break;

        case E_VIEWOPTIONS_TABDIALOG :
            rItem.pItem = new SvtViewOptions(E_TABDIALOG, ::rtl::OUString());
            break;

        case E_VIEWOPTIONS_TABPAGE :
            rItem.pItem = new SvtViewOptions(E_TABPAGE, ::rtl::OUString());
            break;

        case E_VIEWOPTIONS_WINDOW :
            rItem.pItem = new SvtViewOptions(E_WINDOW, ::rtl::OUString());
            break;

        case E_WORKINGSETOPTIONS :
            rItem.pItem = new SvtWorkingSetOptions();
            break;

        case E_XMLACCELCFG :
            // ??? TODO
            break;
    }
}

//-----------------------------------------------
void ItemHolder1::impl_deleteItem(TItemInfo& rItem)
{
    if (rItem.pItem)
    {
        delete rItem.pItem;
        rItem.pItem = 0;
    }
}

