/*************************************************************************
 *
 *  $RCSfile: i18n_ic.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: svesik $ $Date: 2001-02-16 01:04:00 $
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
#if defined(NETBSD)
# include <stdlib.h>
#else
# include <alloca.h>
#endif

#include <prex.h>
#include <X11/Xlocale.h>
#include <X11/Xlib.h>
#include <postx.h>

#include <salunx.h>

#include <XIM.h>

#ifndef _SAL_I18N_INPUTCONTEXT_HXX
#include "i18n_ic.hxx"
#endif
#ifndef _SAL_I18N_INPUTMETHOD_HXX
#include "i18n_im.hxx"
#endif
#ifndef _SV_SALFRAME_HXX
#include <salframe.hxx>
#endif
#ifndef _SV_SALDISP_HXX
#include <saldisp.hxx>
#endif

// ---------------------------------------------------------------------------
//
// Constructor / Destructor, the InputContext is bound to the SalFrame, as it
// needs the shell window as a focus window
//
// ----------------------------------------------------------------------------

SalI18N_InputContext::~SalI18N_InputContext()
{
    if ( maContext != NULL )
        XDestroyIC( maContext );
    if ( mpAttributes != NULL )
        XFree( mpAttributes );
    if ( mpStatusAttributes != NULL )
        XFree( mpStatusAttributes );
    if ( mpPreeditAttributes != NULL )
        XFree( mpPreeditAttributes );
    #ifdef SOLARIS
    if ( mpFontSet != NULL )
        XFreeFontSet( mpDisplay, mpFontSet );
    #endif
}

#ifdef DEBUG

// ----------------------------------------------------------------------------
// debug routines, that keep track of fonts we've chosen in a fontset
// ----------------------------------------------------------------------------

// report which fonts are missing for the requested
// basefont and which fonts are actually selected

static void
v_print_missing_fonts( char **pp_list, int n_listsz, const char *p_pattern )
{
    if ( !(n_listsz > 0) || (pp_list == NULL) || (p_pattern == NULL) )
        return;

    for (int i = 0; i < n_listsz; i++ )
          fprintf(stderr, "missing charset@%s:\"%s\"\n", p_pattern, pp_list[i] );
}

static void
v_print_chosen_fonts( XFontSet a_fontset )
{
    if ( a_fontset == 0 )
        return;

    char         *p_locale_name;
    XFontStruct **pp_font_struct_list;
    char        **pp_font_name_list;
    int           n_listsz;

    p_locale_name = XLocaleOfFontSet( a_fontset );
    n_listsz      = XFontsOfFontSet(  a_fontset,
                                         &pp_font_struct_list,
                                      &pp_font_name_list );
    for ( int i = 0; i < n_listsz; i++ )
    {
         fprintf( stderr, "font@%s:\"%s\"\n",
                  p_locale_name, pp_font_name_list[i] );
    }
}

#endif // DEBUG

// Wrapper for XCreateFontSet to create a default font set
static XFontSet
p_create_fontset( Display *p_display )
{
    // a fontset, as there is no valid default for russian and all other
    // eastern european locales in solaris 8
    const char *p_base_font[]       = {
        "-*-application-medium-r-*-sans-12-*",
        "-*-application-*-r-*-*-12-*",
        "-*-*-*-r-*-*-12-*",
        "-*" };
    char **pp_missing_charset_list  = NULL;
    int    n_missing_charset_count  = 0;
    char  *p_def_string             = NULL;

    XFontSet a_fontset;

    for (int i = 0; i < (sizeof(p_base_font) / sizeof(char*)); i++ )
    {
        a_fontset = XCreateFontSet( p_display, p_base_font[i],
                                     &pp_missing_charset_list,
                                    &n_missing_charset_count, &p_def_string);
        #ifdef DEBUG
        v_print_missing_fonts ( pp_missing_charset_list,
                                n_missing_charset_count, p_base_font[i] );
        #endif

        if ( a_fontset != 0 )
            break;
    }

    #ifdef DEBUG
    v_print_chosen_fonts( a_fontset );
    #endif

    return a_fontset;
}

// ----------------------------------------------------------------------------
// convenience routine to add items to a XVaNestedList
// ----------------------------------------------------------------------------

static XVaNestedList
XVaAddToNestedList( XVaNestedList a_srclist, char* name, XPointer value )
{
    XVaNestedList a_dstlist;

    // if ( value == NULL )
    //  return a_srclist;

    if ( a_srclist == NULL )
    {
        a_dstlist = XVaCreateNestedList(
            0,
            name,           value,
            0 );
    }
    else
    {
        a_dstlist = XVaCreateNestedList(
            0,
            XNVaNestedList, a_srclist,
            name,           value,
            0 );
    }

    return a_dstlist != NULL ? a_dstlist : a_srclist ;
}

// ---------------------------------------------------------------------------
//
// Constructor for a InputContext (IC)
//
// ----------------------------------------------------------------------------

SalI18N_InputContext::SalI18N_InputContext ( SalFrame *pFrame,
                                             Bool aIsOnTheSpot) :
            mbUseable( True ),
            maContext( (XIC)NULL ),
            mpAttributes( NULL ),
            mpStatusAttributes( NULL ),
            mpPreeditAttributes( NULL ),
            #ifdef SOLARIS
            mpFontSet( NULL ),
            #endif
            mnStatusStyle( 0 ),
            mnPreeditStyle( 0 ),
            mnSupportedStatusStyle(
                    #ifdef LINUX
                    XIMStatusCallbacks |
                    #endif
                    XIMStatusNothing | XIMStatusNone
                                /*  | XIMStatusCallbacks
                                    | XIMStatusArea */ )
{
    SalI18N_InputMethod *pInputMethod;
    pInputMethod = pFrame->maFrameData.GetDisplay()->GetInputMethod();
    mbMultiLingual = pInputMethod->IsMultiLingual();

    mnSupportedPreeditStyle = (aIsOnTheSpot?
                               XIMPreeditCallbacks |
                               XIMPreeditNothing | XIMPreeditNone :
                               XIMPreeditCallbacks |
                               XIMPreeditNothing | XIMPreeditNone);
    if (pInputMethod->UseMethod()
        && SupportInputMethodStyle( pInputMethod->GetSupportedStyles() ) )
    {
        XLIB_Window  aClientWindow= pFrame->maFrameData.GetShellWindow();
        XLIB_Window  aFocusWindow= pFrame->maFrameData.GetWindow();
        Display     *pDisplay    = XDisplayOfIM( pInputMethod->GetMethod() );

        // for status callbacks and commit string callbacks
        maClientData.bIsMultilingual        = mbMultiLingual;
        maClientData.eState                 = ePreeditStatusStartPending;
        maClientData.pFrame                 = pFrame;
          maClientData.aText.pUnicodeBuffer     = NULL;
          maClientData.aText.pCharStyle         = NULL;
          maClientData.aText.nCursorPos         = 0;
          maClientData.aText.nLength            = 0;
          maClientData.aText.nSize              = 0;

        // set status attributes
        // only none and nothing are supported which do not need special
        // arguments, area and position are just hacks, preedit callback
        // does work but cannot be connected without unicode/multibyte
        // support of independent vcl layer

        switch ( mnStatusStyle )
        {
            case XIMStatusCallbacks:
            {
                static XIMCallback aStatusStartCallback;
                static XIMCallback aStatusDoneCallback;
                static XIMCallback aStatusDrawCallback;

                aStatusStartCallback.callback    = (XIMProc)StatusStartCallback;
                aStatusStartCallback.client_data = (XPointer)NULL;
                aStatusDoneCallback.callback     = (XIMProc)StatusDoneCallback;
                aStatusDoneCallback.client_data  = (XPointer)NULL;
                aStatusDrawCallback.callback     = (XIMProc)StatusDrawCallback;
                aStatusDrawCallback.client_data  = (XPointer)NULL;

                mpStatusAttributes = XVaCreateNestedList (
                        0,
                        XNStatusStartCallback, &aStatusStartCallback,
                        XNStatusDoneCallback,  &aStatusDoneCallback,
                        XNStatusDrawCallback,  &aStatusDrawCallback,
                        0 );

                break;
            }

            case XIMStatusArea:
            {
                XRectangle *pStatusArea = new XRectangle;
                pStatusArea->x      = 0;
                pStatusArea->y      = 64;
                pStatusArea->width  = 256;
                pStatusArea->height = 64;

                mpStatusAttributes = XVaCreateNestedList (
                        0,
                        XNAreaNeeded, pStatusArea,
                        XNArea,       pStatusArea,
                        0 );

                break;
            }

            case XIMStatusNone:
            case XIMStatusNothing:
            default:

                break;
        }

        //
        // set preedit attributes
        //

        switch ( mnPreeditStyle )
        {
            case XIMPreeditCallbacks:

                maPreeditCaretCallback.callback = (XIMProc)PreeditCaretCallback;
                maPreeditStartCallback.callback = (XIMProc)PreeditStartCallback;
                maPreeditDoneCallback.callback  = (XIMProc)PreeditDoneCallback;
                maPreeditDrawCallback.callback  = (XIMProc)PreeditDrawCallback;
                maPreeditCaretCallback.client_data = (XPointer)&maClientData;
                maPreeditStartCallback.client_data = (XPointer)&maClientData;
                maPreeditDoneCallback.client_data  = (XPointer)&maClientData;
                maPreeditDrawCallback.client_data  = (XPointer)&maClientData;

                mpPreeditAttributes = XVaCreateNestedList (
                        0,
                        XNPreeditStartCallback, &maPreeditStartCallback,
                        XNPreeditDoneCallback,  &maPreeditDoneCallback,
                        XNPreeditDrawCallback,  &maPreeditDrawCallback,
                        XNPreeditCaretCallback, &maPreeditCaretCallback,
                        0 );

                break;

            case XIMPreeditPosition:
            case XIMPreeditArea:
            {
                XRectangle *pPreeditArea  = new XRectangle;
                XPoint     *pSpotLocation = new XPoint;

                pSpotLocation->x     = 16;
                pSpotLocation->y     = 32;

                pPreeditArea->x      = 0;
                pPreeditArea->y      = 0;
                pPreeditArea->width  = 256;
                pPreeditArea->height = 64;

                mpPreeditAttributes = XVaCreateNestedList (
                        0,
                        XNArea,         pPreeditArea,
                        XNSpotLocation, pSpotLocation,
                        0 );

                break;
            }

            case XIMPreeditNone:
            case XIMPreeditNothing:
            default:

                break;
        }

        // Create the InputContext by given it exact the information it
        // deserves, because inappropriate attributes
        // let XCreateIC fail on Solaris (eg. for C locale)

        mpAttributes = XVaCreateNestedList(
                0,
                XNFocusWindow,       aFocusWindow,
                XNClientWindow,      aClientWindow,
                XNInputStyle,        mnPreeditStyle | mnStatusStyle,
                0 );

        #ifdef SOLARIS
        mpDisplay = pDisplay;
        if ( 0 && (mpFontSet = p_create_fontset(mpDisplay)) != NULL )
        {
            mpStatusAttributes = XVaAddToNestedList( mpStatusAttributes,
                    XNFontSet, (XPointer)mpFontSet );
            mpPreeditAttributes = XVaAddToNestedList( mpPreeditAttributes,
                    XNFontSet, (XPointer)mpFontSet );
        }
        #endif
        if ( mnPreeditStyle != XIMPreeditNone )
        {
#if defined LINUX || defined FREEBSD || defined(NETBSD)
            if ( mpPreeditAttributes != NULL )
#endif
            mpAttributes = XVaAddToNestedList( mpAttributes,
                    XNPreeditAttributes, (XPointer)mpPreeditAttributes );
        }
        if ( mnStatusStyle != XIMStatusNone )
        {
#if defined LINUX || defined FREEBSD || defined(NETBSD)
            if ( mpStatusAttributes != NULL )
#endif
            mpAttributes = XVaAddToNestedList( mpAttributes,
                    XNStatusAttributes, (XPointer)mpStatusAttributes );
        }
        maContext = XCreateIC( pInputMethod->GetMethod(),
                XNVaNestedList, mpAttributes,
                NULL );
    }

    if ( maContext == NULL )
    {
        #ifdef DEBUG
        fprintf(stderr, "input context creation failed\n");
        #endif

        mbUseable = False;
        mbMultiLingual = False;

        if ( mpAttributes != NULL )
            XFree( mpAttributes );
        if ( mpStatusAttributes != NULL )
            XFree( mpStatusAttributes );
        if ( mpPreeditAttributes != NULL )
            XFree( mpPreeditAttributes );
    }

    if ( maContext != NULL && mbMultiLingual )
    {
        maCommitStringCallback.callback    = (XIMProc)::CommitStringCallback;
        maCommitStringCallback.client_data = (XPointer)&maClientData;
        XSetICValues( maContext,
                XNCommitStringCallback, &maCommitStringCallback,
                NULL );
    }

}

