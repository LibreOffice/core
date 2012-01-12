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



#ifndef _SAL_I18N_INPUTMETHOD_HXX
#define _SAL_I18N_INPUTMETHOD_HXX

#include <vclpluginapi.h>

extern "C" char* GetMethodName( XIMStyle nStyle, char *pBuf, int nBufSize);

#define bUseInputMethodDefault True

class VCLPLUG_GEN_PUBLIC SalI18N_InputMethod
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


