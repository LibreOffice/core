/*************************************************************************
 *
 *  $RCSfile: appbased.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:12:08 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef _APPBASED_HXX
#define _APPBASED_HXX

#ifndef _SB_SBMOD_HXX //autogen
#include <sbmod.hxx>
#endif
#ifndef _SB_APPEDIT_HXX
#include "appedit.hxx"
#endif
#ifndef _SB_TEXTEDIT_HXX
#include "textedit.hxx"
#endif

class BasicFrame;
class BreakpointWindow;

class AppBasEd : public AppEdit  {  // Editor-Window:
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
