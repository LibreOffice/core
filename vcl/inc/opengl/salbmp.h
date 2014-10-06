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

#ifndef INCLUDED_VCL_INC_OPENGL_SALBMP_H
#define INCLUDED_VCL_INC_OPENGL_SALBMP_H

#include "tools/gen.hxx"

#include "basebmp/bitmapdevice.hxx"

#include "vcl/salbtype.hxx"

#include "headless/svpgdi.hxx"

#include "salinst.hxx"
#include "salvd.hxx"
#include "salbmp.hxx"

// - SalBitmap  -

struct  BitmapBuffer;
class   BitmapPalette;

class OpenGLSalBitmap : public SalBitmap
{
public:
    OpenGLSalBitmap() {}
    virtual ~OpenGLSalBitmap() {}

public:

    // SalBitmap methods
    bool            Create( const Size& /* rSize*/, sal_uInt16 /* nBitCount */, const BitmapPalette& /* rPal */ ) SAL_OVERRIDE
    {
        return false;
    }
    bool            Create( const SalBitmap& /* rSalBmp */ ) SAL_OVERRIDE
    {
        return false;
    }
    bool            Create( const SalBitmap& /* rSalBmp */, SalGraphics* /* pGraphics */ ) SAL_OVERRIDE
    {
        return false;
    }
    bool            Create( const SalBitmap& /* rSalBmp */, sal_uInt16 /* nNewBitCount */ ) SAL_OVERRIDE
    {
        return false;
    }
    virtual bool    Create( const ::com::sun::star::uno::Reference< ::com::sun::star::rendering::XBitmapCanvas > /* xBitmapCanvas */,
                            Size& /* rSize */,
                            bool /* bMask */ ) SAL_OVERRIDE
    {
        return false;
    }

    void            Destroy() SAL_OVERRIDE {}

    Size            GetSize() const SAL_OVERRIDE
    {
        return {};
    }
    sal_uInt16          GetBitCount() const SAL_OVERRIDE
    {
        return 0;
    }

    BitmapBuffer   *AcquireBuffer( bool /* bReadOnly */ ) SAL_OVERRIDE
    {
        return nullptr;
    }
    void            ReleaseBuffer( BitmapBuffer* /* pBuffer */, bool /* bReadOnly */ ) SAL_OVERRIDE {}

    bool            GetSystemData( BitmapSystemData& /* rData */ ) SAL_OVERRIDE
    {
        return false;
    }

};

#endif // INCLUDED_VCL_INC_OPENGL_SALBMP_H

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
