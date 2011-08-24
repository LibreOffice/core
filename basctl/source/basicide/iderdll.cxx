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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_basctl.hxx"

#include <comphelper/scoped_disposing_ptr.hxx>
#include <comphelper/processfactory.hxx>

#include <ide_pch.hxx>


#include <svheader.hxx>
#include <sfx2/app.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/genlink.hxx>


#include <svl/solar.hrc>
#include <iderdll.hxx>
#include <iderdll2.hxx>
#include <iderid.hxx>
#include <svx/svxids.hrc>
#include <basidesh.hxx>
#include <basidesh.hrc>
#include <basobj.hxx>
#include <bastypes.hxx>
#include <basdoc.hxx>
#include <basicmod.hxx>
#include <propbrw.hxx>


#define ITEMID_SEARCH   0
#include <svl/srchitem.hxx>
#include <com/sun/star/script/XLibraryContainerPassword.hpp>

using ::rtl::OUString;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;

class BasicIDEDLL
{
    BasicIDEShell* m_pShell;
    BasicIDEData* m_pExtraData;

public:
    BasicIDEDLL();
    ~BasicIDEDLL();

    BasicIDEShell* GetShell() const { return m_pShell; }
    void SetShell(BasicIDEShell* pShell) { m_pShell = pShell; }
    BasicIDEData* GetExtraData();
    static BasicIDEDLL* GetDLL();
};

