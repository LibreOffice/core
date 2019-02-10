/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include <stdio.h>
#include <string.h>

#include <sal/alloca.h>
#include <osl/thread.h>

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xlocale.h>

#include <vcl/commandevent.hxx>
#include <unx/salunx.h>
#include <unx/XIM.h>
#include <unx/i18n_cb.hxx>
#include <unx/i18n_status.hxx>
#include <unx/i18n_ic.hxx>
#include <unx/i18n_im.hxx>
#include <salframe.hxx>

// i. preedit start callback

int
PreeditStartCallback ( XIC, XPointer client_data, XPointer )
{
    preedit_data_t* pPreeditData = reinterpret_cast<preedit_data_t*>(client_data);
    if ( pPreeditData->eState == PreeditStatus::ActivationRequired )
    {
        pPreeditData->eState = PreeditStatus::Active;
        pPreeditData->aText.nLength    = 0;
    }

    return -1;
}

// ii. preedit done callback

void
PreeditDoneCallback ( XIC, XPointer client_data, XPointer )
{
    preedit_data_t* pPreeditData = reinterpret_cast<preedit_data_t*>(client_data);
    if (pPreeditData->eState == PreeditStatus::Active )
    {
        if( pPreeditData->pFrame )
            pPreeditData->pFrame->CallCallback( SalEvent::EndExtTextInput, nullptr );
    }
    pPreeditData->eState = PreeditStatus::StartPending;
}

// iii. preedit draw callback

// Handle deletion of text in a preedit_draw_callback
// from and howmuch are guaranteed to be nonnegative

static void
Preedit_DeleteText(preedit_text_t *ptext, int from, int howmuch)
{
    // If we've been asked to delete no text then just set
    // nLength correctly and return
    if (ptext->nLength == 0)
    {
        ptext->nLength = from;
        return;
    }

    int to = from + howmuch;

    if (to == static_cast<int>(ptext->nLength))
    {
        // delete from the end of the text
        ptext->nLength = from;
    }
    else if (to < static_cast<int>(ptext->nLength))
    {
        // cut out of the middle of the text
        memmove( static_cast<void*>(ptext->pUnicodeBuffer + from),
                static_cast<void*>(ptext->pUnicodeBuffer + to),
                (ptext->nLength - to) * sizeof(sal_Unicode));
        memmove( static_cast<void*>(ptext->pCharStyle + from),
                static_cast<void*>(ptext->pCharStyle + to),
                (ptext->nLength - to) * sizeof(XIMFeedback));
        ptext->nLength -= howmuch;
    }
    else
    {
          // XXX this indicates an error, are we out of sync ?
          fprintf(stderr, "Preedit_DeleteText( from=%i to=%i length=%i )\n",
                from, to, ptext->nLength );
          fprintf (stderr, "\t XXX internal error, out of sync XXX\n");

          ptext->nLength = from;
    }

    // NULL-terminate the string
    ptext->pUnicodeBuffer[ptext->nLength] = u'\0';
}

// reallocate the textbuffer with sufficiently large size 2^x
// nnewlimit is presupposed to be larger than ptext->size
static void
enlarge_buffer ( preedit_text_t *ptext, int nnewlimit )
{
      size_t nnewsize = ptext->nSize;

      while ( nnewsize <= static_cast<size_t>(nnewlimit) )
        nnewsize *= 2;

      ptext->nSize = nnewsize;
      ptext->pUnicodeBuffer = static_cast<sal_Unicode*>(realloc(static_cast<void*>(ptext->pUnicodeBuffer),
            nnewsize * sizeof(sal_Unicode)));
      ptext->pCharStyle = static_cast<XIMFeedback*>(realloc(static_cast<void*>(ptext->pCharStyle),
            nnewsize * sizeof(XIMFeedback)));
}

// Handle insertion of text in a preedit_draw_callback
// string field of XIMText struct is guaranteed to be != NULL

