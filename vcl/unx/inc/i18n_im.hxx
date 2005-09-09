/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: i18n_im.hxx,v $
 *
 *  $Revision: 1.13 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 12:40:19 $
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

#ifndef _SAL_I18N_INPUTMETHOD_HXX
#define _SAL_I18N_INPUTMETHOD_HXX

#ifndef _VCL_DLLAPI_H
#include "dllapi.h"
#endif

extern "C" char* GetMethodName( XIMStyle nStyle, char *pBuf, int nBufSize);

#define bUseInputMethodDefault True

class VCL_DLLPUBLIC SalI18N_InputMethod
{
    Bool        mbUseable;  // system supports locale as well as status
                            // and preedit style ?
    Bool        mbMultiLingual; // system supports iiimp
    XIM         maMethod;
    XIMCallback maDestroyCallback;
    XIMStyles  *mpStyles;

public:

    Bool        IsMultiLingual()        { return mbMultiLingual;    }
    Bool        PosixLocale();
    Bool        UseMethod()             { return mbUseable; }
    XIM         GetMethod()             { return maMethod;  }
    void        HandleDestroyIM();
    Bool        CreateMethod( Display *pDisplay );
    XIMStyles  *GetSupportedStyles()    { return mpStyles;  }
    Bool        SetLocale( const char* pLocale = "" );
    Bool        FilterEvent( XEvent *pEvent, XLIB_Window window );
    Bool        AddConnectionWatch (Display *pDisplay, void *pConnectionHandler);

    #ifdef _USE_PRINT_EXTENSION_
    void            Invalidate()                    { mbUseable = False; }
    #endif

    SalI18N_InputMethod();
    ~SalI18N_InputMethod();
};

#endif // _SAL_I18N_INPUTMETHOD_HXX


