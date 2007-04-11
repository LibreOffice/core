/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: wrkwin.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-04-11 18:17:29 $
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

#ifndef _SV_WRKWIN_HXX
#define _SV_WRKWIN_HXX

#ifndef _SV_SV_H
#include <vcl/sv.h>
#endif

#ifndef _VCL_DLLAPI_H
#include <vcl/dllapi.h>
#endif

#ifndef _SV_SYSWIN_HXX
#include <vcl/syswin.hxx>
#endif

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
