/*************************************************************************
 *
 *  $RCSfile: i18n_cb.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: cp $ $Date: 2001-03-02 07:50:44 $
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

#include <stdio.h>
#ifdef SOLARIS
#include <alloca.h>
#endif
#include <prex.h>
#include <X11/Xlocale.h>
#include <X11/Xlib.h>
#include <postx.h>

#include <salunx.h>
#include "XIM.h"

#ifndef _SAL_I18N_CALLBACK_HXX
#include "i18n_cb.hxx"
#endif
#ifndef _SV_SALFRAME_HXX
#include <salframe.hxx>
#endif

// -------------------------------------------------------------------------
//
// i. preedit start callback
//
// -------------------------------------------------------------------------

int
PreeditStartCallback ( XIC ic, XPointer client_data, XPointer call_data )
{
    #define PREEDIT_BUFSZ 16

      preedit_data_t* pPreeditData = (preedit_data_t*)client_data;

    if ( pPreeditData->eState == ePreeditStatusActivationRequired )
    {
        pPreeditData->eState = ePreeditStatusActive;

        pPreeditData->aText.pUnicodeBuffer =
                (sal_Unicode*)malloc(PREEDIT_BUFSZ * sizeof(sal_Unicode));
        pPreeditData->aText.pCharStyle  =
                (XIMFeedback*)malloc(PREEDIT_BUFSZ * sizeof(XIMFeedback));
        pPreeditData->aText.nCursorPos  = 0;
        pPreeditData->aText.nLength     = 0;
        pPreeditData->aText.nSize       = PREEDIT_BUFSZ;

    }

    return -1;
}

// -------------------------------------------------------------------------
//
// ii. preedit done callback
//
// -------------------------------------------------------------------------

void
PreeditDoneCallback ( XIC ic, XPointer client_data, XPointer call_data )
{
      preedit_data_t* pPreeditData = (preedit_data_t*)client_data;

      if (pPreeditData->eState == ePreeditStatusActive )
    {
        #ifdef __synchronous_extinput__
        pPreeditData->pFrame->maFrameData.Call(
                SALEVENT_ENDEXTTEXTINPUT, (void*)NULL );
        #else
        pPreeditData->pFrame->maFrameData.PostExtTextEvent(
                SALEVENT_ENDEXTTEXTINPUT, (void*)NULL );
        #endif
    }
    pPreeditData->eState = ePreeditStatusStartPending;

    if ( pPreeditData->aText.pUnicodeBuffer != NULL )
    {
          free (pPreeditData->aText.pUnicodeBuffer);
        pPreeditData->aText.pUnicodeBuffer = NULL;
    }
    if ( pPreeditData->aText.pCharStyle != NULL )
    {
          free (pPreeditData->aText.pCharStyle);
        pPreeditData->aText.pCharStyle = NULL;
    }
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
    int to = from + howmuch;

      if (to == ptext->nLength)
    {
        // delete from the end of the text
        ptext->nLength = from;
      }
    else
    if (to < ptext->nLength)
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

      while ( nnewsize <= nnewlimit )
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
        rtl_TextEncoding nEncoding = gsl_getSystemTextEncoding();

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

              sal_Size nSize =
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
    if ( (from + amount) > ptext->nLength )
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
USHORT*
Preedit_FeedbackToSAL ( XIMFeedback* pfeedback, int nlength )
{
      USHORT *psalattr;
      USHORT  nval;
      USHORT  noldval = 0;
      XIMFeedback nfeedback;

      // only work with reasonable length
      if (nlength > 0)
        psalattr = (USHORT*)malloc(nlength * sizeof(USHORT));
      else
        return (USHORT*)NULL;

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
                nval |= SAL_EXTTEXTINPUT_ATTR_DOTTEDUNDERLINE;
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

            #if 0 // visibility feedback not supported now
              if (   (nfeedback & XIMVisibleToForward)
                  || (nfeedback & XIMVisibleToBackward)
                  || (nfeedback & XIMVisibleCenter) )
            { }
            #endif
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
    if ( (call_data->text == NULL) && (call_data->chg_length == 0) )
        return;

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

    #ifdef __synchronous_extinput__
      SalExtTextInputEvent aTextEvent;

      aTextEvent.mnTime = 0;
      aTextEvent.mpTextAttr = Preedit_FeedbackToSAL(
            pPreeditData->aText.pCharStyle, pPreeditData->aText.nLength);
      aTextEvent.mnCursorPos = call_data->caret;
      aTextEvent.maText = pPreeditData->aText.pUnicodeBuffer;
    aTextEvent.mnCursorFlags    = 0; // default: make cursor visible
      aTextEvent.mnDeltaStart = 0; // call_data->chg_first;
      aTextEvent.mbOnlyCursor = False;

      if ( pPreeditData->eState == ePreeditStatusActive )
        pPreeditData->pFrame->maFrameData.Call(SALEVENT_EXTTEXTINPUT,
                (void*)&aTextEvent);
      if (aTextEvent.mpTextAttr)
        free((void*)aTextEvent.mpTextAttr);

    #else

      SalExtTextInputEvent *pTextEvent = new SalExtTextInputEvent;

      pTextEvent->mnTime            = 0;
      pTextEvent->mpTextAttr        = Preedit_FeedbackToSAL(
            pPreeditData->aText.pCharStyle, pPreeditData->aText.nLength);
      pTextEvent->mnCursorPos   = call_data->caret;
      pTextEvent->maText            =
            UniString (pPreeditData->aText.pUnicodeBuffer, pPreeditData->aText.nLength);
    pTextEvent->mnCursorFlags   = 0; // default: make cursor visible
      pTextEvent->mnDeltaStart  = 0; // call_data->chg_first;
      pTextEvent->mbOnlyCursor  = False;

      if (pPreeditData->eState == ePreeditStatusActive)
    {
        pPreeditData->pFrame->maFrameData.PostExtTextEvent (SALEVENT_EXTTEXTINPUT,
                (void*)pTextEvent);
    }
    #endif

    GetPreeditSpotLocation(ic, (XPointer)pPreeditData);
}

void
GetPreeditSpotLocation(XIC ic, XPointer client_data)
{
    #ifdef __synchronous_extinput__

      //
      // Send SalEventExtTextInputPos event to get spotlocation
      //
      SalExtTextInputPosEvent mPosEvent;
      preedit_data_t* pPreeditData = (preedit_data_t*)client_data;

      pPreeditData->pFrame->maFrameData.Call(SALEVENT_EXTTEXTINPUTPOS,
                                             (void*)&mPosEvent);

      XPoint point;
      point.x = mPosEvent.mnX + mPosEvent.mnWidth;
      point.y = mPosEvent.mnY + mPosEvent.mnHeight;

      XVaNestedList preedit_attr;
      preedit_attr = XVaCreateNestedList(0, XNSpotLocation, point, 0);
      XSetICValues(ic, XNPreeditAttributes, preedit_attr, NULL);
      XFree(preedit_attr);

    #endif /* __synchronous_extinput__ */

    return;
}

