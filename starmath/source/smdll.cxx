/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_starmath.hxx"


#include <sot/factory.hxx>
#ifndef _SVX_SVXIDS_HRC //autogen
#include <svx/svxids.hrc>
#endif
#include <svx/modctrl.hxx>
#include <svx/zoomctrl.hxx>
#include <sfx2/docfac.hxx>
#include <svx/lboxctrl.hxx>
#include <sfx2/docfile.hxx>
#include <sfx2/docfilt.hxx>
#include <sfx2/app.hxx>
#include <sfx2/taskpane.hxx>

#ifndef _SMDLL_HXX
#include <smdll.hxx>
#endif
#include <document.hxx>
#include <toolbox.hxx>
#include <view.hxx>

#ifndef _STARMATH_HRC
#include <starmath.hrc>
#endif

#include <svx/xmlsecctrl.hxx>



sal_Bool SmDLL::bInitialized = sal_False;

/*************************************************************************
|*
|* Initialisierung
|*
\************************************************************************/
void SmDLL::Init()
{
    if ( bInitialized )
        return;

    bInitialized = sal_True;

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

    SmToolBoxWrapper::RegisterChildWindow(sal_True);
    SmCmdBoxWrapper::RegisterChildWindow(sal_True);

    ::sfx2::TaskPaneWrapper::RegisterChildWindow( sal_False, pp );
}

/*************************************************************************
|*
|* Deinitialisierung
|*
\************************************************************************/
void SmDLL::Exit()
{
    // the SdModule must be destroyed
    SmModule** ppShlPtr = (SmModule**) GetAppData(SHL_SM);
    delete (*ppShlPtr);
    (*ppShlPtr) = NULL;

    *GetAppData(SHL_SM) = 0;
}
