/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#ifndef _DASHEDLINE_HXX
#define _DASHEDLINE_HXX

#include <basegfx/color/bcolor.hxx>
#include <vcl/fixed.hxx>

/** Class for displaying a dashed line in the Writer GUI.
  */
class SwDashedLine : public FixedLine
{
    Color& (*m_pColorFn)();

public:
    SwDashedLine( Window* pParent, Color& ( *pColorFn )() );
    ~SwDashedLine( );

    virtual void Paint( const Rectangle& rRect );
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
