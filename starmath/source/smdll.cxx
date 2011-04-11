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
#include "precompiled_starmath.hxx"


#include <sot/factory.hxx>
#include <svx/svxids.hrc>
#include <svx/modctrl.hxx>
#include <svx/zoomctrl.hxx>
#include <sfx2/docfac.hxx>
#include <svx/lboxctrl.hxx>
#include <sfx2/docfile.hxx>
#include <sfx2/docfilt.hxx>
#include <sfx2/app.hxx>
#include <sfx2/taskpane.hxx>

#include <smdll.hxx>
#include <document.hxx>
#include <toolbox.hxx>
#include <view.hxx>

#include <starmath.hrc>

#include <svx/xmlsecctrl.hxx>



bool SmDLL::bInitialized = false;


// Initialization

void SmDLL::Init()
{
    if ( bInitialized )
        return;

    bInitialized = true;

    SfxObjectFactory& rFactory = SmDocShell::Factory();

    SmModule** ppShlPtr = (SmModule**) GetAppData(SHL_SM);
    *ppShlPtr = new SmModule( &rFactory );

    SfxModule *p = SM_MOD();
    SmModule *pp = (SmModule *) p;

    rFactory.SetDocumentServiceName( String::CreateFromAscii("com.sun.star.formula.FormulaProperties") );

    SmModule::RegisterInterface(pp);
    SmDocShell::RegisterInterface(pp);
    SmViewShell::RegisterInterface(pp);

    SmViewShell::RegisterFactory(1);

    SvxZoomStatusBarControl::RegisterControl( SID_ATTR_ZOOM, pp );
    SvxModifyControl::RegisterControl( SID_TEXTSTATUS, pp );
    SvxUndoRedoControl::RegisterControl( SID_UNDO, pp );
    SvxUndoRedoControl::RegisterControl( SID_REDO, pp );
    XmlSecStatusBarControl::RegisterControl( SID_SIGNATURE, pp );

    SmToolBoxWrapper::RegisterChildWindow(true);
    SmCmdBoxWrapper::RegisterChildWindow(true);

    ::sfx2::TaskPaneWrapper::RegisterChildWindow( false, pp );
}


// Deinitialization

void SmDLL::Exit()
{
    // the SdModule must be destroyed
    SmModule** ppShlPtr = (SmModule**) GetAppData(SHL_SM);
    delete (*ppShlPtr);
    (*ppShlPtr) = NULL;

    *GetAppData(SHL_SM) = 0;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
