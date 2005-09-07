/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: textedit.hxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 21:22:00 $
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

#ifndef _TEXTEDIT_HXX
#define _TEXTEDIT_HXX

class AppEdit;
class TextEngine;
class TextView;
class TextEdit;
class BreakpointWindow;

#ifndef _SV_TIMER_HXX //autogen
#include <vcl/timer.hxx>
#endif
#ifndef _TOOLS_TABLE_HXX //autogen
#include <tools/table.hxx>
#endif
#ifndef _TOOLS_DEBUG_HXX //autogen
#include <tools/debug.hxx>
#endif
#ifndef _SFXLSTNER_HXX //autogen
#include <svtools/lstner.hxx>
#endif
#include <svtools/svmedit.hxx>

#include "dataedit.hxx"

//#include <xtextedt.hxx>

class TextEditImp : public Window, public SfxListener
{
protected:
    void            DoSyntaxHighlight( ULONG nPara );


private:
    AppEdit *pAppEdit;
    Link ModifyHdl;

    Timer           aSyntaxIdleTimer;
    Timer           aImplSyntaxIdleTimer;
    DECL_LINK( SyntaxTimerHdl, Timer * );
    Table           aSyntaxLineTable;

    void Notify( SfxBroadcaster& rBC, const SfxHint& rHint );

    void            ImpDoHighlight( const String& rSource, ULONG nLineOff );
    BOOL            bHighlightning;
    BOOL            bDoSyntaxHighlight;
    BOOL            bDelayHighlight;


    SbxBase* GetSbxAtMousePos( String &aWord );
    virtual void MouseMove( const MouseEvent& rMEvt );
    DECL_LINK( HideVarContents, void* );
    DECL_LINK( ShowVarContents, void* );
    Point aTipPos;
    String aTipWord;
    ULONG nTipId;

    Timer HideTipTimer;
    Timer ShowTipTimer;

    BOOL bViewMoved;

public:
    TextEditImp( AppEdit *pParent, const WinBits& aBits );
    ~TextEditImp();

    TextEngine *pTextEngine;
    TextView *pTextView;

    void SetFont( const Font& rNewFont );
    BOOL IsModified();
    void SetModifyHdl( Link l ){ ModifyHdl = l; }

    void                KeyInput( const KeyEvent& rKeyEvent );
    void                Paint( const Rectangle& rRect );
    void                MouseButtonUp( const MouseEvent& rMouseEvent );
    void                MouseButtonDown( const MouseEvent& rMouseEvent );
//  void                MouseMove( const MouseEvent& rMouseEvent );
    void                Command( const CommandEvent& rCEvt );
    //BOOL              Drop( const DropEvent& rEvt );
    //BOOL              QueryDrop( DropEvent& rEvt );

    BOOL                ViewMoved();

    void DoDelayedSyntaxHighlight( xub_StrLen nPara );
    void InvalidateSyntaxHighlight();
    void SyntaxHighlight( BOOL bNew );
    void BuildKontextMenu( PopupMenu *&pMenu );
};



DBG_NAMEEX(TextEdit)
class TextEdit : public DataEdit {

    BreakpointWindow    *pBreakpointWindow;
    BOOL bFileWasUTF8;
    BOOL bSaveAsUTF8;

public:
    TextEdit( AppEdit*, const WinBits& );
    ~TextEdit();
    void Highlight( ULONG nLine, xub_StrLen nCol1, xub_StrLen nCol2 );
    TextEditImp& GetTextEditImp() { return aEdit; }

    void                SetBreakpointWindow( BreakpointWindow *pBPWindow ){ pBreakpointWindow = pBPWindow; }
    BreakpointWindow    *GetBreakpointWindow(){ return pBreakpointWindow; }

    DATA_FUNC_DEF( aEdit, TextEditImp )

    virtual void BuildKontextMenu( PopupMenu *&pMenu );

    void SaveAsUTF8( BOOL bUTF8 ) { bSaveAsUTF8 = bUTF8; }
};

#endif
