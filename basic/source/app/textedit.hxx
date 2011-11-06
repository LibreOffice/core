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



#ifndef _TEXTEDIT_HXX
#define _TEXTEDIT_HXX

class AppEdit;
class TextEngine;
class TextView;
class TextEdit;
class BreakpointWindow;

#include <vcl/timer.hxx>
#include <tools/table.hxx>
#include <tools/debug.hxx>
#include <svl/lstner.hxx>
#include <svtools/svmedit.hxx>

#include "dataedit.hxx"

//#include <xtextedt.hxx>

class TextEditImp : public Window, public SfxListener
{
using Window::Notify;

protected:
    void            DoSyntaxHighlight( sal_uIntPtr nPara );


private:
    AppEdit *pAppEdit;
    Link ModifyHdl;

    Timer           aSyntaxIdleTimer;
    Timer           aImplSyntaxIdleTimer;
    DECL_LINK( SyntaxTimerHdl, Timer * );
    Table           aSyntaxLineTable;

    void Notify( SfxBroadcaster& rBC, const SfxHint& rHint );

    void            ImpDoHighlight( const String& rSource, sal_uIntPtr nLineOff );
    sal_Bool            bHighlightning;
    sal_Bool            bDoSyntaxHighlight;
    sal_Bool            bDelayHighlight;


    SbxBase* GetSbxAtMousePos( String &aWord );
    virtual void MouseMove( const MouseEvent& rMEvt );
    DECL_LINK( HideVarContents, void* );
    DECL_LINK( ShowVarContents, void* );
    Point aTipPos;
    String aTipWord;
    sal_uIntPtr nTipId;

    Timer HideTipTimer;
    Timer ShowTipTimer;

    sal_Bool bViewMoved;

public:
    TextEditImp( AppEdit *pParent, const WinBits& aBits );
    ~TextEditImp();

    TextEngine *pTextEngine;
    TextView *pTextView;

    void SetFont( const Font& rNewFont );
    sal_Bool IsModified();
    void SetModifyHdl( Link l ){ ModifyHdl = l; }

    void                KeyInput( const KeyEvent& rKeyEvent );
    void                Paint( const Rectangle& rRect );
    void                MouseButtonUp( const MouseEvent& rMouseEvent );
    void                MouseButtonDown( const MouseEvent& rMouseEvent );
//  void                MouseMove( const MouseEvent& rMouseEvent );
    void                Command( const CommandEvent& rCEvt );
    //sal_Bool              Drop( const DropEvent& rEvt );
    //sal_Bool              QueryDrop( DropEvent& rEvt );

    sal_Bool                ViewMoved();

    void DoDelayedSyntaxHighlight( xub_StrLen nPara );
    void InvalidateSyntaxHighlight();
    void SyntaxHighlight( sal_Bool bNew );
    void BuildKontextMenu( PopupMenu *&pMenu );
};



DBG_NAMEEX(TextEdit)
class TextEdit : public DataEdit {

    BreakpointWindow    *pBreakpointWindow;
    sal_Bool bFileWasUTF8;
    sal_Bool bSaveAsUTF8;

public:
    TextEdit( AppEdit*, const WinBits& );
    ~TextEdit();
    void Highlight( sal_uIntPtr nLine, xub_StrLen nCol1, xub_StrLen nCol2 );
    TextEditImp& GetTextEditImp() { return aEdit; }

    void                SetBreakpointWindow( BreakpointWindow *pBPWindow ){ pBreakpointWindow = pBPWindow; }
    BreakpointWindow    *GetBreakpointWindow(){ return pBreakpointWindow; }

    DATA_FUNC_DEF( aEdit, TextEditImp )

    virtual void BuildKontextMenu( PopupMenu *&pMenu );

    void SaveAsUTF8( sal_Bool bUTF8 ) { bSaveAsUTF8 = bUTF8; }
};

#endif
