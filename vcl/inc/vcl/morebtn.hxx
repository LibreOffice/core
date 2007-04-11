/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: morebtn.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-04-11 18:01:17 $
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

#ifndef _SV_MOREBTN_HXX
#define _SV_MOREBTN_HXX

#ifndef _SV_SV_H
#include <vcl/sv.h>
#endif

#ifndef _VCL_DLLAPI_H
#include <vcl/dllapi.h>
#endif

#ifndef _SV_MAPMOD_HXX
#include <vcl/mapmod.hxx>
#endif
#ifndef _SV_BUTTON_HXX
#include <vcl/button.hxx>
#endif

struct ImplMoreButtonData;

// --------------
// - MoreButton -
// --------------

class VCL_DLLPUBLIC MoreButton : public PushButton
{
private:
    ImplMoreButtonData* mpMBData;
    ULONG               mnDelta;
    MapUnit             meUnit;
    BOOL                mbState;

    // Copy assignment is forbidden and not implemented.
    SAL_DLLPRIVATE      MoreButton( const MoreButton & );
    SAL_DLLPRIVATE      MoreButton& operator=( const MoreButton & );
    SAL_DLLPRIVATE void ShowState();

protected:
    using Window::ImplInit;
    SAL_DLLPRIVATE void ImplInit( Window* pParent, WinBits nStyle );
    SAL_DLLPRIVATE void ImplLoadRes( const ResId& rResId );

public:
                        MoreButton( Window* pParent, WinBits nStyle = 0 );
                        MoreButton( Window* pParent, const ResId& rResId );
                        ~MoreButton();

    void                Click();

    void                AddWindow( Window* pWindow );
    void                RemoveWindow( Window* pWindow );

    void                SetDelta( ULONG nNewDelta ) { mnDelta = nNewDelta; }
    ULONG               GetDelta() const { return mnDelta; }

    void                SetMapUnit( MapUnit eNewUnit = MAP_PIXEL ) { meUnit = eNewUnit; }
    MapUnit             GetMapUnit() const { return meUnit; }

    using PushButton::SetState;
    void                SetState( BOOL bNewState = TRUE );
    BOOL                GetState() const { return mbState; }

    void                SetText( const XubString& rNewText );
    XubString           GetText() const;

    void                SetMoreText( const XubString& rNewText );
    void                SetLessText( const XubString& rNewText );
    XubString           GetMoreText() const;
    XubString           GetLessText() const;
};

inline void MoreButton::SetState( BOOL bNewState )
{
    if ( mbState != bNewState )
        Click();
}

#endif  // _SV_MOREBTN_HXX
