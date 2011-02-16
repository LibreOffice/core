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

#ifndef _SV_MOREBTN_HXX
#define _SV_MOREBTN_HXX

#include <vcl/sv.h>
#include <vcl/dllapi.h>
#include <vcl/mapmod.hxx>
#include <vcl/button.hxx>

struct ImplMoreButtonData;

// --------------
// - MoreButton -
// --------------

class VCL_DLLPUBLIC MoreButton : public PushButton
{
private:
    ImplMoreButtonData* mpMBData;
    sal_uLong               mnDelta;
    MapUnit             meUnit;
    sal_Bool                mbState;

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

    void                SetDelta( sal_uLong nNewDelta ) { mnDelta = nNewDelta; }
    sal_uLong               GetDelta() const { return mnDelta; }

    void                SetMapUnit( MapUnit eNewUnit = MAP_PIXEL ) { meUnit = eNewUnit; }
    MapUnit             GetMapUnit() const { return meUnit; }

    using PushButton::SetState;
    void                SetState( sal_Bool bNewState = sal_True );
    sal_Bool                GetState() const { return mbState; }

    void                SetText( const XubString& rNewText );
    XubString           GetText() const;

    void                SetMoreText( const XubString& rNewText );
    void                SetLessText( const XubString& rNewText );
    XubString           GetMoreText() const;
    XubString           GetLessText() const;
};

inline void MoreButton::SetState( sal_Bool bNewState )
{
    if ( mbState != bNewState )
        Click();
}

#endif  // _SV_MOREBTN_HXX
