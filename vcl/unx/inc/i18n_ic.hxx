/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: i18n_ic.hxx,v $
 *
 *  $Revision: 1.11 $
 *
 *  last change: $Author: kz $ $Date: 2005-11-01 10:36:12 $
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

#ifndef _SAL_I18N_INPUTCONTEXT_HXX
#define _SAL_I18N_INPUTCONTEXT_HXX

#ifndef _TOOLS_LANG_HXX
#include <tools/lang.hxx>
#endif

#ifndef _SAL_I18N_CALLBACK_HXX
#include "i18n_cb.hxx"
#endif

class SalI18N_InputContext
{

private:

    Bool    mbUseable; // system supports current locale ?
    Bool    mbMultiLingual; // system supports iiimp ?
    XIC     maContext;

    XIMStyle mnSupportedStatusStyle;
    XIMStyle mnSupportedPreeditStyle;
    XIMStyle mnStatusStyle;
    XIMStyle mnPreeditStyle;

    preedit_data_t maClientData;
    XIMCallback maPreeditStartCallback;
    XIMCallback maPreeditDoneCallback;
    XIMCallback maPreeditDrawCallback;
    XIMCallback maPreeditCaretCallback;
    XIMCallback maCommitStringCallback;
    XIMCallback maSwitchIMCallback;
    XIMCallback maDestroyCallback;

    XVaNestedList mpAttributes;
    XVaNestedList mpStatusAttributes;
    XVaNestedList mpPreeditAttributes;

    Bool         SupportInputMethodStyle( XIMStyles *pIMStyles );
    unsigned int GetWeightingOfIMStyle(   XIMStyle n_style ) const ;
    Bool         IsSupportedIMStyle(      XIMStyle n_style ) const ;

public:

    Bool UseContext()       { return mbUseable; }
    Bool IsMultiLingual()   { return mbMultiLingual; }
    Bool IsPreeditMode()    { return maClientData.eState == ePreeditStatusActive; }
    XIC  GetContext()       { return maContext; }

    void ExtendEventMask(  XLIB_Window aFocusWindow );
    void SetICFocus( SalFrame* pFocusFrame );
    void UnsetICFocus( SalFrame* pFrame );
    void HandleDestroyIM();

    int  HandleKeyEvent( XKeyEvent *pEvent, SalFrame *pFrame ); // unused
    void EndExtTextInput( USHORT nFlags );                      // unused
    int  CommitStringCallback( sal_Unicode* pText, sal_Size nLength );
    int  CommitKeyEvent( sal_Unicode* pText, sal_Size nLength );
    int  UpdateSpotLocation();

    void Map( SalFrame *pFrame );
    void Unmap( SalFrame* pFrame );

    void SetPreeditState(Bool aPreeditState);
    void SetLanguage(LanguageType aInputLanguage);

    SalI18N_InputContext( SalFrame *aFrame );
    ~SalI18N_InputContext();

private:

    SalI18N_InputContext(); // do not use this

};

#endif // _SAL_I18N_INPUTCONTEXT_HXX


