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

#include <basebmp/bitmapdevice.hxx>
#include <vcl/opengl/OpenGLContext.hxx>

#include "vcl/salbtype.hxx"
#include "opengl/bmpop.hxx"
#include "opengl/texture.hxx"

#include <salbmp.hxx>

#include <deque>

// - SalBitmap  -

struct  BitmapBuffer;
class   BitmapPalette;

class VCL_PLUGIN_PUBLIC OpenGLSalBitmap : public SalBitmap
{
private:
    OpenGLContext*                      mpContext;
    OpenGLTexture                       maTexture;
    bool                                mbDirtyTexture;
    BitmapPalette                       maPalette;
    basebmp::RawMemorySharedArray       maUserBuffer;
    sal_uInt16                          mnBits;
    sal_uInt16                          mnBytesPerRow;
    int                                 mnWidth;
    int                                 mnHeight;
    int                                 mnBufWidth;
    int                                 mnBufHeight;
    std::deque< OpenGLSalBitmapOp* >    maPendingOps;

    void makeCurrent();
    virtual void updateChecksum() const;

    bool calcChecksumGL(OpenGLTexture& rHashTexture, ChecksumType& rHash) const;

public:
    OpenGLSalBitmap();
    virtual ~OpenGLSalBitmap();

public:

    // SalBitmap methods
    bool            Create( const Size& rSize, sal_uInt16 nBitCount, const BitmapPalette& rPal ) SAL_OVERRIDE;
    bool            Create( const SalBitmap& rSalBmp ) SAL_OVERRIDE;
    bool            Create( const SalBitmap& rSalBmp, SalGraphics* pGraphics ) SAL_OVERRIDE;
    bool            Create( const SalBitmap& rSalBmp, sal_uInt16 nNewBitCount ) SAL_OVERRIDE;
    virtual bool    Create( const ::com::sun::star::uno::Reference< ::com::sun::star::rendering::XBitmapCanvas >& rBitmapCanvas,
                            Size& rSize,
                            bool bMask = false ) SAL_OVERRIDE;

    void            Destroy() SAL_OVERRIDE;

    Size            GetSize() const SAL_OVERRIDE;
    sal_uInt16      GetBitCount() const SAL_OVERRIDE;

    BitmapBuffer   *AcquireBuffer( BitmapAccessMode nMode ) SAL_OVERRIDE;
    void            ReleaseBuffer( BitmapBuffer* pBuffer, BitmapAccessMode nMode ) SAL_OVERRIDE;

    bool            GetSystemData( BitmapSystemData& rData ) SAL_OVERRIDE;

    bool            Crop( const Rectangle& rRectPixel ) SAL_OVERRIDE;
    bool            Erase( const Color& rFillColor ) SAL_OVERRIDE;
    bool            Scale( const double& rScaleX, const double& rScaleY, BmpScaleFlag nScaleFlag ) SAL_OVERRIDE;
    bool            Replace( const Color& rSearchColor, const Color& rReplaceColor, sal_uLong nTol ) SAL_OVERRIDE;

public:

    bool            Create( const OpenGLTexture& rTex, long nX, long nY, long nWidth, long nHeight );
    OpenGLTexture&  GetTexture() const;
    static OpenGLContext* GetBitmapContext();

private:

    void            ExecuteOperations();
    GLuint          CreateTexture();
    void            DeleteTexture();
    void            DrawTexture( GLuint nTexture, const SalTwoRect& rPosAry );
    bool            AllocateUserData();
    bool            ReadTexture();

private:

    bool ImplScaleFilter( const double& rScaleX, const double& rScaleY, GLenum nFilter );
    static void ImplCreateKernel( const double& fScale, const Kernel& rKernel, GLfloat*& pWeights, sal_uInt32& aKernelSize );
    bool ImplScaleConvolution( const double& rScaleX, const double& rScaleY, const Kernel& aKernel );
    bool ImplScaleArea( double rScaleX, double rScaleY );

    bool getFormatAndType(GLenum& nFormat, GLenum& nType);

public:

    bool ImplScale( const double& rScaleX, const double& rScaleY, BmpScaleFlag nScaleFlag );
};

#endif // INCLUDED_VCL_INC_OPENGL_SALBMP_H

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
