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

#ifndef INCLUDED_VCL_TABPAGE_HXX
#define INCLUDED_VCL_TABPAGE_HXX

#include <vcl/dllapi.h>
#include <vcl/builder.hxx>
#include <vcl/window.hxx>
#include <vcl/IContext.hxx>

class VCL_DLLPUBLIC TabPage
    : public vcl::Window
    , public VclBuilderContainer
    , public vcl::IContext
{
private:
    using Window::ImplInit;
    SAL_DLLPRIVATE void ImplInit( vcl::Window* pParent, WinBits nStyle );
    SAL_DLLPRIVATE void ImplInitSettings();

public:
    explicit        TabPage( vcl::Window* pParent, WinBits nStyle = 0 );
    explicit        TabPage( vcl::Window *pParent, const OString& rID, const OUString& rUIXMLDescription );
    virtual         ~TabPage() override;
    virtual void    dispose() override;

    virtual void    Paint( vcl::RenderContext& rRenderContext, const tools::Rectangle& rRect ) override;
    virtual void    Draw( OutputDevice* pDev, const Point& rPos, DrawFlags nFlags ) override;

    virtual void    StateChanged( StateChangedType nStateChange ) override;
    virtual void    DataChanged( const DataChangedEvent& rDCEvt ) override;

    //To-Do, consider inheriting from VclContainer
    virtual void    SetPosSizePixel(const Point& rNewPos, const Size& rNewSize) override;
    virtual void    SetPosPixel(const Point& rNewPos) override;
    virtual void    SetSizePixel(const Size& rNewSize) override;
    virtual Size    GetOptimalSize() const override;
};

#endif // INCLUDED_VCL_TABPAGE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
