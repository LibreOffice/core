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
#ifndef _SFXAPPIMP_HXX
#define _SFXAPPIMP_HXX

#include <tools/time.hxx>
#include <tools/string.hxx>
class MenuBar;
class UniqueIndex;
class BasicManager;
class Timer;
namespace binfilter {

class SfxTbxCtrlFactArr_Impl;
class SfxStbCtrlFactArr_Impl;
class SfxMenuCtrlFactArr_Impl;
class SfxViewFrameArr_Impl;
class SfxViewShellArr_Impl;
class SfxObjectShellArr_Impl;
class IntroWindow_Impl;
class SfxTemplateDialog;
class SfxDialogLibraryContainer;
class SfxScriptLibraryContainer;
class SfxBasicTestWin;

struct SfxApplication_Impl
{
    Time                        aAutoSaveTime;
    String                      aMemExceptionString;
    String                      aResWarningString;
    String                      aResExceptionString;
    String                      aSysResExceptionString;
    String                      aDoubleExceptionString;
    String                      aBasicSourceName;
    SfxTbxCtrlFactArr_Impl*     pTbxCtrlFac;
    SfxStbCtrlFactArr_Impl*     pStbCtrlFac;
    SfxMenuCtrlFactArr_Impl*    pMenuCtrlFac;
    SfxViewFrameArr_Impl*       pViewFrames;
    SfxViewShellArr_Impl*       pViewShells;
    SfxObjectShellArr_Impl*     pObjShells;
    MenuBar*                    pEmptyMenu;     	// dummy for no-menu
    IntroWindow_Impl*			pIntro;
    UniqueIndex*                pEventHdl;  		// Hook-Liste fuer UserEvents
    SfxTemplateDialog*          pTemplateDlg;
    SfxScriptLibraryContainer*	pBasicLibContainer;
    SfxDialogLibraryContainer*	pDialogLibContainer;
    SfxBasicTestWin*            pBasicTestWin;
    Timer*                		pAutoSaveTimer;
    USHORT                      nDocNo;     		// Laufende Doc-Nummer (AutoName)
    USHORT                      nWarnLevel;
    BOOL                        bConfigLoaded:1;
    BOOL                        bAutoSaveNow:1; 	// ist TRUE, wenn der Timer abgelaufen ist, wenn die App nicht aktiv war
};

}//end of namespace binfilter
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
