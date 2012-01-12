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


#ifndef _SAL_I18N_CALLBACK_HXX
#define _SAL_I18N_CALLBACK_HXX

#include <salwtype.hxx>
#include <vector>

#ifdef __cplusplus
extern "C" {
#endif

// for iiimp / ml input
int  CommitStringCallback( XIC ic, XPointer client_data, XPointer call_data);

// xim callbacks
void PreeditDoneCallback ( XIC ic, XPointer client_data, XPointer call_data);
int  PreeditStartCallback( XIC ic, XPointer client_data, XPointer call_data);
void PreeditDoneCallback ( XIC ic, XPointer client_data, XPointer call_data);
void PreeditDrawCallback ( XIC ic, XPointer client_data,
                           XIMPreeditDrawCallbackStruct *call_data );
void PreeditCaretCallback( XIC ic, XPointer client_data,
                           XIMPreeditCaretCallbackStruct *call_data );
void GetPreeditSpotLocation(XIC ic, XPointer client_data);

// private hook to prevent from sending further edit events
void PreeditCancelCallback( XPointer client_data );

void StatusStartCallback (XIC ic, XPointer client_data, XPointer call_data);
void StatusDoneCallback  (XIC ic, XPointer client_data, XPointer call_data);
void StatusDrawCallback  (XIC ic, XPointer client_data,
            XIMStatusDrawCallbackStruct *call_data);
void SwitchIMCallback (XIC ix, XPointer client_data, XPointer call_data );

// keep informed if kinput2 crashed again
void IC_IMDestroyCallback (XIM im, XPointer client_data, XPointer call_data);
void IM_IMDestroyCallback (XIM im, XPointer client_data, XPointer call_data);

Bool IsControlCode(sal_Unicode nChar);

#ifdef __cplusplus
} /* extern "C" */
#endif

typedef struct {
  sal_Unicode   *pUnicodeBuffer;
  XIMFeedback   *pCharStyle;
  unsigned int   nCursorPos;
  unsigned int   nLength;
  unsigned int   nSize;
} preedit_text_t;

class SalFrame;

typedef enum {
    ePreeditStatusDontKnow = 0,
    ePreeditStatusActive,
    ePreeditStatusActivationRequired,
    ePreeditStatusStartPending
} preedit_status_t;

typedef struct {
    SalFrame*               pFrame;
    Bool                    bIsMultilingual;
    preedit_status_t        eState;
    preedit_text_t          aText;
    SalExtTextInputEvent    aInputEv;
    std::vector< sal_uInt16 >   aInputFlags;
} preedit_data_t;

#endif /* _SAL_I18N_CALLBACK_HXX */
