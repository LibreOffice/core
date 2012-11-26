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

    SbModuleRef pMod;               // compile module
    sal_Bool bCompiled;                 // sal_True if compiled
protected:
    DECL_LINK( EditChange, void * );
#define BREAKPOINTSWIDTH 15
    BreakpointWindow *pBreakpoints;
    virtual sal_uInt16 ImplSave();              // Save file

public:
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