static void
Preedit_InsertText(preedit_text_t *pText, XIMText *pInsertText, int where)
{
    sal_Unicode *pInsertTextString;
    int nInsertTextLength = 0;
    XIMFeedback *pInsertTextCharStyle = pInsertText->feedback;

    nInsertTextLength = pInsertText->length;

    //  can't handle wchar_t strings, so convert to multibyte chars first
    char *pMBString;
    size_t nMBLength;
    if (pInsertText->encoding_is_wchar)
    {
        wchar_t *pWCString = pInsertText->string.wide_char;
        size_t nBytes = wcstombs ( nullptr, pWCString, 1024 /* don't care */);
        pMBString = static_cast<char*>(alloca( nBytes + 1 ));
        nMBLength = wcstombs ( pMBString, pWCString, nBytes + 1);
    }
    else
    {
          pMBString = pInsertText->string.multi_byte;
          nMBLength = strlen(pMBString); // xxx
    }

    // convert multibyte chars to unicode
    rtl_TextEncoding nEncoding = osl_getThreadTextEncoding();

    if (nEncoding != RTL_TEXTENCODING_UNICODE)
    {
        rtl_TextToUnicodeConverter aConverter =
                rtl_createTextToUnicodeConverter( nEncoding );
        rtl_TextToUnicodeContext aContext =
                rtl_createTextToUnicodeContext(aConverter);

        sal_Size nBufferSize = nInsertTextLength * 2;

        pInsertTextString = static_cast<sal_Unicode*>(alloca(nBufferSize));

        sal_uInt32  nConversionInfo;
        sal_Size    nConvertedChars;

        rtl_convertTextToUnicode( aConverter, aContext,
                pMBString, nMBLength,
                 pInsertTextString, nBufferSize,
                  RTL_TEXTTOUNICODE_FLAGS_UNDEFINED_IGNORE
                | RTL_TEXTTOUNICODE_FLAGS_INVALID_IGNORE,
                &nConversionInfo, &nConvertedChars );

        rtl_destroyTextToUnicodeContext(aConverter, aContext);
        rtl_destroyTextToUnicodeConverter(aConverter);

    }
    else
    {
          pInsertTextString = reinterpret_cast<sal_Unicode*>(pMBString);
    }

    // enlarge target text-buffer if necessary
    if (pText->nSize <= (pText->nLength + nInsertTextLength))
      enlarge_buffer(pText, pText->nLength + nInsertTextLength);

    // insert text: displace old mem and put new bytes in
    int from    = where;
    int to      = where + nInsertTextLength;
    int howmany = pText->nLength - where;

    memmove(static_cast<void*>(pText->pUnicodeBuffer + to),
          static_cast<void*>(pText->pUnicodeBuffer + from),
            howmany * sizeof(sal_Unicode));
    memmove(static_cast<void*>(pText->pCharStyle + to),
            static_cast<void*>(pText->pCharStyle + from),
            howmany * sizeof(XIMFeedback));

    to = from;
    howmany = nInsertTextLength;

    memcpy(static_cast<void*>(pText->pUnicodeBuffer + to), static_cast<void*>(pInsertTextString),
           howmany * sizeof(sal_Unicode));
    memcpy(static_cast<void*>(pText->pCharStyle + to), static_cast<void*>(pInsertTextCharStyle),
            howmany * sizeof(XIMFeedback));

    pText->nLength += howmany;

    // NULL-terminate the string
    pText->pUnicodeBuffer[pText->nLength] = u'\0';
}

// Handle the change of attributes in a preedit_draw_callback

static void
Preedit_UpdateAttributes ( preedit_text_t* ptext, XIMFeedback const * feedback,
        int from, int amount )
{
    if ( (from + amount) > static_cast<int>(ptext->nLength) )
    {
        // XXX this indicates an error, are we out of sync ?
        fprintf (stderr, "Preedit_UpdateAttributes( %i + %i > %i )\n",
            from, amount, ptext->nLength );
        fprintf (stderr, "\t XXX internal error, out of sync XXX\n");

        return;
    }

    memcpy ( ptext->pCharStyle + from,
        feedback, amount * sizeof(XIMFeedback) );
}

// Convert the XIM feedback values into appropriate VCL
// EXTTEXTINPUT_ATTR values
// returns an allocate list of attributes, which must be freed by caller
static ExtTextInputAttr*
Preedit_FeedbackToSAL ( const XIMFeedback* pfeedback, int nlength, std::vector<ExtTextInputAttr>& rSalAttr )
{
    ExtTextInputAttr *psalattr;
    ExtTextInputAttr  nval;
    ExtTextInputAttr  noldval = ExtTextInputAttr::NONE;
    XIMFeedback nfeedback;

      // only work with reasonable length
    if (nlength > 0 && nlength > sal::static_int_cast<int>(rSalAttr.size()) )
    {
        rSalAttr.reserve( nlength );
        psalattr = &rSalAttr[0];
    }
    else
        return nullptr;

    for (int npos = 0; npos < nlength; npos++)
    {
        nval = ExtTextInputAttr::NONE;
        nfeedback = pfeedback[npos];

        // means to use the feedback of the previous char
        if (nfeedback == 0)
        {
              nval = noldval;
        }
        // convert feedback to attributes
        else
        {
              if (nfeedback & XIMReverse)
                nval |= ExtTextInputAttr::Highlight;
              if (nfeedback & XIMUnderline)
                nval |= ExtTextInputAttr::Underline;
              if (nfeedback & XIMHighlight)
                nval |= ExtTextInputAttr::Highlight;
              if (nfeedback & XIMPrimary)
                nval |= ExtTextInputAttr::DottedUnderline;
              if (nfeedback & XIMSecondary)
                nval |= ExtTextInputAttr::DashDotUnderline;
              if (nfeedback & XIMTertiary) // same as 2ery
                nval |= ExtTextInputAttr::DashDotUnderline;

        }
        // copy in list
        psalattr[npos] = nval;
        noldval = nval;
    }
    // return list of sal attributes
    return psalattr;
}

