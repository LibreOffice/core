/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include <comphelper/unique_disposing_ptr.hxx>
#include <comphelper/processfactory.hxx>

#include <iderdll.hxx>
#include <iderdll2.hxx>
#include <iderid.hxx>
#include <basidesh.hxx>
#include <basidesh.hrc>
#include <basdoc.hxx>
#include <basicmod.hxx>

#include <svl/srchitem.hxx>
#include <com/sun/star/frame/Desktop.hpp>
#include <com/sun/star/script/XLibraryContainerPassword.hpp>
#include <vcl/settings.hxx>


namespace basctl
{

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;

Module* Module::mpModule = nullptr;

namespace
{

class Dll
{
    Shell* m_pShell;
    std::unique_ptr<ExtraData> m_xExtraData;

public:
    Dll ();

    Shell* GetShell() const { return m_pShell; }
    void SetShell (Shell* pShell) { m_pShell = pShell; }
    ExtraData* GetExtraData ();
};

// Holds a basctl::Dll and release it on exit, or dispose of the
//default XComponent, whichever comes first
class DllInstance : public comphelper::unique_disposing_solar_mutex_reset_ptr<Dll>
{
public:
    DllInstance() : comphelper::unique_disposing_solar_mutex_reset_ptr<Dll>(Reference<lang::XComponent>( frame::Desktop::create(comphelper::getProcessComponentContext()), UNO_QUERY_THROW), new Dll)
    { }
};

struct theDllInstance : public rtl::Static<DllInstance, theDllInstance> { };

} // namespace

void EnsureIde ()
{
    theDllInstance::get();
}

Shell* GetShell ()
{
    if (Dll* pDll = theDllInstance::get().get())
        return pDll->GetShell();
    return nullptr;
}

void ShellCreated (Shell* pShell)
{
    Dll* pDll = theDllInstance::get().get();
    if (pDll && !pDll->GetShell())
        pDll->SetShell(pShell);
}

void ShellDestroyed (Shell* pShell)
{
    Dll* pDll = theDllInstance::get().get();
    if (pDll && pDll->GetShell() == pShell)
        pDll->SetShell(nullptr);
}

ExtraData* GetExtraData()
{
    if (Dll* pDll = theDllInstance::get().get())
        return pDll->GetExtraData();
    return nullptr;
}


IDEResId::IDEResId( sal_uInt16 nId ):
    ResId(nId, *Module::Get()->GetResMgr())
{ }

namespace
{

Dll::Dll () :
    m_pShell(nullptr)
{
    SfxObjectFactory* pFact = &DocShell::Factory();
    (void)pFact;

    ResMgr* pMgr = ResMgr::CreateResMgr(
        "basctl", Application::GetSettings().GetUILanguageTag());

    Module::Get() = new Module( pMgr, &DocShell::Factory() );

    GetExtraData(); // to cause GlobalErrorHdl to be set

    SfxModule* pMod = Module::Get();

    SfxObjectFactory& rFactory = DocShell::Factory();
    rFactory.SetDocumentServiceName( "com.sun.star.script.BasicIDE" );

    DocShell::RegisterInterface( pMod );
    Shell::RegisterFactory( SVX_INTERFACE_BASIDE_VIEWSH );
    Shell::RegisterInterface( pMod );
}

ExtraData* Dll::GetExtraData ()
{
    if (!m_xExtraData)
        m_xExtraData.reset(new ExtraData);
    return m_xExtraData.get();
}

} // namespace


// basctl::ExtraData


ExtraData::ExtraData () :
    pSearchItem(new SvxSearchItem(SID_SEARCH_ITEM)),
    bChoosingMacro(false),
    bShellInCriticalSection(false)
{
    StarBASIC::SetGlobalBreakHdl(LINK(this, ExtraData, GlobalBasicBreakHdl));
}

ExtraData::~ExtraData ()
{
    // Resetting ErrorHdl is cleaner indeed but this instance is destroyed
    // pretty late, after the last Basic, anyway.
    // Due to the call there is AppData created then though and not
    // destroyed anymore => MLK's at Purify
//  StarBASIC::SetGlobalErrorHdl( Link() );
//  StarBASIC::SetGlobalBreakHdl( Link() );
//  StarBASIC::setGlobalStarScriptListener( XEngineListenerRef() );
}

void ExtraData::SetSearchItem (const SvxSearchItem& rItem)
{
    pSearchItem.reset(static_cast<SvxSearchItem*>(rItem.Clone()));
}

IMPL_STATIC_LINK_TYPED(ExtraData, GlobalBasicBreakHdl, StarBASIC *, pBasic, sal_uInt16)
{
    sal_uInt16 nRet = 0;
    if (Shell* pShell = GetShell())
    {
        if (BasicManager* pBasMgr = FindBasicManager(pBasic))
        {
            // I do get here twice if Step into protected Basic
            // => bad, if password query twice, also you don't see
            // the lib in the PasswordDlg...
            // => start no password query at this point
            ScriptDocument aDocument( ScriptDocument::getDocumentForBasicManager( pBasMgr ) );
            OSL_ENSURE( aDocument.isValid(), "basctl::ExtraData::GlobalBasicBreakHdl: no document for the basic manager!" );
            if ( aDocument.isValid() )
            {
                OUString aOULibName( pBasic->GetName() );
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
                        nRet = pShell->CallBasicBreakHdl( pBasic );
                    }
                }
            }
        }
    }

    return nRet;
}


} // namespace basctl

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
