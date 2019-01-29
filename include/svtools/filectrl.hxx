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

#ifndef INCLUDED_SVTOOLS_FILECTRL_HXX
#define INCLUDED_SVTOOLS_FILECTRL_HXX

#include <svtools/svtdllapi.h>
#include <vcl/window.hxx>
#include <vcl/button.hxx>

class Edit;

// Flags for internal use of FileControl
enum class FileControlMode_Internal
{
    INRESIZE               = 0x0001,
    ORIGINALBUTTONTEXT     = 0x0002,
};

namespace o3tl
{
    template<> struct typed_flags<FileControlMode_Internal> : is_typed_flags<FileControlMode_Internal, 0x03> {};
}


class SVT_DLLPUBLIC FileControl final : public vcl::Window
{
    VclPtr<Edit>       maEdit;
    VclPtr<PushButton> maButton;
    OUString const           maButtonText;
    FileControlMode_Internal    mnInternalFlags;

    SVT_DLLPRIVATE void     Resize() override;
    SVT_DLLPRIVATE void     GetFocus() override;
    SVT_DLLPRIVATE void     StateChanged( StateChangedType nType ) override;
    SVT_DLLPRIVATE WinBits  ImplInitStyle( WinBits nStyle );
    DECL_DLLPRIVATE_LINK( ButtonHdl, Button*, void );

public:
                    FileControl( vcl::Window* pParent, WinBits nStyle );
    virtual         ~FileControl() override;
    virtual void    dispose() override;

    Edit&           GetEdit() { return *maEdit; }
    PushButton&     GetButton() { return *maButton; }

    void            Draw( OutputDevice* pDev, const Point& rPos, const Size& rSize, DrawFlags nFlags ) override;

    void            SetText( const OUString& rStr ) override;
    OUString        GetText() const override;

    void            SetEditModifyHdl( const Link<Edit&,void>& rLink );
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
