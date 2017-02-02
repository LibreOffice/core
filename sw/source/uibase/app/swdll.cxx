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

#include <config_features.h>

#include <svx/svdobj.hxx>

#include "globdoc.hrc"

#include <swdll.hxx>
#include <wdocsh.hxx>
#include <globdoc.hxx>
#include <initui.hxx>
#include <swmodule.hxx>
#include <swtypes.hxx>
#include <init.hxx>
#include <dobjfac.hxx>
#include <cfgid.h>

#include <com/sun/star/frame/Desktop.hpp>
#include <unotools/configmgr.hxx>
#include <unotools/moduleoptions.hxx>
#include <comphelper/unique_disposing_ptr.hxx>
#include <comphelper/processfactory.hxx>

#include <svx/fmobjfac.hxx>
#include <svx/svdfield.hxx>
#include <svx/objfac3d.hxx>
#include <editeng/acorrcfg.hxx>

#include <swacorr.hxx>
#include <unomid.h>

#include "swdllimpl.hxx"
#include <o3tl/make_unique.hxx>
using namespace com::sun::star;

namespace
{
    //Holds a SwDLL and release it on exit, or dispose of the
    //default XComponent, whichever comes first
    class SwDLLInstance : public comphelper::unique_disposing_solar_mutex_reset_ptr<SwDLL>
    {
    public:
        SwDLLInstance() : comphelper::unique_disposing_solar_mutex_reset_ptr<SwDLL>(uno::Reference<lang::XComponent>( frame::Desktop::create(comphelper::getProcessComponentContext()), uno::UNO_QUERY_THROW), new SwDLL, true)
        {
        }
    };

    struct theSwDLLInstance : public rtl::Static<SwDLLInstance, theSwDLLInstance> {};
}

namespace SwGlobals
{
    void ensure()
    {
        theSwDLLInstance::get();
    }

    sw::Filters & getFilters()
    {
        return theSwDLLInstance::get().get()->getFilters();
    }
}

SwDLL::SwDLL()
{
    if ( SfxApplication::GetModule(SfxToolsModule::Writer) )    // Module already active
        return;

    std::unique_ptr<SvtModuleOptions> xOpt;
    if (!utl::ConfigManager::IsAvoidConfig())
        xOpt.reset(new SvtModuleOptions);
    SfxObjectFactory* pDocFact = nullptr;
    SfxObjectFactory* pGlobDocFact = nullptr;
    if (xOpt && xOpt->IsWriter())
    {
        pDocFact = &SwDocShell::Factory();
        pGlobDocFact = &SwGlobalDocShell::Factory();
    }

    SfxObjectFactory* pWDocFact = &SwWebDocShell::Factory();

    auto pUniqueModule = o3tl::make_unique<SwModule>(pWDocFact, pDocFact, pGlobDocFact);
    SwModule* pModule = pUniqueModule.get();
    SfxApplication::SetModule(SfxToolsModule::Writer, std::move(pUniqueModule));

    pWDocFact->SetDocumentServiceName("com.sun.star.text.WebDocument");

    if (xOpt && xOpt->IsWriter())
    {
        pGlobDocFact->SetDocumentServiceName("com.sun.star.text.GlobalDocument");
        pDocFact->SetDocumentServiceName("com.sun.star.text.TextDocument");
    }

    // register SvDraw-Fields
    SdrRegisterFieldClasses();

    // register 3D-Objekt-Factory
    E3dObjFactory();

    // register form::component::Form-Objekt-Factory
    FmFormObjFactory();

    SdrObjFactory::InsertMakeObjectHdl( LINK( &aSwObjectFactory, SwObjectFactory, MakeObject ) );

    SAL_INFO( "sw.ui", "Init Core/UI/Filter" );
    // Initialisation of Statics
    ::InitCore();
    filters_.reset(new sw::Filters);
    ::InitUI();

    pModule->InitAttrPool();
    // now SWModule can create its Pool

    // register your view-factories here
    RegisterFactories();

    // register your shell-interfaces here
    RegisterInterfaces();

#if HAVE_FEATURE_DESKTOP
    // register your controllers here
    RegisterControls();
#endif

    if (!utl::ConfigManager::IsAvoidConfig())
    {
        // replace SvxAutocorrect with SwAutocorrect
        SvxAutoCorrCfg& rACfg = SvxAutoCorrCfg::Get();
        const SvxAutoCorrect* pOld = rACfg.GetAutoCorrect();
        rACfg.SetAutoCorrect(new SwAutoCorrect( *pOld ));
    }
}

SwDLL::~SwDLL()
{
    if (!utl::ConfigManager::IsAvoidConfig())
    {
        // fdo#86494 SwAutoCorrect must be deleted before FinitCore
        SvxAutoCorrCfg& rACfg = SvxAutoCorrCfg::Get();
        rACfg.SetAutoCorrect(nullptr); // delete SwAutoCorrect before exit handlers
    }

    // Pool has to be deleted before statics are
    SW_MOD()->RemoveAttrPool();

    ::FinitUI();
    filters_.reset();
    ::FinitCore();
    // sign out Objekt-Factory
    SdrObjFactory::RemoveMakeObjectHdl(LINK(&aSwObjectFactory, SwObjectFactory, MakeObject ));
}

sw::Filters & SwDLL::getFilters()
{
    OSL_ASSERT(filters_);
    return *filters_.get();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
