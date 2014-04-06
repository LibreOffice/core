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

#ifndef SC_CBUTTON_HXX
#define SC_CBUTTON_HXX

#include <tools/gen.hxx>
#include <tools/color.hxx>

class OutputDevice;


class ScDDComboBoxButton
{
public:
            ScDDComboBoxButton( OutputDevice* pOutputDevice );
            ~ScDDComboBoxButton();

    void    SetOutputDevice( OutputDevice* pOutputDevice );

    void    Draw( const Point&  rAt,
                  const Size&   rSize,
                  bool          bState,
                  bool          bBtnIn = false );

    void    Draw( const Point&  rAt,
                  bool          bState,
                  bool          bBtnIn = false )
                { Draw( rAt, aBtnSize, bState, bBtnIn ); }

    void    Draw( bool          bState,
                  bool          bBtnIn = false )
                { Draw( aBtnPos, aBtnSize, bState, bBtnIn ); }

    void    SetOptSizePixel();

    void    SetPosPixel( const Point& rNewPos )  { aBtnPos = rNewPos; }
    Point   GetPosPixel() const                  { return aBtnPos; }

    void    SetSizePixel( const Size& rNewSize ) { aBtnSize = rNewSize; }
    Size    GetSizePixel() const                 { return aBtnSize; }

private:
    void    ImpDrawArrow( const Rectangle&  rRect,
                          bool              bState );

protected:
    OutputDevice* pOut;
    Point   aBtnPos;
    Size    aBtnSize;
};


#endif // SC_CBUTTON_HXX


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
