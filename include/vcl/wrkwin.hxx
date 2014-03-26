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

#ifndef INCLUDED_VCL_WRKWIN_HXX
#define INCLUDED_VCL_WRKWIN_HXX

#include <tools/solar.h>
#include <vcl/dllapi.h>
#include <vcl/syswin.hxx>

namespace com { namespace sun { namespace star { namespace uno { class Any; }}}}
struct SystemParentData;


// - WorkWindow - Types -


// Presentation Flags
#define PRESENTATION_HIDEALLAPPS    ((sal_uInt16)0x0001)
#define PRESENTATION_NOFULLSCREEN   ((sal_uInt16)0x0002)
#define PRESENTATION_NOAUTOSHOW     ((sal_uInt16)0x0004)


// - WorkWindow -


class VCL_DLLPUBLIC WorkWindow : public SystemWindow
{
private:
    sal_uInt16          mnPresentationFlags;
    bool            mbPresentationMode:1,
                    mbPresentationVisible:1,
                    mbPresentationFull:1,
                    mbFullScreenMode:1;

    SAL_DLLPRIVATE void ImplInitWorkWindowData();
    SAL_DLLPRIVATE void ImplInit( Window* pParent, WinBits nStyle, const ::com::sun::star::uno::Any& aSystemWorkWindowToken );

private:
    SAL_DLLPRIVATE              WorkWindow( const WorkWindow& rWin );
    SAL_DLLPRIVATE WorkWindow&  operator =( const WorkWindow& rWin );

protected:
    explicit        WorkWindow( WindowType nType );
    SAL_DLLPRIVATE void ImplInit( Window* pParent, WinBits nStyle, SystemParentData* pSystemParentData = NULL );
    SAL_DLLPRIVATE void ImplSetFrameState( sal_uLong aFrameState );

public:
    explicit        WorkWindow( Window* pParent, WinBits nStyle = WB_STDWORK );
    explicit        WorkWindow( Window* pParent, const ::com::sun::star::uno::Any& aSystemWorkWindowToken, WinBits nStyle = WB_STDWORK );
    explicit        WorkWindow( SystemParentData* pParent ); // Not in the REMOTE-Version
    virtual         ~WorkWindow();

    virtual bool    Close() SAL_OVERRIDE;

    void            ShowFullScreenMode( bool bFullScreenMode,
                                        sal_Int32 nDisplayScreen );
    /**
     @overload void ShowFullScreenMode(bool bFullScreenMode, sal_Int32 nDisplayScreen)
    */
    void            ShowFullScreenMode( bool bFullScreenMode = true );
    void            EndFullScreenMode() { ShowFullScreenMode( false ); }
    bool        IsFullScreenMode() const { return mbFullScreenMode; }

    void            StartPresentationMode( bool   bPresentation,
                                           sal_uInt16 nFlags,
                                           sal_Int32  nDisplayScreen );
    /**
     @overload void StartPresentationMode( bool bPresentation, sal_uInt16 nFlags, sal_uInt32 nDisplayScreen)
    */
    void            StartPresentationMode( bool   bPresentation = true,
                                           sal_uInt16 nFlags = 0 );
    void            EndPresentationMode() {  StartPresentationMode( false ); }
    bool        IsPresentationMode() const { return mbPresentationMode; }

    bool        IsMinimized() const;

    bool        SetPluginParent( SystemParentData* pParent );

    void            Minimize();
    void            Restore();

    void            Maximize( bool bMaximize = true );
    bool        IsMaximized() const;
};

#endif // INCLUDED_VCL_WRKWIN_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
