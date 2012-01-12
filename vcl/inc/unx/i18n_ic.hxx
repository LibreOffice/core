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



#ifndef _SAL_I18N_INPUTCONTEXT_HXX
#define _SAL_I18N_INPUTCONTEXT_HXX

#include <i18npool/lang.h>
#include "i18n_cb.hxx"

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
    void EndExtTextInput( sal_uInt16 nFlags );                      // unused
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


