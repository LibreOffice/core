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

#ifndef INCLUDED_SVX_XBITMAP_HXX
#define INCLUDED_SVX_XBITMAP_HXX

#include <tools/color.hxx>
#include <vcl/bitmap.hxx>
#include <svtools/grfmgr.hxx>
#include <svx/xenum.hxx>
#include <svx/svxdllapi.h>

class SVX_DLLPUBLIC XOBitmap
{
private:
    XBitmapType     eType;
    GraphicObject   aGraphicObject;
    sal_uInt16*     pPixelArray;
    Size            aArraySize;
    Color           aPixelColor;
    Color           aBckgrColor;
    bool            bGraphicDirty;

    const GraphicObject& GetGraphicObject() const;

public:
    XOBitmap( const Bitmap& rBitmap );
    XOBitmap( const XOBitmap& rXBmp );
    ~XOBitmap();

    XOBitmap& operator=( const XOBitmap& rXOBitmap );
    int      operator==( const XOBitmap& rXOBitmap ) const;

    void Bitmap2Array();
    void Array2Bitmap();

    void SetBitmapType( XBitmapType eNewType )          { eType = eNewType; }
    void SetPixelColor( const Color& rColor )           { aPixelColor = rColor; bGraphicDirty = true; }
    void SetPixelSize( const Size& rSize )              { aArraySize  = rSize;  bGraphicDirty = true; }
    void SetBackgroundColor( const Color& rColor )      { aBckgrColor = rColor; bGraphicDirty = true; }

    XBitmapType             GetBitmapType() const       { return eType; }
    Bitmap                  GetBitmap() const;
    Color                   GetPixelColor() const       { return aPixelColor; }
    Color                   GetBackgroundColor() const  { return aBckgrColor; }
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
