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
class VCL_DLLPUBLIC DockingAreaWindow : public vcl::Window
{
    class ImplData;

private:
    ImplData*       mpImplData;

                    DockingAreaWindow (const DockingAreaWindow &) = delete;
                    DockingAreaWindow & operator= (const DockingAreaWindow &) = delete;

public:
    explicit        DockingAreaWindow( vcl::Window* pParent );
    virtual         ~DockingAreaWindow();
    virtual void    dispose() override;

    void            SetAlign( WindowAlign eNewAlign );
    WindowAlign     GetAlign() const;
    bool            IsHorizontal() const;

    virtual void    ApplySettings(vcl::RenderContext& rRenderContext) override;
    virtual void    Paint( vcl::RenderContext& rRenderContext, const Rectangle& rRect ) override;
    virtual void    Resize() override;
    virtual void    DataChanged( const DataChangedEvent& rDCEvt ) override;
    virtual void    StateChanged( StateChangedType nType ) override;

};

#endif // INCLUDED_VCL_DOCKINGAREA_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
