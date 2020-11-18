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
#pragma once

#include <svtools/svtdllapi.h>
#include <unotools/configitem.hxx>

class Application;
enum class MouseMiddleButtonAction;

enum class SnapType {
    ToButton = 0,
    ToMiddle,
    NONE
};

enum class DragMode {
    FullWindow,
    Frame,
    SystemDep
};


class SVT_DLLPUBLIC SvtTabAppearanceCfg final : public utl::ConfigItem
{
    DragMode        nDragMode           ;
    SnapType        nSnapMode           ;
    MouseMiddleButtonAction nMiddleMouse;
    short           nAAMinPixelHeight   ;
    bool            bFontAntialiasing       ;

    bool            bMenuMouseFollow        ;

    static bool  bInitialized ;

    SVT_DLLPRIVATE static const css::uno::Sequence<OUString>& GetPropertyNames();
    virtual void    ImplCommit() override;

public:
    SvtTabAppearanceCfg( );
    virtual ~SvtTabAppearanceCfg( ) override;

    virtual void Notify( const css::uno::Sequence< OUString >& _rPropertyNames) override;

    DragMode  GetDragMode  () const { return nDragMode; }

    SnapType    GetSnapMode () const { return nSnapMode; }
    void        SetSnapMode ( SnapType nSet );

    MouseMiddleButtonAction GetMiddleMouseButton () const { return nMiddleMouse; }
    void        SetMiddleMouseButton ( MouseMiddleButtonAction nSet );

    void        SetApplicationDefaults ( Application* pApp );

    bool        IsMenuMouseFollow() const{return bMenuMouseFollow;}

    void        SetFontAntiAliasing( bool bSet )    { bFontAntialiasing = bSet; SetModified(); }
    bool        IsFontAntiAliasing() const { return bFontAntialiasing; }

    sal_uInt16  GetFontAntialiasingMinPixelHeight( ) const { return nAAMinPixelHeight; }
    void        SetFontAntialiasingMinPixelHeight( sal_uInt16 _nMinHeight ) { nAAMinPixelHeight = _nMinHeight; SetModified(); }

    static bool IsInitialized()  { return bInitialized; }
    static void SetInitialized() { bInitialized = true; }
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
