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

#include <tools/config.hxx>
#include <svl/stritem.hxx>

#include <osl/mutex.hxx>

#include <vcl/menu.hxx>
#include <vcl/msgbox.hxx>
#include <svl/dateitem.hxx>
#include <vcl/wrkwin.hxx>
#include "comphelper/processfactory.hxx"

#include <sfx2/viewfrm.hxx>
#include "appdata.hxx"
#include <sfx2/dispatch.hxx>
#include <sfx2/event.hxx>
#include "sfxtypes.hxx"
#include <sfx2/doctempl.hxx>
#include "arrdecl.hxx"
#include <sfx2/docfac.hxx>
#include <sfx2/docfile.hxx>
#include <sfx2/request.hxx>
#include "referers.hxx"
#include "app.hrc"
#include "sfx2/sfxresid.hxx"
#include "objshimp.hxx"
#include <sfx2/appuno.hxx>
#include "imestatuswindow.hxx"
#include "appbaslib.hxx"

#include <basic/basicmanagerrepository.hxx>
#include <basic/basmgr.hxx>

using ::basic::BasicManagerRepository;
using ::basic::BasicManagerCreationListener;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::frame::XModel;
using ::com::sun::star::uno::XInterface;

class SfxBasicManagerCreationListener : public ::basic::BasicManagerCreationListener
{
private:
    SfxAppData_Impl& m_rAppData;

public:
    SfxBasicManagerCreationListener( SfxAppData_Impl& _rAppData ) :m_rAppData( _rAppData ) { }

    virtual ~SfxBasicManagerCreationListener();

    virtual void onBasicManagerCreated( const Reference< XModel >& _rxForDocument, BasicManager& _rBasicManager );
};

SfxBasicManagerCreationListener::~SfxBasicManagerCreationListener()
{
}

void SfxBasicManagerCreationListener::onBasicManagerCreated( const Reference< XModel >& _rxForDocument, BasicManager& _rBasicManager )
{
    if ( _rxForDocument == NULL )
        m_rAppData.OnApplicationBasicManagerCreated( _rBasicManager );
}

SfxAppData_Impl::SfxAppData_Impl( SfxApplication* )
    : pDdeService( 0 )
    , pDocTopics( 0 )
    , pTriggerTopic(0)
    , pDdeService2(0)
    , pFactArr(0)
    , pTopFrames( new SfxFrameArr_Impl )
    , pMatcher( 0 )
    , pBasicResMgr( 0 )
    , pSvtResMgr( 0 )
    , pAppDispatch(NULL)
    , pTemplates( 0 )
    , pPool(0)
    , pDisabledSlotList( 0 )
    , pSaveOptions( 0 )
    , pHelpOptions( 0 )
    , pProgress(0)
    , pTemplateCommon( 0 )
    , nDocModalMode(0)
    , nAutoTabPageId(0)
    , nRescheduleLocks(0)
    , nInReschedule(0)
    , m_xImeStatusWindow(new sfx2::appl::ImeStatusWindow(comphelper::getProcessComponentContext()))
    , pTbxCtrlFac(0)
    , pStbCtrlFac(0)
    , pViewFrames(0)
    , pObjShells(0)
    , pSfxResManager(0)
    , pOfaResMgr(0)
    , pBasicManager( new SfxBasicManagerHolder )
    , pBasMgrListener( new SfxBasicManagerCreationListener( *this ) )
    , pViewFrame( 0 )
    , pSlotPool( 0 )
    , pResMgr( 0 )
    , pAppDispat( 0 )
    , pInterfaces( 0 )
    , nDocNo(0)
    , nInterfaces( 0 )
    , bDowning( sal_True )
    , bInQuit( sal_False )
    , bInvalidateOnUnlock( sal_False )
    , bODFVersionWarningLater( sal_False )

{
#ifndef DISABLE_SCRIPTING
    BasicManagerRepository::registerCreationListener( *pBasMgrListener );
#endif
}

SfxAppData_Impl::~SfxAppData_Impl()
{
    DeInitDDE();
    delete pTopFrames;
    delete pBasicManager;

#ifndef DISABLE_SCRIPTING
    BasicManagerRepository::revokeCreationListener( *pBasMgrListener );
    delete pBasMgrListener;
#endif
}

SfxDocumentTemplates* SfxAppData_Impl::GetDocumentTemplates()
{
    if ( !pTemplates )
        pTemplates = new SfxDocumentTemplates;
    else
        pTemplates->ReInitFromComponent();
    return pTemplates;
}

void SfxAppData_Impl::OnApplicationBasicManagerCreated( BasicManager& _rBasicManager )
{
#ifdef DISABLE_SCRIPTING
    (void) _rBasicManager;
#else
    pBasicManager->reset( &_rBasicManager );

    // global constants, additionally to the ones already added by createApplicationBasicManager:
    // ThisComponent
    Reference< XInterface > xCurrentComponent = SfxObjectShell::GetCurrentComponent();
    _rBasicManager.SetGlobalUNOConstant( "ThisComponent", makeAny( xCurrentComponent ) );
#endif
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
