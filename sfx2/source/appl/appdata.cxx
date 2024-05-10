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

#include <appdata.hxx>
#include <sfx2/tbxctrl.hxx>
#include <sfx2/stbitem.hxx>
#include <sfx2/childwin.hxx>
#include <sfx2/doctempl.hxx>
#include <sfx2/module.hxx>
#include <sfx2/sidebar/Theme.hxx>
#include <sfx2/objsh.hxx>
#include <appbaslib.hxx>
#include <unoctitm.hxx>
#include <svl/svdde.hxx>

#include <basic/basicmanagerrepository.hxx>
#include <basic/basmgr.hxx>
#include <basic/basrdll.hxx>

using ::basic::BasicManagerRepository;
using ::basic::BasicManagerCreationListener;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::frame::XModel;
using ::com::sun::star::uno::XInterface;

static BasicDLL* pBasic = nullptr;

class SfxBasicManagerCreationListener : public ::basic::BasicManagerCreationListener
{
private:
    SfxAppData_Impl& m_rAppData;

public:
    explicit SfxBasicManagerCreationListener(SfxAppData_Impl& _rAppData)
        : m_rAppData(_rAppData)
    {
    }

    virtual ~SfxBasicManagerCreationListener();

    virtual void onBasicManagerCreated( const Reference< XModel >& _rxForDocument, BasicManager& _rBasicManager ) override;
};

SfxBasicManagerCreationListener::~SfxBasicManagerCreationListener()
{
}

void SfxBasicManagerCreationListener::onBasicManagerCreated( const Reference< XModel >& _rxForDocument, BasicManager& _rBasicManager )
{
    if ( _rxForDocument == nullptr )
        m_rAppData.OnApplicationBasicManagerCreated( _rBasicManager );
}

SfxAppData_Impl::SfxAppData_Impl()
    : pPool(nullptr)
    , pProgress(nullptr)
    , nDocModalMode(0)
    , nRescheduleLocks(0)
    , pBasicManager( new SfxBasicManagerHolder )
    , pBasMgrListener( new SfxBasicManagerCreationListener( *this ) )
    , pViewFrame( nullptr )
    , bDowning( true )
    , bInQuit( false )

{
    pBasic = new BasicDLL;

#if HAVE_FEATURE_SCRIPTING
    BasicManagerRepository::registerCreationListener( *pBasMgrListener );
#endif
}

SfxAppData_Impl::~SfxAppData_Impl()
{
    DeInitDDE();
    pBasicManager.reset();

#if HAVE_FEATURE_SCRIPTING
    BasicManagerRepository::revokeCreationListener( *pBasMgrListener );
    pBasMgrListener.reset();
#endif

    delete pBasic;
}

SfxDocumentTemplates* SfxAppData_Impl::GetDocumentTemplates()
{
    if ( !pTemplates )
        pTemplates.emplace();
    else
        pTemplates->ReInitFromComponent();
    return &*pTemplates;
}

void SfxAppData_Impl::OnApplicationBasicManagerCreated( BasicManager& _rBasicManager )
{
#if !HAVE_FEATURE_SCRIPTING
    (void) _rBasicManager;
#else
    pBasicManager->reset( &_rBasicManager );

    // global constants, additionally to the ones already added by createApplicationBasicManager:
    // ThisComponent
    Reference< XInterface > xCurrentComponent = SfxObjectShell::GetCurrentComponent();
    _rBasicManager.SetGlobalUNOConstant( u"ThisComponent"_ustr, css::uno::Any( xCurrentComponent ) );
#endif
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
