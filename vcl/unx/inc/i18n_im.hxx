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

#ifndef _SAL_I18N_INPUTMETHOD_HXX
#define _SAL_I18N_INPUTMETHOD_HXX

#include <vcl/dllapi.h>

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

    SalI18N_InputMethod();
    ~SalI18N_InputMethod();
};

#endif // _SAL_I18N_INPUTMETHOD_HXX


