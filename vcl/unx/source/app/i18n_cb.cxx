/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_vcl.hxx"

#include <stdio.h>
#include <string.h>
#include <sal/alloca.h>
#include <tools/prex.h>
#include <X11/Xlocale.h>
#include <X11/Xlib.h>
#include <tools/postx.h>

#include <salunx.h>
#include <XIM.h>
#include <i18n_cb.hxx>
#include <i18n_status.hxx>
#include "i18n_ic.hxx"
#include "i18n_im.hxx"
#ifndef _OSL_THREAD_H
#include <osl/thread.h>
#endif
#include <vcl/salframe.hxx>

// -------------------------------------------------------------------------
//
// i. preedit start callback
//
// -------------------------------------------------------------------------

int
PreeditStartCallback ( XIC, XPointer client_data, XPointer )
{
      preedit_data_t* pPreeditData = (preedit_data_t*)client_data;
    if ( pPreeditData->eState == ePreeditStatusActivationRequired )
    {
        pPreeditData->eState = ePreeditStatusActive;
        pPreeditData->aText.nCursorPos = 0;
        pPreeditData->aText.nLength    = 0;
    }

    return -1;
}

// -------------------------------------------------------------------------
//
// ii. preedit done callback
//
// -------------------------------------------------------------------------

void
PreeditDoneCallback ( XIC, XPointer client_data, XPointer )
{
      preedit_data_t* pPreeditData = (preedit_data_t*)client_data;
     if (pPreeditData->eState == ePreeditStatusActive )
    {
        if( pPreeditData->pFrame )
            pPreeditData->pFrame->CallCallback( SALEVENT_ENDEXTTEXTINPUT, (void*)NULL );
    }
    pPreeditData->eState = ePreeditStatusStartPending;
}

// -------------------------------------------------------------------------
//
// iii. preedit draw callback
//
// -------------------------------------------------------------------------

//
// Handle deletion of text in a preedit_draw_callback
// from and howmuch are guaranteed to be nonnegative
//

void
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

      if (to == (int)ptext->nLength)
    {
        // delete from the end of the text
        ptext->nLength = from;
      }
    else
        if (to < (int)ptext->nLength)
    {
        // cut out of the middle of the text
        memmove( (void*)(ptext->pUnicodeBuffer + from),
                (void*)(ptext->pUnicodeBuffer + to),
                (ptext->nLength - to) * sizeof(sal_Unicode));
        memmove( (void*)(ptext->pCharStyle + from),
                (void*)(ptext->pCharStyle + to),
                (ptext->nLength - to) * sizeof(XIMFeedback));
        ptext->nLength -= howmuch;
      }
    else
    // if ( to > pText->nLength )
    {
          // XXX this indicates an error, are we out of sync ?
          fprintf(stderr, "Preedit_DeleteText( from=%i to=%i length=%i )\n",
                from, to, ptext->nLength );
          fprintf (stderr, "\t XXX internal error, out of sync XXX\n");

          ptext->nLength = from;
    }

      // NULL-terminate the string
      ptext->pUnicodeBuffer[ptext->nLength] = (sal_Unicode)0;
}

// reallocate the textbuffer with sufficiently large size 2^x
// nnewlimit is presupposed to be larger than ptext->size
void
enlarge_buffer ( preedit_text_t *ptext, int nnewlimit )
{
      size_t nnewsize = ptext->nSize;

      while ( nnewsize <= (size_t)nnewlimit )
        nnewsize *= 2;

      ptext->nSize = nnewsize;
      ptext->pUnicodeBuffer = (sal_Unicode*)realloc((void*)ptext->pUnicodeBuffer,
            nnewsize * sizeof(sal_Unicode));
      ptext->pCharStyle = (XIMFeedback*)realloc((void*)ptext->pCharStyle,
            nnewsize * sizeof(XIMFeedback));
}

//
// Handle insertion of text in a preedit_draw_callback
// string field of XIMText struct is guaranteed to be != NULL
//

