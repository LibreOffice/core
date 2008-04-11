/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: appbased.hxx,v $
 * $Revision: 1.5 $
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
#ifndef _SB_APPEDIT_HXX
#include "appedit.hxx"
#endif
#ifndef _SB_TEXTEDIT_HXX
#include "textedit.hxx"
#endif

class BasicFrame;
class BreakpointWindow;

class AppBasEd : public AppEdit  {  // Editor-Window:
using DockingWindow::Notify;

    SbModuleRef pMod;               // compiliertes Modul
    BOOL bCompiled;                 // TRUE, wenn compiliert
protected:
    DECL_LINK( EditChange, void * );
#define BREAKPOINTSWIDTH 15
    BreakpointWindow *pBreakpoints;
    virtual USHORT ImplSave();              // Datei speichern

public:
    TYPEINFO();
    AppBasEd( BasicFrame*, SbModule* );
    ~AppBasEd();
    FileType GetFileType();         // Liefert den Filetype
    SbModule* GetModule()           { return pMod; }
    long InitMenu( Menu* );         // Initialisierung des Menues
    virtual long DeInitMenu( Menu* );   // rücksetzen, so daß wieder alle Shortcuts enabled sind
    virtual void Command( const CommandEvent& rCEvt );  // Kommando-Handler
    virtual void Resize();              // Berücksichtigt die Breakpointleiste
    virtual void PostLoad();            // Nachbearbeiten des geladenen (Source am Modul setzen)
    virtual void PostSaveAs();          // Nachbearbeiten des Modils ...
    void Reload();
    void LoadSource();              // Quelltext zu Objekt laden
    BOOL Compile();                 // Text compilieren
    void Run();                     // Image laufenlassen
    void Disassemble();             // Image disassemblieren
    const String& GetModName() const { return pMod->GetName(); }
    virtual void Notify( SfxBroadcaster& rBC, const SfxHint& rHint );
};

#endif