// ---------------------------------------------------------------------------
//
// In Solaris 8 the status window does not unmap if the frame unmapps, so
// unmap it the hard way
//
// ---------------------------------------------------------------------------

void
SalI18N_InputContext::Unmap()
{
    if ( maContext != NULL )
    {
        XDestroyIC( maContext );
        maContext = NULL;
    }
}

void
SalI18N_InputContext::Map( SalFrame *pFrame )
{
    if ( (maContext == NULL) && mbUseable)
    {
        SalI18N_InputMethod *pInputMethod;
        pInputMethod = pFrame->maFrameData.GetDisplay()->GetInputMethod();

        maContext = XCreateIC( pInputMethod->GetMethod(),
                               XNVaNestedList, mpAttributes,
                               NULL );
        if ( maContext != NULL && mbMultiLingual )
            XSetICValues( maContext,
                    XNCommitStringCallback, &maCommitStringCallback,
                    NULL );
        SetICFocus();
    }
}

// ---------------------------------------------------------------------------
//
//  make sure, the input method gets all the X-Events it needs, this is only
//  called once on each frame, it relys on a valid maContext
//
// ---------------------------------------------------------------------------

void
SalI18N_InputContext::ExtendEventMask( XLIB_Window aFocusWindow )
{
    unsigned long nIMEventMask;
    XWindowAttributes aWindowAttributes;

    if ( mbUseable )
    {
        Display *pDisplay = XDisplayOfIM( XIMOfIC(maContext) );

        XGetWindowAttributes( pDisplay, aFocusWindow,
            &aWindowAttributes );
        XGetICValues ( maContext,
            XNFilterEvents, &nIMEventMask,
            NULL);
        nIMEventMask |= aWindowAttributes.your_event_mask;
        XSelectInput ( pDisplay, aFocusWindow, nIMEventMask );
    }
}

