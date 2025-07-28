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

#include <memory>
#include <tools/color.hxx>
#include <vcl/GraphicObject.hxx>
#include <svx/svxdllapi.h>

class SVXCORE_DLLPUBLIC XOBitmap
{
private:
    std::unique_ptr<GraphicObject> m_xGraphicObject;
    std::unique_ptr<sal_uInt16[]>  m_pPixelArray;
    Color           m_aPixelColor;
    Color           m_aBckgrColor;
    bool            m_bGraphicDirty;

    const GraphicObject& GetGraphicObject() const;

    XOBitmap(const XOBitmap& rXBmp) = delete;
    XOBitmap& operator=(const XOBitmap& rXOBitmap) = delete;


public:
    XOBitmap( const BitmapEx& rBitmap );
    ~XOBitmap();

    void Bitmap2Array();
    void Array2Bitmap();

    void SetPixelColor( const Color& rColor )           { m_aPixelColor = rColor; m_bGraphicDirty = true; }
    void SetBackgroundColor( const Color& rColor )      { m_aBckgrColor = rColor; m_bGraphicDirty = true; }

    BitmapEx                GetBitmap() const;
    const Color&            GetBackgroundColor() const  { return m_aBckgrColor; }
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
