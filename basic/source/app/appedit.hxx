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



#ifndef _APPEDIT_HXX
#define _APPEDIT_HXX

#ifndef _SCRBAR_HXX //autogen
#include <vcl/scrbar.hxx>
#endif

#include "appwin.hxx"
#include "textedit.hxx"

class BasicFrame;

class AppEdit : public AppWin    {  // Editor window
using Window::Scroll;

public:
    ScrollBar *pVScroll;
    ScrollBar *pHScroll;
    void SetScrollBarRanges();
    sal_uIntPtr nCurTextWidth;
private:
    void InitScrollBars();
protected:
    DECL_LINK( Scroll, ScrollBar* );
public:
    AppEdit( BasicFrame* );
    ~AppEdit();
    sal_uInt16 GetLineNr();                                 // Current line number
    FileType GetFileType();                             // Returns the file type
    virtual long InitMenu( Menu* );                     // Inits the menu
    virtual long DeInitMenu( Menu* );                   // Reset to enable all Shortcuts
    virtual void Command( const CommandEvent& rCEvt );  // Command Handler
    void Resize();
    void PostLoad();
    void PostSaveAs();
    void Mark( short, short, short );                   // Select text
    void Highlight( sal_uInt16 nLine, sal_uInt16 nCol1, sal_uInt16 nCol2 );
    virtual sal_Bool ReloadAllowed(){ return !StarBASIC::IsRunning(); }
    virtual void LoadIniFile();     // (re)load ini file after change
};

#endif
