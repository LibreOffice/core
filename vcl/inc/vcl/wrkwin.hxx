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
    SAL_DLLPRIVATE void ImplLoadRes( const ResId& rResId );
    SAL_DLLPRIVATE void ImplSetFrameState( sal_uLong aFrameState );

public:
                    WorkWindow( Window* pParent, const ResId& rResId );
                    WorkWindow( Window* pParent, WinBits nStyle = WB_STDWORK );
                    WorkWindow( Window* pParent, const ::com::sun::star::uno::Any& aSystemWorkWindowToken, WinBits nStyle = WB_STDWORK );
                    WorkWindow( SystemParentData* pParent ); // Not in the REMOTE-Version
                    ~WorkWindow();

    virtual sal_Bool    Close();

    /** The default value of nDisplay = -1 means "don't care" and
        allows to backends to use any screen [** or display? terminology!]
        they like (most probably the current one).

        NOTE: The default value cannot be 0, because 0 is a legitimate
        screen number.
     */
    void            ShowFullScreenMode( sal_Bool bFullScreenMode = sal_True, sal_Int32 nDisplay = -1 );
    void            EndFullScreenMode() { ShowFullScreenMode( sal_False ); }
    sal_Bool            IsFullScreenMode() const { return mbFullScreenMode; }

    void            StartPresentationMode( sal_Bool bPresentation = sal_True, sal_uInt16 nFlags = 0, sal_Int32 nDisplay = 0 );
    void            EndPresentationMode() {  StartPresentationMode( sal_False ); }
    sal_Bool            IsPresentationMode() const { return mbPresentationMode; }

    sal_Bool            IsMinimized() const;

    sal_Bool            SetPluginParent( SystemParentData* pParent );

    void            Minimize();
    void            Restore();

    void            Maximize( sal_Bool bMaximize = sal_True );
    sal_Bool            IsMaximized() const;
};

#endif // _SV_WRKWIN_HXX
