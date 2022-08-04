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

#include <svtools/scrolladaptor.hxx>

class SwScrollbar final : public ScrollAdaptor
{
    Size    m_aDocSz;
    bool    m_bAuto       :1;     // for scrolling mode
    bool    m_bVisible    :1;     // show/hide should only set this flag
    bool    m_bSizeSet    :1;     // was the size already set?

    void    AutoShow();

public:
    void    ExtendedShow( bool bVisible = true );
    void    SetPosSizePixel( const Point& rNewPos, const Size& rNewSize ) override;
    bool    IsScrollbarVisible(bool bReal) const { return bReal ? ScrollAdaptor::IsVisible() : m_bVisible; }

        // changing of document size
    void    DocSzChgd(const Size &rNewSize);
        // changing of visible region
    void    ViewPortChgd(const tools::Rectangle &rRectangle);

    void    SetAuto(bool bSet);
    bool    IsAuto() const { return m_bAuto;}

    SwScrollbar(vcl::Window *pParent, bool bHori );
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