// -------------------------------------------------------------------------
//
// iv. preedit caret callback
//
// -------------------------------------------------------------------------

void
PreeditCaretCallback ( XIC ic, XPointer client_data,
    XIMPreeditCaretCallbackStruct *call_data )
{
    // XXX PreeditCaretCallback is pure debug code for now
    char *direction = "?";
    char *style = "?";

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

    fprintf (stderr, "PreeditCaretCallback( ic=%i, client=%i,\n",
        (unsigned int)ic, (unsigned int)client_data );
    fprintf (stderr, "\t position=%i, direction=\"%s\", style=\"%s\" )\n",
        call_data->position, direction, style );

    // XXX
}

// -----------------------------------------------------------------------
//
// v. commit string callback: convert an extended text input (iiimp ... )
//     into an ordinary key-event
//
// -----------------------------------------------------------------------

int
CommitStringCallback( XIC ic, XPointer client_data, XPointer call_data )
{
    preedit_data_t* pPreeditData = (preedit_data_t*)client_data;


      XIMUnicodeText *cbtext = (XIMUnicodeText *)call_data;
      sal_Unicode *p_unicode_data = (sal_Unicode*)cbtext->string.utf16_char;
      // p_unicode_data[cbtext->length] = (sal_Unicode)0;

    SalExtTextInputEvent aTextEvent;

    aTextEvent.mnTime           = 0;
    aTextEvent.mpTextAttr       = 0;
    aTextEvent.mnCursorPos      = cbtext->length;
    aTextEvent.maText           = p_unicode_data;
    aTextEvent.mnCursorFlags    = 0; // default: make cursor visible
    aTextEvent.mnDeltaStart     = 0;
    aTextEvent.mbOnlyCursor     = False;

    #ifdef __synchronous_extinput__
    pPreeditData->pFrame->maFrameData.Call( SALEVENT_EXTTEXTINPUT,
            (void*)&aTextEvent);
    pPreeditData->pFrame->maFrameData.Call( SALEVENT_ENDEXTTEXTINPUT,
            (void*)NULL );
    #else

    SalExtTextInputEvent *pTextEvent = new SalExtTextInputEvent;

    pTextEvent->mnTime          = 0;
    pTextEvent->mpTextAttr      = 0;
    pTextEvent->mnCursorPos     = cbtext->length;
    pTextEvent->maText          = UniString(p_unicode_data, cbtext->length);
    pTextEvent->mnCursorFlags   = 0; // default: make cursor visible
    pTextEvent->mnDeltaStart    = 0;
    pTextEvent->mbOnlyCursor    = False;

    pPreeditData->pFrame->maFrameData.PostExtTextEvent( SALEVENT_EXTTEXTINPUT,
            (void*)pTextEvent);
    pPreeditData->pFrame->maFrameData.PostExtTextEvent( SALEVENT_ENDEXTTEXTINPUT,
            (void*)NULL );
    #endif
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
StatusStartCallback (XIC ic, XPointer client_data, XPointer call_data)
{
    return;
}

void
StatusDoneCallback (XIC ic, XPointer client_data, XPointer call_data)
{
    return;
}

void
StatusDrawCallback (XIC ic, XPointer client_data, XIMStatusDrawCallbackStruct *call_data)
{
    return;
}