// ---------------------------------------------------------------------------
//
// tune the styles provided by the input method with the supported one
//
// ---------------------------------------------------------------------------

unsigned int
SalI18N_InputContext::GetWeightingOfIMStyle( XIMStyle nStyle ) const
{
    struct StyleWeightingT {
        const XIMStyle      nStyle;
        const unsigned int  nWeight;
    };

    StyleWeightingT const *pWeightPtr;
    const StyleWeightingT pWeight[] = {
        { XIMPreeditCallbacks, 0x10000000 },
        { XIMPreeditPosition,  0x02000000 },
        { XIMPreeditArea,      0x01000000 },
        { XIMPreeditNothing,   0x00100000 },
        { XIMPreeditNone,      0x00010000 },
        { XIMStatusCallbacks,      0x1000 },
        { XIMStatusArea,           0x0100 },
        { XIMStatusNothing,        0x0010 },
        { XIMStatusNone,           0x0001 },
        { 0, 0x0 }
    };

    int nWeight = 0;
    for ( pWeightPtr = pWeight; pWeightPtr->nStyle != 0; pWeightPtr++ )
    {
        if ( (pWeightPtr->nStyle & nStyle) != 0 )
            nWeight += pWeightPtr->nWeight;
    }
    return nWeight;
}

Bool
SalI18N_InputContext::IsSupportedIMStyle( XIMStyle nStyle ) const
{
    if (   (nStyle & mnSupportedPreeditStyle)
        && (nStyle & mnSupportedStatusStyle) )
    {
        return True;
    }
    return False;
}