void
Preedit_InsertText(preedit_text_t *pText, XIMText *pInsertText, int where,
           Bool isMultilingual)
{
      sal_Unicode *pInsertTextString;
      int nInsertTextLength = 0;
      XIMFeedback *pInsertTextCharStyle = pInsertText->feedback;

      nInsertTextLength = pInsertText->length;

      if (isMultilingual)
    {
        XIMUnicodeText *pUniText = (XIMUnicodeText*)pInsertText;
        pInsertTextString = pUniText->string.utf16_char;
      }
    else
    {
        //  can't handle wchar_t strings, so convert to multibyte chars first
        char *pMBString;
        size_t nMBLength;
        if (pInsertText->encoding_is_wchar)
        {
            wchar_t *pWCString = pInsertText->string.wide_char;
              size_t nBytes = wcstombs ( NULL, pWCString, 1024 /* dont care */);
              pMBString = (char*)alloca( nBytes + 1 );
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

              pInsertTextString = (sal_Unicode*)alloca(nBufferSize);

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
              pInsertTextString = (sal_Unicode*)pMBString;
        }
      }

      // enlarge target text-buffer if necessary
      if (pText->nSize <= (pText->nLength + nInsertTextLength))
        enlarge_buffer(pText, pText->nLength + nInsertTextLength);

      // insert text: displace old mem and put new bytes in
      int from    = where;
      int to      = where + nInsertTextLength;
      int howmany = pText->nLength - where;

      memmove((void*)(pText->pUnicodeBuffer + to),
            (void*)(pText->pUnicodeBuffer + from),
              howmany * sizeof(sal_Unicode));
      memmove((void*)(pText->pCharStyle + to),
              (void*)(pText->pCharStyle + from),
              howmany * sizeof(XIMFeedback));

      to = from;
      howmany = nInsertTextLength;

      memcpy((void*)(pText->pUnicodeBuffer + to), (void*)pInsertTextString,
             howmany * sizeof(sal_Unicode));
      memcpy((void*)(pText->pCharStyle + to), (void*)pInsertTextCharStyle,
              howmany * sizeof(XIMFeedback));

      pText->nLength += howmany;

      // NULL-terminate the string
      pText->pUnicodeBuffer[pText->nLength] = (sal_Unicode)0;
}