namespace
{
    //Holds a BasicIDEDLL and release it on exit, or dispose of the
    //default XComponent, whichever comes first
    class BasicIDEDLLInstance : public comphelper::scoped_disposing_solar_mutex_reset_ptr<BasicIDEDLL>
    {
    public:
        BasicIDEDLLInstance() : comphelper::scoped_disposing_solar_mutex_reset_ptr<BasicIDEDLL>(::com::sun::star::uno::Reference<com::sun::star::lang::XComponent>(comphelper::getProcessServiceFactory()->createInstance(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.frame.Desktop"))), ::com::sun::star::uno::UNO_QUERY_THROW), new BasicIDEDLL)
        {
        }
    };

    struct theBasicIDEDLLInstance : public rtl::Static<BasicIDEDLLInstance, theBasicIDEDLLInstance> {};
}

namespace BasicIDEGlobals
{
    void ensure()
    {
        theBasicIDEDLLInstance::get();
    }

    BasicIDEShell* GetShell()
    {
        BasicIDEDLL *pIDEGlobals = theBasicIDEDLLInstance::get().get();
        return pIDEGlobals ? pIDEGlobals->GetShell() : NULL;
    }

    void ShellCreated(BasicIDEShell* pShell)
    {
        BasicIDEDLL *pIDEGlobals = theBasicIDEDLLInstance::get().get();
        if (pIDEGlobals && pIDEGlobals->GetShell() == pShell)
            pIDEGlobals->SetShell(pShell);
    }

    void ShellDestroyed(BasicIDEShell* pShell)
    {
        BasicIDEDLL *pIDEGlobals = theBasicIDEDLLInstance::get().get();
        if (pIDEGlobals && pIDEGlobals->GetShell() == pShell)
            pIDEGlobals->SetShell(NULL);
    }

    BasicIDEData* GetExtraData()
    {
        BasicIDEDLL *pIDEGlobals = theBasicIDEDLLInstance::get().get();
        return pIDEGlobals ? pIDEGlobals->GetExtraData() : NULL;
    }
}

BasicIDEDLL* BasicIDEDLL::GetDLL()
{
    return theBasicIDEDLLInstance::get().get();
}

IDEResId::IDEResId( sal_uInt16 nId ):
    ResId( nId, *(*(BasicIDEModule**)GetAppData(SHL_IDE))->GetResMgr() )
{
}

BasicIDEDLL::~BasicIDEDLL()
{
    delete m_pExtraData;
#if 0
    *(BasicIDEDLL**)GetAppData(SHL_IDE) = NULL;
#endif
}

BasicIDEDLL::BasicIDEDLL()
    : m_pShell(0)
    , m_pExtraData(0)
{
    SfxObjectFactory* pFact = &BasicDocShell::Factory();
    (void)pFact;

    ResMgr* pMgr = ResMgr::CreateResMgr(
        "basctl", Application::GetSettings().GetUILocale() );

    BASIC_MOD() = new BasicIDEModule( pMgr, &BasicDocShell::Factory() );

    GetExtraData(); // to cause GlobalErrorHdl to be set

    SfxModule* pMod = BASIC_MOD();

    SfxObjectFactory& rFactory = BasicDocShell::Factory();
    rFactory.SetDocumentServiceName( String::CreateFromAscii( "com.sun.star.script.BasicIDE" ) );

    BasicDocShell::RegisterInterface( pMod );
    BasicIDEShell::RegisterFactory( SVX_INTERFACE_BASIDE_VIEWSH );
    BasicIDEShell::RegisterInterface( pMod );

    PropBrwMgr::RegisterChildWindow();
}

BasicIDEData* BasicIDEDLL::GetExtraData()
{
    if (!m_pExtraData)
        m_pExtraData = new BasicIDEData;
    return m_pExtraData;
}

BasicIDEData::BasicIDEData() : aObjCatPos( INVPOSITION, INVPOSITION )
{
    nBasicDialogCount = 0;
    bChoosingMacro = sal_False;
    bShellInCriticalSection = sal_False;
    pSearchItem = new SvxSearchItem( SID_SEARCH_ITEM );

    StarBASIC::SetGlobalBreakHdl( LINK( this, BasicIDEData, GlobalBasicBreakHdl ) );

    pAccelerator = 0;
}

BasicIDEData::~BasicIDEData()
{
    // Resetting ErrorHdl is cleaner indeed but this instance is destroyed
    // pretty late, after the last Basic, anyway.
    // Due to the call there is AppData created then though and not
    // destroyed anymore => MLK's at Purify
//  StarBASIC::SetGlobalErrorHdl( Link() );
//  StarBASIC::SetGlobalBreakHdl( Link() );
//  StarBASIC::setGlobalStarScriptListener( XEngineListenerRef() );

    delete pSearchItem;
    //delete pAccelerator;
}

SvxSearchItem& BasicIDEData::GetSearchItem() const
{
    return *pSearchItem;
}

void BasicIDEData::SetSearchItem( const SvxSearchItem& rItem )
{
    delete pSearchItem;
    pSearchItem = (SvxSearchItem*)rItem.Clone();
}

IMPL_LINK( BasicIDEData, GlobalBasicBreakHdl, StarBASIC *, pBasic )
{
    long nRet = 0;
    BasicIDEShell* pIDEShell = BasicIDEGlobals::GetShell();
    if ( pIDEShell )
    {
        BasicManager* pBasMgr = BasicIDE::FindBasicManager( pBasic );
        if ( pBasMgr )
        {
            // I do get here twice if Step into protected Basic
            // => bad, if password query twice, also you don't see
            // the lib in the PasswordDlg...
            // => start no password query at this point
            ScriptDocument aDocument( ScriptDocument::getDocumentForBasicManager( pBasMgr ) );
            OSL_ENSURE( aDocument.isValid(), "BasicIDEData::GlobalBasicBreakHdl: no document for the basic manager!" );
            if ( aDocument.isValid() )
            {
                ::rtl::OUString aOULibName( pBasic->GetName() );
                Reference< script::XLibraryContainer > xModLibContainer( aDocument.getLibraryContainer( E_SCRIPTS ), UNO_QUERY );
                if ( xModLibContainer.is() && xModLibContainer->hasByName( aOULibName ) )
                {
                    Reference< script::XLibraryContainerPassword > xPasswd( xModLibContainer, UNO_QUERY );
                    if ( xPasswd.is() && xPasswd->isLibraryPasswordProtected( aOULibName ) && !xPasswd->isLibraryPasswordVerified( aOULibName ) )
                    {
                           // a step-out should get me out of the protected area...
                        nRet = SbDEBUG_STEPOUT;
                    }
                    else
                    {
                          nRet = pIDEShell->CallBasicBreakHdl( pBasic );
                    }
                }
            }
        }
    }

    return nRet;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
