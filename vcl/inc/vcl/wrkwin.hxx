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

#ifndef _SV_WRKWIN_HXX
#define _SV_WRKWIN_HXX

#include <tools/solar.h>
#include <vcl/dllapi.h>
#include <vcl/syswin.hxx>

namespace com { namespace sun { namespace star { namespace uno { class Any; }}}}
struct SystemParentData;

// ----------------------
// - WorkWindow - Types -
// ----------------------

// Presentation Flags
#define PRESENTATION_HIDEALLAPPS    ((sal_uInt16)0x0001)
#define PRESENTATION_NOFULLSCREEN   ((sal_uInt16)0x0002)
#define PRESENTATION_NOAUTOSHOW     ((sal_uInt16)0x0004)

// --------------
// - WorkWindow -
// --------------

class VCL_DLLPUBLIC WorkWindow : public SystemWindow
{
private:
    sal_uInt16          mnPresentationFlags;
    sal_Bool            mbPresentationMode:1,
                    mbPresentationVisible:1,
                    mbPresentationFull:1,
                    mbFullScreenMode:1;

    SAL_DLLPRIVATE void ImplInitWorkWindowData();
    SAL_DLLPRIVATE void ImplInit( Window* pParent, WinBits nStyle, const ::com::sun::star::uno::Any& aSystemWorkWindowToken );

private:
    SAL_DLLPRIVATE              WorkWindow( const WorkWindow& rWin );
    SAL_DLLPRIVATE WorkWindow&  operator =( const WorkWindow& rWin );

protected:
                        WorkWindow( WindowType nType );
    SAL_DLLPRIVATE void ImplInit( Window* pParent, WinBits nStyle, SystemParentData* pSystemParentData = NULL );
    SAL_DLLPRIVATE void ImplSetFrameState( sal_uLong aFrameState );

public:
                    WorkWindow( Window* pParent, WinBits nStyle = WB_STDWORK );
                    WorkWindow( Window* pParent, const ::com::sun::star::uno::Any& aSystemWorkWindowToken, WinBits nStyle = WB_STDWORK );
                    WorkWindow( SystemParentData* pParent ); // Not in the REMOTE-Version
                    ~WorkWindow();

    virtual sal_Bool    Close();

    void            ShowFullScreenMode( sal_Bool bFullScreenMode,
                                        sal_Int32 nDisplayScreen );
    /**
     @overload Calls ShowFullScreenMode( bFullScreenMode, GetScreenNumber()).
    */
    void            ShowFullScreenMode( sal_Bool bFullScreenMode = sal_True );
    void            EndFullScreenMode() { ShowFullScreenMode( sal_False ); }
    sal_Bool        IsFullScreenMode() const { return mbFullScreenMode; }

    void            StartPresentationMode( sal_Bool   bPresentation,
                                           sal_uInt16 nFlags,
                                           sal_Int32  nDisplayScreen );
    /**
     @overload Calls StartPresentationMode( bFullScreenMode, nFlags, GetScreenNumber()).
    */
    void            StartPresentationMode( sal_Bool   bPresentation = sal_True,
                                           sal_uInt16 nFlags = 0 );
    void            EndPresentationMode() {  StartPresentationMode( sal_False ); }
    sal_Bool        IsPresentationMode() const { return mbPresentationMode; }

    sal_Bool        IsMinimized() const;

    sal_Bool        SetPluginParent( SystemParentData* pParent );

    void            Minimize();
    void            Restore();

    void            Maximize( sal_Bool bMaximize = sal_True );
    sal_Bool        IsMaximized() const;
};

#endif // _SV_WRKWIN_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