void
PreeditDrawCallback(XIC ic, XPointer client_data,
            XIMPreeditDrawCallbackStruct *call_data)
{
    preedit_data_t* pPreeditData = reinterpret_cast<preedit_data_t*>(client_data);

    // if there's nothing to change then change nothing
    if ( ( (call_data->text == nullptr) && (call_data->chg_length == 0) )
         || pPreeditData->pFrame == nullptr )
        return;

    // Solaris 7 deletes the preedit buffer after commit
    // since the next call to preeditstart will have the same effect just skip this.
    // if (pPreeditData->eState == ePreeditStatusStartPending && call_data->text == NULL)
    //    return;

    if ( pPreeditData->eState == PreeditStatus::StartPending )
        pPreeditData->eState = PreeditStatus::ActivationRequired;
    PreeditStartCallback( ic, client_data, nullptr );

      // Edit the internal textbuffer as indicated by the call_data,
      // chg_first and chg_length are guaranteed to be nonnegative

      // handle text deletion
    if (call_data->text == nullptr)
    {
        Preedit_DeleteText(&(pPreeditData->aText),
               call_data->chg_first, call_data->chg_length );
    }
    else
    {
        // handle text insertion
        if (   (call_data->chg_length == 0)
            && (call_data->text->string.wide_char != nullptr))
        {
              Preedit_InsertText(&(pPreeditData->aText), call_data->text,
                     call_data->chg_first);
        }
        else if (   (call_data->chg_length != 0)
              && (call_data->text->string.wide_char != nullptr))
        {
            // handle text replacement by deletion and insertion of text,
            // not smart, just good enough

            Preedit_DeleteText(&(pPreeditData->aText),
                       call_data->chg_first, call_data->chg_length);
            Preedit_InsertText(&(pPreeditData->aText), call_data->text,
                       call_data->chg_first);
          }
        else if (   (call_data->chg_length != 0)
            && (call_data->text->string.wide_char == nullptr))
        {
            // not really a text update, only attributes are concerned
              Preedit_UpdateAttributes(&(pPreeditData->aText),
                   call_data->text->feedback,
                   call_data->chg_first, call_data->chg_length);
        }
    }

    // build the SalExtTextInputEvent and send it up

    pPreeditData->aInputEv.mpTextAttr = Preedit_FeedbackToSAL(
        pPreeditData->aText.pCharStyle, pPreeditData->aText.nLength, pPreeditData->aInputFlags);
    pPreeditData->aInputEv.mnCursorPos = call_data->caret;
    pPreeditData->aInputEv.maText = OUString(pPreeditData->aText.pUnicodeBuffer,
                                pPreeditData->aText.nLength);
    pPreeditData->aInputEv.mnCursorFlags    = 0; // default: make cursor visible

    if ( pPreeditData->eState == PreeditStatus::Active && pPreeditData->pFrame )
        pPreeditData->pFrame->CallCallback(SalEvent::ExtTextInput, static_cast<void*>(&pPreeditData->aInputEv));
    if (pPreeditData->aText.nLength == 0 && pPreeditData->pFrame )
        pPreeditData->pFrame->CallCallback( SalEvent::EndExtTextInput, nullptr );

    if (pPreeditData->aText.nLength == 0)
        pPreeditData->eState = PreeditStatus::StartPending;

    GetPreeditSpotLocation(ic, reinterpret_cast<XPointer>(pPreeditData));
}

void
GetPreeditSpotLocation(XIC ic, XPointer client_data)
{

    // Send SalEventExtTextInputPos event to get spotlocation

    SalExtTextInputPosEvent aPosEvent;
    preedit_data_t* pPreeditData = reinterpret_cast<preedit_data_t*>(client_data);

    if( pPreeditData->pFrame )
        pPreeditData->pFrame->CallCallback(SalEvent::ExtTextInputPos, static_cast<void*>(&aPosEvent));

    XPoint point;
    point.x = aPosEvent.mnX + aPosEvent.mnWidth;
    point.y = aPosEvent.mnY + aPosEvent.mnHeight;

    XVaNestedList preedit_attr;
    preedit_attr = XVaCreateNestedList(0, XNSpotLocation, &point, nullptr);
    XSetICValues(ic, XNPreeditAttributes, preedit_attr, nullptr);
    XFree(preedit_attr);
}

