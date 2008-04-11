/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: wrkwin.hxx,v $
 * $Revision: 1.3 $
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

#ifndef _SV_WRKWIN_HXX
#define _SV_WRKWIN_HXX

#include <vcl/sv.h>
#include <vcl/dllapi.h>
#include <vcl/syswin.hxx>

namespace com { namespace sun { namespace star { namespace uno { class Any; }}}}
struct SystemParentData;

// ----------------------
// - WorkWindow - Types -
// ----------------------

// Presentation Flags
#define PRESENTATION_HIDEALLAPPS    ((USHORT)0x0001)
#define PRESENTATION_NOFULLSCREEN   ((USHORT)0x0002)
#define PRESENTATION_NOAUTOSHOW     ((USHORT)0x0004)

// --------------
// - WorkWindow -
// --------------

class VCL_DLLPUBLIC WorkWindow : public SystemWindow
{
private:
    USHORT          mnPresentationFlags;
    BOOL            mbPresentationMode:1,
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
    SAL_DLLPRIVATE void ImplLoadRes( const ResId& rResId );
    SAL_DLLPRIVATE void ImplSetFrameState( ULONG aFrameState );

public:
                    WorkWindow( Window* pParent, const ResId& rResId );
                    WorkWindow( Window* pParent, WinBits nStyle = WB_STDWORK );
                    WorkWindow( Window* pParent, const ::com::sun::star::uno::Any& aSystemWorkWindowToken, WinBits nStyle = WB_STDWORK );
                    WorkWindow( SystemParentData* pParent ); // Not in the REMOTE-Version
                    ~WorkWindow();

    virtual BOOL    Close();

    void            ShowFullScreenMode( BOOL bFullScreenMode = TRUE, sal_Int32 nDisplay = 0 );
    void            EndFullScreenMode() { ShowFullScreenMode( FALSE ); }
    BOOL            IsFullScreenMode() const { return mbFullScreenMode; }

    void            StartPresentationMode( BOOL bPresentation = TRUE, USHORT nFlags = 0, sal_Int32 nDisplay = 0 );
    void            EndPresentationMode() {  StartPresentationMode( FALSE ); }
    BOOL            IsPresentationMode() const { return mbPresentationMode; }

    BOOL            IsMinimized() const;

    BOOL            SetPluginParent( SystemParentData* pParent );

    void            Minimize();
    void            Restore();

    void            Maximize( BOOL bMaximize = TRUE );
    BOOL            IsMaximized();
};

#endif // _SV_WRKWIN_HXX