//
// Handle the change of attributes in a preedit_draw_callback
//
void
Preedit_UpdateAttributes ( preedit_text_t* ptext, XIMFeedback* feedback,
        int from, int amount )
{
    if ( (from + amount) > (int)ptext->nLength )
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
// SAL_EXTTEXTINPUT_ATTR values
// returns an allocate list of attributes, which must be freed by caller
sal_uInt16*
Preedit_FeedbackToSAL ( XIMFeedback* pfeedback, int nlength, std::vector<sal_uInt16>& rSalAttr )
{
      sal_uInt16 *psalattr;
      sal_uInt16  nval;
      sal_uInt16  noldval = 0;
      XIMFeedback nfeedback;

      // only work with reasonable length
      if (nlength > 0 && nlength > sal::static_int_cast<int>(rSalAttr.size()) )
    {
        rSalAttr.reserve( nlength );
        psalattr = &rSalAttr[0];
    }
      else
        return (sal_uInt16*)NULL;

      for (int npos = 0; npos < nlength; npos++)
    {
        nval = 0;
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
                nval |= SAL_EXTTEXTINPUT_ATTR_HIGHLIGHT;
              if (nfeedback & XIMUnderline)
                nval |= SAL_EXTTEXTINPUT_ATTR_UNDERLINE;
              if (nfeedback & XIMHighlight)
                nval |= SAL_EXTTEXTINPUT_ATTR_HIGHLIGHT;
              if (nfeedback & XIMPrimary)
                nval |= SAL_EXTTEXTINPUT_ATTR_DOTTEDUNDERLINE;
              if (nfeedback & XIMSecondary)
                nval |= SAL_EXTTEXTINPUT_ATTR_DASHDOTUNDERLINE;
              if (nfeedback & XIMTertiary) // same as 2ery
                nval |= SAL_EXTTEXTINPUT_ATTR_DASHDOTUNDERLINE;

            /*
            // visibility feedback not supported now
              if (   (nfeedback & XIMVisibleToForward)
                  || (nfeedback & XIMVisibleToBackward)
                  || (nfeedback & XIMVisibleCenter) )
            { }
            */
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
      preedit_data_t* pPreeditData = (preedit_data_t*)client_data;

    // if there's nothing to change then change nothing
    if ( ( (call_data->text == NULL) && (call_data->chg_length == 0) )
         || pPreeditData->pFrame == NULL )
        return;

    // #88564# Solaris 7 deletes the preedit buffer after commit
    // since the next call to preeditstart will have the same effect just skip this.
    // if (pPreeditData->eState == ePreeditStatusStartPending && call_data->text == NULL)
    //    return;

    if ( pPreeditData->eState == ePreeditStatusStartPending )
        pPreeditData->eState = ePreeditStatusActivationRequired;
    PreeditStartCallback( ic, client_data, NULL );

      // Edit the internal textbuffer as indicated by the call_data,
      // chg_first and chg_length are guaranteed to be nonnegative

      // handle text deletion
      if (call_data->text == NULL)
    {
        Preedit_DeleteText(&(pPreeditData->aText),
               call_data->chg_first, call_data->chg_length );
      }
    else
    {
        // handle text insertion
        if (   (call_data->chg_length == 0)
            && (call_data->text->string.wide_char != NULL))
        {
              Preedit_InsertText(&(pPreeditData->aText), call_data->text,
                     call_data->chg_first, pPreeditData->bIsMultilingual);
        }
        else
          // handle text replacement by deletion and insertion of text,
          // not smart, just good enough
          if (   (call_data->chg_length != 0)
              && (call_data->text->string.wide_char != NULL))
        {
            Preedit_DeleteText(&(pPreeditData->aText),
                       call_data->chg_first, call_data->chg_length);
            Preedit_InsertText(&(pPreeditData->aText), call_data->text,
                       call_data->chg_first, pPreeditData->bIsMultilingual);
          }
        else
        // not really a text update, only attributes are concerned
        if (   (call_data->chg_length != 0)
            && (call_data->text->string.wide_char == NULL))
        {
              Preedit_UpdateAttributes(&(pPreeditData->aText),
                   call_data->text->feedback,
                   call_data->chg_first, call_data->chg_length);
        }
      }

      //
      // build the SalExtTextInputEvent and send it up
      //
      pPreeditData->aInputEv.mnTime = 0;
      pPreeditData->aInputEv.mpTextAttr = Preedit_FeedbackToSAL(
            pPreeditData->aText.pCharStyle, pPreeditData->aText.nLength, pPreeditData->aInputFlags);
      pPreeditData->aInputEv.mnCursorPos = call_data->caret;
      pPreeditData->aInputEv.maText = String (pPreeditData->aText.pUnicodeBuffer,
                                pPreeditData->aText.nLength);
    pPreeditData->aInputEv.mnCursorFlags    = 0; // default: make cursor visible
      pPreeditData->aInputEv.mnDeltaStart = 0; // call_data->chg_first;
      pPreeditData->aInputEv.mbOnlyCursor = False;

      if ( pPreeditData->eState == ePreeditStatusActive && pPreeditData->pFrame )
        pPreeditData->pFrame->CallCallback(SALEVENT_EXTTEXTINPUT, (void*)&pPreeditData->aInputEv);
    if (pPreeditData->aText.nLength == 0 && pPreeditData->pFrame )
        pPreeditData->pFrame->CallCallback( SALEVENT_ENDEXTTEXTINPUT, (void*)NULL );

    if (pPreeditData->aText.nLength == 0)
        pPreeditData->eState = ePreeditStatusStartPending;

    GetPreeditSpotLocation(ic, (XPointer)pPreeditData);
}

void
GetPreeditSpotLocation(XIC ic, XPointer client_data)
{
      //
      // Send SalEventExtTextInputPos event to get spotlocation
      //
      SalExtTextInputPosEvent mPosEvent;
      preedit_data_t* pPreeditData = (preedit_data_t*)client_data;

    if( pPreeditData->pFrame )
        pPreeditData->pFrame->CallCallback(SALEVENT_EXTTEXTINPUTPOS, (void*)&mPosEvent);

      XPoint point;
      point.x = mPosEvent.mnX + mPosEvent.mnWidth;
      point.y = mPosEvent.mnY + mPosEvent.mnHeight;

      XVaNestedList preedit_attr;
      preedit_attr = XVaCreateNestedList(0, XNSpotLocation, &point, NULL);
      XSetICValues(ic, XNPreeditAttributes, preedit_attr, NULL);
      XFree(preedit_attr);

    return;
}

// -------------------------------------------------------------------------
//
// iv. preedit caret callback
//
// -------------------------------------------------------------------------

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
        case XIMDontChange:   direction = "Dont change";   break;
    }

    fprintf (stderr, "PreeditCaretCallback( ic=%p, client=%p,\n",
        ic, client_data );
    fprintf (stderr, "\t position=%i, direction=\"%s\", style=\"%s\" )\n",
        call_data->position, direction, style );
    #endif
}

// -----------------------------------------------------------------------
//
// v. commit string callback: convert an extended text input (iiimp ... )
//     into an ordinary key-event
//
// -----------------------------------------------------------------------

Bool
IsControlCode(sal_Unicode nChar)
{
    if ( nChar <= 0x1F // C0 controls
     /* || (0x80 <= nChar && nChar <= 0x9F) C1 controls */ )
        return True;
    else
        return False;
}

