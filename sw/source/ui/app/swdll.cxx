/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
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
#include <unotools/moduleoptions.hxx>
#include <comphelper/scoped_disposing_ptr.hxx>
#include <comphelper/processfactory.hxx>

#include <svx/fmobjfac.hxx>
#include <svx/svdfield.hxx>
#include <svx/objfac3d.hxx>

#include <unomid.h>

#include "swdllimpl.hxx"

using namespace com::sun::star;

namespace
{
    
    
    class SwDLLInstance : public comphelper::scoped_disposing_solar_mutex_reset_ptr<SwDLL>
    {
    public:
        SwDLLInstance() : comphelper::scoped_disposing_solar_mutex_reset_ptr<SwDLL>(uno::Reference<lang::XComponent>( frame::Desktop::create(comphelper::getProcessComponentContext()), uno::UNO_QUERY_THROW), new SwDLL)
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
    
    SwModule** ppShlPtr = (SwModule**) GetAppData(SHL_WRITER);
    if ( *ppShlPtr )
        return;

    SvtModuleOptions aOpt;
    SfxObjectFactory* pDocFact = 0;
    SfxObjectFactory* pGlobDocFact = 0;
    if ( aOpt.IsWriter() )
    {
        pDocFact = &SwDocShell::Factory();
        pGlobDocFact = &SwGlobalDocShell::Factory();
    }

    SfxObjectFactory* pWDocFact = &SwWebDocShell::Factory();

    SwModule* pModule = new SwModule( pWDocFact, pDocFact, pGlobDocFact );
    *ppShlPtr = pModule;

    pWDocFact->SetDocumentServiceName(OUString("com.sun.star.text.WebDocument"));

    if ( aOpt.IsWriter() )
    {
        pGlobDocFact->SetDocumentServiceName(OUString("com.sun.star.text.GlobalDocument"));
        pDocFact->SetDocumentServiceName(OUString("com.sun.star.text.TextDocument"));
    }

    
    SdrRegisterFieldClasses();

    
    E3dObjFactory();

    
    FmFormObjFactory();

    SdrObjFactory::InsertMakeObjectHdl( LINK( &aSwObjectFactory, SwObjectFactory, MakeObject ) );

    SAL_INFO( "sw.ui", "Init Core/UI/Filter" );
    
    ::_InitCore();
    filters_.reset(new sw::Filters);
    ::_InitUI();

    pModule->InitAttrPool();
    

    
    RegisterFactories();

#if HAVE_FEATURE_DESKTOP
    
    RegisterInterfaces();

    
    RegisterControls();
#endif
}

SwDLL::~SwDLL()
{
    
    SW_MOD()->RemoveAttrPool();

    ::_FinitUI();
    filters_.reset();
    ::_FinitCore();
    
    SdrObjFactory::RemoveMakeObjectHdl(LINK(&aSwObjectFactory, SwObjectFactory, MakeObject ));
#if 0
    
    SwModule** ppShlPtr = (SwModule**) GetAppData(SHL_WRITER);
    delete (*ppShlPtr);
    (*ppShlPtr) = NULL;
#endif
}

sw::Filters & SwDLL::getFilters()
{
    OSL_ASSERT(filters_);
    return *filters_.get();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