Bool
SalI18N_InputContext::SupportInputMethodStyle( XIMStyles *pIMStyles )
{
    int nBestScore   = 0;
    int nActualScore = 0;

    mnPreeditStyle = 0;
    mnStatusStyle  = 0;

    if ( pIMStyles != NULL )
    {
        // check whether the XIM supports one of the desired styles
        // only a single preedit and a single status style must occure
        // in a inpuut method style. Hideki said so, so i trust him
        for ( int nStyle = 0; nStyle < pIMStyles->count_styles; nStyle++ )
        {
            XIMStyle nProvidedStyle = pIMStyles->supported_styles[ nStyle ];
            if ( IsSupportedIMStyle(nProvidedStyle) )
            {
                nActualScore = GetWeightingOfIMStyle( nProvidedStyle );
                if ( nActualScore >= nBestScore )
                {
                    nBestScore = nActualScore;
                    mnPreeditStyle = nProvidedStyle & mnSupportedPreeditStyle;
                    mnStatusStyle  = nProvidedStyle & mnSupportedStatusStyle;
                }
            }
        }
    }

    #ifdef DEBUG
    char pBuf[ 128 ];
    fprintf( stderr, "selected inputmethod style = %s\n",
        GetMethodName(mnPreeditStyle | mnStatusStyle, pBuf, sizeof(pBuf)) );
    #endif

    return (mnPreeditStyle != 0) && (mnStatusStyle != 0) ;
}

