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

#ifndef INCLUDED_VCL_DOCKINGAREA_HXX
#define INCLUDED_VCL_DOCKINGAREA_HXX

#include <vcl/window.hxx>

//  A simple container for docked toolbars
//  - its main purpose is theming support
class VCL_DLLPUBLIC DockingAreaWindow : public Window
{
    class ImplData;

private:
    ImplData*       mpImplData;

    // Copy assignment is forbidden and not implemented.
    SAL_DLLPRIVATE  DockingAreaWindow (const DockingAreaWindow &);
    SAL_DLLPRIVATE  DockingAreaWindow & operator= (const DockingAreaWindow &);

public:
    explicit        DockingAreaWindow( Window* pParent );
    virtual         ~DockingAreaWindow();

    void            SetAlign( WindowAlign eNewAlign );
    WindowAlign     GetAlign() const;
    bool            IsHorizontal() const;

    virtual void    Paint( const Rectangle& rRect );
    virtual void    Resize();
    virtual void    DataChanged( const DataChangedEvent& rDCEvt );
    virtual void    StateChanged( StateChangedType nType );

};

#endif // INCLUDED_VCL_DOCKINGAREA_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
