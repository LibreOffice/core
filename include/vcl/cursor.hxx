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

#ifndef INCLUDED_VCL_CURSOR_HXX
#define INCLUDED_VCL_CURSOR_HXX

#include <tools/gen.hxx>
#include <tools/link.hxx>
#include <vcl/dllapi.h>
#include <vcl/vclptr.hxx>
#include <rtl/ustring.hxx>
#include <memory>

class Timer;
struct ImplCursorData;
class OutputDevice;
namespace vcl { class Window; }

// Cursor styles
#define CURSOR_SHADOW                   (sal_uInt16(0x0001))

enum class CursorDirection
{
    NONE, LTR, RTL
};

namespace vcl
{

class VCL_DLLPUBLIC Cursor
{
private:
    std::unique_ptr<ImplCursorData> mpData;
    VclPtr<vcl::Window> mpWindow;           // only for shadow cursor
    Size            maSize;
    Point           maPos;
    Degree10        mnOrientation;
    sal_uInt16      mnStyle;
    bool            mbVisible;
    CursorDirection mnDirection;

public:
    SAL_DLLPRIVATE void         ImplDraw();
    DECL_DLLPRIVATE_LINK( ImplTimerHdl, Timer*, void );
    SAL_DLLPRIVATE void         ImplShow();
    SAL_DLLPRIVATE void         ImplHide();
    SAL_DLLPRIVATE void         ImplResume( bool bRestore = false );
    SAL_DLLPRIVATE bool         ImplSuspend();
    SAL_DLLPRIVATE void         ImplNew();

public:
                    Cursor();
                    Cursor( const Cursor& rCursor );
                    ~Cursor();

    void            SetStyle( sal_uInt16 nStyle );
    sal_uInt16      GetStyle() const { return mnStyle; }

    void            Show();
    void            Hide();
    bool            IsVisible() const { return mbVisible; }

    void            SetWindow( vcl::Window* pWindow );

    void            SetPos( const Point& rNewPos );
    const Point&    GetPos() const { return maPos; }

    void            SetSize( const Size& rNewSize );
    const Size&     GetSize() const { return maSize; }
    void            SetWidth( tools::Long nNewWidth );
    tools::Long            GetWidth() const { return maSize.Width(); }
    tools::Long            GetHeight() const { return maSize.Height(); }

    void            SetOrientation( Degree10 nOrientation = 0_deg10 );

    void            SetDirection( CursorDirection nDirection = CursorDirection::NONE );

    Cursor&         operator=( const Cursor& rCursor );
    bool            operator==( const Cursor& rCursor ) const;
    bool            operator!=( const Cursor& rCursor ) const
                        { return !(Cursor::operator==( rCursor )); }

    void            DrawToDevice(OutputDevice& rRenderContext);

private:
    void LOKNotify( vcl::Window* pWindow, const OUString& rAction );
    bool ImplPrepForDraw(const OutputDevice* pDevice, ImplCursorData& rData);
    void ImplRestore();
    void ImplDoShow( bool bDrawDirect, bool bRestore );
    bool ImplDoHide( bool bStop );
};

} // namespace vcl

#endif // INCLUDED_VCL_CURSOR_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