int
SalI18N_InputContext::CommitStringCallback( sal_Unicode* pText, sal_Size nLength )
{
    XIMUnicodeText call_data;

    call_data.string.utf16_char = pText;
    call_data.length            = nLength;
    call_data.annotations       = NULL;
    call_data.count_annotations = 0;
    call_data.feedback          = NULL;

    return ::CommitStringCallback( maContext,
                    (XPointer)&maClientData, (XPointer)&call_data );
}

// ---------------------------------------------------------------------------
//
// set and unset the focus for the Input Context
// the context may be NULL despite it is useable if the framewindow is
// in unmapped state
//
// ---------------------------------------------------------------------------

void
SalI18N_InputContext::SetICFocus()
{
    if ( mbUseable && (maContext != NULL) )
        XSetICFocus( maContext );
}

void
SalI18N_InputContext::UnsetICFocus()
{
    if ( mbUseable && (maContext != NULL) )
        XUnsetICFocus( maContext );
}

// ---------------------------------------------------------------------------
//
// multi byte input method only
//
// ---------------------------------------------------------------------------

void
SalI18N_InputContext::SetPreeditState(Bool aPreeditState) {
#if XlibSpecificationRelease >= 6
  XIMPreeditState preedit_state = XIMPreeditUnKnown;
  XVaNestedList preedit_attr;

  preedit_attr = XVaCreateNestedList(0,
                                     XNPreeditState, &preedit_state,
                                     0);
  if (!XGetICValues(maContext,
                    XNPreeditAttributes, preedit_attr,
                    NULL)) {
    XFree(preedit_attr);

    preedit_state = (aPreeditState? XIMPreeditEnable : XIMPreeditDisable);
    preedit_attr = XVaCreateNestedList(0,
                                       XNPreeditState, preedit_state,
                                       0);
    XSetICValues(maContext,
                 XNPreeditAttributes, preedit_attr,
                 NULL);
  }
  XFree(preedit_attr);
#endif
  return;
}

