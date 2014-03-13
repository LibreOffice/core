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

#ifndef INCLUDED_VCL_MOREBTN_HXX
#define INCLUDED_VCL_MOREBTN_HXX

#include <tools/solar.h>
#include <vcl/dllapi.h>
#include <vcl/mapmod.hxx>
#include <vcl/button.hxx>

struct ImplMoreButtonData;


// - MoreButton -


class VCL_DLLPUBLIC MoreButton : public PushButton
{
private:
    ImplMoreButtonData* mpMBData;
    sal_uLong               mnDelta;
    MapUnit             meUnit;
    bool                mbState;

    // Copy assignment is forbidden and not implemented.
    SAL_DLLPRIVATE      MoreButton( const MoreButton & );
    SAL_DLLPRIVATE      MoreButton& operator=( const MoreButton & );
    SAL_DLLPRIVATE void ShowState();

protected:
    using Window::ImplInit;
    SAL_DLLPRIVATE void ImplInit( Window* pParent, WinBits nStyle );

public:
    explicit            MoreButton( Window* pParent, WinBits nStyle = 0 );
    virtual             ~MoreButton();

    void                Click();

    void                SetDelta( sal_uLong nNewDelta ) { mnDelta = nNewDelta; }
    sal_uLong           GetDelta() const { return mnDelta; }

    void                SetMapUnit( MapUnit eNewUnit = MAP_PIXEL ) { meUnit = eNewUnit; }
    MapUnit             GetMapUnit() const { return meUnit; }

    using PushButton::SetState;
    void                SetState( bool bNewState = true );
    bool            GetState() const { return mbState; }

    void                SetText( const OUString& rNewText );
    OUString            GetText() const;
};

inline void MoreButton::SetState( bool bNewState )
{
    if ( mbState != bNewState )
        Click();
}

#endif // INCLUDED_VCL_MOREBTN_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
