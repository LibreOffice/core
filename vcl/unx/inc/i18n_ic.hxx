/*************************************************************************
 *
 *  $RCSfile: i18n_ic.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: cp $ $Date: 2001-03-02 07:51:31 $
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

    XVaNestedList mpAttributes;
    XVaNestedList mpStatusAttributes;
    XVaNestedList mpPreeditAttributes;
    #ifdef SOLARIS
    XFontSet      mpFontSet;
    Display      *mpDisplay;
    #endif

    Bool         SupportInputMethodStyle( XIMStyles *pIMStyles );
    unsigned int GetWeightingOfIMStyle(   XIMStyle n_style ) const ;
    Bool         IsSupportedIMStyle(      XIMStyle n_style ) const ;

public:

    Bool UseContext()       { return mbUseable; }
    Bool IsMultiLingual()   { return mbMultiLingual; }
    Bool IsPreeditMode()    { return maClientData.eState == ePreeditStatusActive; }
    XIC  GetContext()       { return maContext; }

    void ExtendEventMask(  XLIB_Window aFocusWindow );
    void SetICFocus();
    void UnsetICFocus();
    int  HandleKeyEvent( XKeyEvent *pEvent, SalFrame *pFrame ); // unused
    void EndExtTextInput( USHORT nFlags );                      // unused
    int  CommitStringCallback( sal_Unicode* pText, sal_Size nLength );

    void Map( SalFrame *pFrame );
    void Unmap();

    void SetPreeditState(Bool aPreeditState);
    void SetLanguage(LanguageType aInputLanguage);

    SalI18N_InputContext( SalFrame *aFrame, Bool aIsOnTheSpot);
    ~SalI18N_InputContext();

private:

    SalI18N_InputContext(); // do not use this

};

#endif _SAL_I18N_INPUTCONTEXT_HXX


