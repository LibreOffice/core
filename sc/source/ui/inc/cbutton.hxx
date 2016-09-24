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

#ifndef INCLUDED_SC_SOURCE_UI_INC_CBUTTON_HXX
#define INCLUDED_SC_SOURCE_UI_INC_CBUTTON_HXX

#include <tools/gen.hxx>
#include <tools/color.hxx>
#include <vcl/vclptr.hxx>

class OutputDevice;

class ScDDComboBoxButton
{
public:
            ScDDComboBoxButton( OutputDevice* pOutputDevice );
            ~ScDDComboBoxButton();

    void    SetOutputDevice( OutputDevice* pOutputDevice );

    void    Draw( const Point&  rAt,
                  const Size&   rSize );
    void    Draw()
                { Draw( aBtnPos, aBtnSize ); }

    void    SetOptSizePixel();

    void    SetPosPixel( const Point& rNewPos )  { aBtnPos = rNewPos; }
    const Point& GetPosPixel() const             { return aBtnPos; }

    void    SetSizePixel( const Size& rNewSize ) { aBtnSize = rNewSize; }
    const Size&  GetSizePixel() const            { return aBtnSize; }

private:
    void    ImpDrawArrow( const Rectangle&  rRect );

protected:
    VclPtr<OutputDevice> pOut;
    Point   aBtnPos;
    Size    aBtnSize;
};

#endif // INCLUDED_SC_SOURCE_UI_INC_CBUTTON_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
