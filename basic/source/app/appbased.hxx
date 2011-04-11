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

#ifndef _APPBASED_HXX
#define _APPBASED_HXX

#include <basic/sbmod.hxx>
#include "appedit.hxx"
#include "textedit.hxx"

class BasicFrame;
class BreakpointWindow;

class AppBasEd : public AppEdit  {  // Editor-Window:
using DockingWindow::Notify;

    SbModuleRef pMod;               // compile module
    sal_Bool bCompiled;                 // sal_True if compiled
protected:
    DECL_LINK( EditChange, void * );
#define BREAKPOINTSWIDTH 15
    BreakpointWindow *pBreakpoints;
    virtual sal_uInt16 ImplSave();              // Save file

public:
    TYPEINFO();
    AppBasEd( BasicFrame*, SbModule* );
    ~AppBasEd();
    FileType GetFileType();         // Returns Filetype
    SbModule* GetModule()           { return pMod; }
    long InitMenu( Menu* );         // Initialision of the menus
    virtual long DeInitMenu( Menu* );   // Reset to enable all shortcuts
    virtual void Command( const CommandEvent& rCEvt );  // Command handler
    virtual void Resize();              // Includes the breakpoint bar
    virtual void PostLoad();         // Set source of module
    virtual void PostSaveAs();       // Postprocess of module...
    void Reload();
    void LoadSource();               // Load source for object
    sal_Bool Compile();                  // Compile text
    void Run();                      // Run image
    void Disassemble();              // Disassemble image
    const String& GetModName() const { return pMod->GetName(); }
    virtual void Notify( SfxBroadcaster& rBC, const SfxHint& rHint );
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
