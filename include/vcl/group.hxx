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

#ifndef INCLUDED_VCL_GROUP_HXX
#define INCLUDED_VCL_GROUP_HXX

#include <tools/solar.h>
#include <vcl/dllapi.h>
#include <vcl/ctrl.hxx>


// - GroupBox -


class VCL_DLLPUBLIC GroupBox : public Control
{
private:
    using Control::ImplInitSettings;
    using Window::ImplInit;
    SAL_DLLPRIVATE void     ImplInit( Window* pParent, WinBits nStyle );
    SAL_DLLPRIVATE WinBits  ImplInitStyle( WinBits nStyle );
    SAL_DLLPRIVATE void     ImplInitSettings( bool bFont, bool bForeground, bool bBackground );
    SAL_DLLPRIVATE void     ImplDraw( OutputDevice* pDev, sal_uLong nDrawFlags,
                                      const Point& rPos, const Size& rSize, bool bLayout = false );

    virtual void    FillLayoutData() const SAL_OVERRIDE;
    virtual const Font&
                    GetCanonicalFont( const StyleSettings& _rStyle ) const SAL_OVERRIDE;
    virtual const Color&
                    GetCanonicalTextColor( const StyleSettings& _rStyle ) const SAL_OVERRIDE;

public:
    explicit        GroupBox( Window* pParent, WinBits nStyle = 0 );
    explicit        GroupBox( Window* pParent, const ResId& );

    virtual void    Paint( const Rectangle& rRect ) SAL_OVERRIDE;
    virtual void    Draw( OutputDevice* pDev, const Point& rPos, const Size& rSize, sal_uLong nFlags ) SAL_OVERRIDE;
    virtual void    Resize() SAL_OVERRIDE;
    virtual void    StateChanged( StateChangedType nType ) SAL_OVERRIDE;
    virtual void    DataChanged( const DataChangedEvent& rDCEvt ) SAL_OVERRIDE;
};

#endif // INCLUDED_VCL_GROUP_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