void
SalI18N_InputContext::SetLanguage(LanguageType aInputLanguage) {
  // not yet implemented
  return;
}

void
SalI18N_InputContext::EndExtTextInput( USHORT nFlags )
{
    if ( mbUseable && (maContext != NULL) )
    {
#if XlibSpecificationRelease >= 6
      // restore conversion state after resetting XIC
      XIMPreeditState preedit_state = XIMPreeditUnKnown;
      XVaNestedList preedit_attr;

      Bool is_preedit_state = False;
      preedit_attr = XVaCreateNestedList(0,
                                         XNPreeditState, &preedit_state,
                                         0);
      if (!XGetICValues(maContext,
                        XNPreeditAttributes, preedit_attr,
                        NULL)) {
        is_preedit_state = True;
      }
      XFree(preedit_attr);
#endif

      char *pPendingChars = XmbResetIC( maContext );

#if XlibSpecificationRelease >= 6
        preedit_attr = XVaCreateNestedList(0,
                                           XNPreeditState, preedit_state,
                                           0);
        if (is_preedit_state) {
          XSetICValues(maContext,
                       XNPreeditAttributes, preedit_attr,
                       NULL);
        }
        XFree(preedit_attr);
#endif
        // text is unicode
        if (   (pPendingChars != NULL)
            && (nFlags & SAL_FRAME_ENDEXTTEXTINPUT_COMPLETE) )
        {
            XIMUnicodeText aPendingText;
            int nLen;
            sal_Unicode* pPtr;
            rtl_TextEncoding nEncoding = gsl_getSystemTextEncoding();

            // buffer is already unicode
            if ( mbMultiLingual || nEncoding == RTL_TEXTENCODING_UNICODE )
            {
                pPtr = (sal_Unicode*)pPendingChars;
                for ( nLen = 0; pPtr[ nLen ] != (sal_Unicode)0; nLen++ )
                    ;
            }
            // else convert buffer to unicode
            else
            {
                for ( nLen = 0; pPendingChars[ nLen ] != (char)0; nLen++ )
                    ;

                // create text converter
                rtl_TextToUnicodeConverter aConverter =
                        rtl_createTextToUnicodeConverter( nEncoding );
                rtl_TextToUnicodeContext aContext =
                         rtl_createTextToUnicodeContext( aConverter );

                sal_Size    nBufferSize = nLen * 2;
                sal_uInt32  nConversionInfo;
                sal_Size    nConvertedChars;

                pPtr = (sal_Unicode*) alloca( nBufferSize );

                // convert to single byte text stream
                nLen = rtl_convertTextToUnicode(
                        aConverter, aContext, (char*)pPendingChars,
                        nLen, pPtr, nBufferSize,
                          RTL_TEXTTOUNICODE_FLAGS_UNDEFINED_IGNORE
                        | RTL_TEXTTOUNICODE_FLAGS_INVALID_IGNORE,
                        &nConversionInfo, &nConvertedChars );

                // destroy converter
                rtl_destroyTextToUnicodeContext( aConverter, aContext );
                rtl_destroyTextToUnicodeConverter( aConverter );
            }
            aPendingText.length = nLen;
            aPendingText.string.utf16_char = pPtr;

            ::CommitStringCallback( maContext,
                    (XPointer)&maClientData, (XPointer)&aPendingText );
        }
        if ( pPendingChars != NULL )
            XFree ( (void*)pPendingChars  );
    }
}


