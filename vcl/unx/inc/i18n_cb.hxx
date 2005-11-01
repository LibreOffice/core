/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: i18n_cb.hxx,v $
 *
 *  $Revision: 1.9 $
 *
 *  last change: $Author: kz $ $Date: 2005-11-01 10:35:58 $
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
#ifndef _SAL_I18N_CALLBACK_HXX
#define _SAL_I18N_CALLBACK_HXX

#ifndef _SV_SALWTYPE_HXX
#include <salwtype.hxx>
#endif
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
    std::vector< USHORT >   aInputFlags;
} preedit_data_t;

#endif /* _SAL_I18N_CALLBACK_HXX */