int
CommitStringCallback( XIC ic, XPointer client_data, XPointer call_data )
{
    preedit_data_t* pPreeditData = (preedit_data_t*)client_data;

      XIMUnicodeText *cbtext = (XIMUnicodeText *)call_data;
      sal_Unicode *p_unicode_data = (sal_Unicode*)cbtext->string.utf16_char;

    // #86964# filter unexpected pure control events
    if (cbtext->length == 1 && IsControlCode(p_unicode_data[0]) )
    {
        if( pPreeditData->pFrame )
        {
            pPreeditData->pFrame->CallCallback( SALEVENT_ENDEXTTEXTINPUT, (void*)NULL );
        }
    }
    else
    {
        if( pPreeditData->pFrame )
        {
            pPreeditData->aInputEv.mnTime           = 0;
            pPreeditData->aInputEv.mpTextAttr       = 0;
            pPreeditData->aInputEv.mnCursorPos      = cbtext->length;
            pPreeditData->aInputEv.maText           = UniString(p_unicode_data, cbtext->length);
            pPreeditData->aInputEv.mnCursorFlags    = 0; // default: make cursor visible
            pPreeditData->aInputEv.mnDeltaStart     = 0;
            pPreeditData->aInputEv.mbOnlyCursor     = False;

            pPreeditData->pFrame->CallCallback( SALEVENT_EXTTEXTINPUT, (void*)&pPreeditData->aInputEv);
            pPreeditData->pFrame->CallCallback( SALEVENT_ENDEXTTEXTINPUT, (void*)NULL );
        }
    }
    pPreeditData->eState = ePreeditStatusStartPending;

    GetPreeditSpotLocation(ic, (XPointer)pPreeditData);

    return 0;
}

// ----------------------------------------------------------------------------------
//
// vi. status callbacks: for now these are empty, they are just needed for turbo linux
//
// ----------------------------------------------------------------------------------

void
StatusStartCallback (XIC, XPointer, XPointer)
{
    return;
}

void
StatusDoneCallback (XIC, XPointer, XPointer)
{
    return;
}

void
StatusDrawCallback (XIC ic, XPointer client_data, XIMStatusDrawCallbackStruct *call_data)
{
      preedit_data_t* pPreeditData = (preedit_data_t*)client_data;
    if( pPreeditData->bIsMultilingual )
    {
        // IIIMP
        XIMUnicodeText *cbtext = (XIMUnicodeText *)call_data->data.text;
        ::vcl::I18NStatus::get().setStatusText( String( cbtext->string.utf16_char, call_data->data.text->length ) );
        XIMUnicodeCharacterSubset* pSubset = NULL;
        if( ! XGetICValues( ic,
                            XNUnicodeCharacterSubset, & pSubset,
                            NULL )
            && pSubset )
        {
            ::vcl::I18NStatus::get().changeIM( String( ByteString( pSubset->name ), RTL_TEXTENCODING_UTF8 ) );
#if OSL_DEBUG_LEVEL > 1
            fprintf( stderr, "got XNUnicodeCharacterSubset\n   %d\n   %d\n   %s\n   %d\n", pSubset->index, pSubset->subset_id, pSubset->name, pSubset->is_active );
#endif
        }
    }
    else if( call_data->type == XIMTextType )
    {
        String aText;
        if( call_data->data.text )
        {
            // XIM with text
            sal_Char* pMBString = NULL;
            size_t nLength = 0;
            if( call_data->data.text->encoding_is_wchar )
            {
                if( call_data->data.text->string.wide_char )
                {
                    wchar_t* pWString = call_data->data.text->string.wide_char;
                    size_t nBytes = wcstombs( NULL, pWString, 1024 );
                    pMBString = (sal_Char*)alloca( nBytes+1 );
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
                aText = String( pMBString, nLength, gsl_getSystemTextEncoding() );
        }
        ::vcl::I18NStatus::get().setStatusText( aText );
    }
#if OSL_DEBUG_LEVEL > 1
    else
        fprintf( stderr, "XIMStatusDataType %s not supported\n",
                 call_data->type == XIMBitmapType ? "XIMBitmapType" : ByteString::CreateFromInt32( call_data->type ).GetBuffer() );
#endif
    return;
}

void
SwitchIMCallback (XIC, XPointer, XPointer call_data)
{
    XIMSwitchIMNotifyCallbackStruct* pCallData = (XIMSwitchIMNotifyCallbackStruct*)call_data;
    ::vcl::I18NStatus::get().changeIM( String( ByteString( pCallData->to->name ), RTL_TEXTENCODING_UTF8 ) );
}

// ----------------------------------------------------------------------------------
//
// vii. destroy callbacks: internally disable all IC/IM calls
//
// ----------------------------------------------------------------------------------

void
IC_IMDestroyCallback (XIM, XPointer client_data, XPointer)
{
    SalI18N_InputContext *pContext = (SalI18N_InputContext*)client_data;
    if (pContext != NULL)
        pContext->HandleDestroyIM();
}

void
IM_IMDestroyCallback (XIM, XPointer client_data, XPointer)
{
    SalI18N_InputMethod *pMethod = (SalI18N_InputMethod*)client_data;
    if (pMethod != NULL)
        pMethod->HandleDestroyIM();
}
