/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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

#ifndef _SV_DOCKINGAREA_HXX
#define _SV_DOCKINGAREA_HXX

#include <vcl/sv.h>
#include <vcl/window.hxx>

// ------------------------------------------
//  A simple container for docked toolbars
//  - its main purpose is theming support
// ------------------------------------------

class VCL_DLLPUBLIC DockingAreaWindow : public Window
{
    class ImplData;

private:
    ImplData*       mpImplData;

    // Copy assignment is forbidden and not implemented.
    SAL_DLLPRIVATE  DockingAreaWindow (const DockingAreaWindow &);
    SAL_DLLPRIVATE  DockingAreaWindow & operator= (const DockingAreaWindow &);

public:
                    DockingAreaWindow( Window *pParent );
                    ~DockingAreaWindow();

    void            SetAlign( WindowAlign eNewAlign );
    WindowAlign     GetAlign() const;
    sal_Bool            IsHorizontal() const;

    virtual void    Paint( const Rectangle& rRect );
    virtual void    Resize();
    virtual void    DataChanged( const DataChangedEvent& rDCEvt );
    virtual void    StateChanged( StateChangedType nType );

};

#endif // _SV_SYSWIN_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
