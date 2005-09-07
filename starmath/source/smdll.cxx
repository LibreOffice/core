/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: smdll.cxx,v $
 *
 *  $Revision: 1.10 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 15:12:25 $
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

#pragma hdrstop

#ifndef _SOT_FACTORY_HXX //autogen
#include <sot/factory.hxx>
#endif
#ifndef _SVX_SVXIDS_HRC //autogen
#include <svx/svxids.hrc>
#endif
#ifndef _SVX_MODCTRL_HXX //autogen
#include <svx/modctrl.hxx>
#endif
#ifndef _SVX_ZOOMCTRL_HXX //autogen
#include <svx/zoomctrl.hxx>
#endif
#ifndef _SFX_OBJFAC_HXX
#include <sfx2/docfac.hxx>
#endif
#ifndef _SVX_LBOXCTRL_HXX_
#include <svx/lboxctrl.hxx>
#endif
#ifndef _SFXDOCFILE_HXX //autogen
#include <sfx2/docfile.hxx>
#endif
#ifndef _SFX_DOCFILT_HACK_HXX //autogen
#include <sfx2/docfilt.hxx>
#endif
#ifndef _SFXAPP_HXX //autogen
#include <sfx2/app.hxx>
#endif

#ifndef _SMDLL_HXX
#include <smdll.hxx>
#endif
#ifndef DOCUMENT_HXX
#include <document.hxx>
#endif
#ifndef TOOLBOX_HXX
#include <toolbox.hxx>
#endif
#ifndef VIEW_HXX
#include <view.hxx>
#endif

#ifndef _STARMATH_HRC
#include <starmath.hrc>
#endif

#include <svx/xmlsecctrl.hxx>



BOOL SmDLL::bInitialized = FALSE;

/*************************************************************************
|*
|* Initialisierung
|*
\************************************************************************/
void SmDLL::Init()
{
    if ( bInitialized )
        return;

    bInitialized = TRUE;

    SfxObjectFactory& rFactory = SmDocShell::Factory();

    SmModule** ppShlPtr = (SmModule**) GetAppData(SHL_SM);
    *ppShlPtr = new SmModule( &rFactory );

    SfxModule *p = SM_MOD1();
    SmModule *pp = (SmModule *) p;

    rFactory.RegisterHelpFile (C2S("smath.svh"));
    rFactory.SetDocumentServiceName( String::CreateFromAscii("com.sun.star.formula.FormulaProperties") );
    rFactory.RegisterMenuBar( SmResId(RID_SMMENU) );
    rFactory.RegisterAccel ( SmResId(RID_SMACCEL) );

    SmModule::RegisterInterface(pp);
    SmDocShell::RegisterInterface(pp);
    SmViewShell::RegisterInterface(pp);

    SmViewShell::RegisterFactory(1);

    SvxZoomStatusBarControl::RegisterControl( SID_ATTR_ZOOM, pp );
    SvxModifyControl::RegisterControl( SID_TEXTSTATUS, pp );
    SvxUndoRedoControl::RegisterControl( SID_UNDO, pp );
    SvxUndoRedoControl::RegisterControl( SID_REDO, pp );
    XmlSecStatusBarControl::RegisterControl( SID_SIGNATURE, pp );

    SmToolBoxWrapper::RegisterChildWindow(TRUE);
    SmCmdBoxWrapper::RegisterChildWindow(TRUE);
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