// iv. preedit caret callback

#if OSL_DEBUG_LEVEL > 1
void
PreeditCaretCallback ( XIC ic, XPointer client_data,
    XIMPreeditCaretCallbackStruct *call_data )
#else
void
PreeditCaretCallback ( XIC, XPointer,XIMPreeditCaretCallbackStruct* )
#endif
{
    #if OSL_DEBUG_LEVEL > 1
    // XXX PreeditCaretCallback is pure debug code for now
    const char *direction = "?";
    const char *style = "?";

    switch ( call_data->style )
    {
        case XIMIsInvisible: style = "Invisible"; break;
        case XIMIsPrimary:   style = "Primary";   break;
        case XIMIsSecondary: style = "Secondary"; break;
    }
    switch ( call_data->direction )
    {
        case XIMForwardChar:  direction = "Forward char";  break;
        case XIMBackwardChar: direction = "Backward char"; break;
        case XIMForwardWord:  direction = "Forward word";  break;
        case XIMBackwardWord: direction = "Backward word"; break;
        case XIMCaretUp:      direction = "Caret up";      break;
        case XIMCaretDown:    direction = "Caret down";    break;
        case XIMNextLine:     direction = "Next line";     break;
        case XIMPreviousLine: direction = "Previous line"; break;
        case XIMLineStart:    direction = "Line start";    break;
        case XIMLineEnd:      direction = "Line end";      break;
        case XIMAbsolutePosition: direction = "Absolute";  break;
        case XIMDontChange:   direction = "Don't change";  break;
    }

    fprintf (stderr, "PreeditCaretCallback( ic=%p, client=%p,\n",
        ic, client_data );
    fprintf (stderr, "\t position=%i, direction=\"%s\", style=\"%s\" )\n",
        call_data->position, direction, style );
    #endif
}

// v. commit string callback: convert an extended text input (iiimp ... )
//     into an ordinary key-event

Bool
IsControlCode(sal_Unicode nChar)
{
    if ( nChar <= 0x1F /* C0 controls */ )
        return True;
    else
        return False;
}

// vi. status callbacks: for now these are empty, they are just needed for turbo linux

void
StatusStartCallback (XIC, XPointer, XPointer)
{
}

void
StatusDoneCallback (XIC, XPointer, XPointer)
{
}

void
StatusDrawCallback (XIC, XPointer, XIMStatusDrawCallbackStruct *call_data)
{
    if( call_data->type == XIMTextType )
    {
        OUString aText;
        if( call_data->data.text )
        {
            // XIM with text
            sal_Char* pMBString = nullptr;
            size_t nLength = 0;
            if( call_data->data.text->encoding_is_wchar )
            {
                if( call_data->data.text->string.wide_char )
                {
                    wchar_t* pWString = call_data->data.text->string.wide_char;
                    size_t nBytes = wcstombs( nullptr, pWString, 1024 );
                    pMBString = static_cast<sal_Char*>(alloca( nBytes+1 ));
                    nLength = wcstombs( pMBString, pWString, nBytes+1 );
                }
            }
            else
            {
                if( call_data->data.text->string.multi_byte )
                {
                    pMBString = call_data->data.text->string.multi_byte;
                    nLength = strlen( pMBString );
                }
            }
            if( nLength )
                aText = OUString( pMBString, nLength, osl_getThreadTextEncoding() );
        }
        vcl::I18NStatus::get().setStatusText( aText );
    }
#if OSL_DEBUG_LEVEL > 1
    else
    {
        fprintf( stderr, "XIMStatusDataType %s not supported\n",
            call_data->type == XIMBitmapType ? "XIMBitmapType" : OString::number(call_data->type).getStr() );
    }
#endif
}

// vii. destroy callbacks: internally disable all IC/IM calls

void
IC_IMDestroyCallback (XIM, XPointer client_data, XPointer)
{
    SalI18N_InputContext *pContext = reinterpret_cast<SalI18N_InputContext*>(client_data);
    if (pContext != nullptr)
        pContext->HandleDestroyIM();
}

void
IM_IMDestroyCallback (XIM, XPointer client_data, XPointer)
{
    SalI18N_InputMethod *pMethod = reinterpret_cast<SalI18N_InputMethod*>(client_data);
    if (pMethod != nullptr)
        pMethod->HandleDestroyIM();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
