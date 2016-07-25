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

#ifndef INCLUDED_VCL_INC_OPENGL_TEXTURE_H
#define INCLUDED_VCL_INC_OPENGL_TEXTURE_H

#include <GL/glew.h>
#include <vcl/dllapi.h>
#include <vcl/salgtype.hxx>
#include <rtl/ustring.hxx>
#include <tools/gen.hxx>

#include <functional>
#include <memory>
#include <vector>

class ImplOpenGLTexture
{
    int    mnRefCount;
public:
    GLuint mnTexture;
    int    mnWidth;
    int    mnHeight;
    GLenum mnFilter;
    GLuint mnOptStencil;

    std::unique_ptr<std::vector<int>> mpSlotReferences;
    std::function<void(int)> mFunctSlotDeallocateCallback;

    ImplOpenGLTexture( int nWidth, int nHeight, bool bAllocate );
    ImplOpenGLTexture( int nWidth, int nHeight, int nFormat, int nType, void const * pData );
    ImplOpenGLTexture( int nX, int nY, int nWidth, int nHeight );
    ~ImplOpenGLTexture();
    void Dispose();

    bool InsertBuffer(int nX, int nY, int nWidth, int nHeight, int nFormat, int nType, sal_uInt8* pData);

    void IncreaseRefCount(int nSlotNumber);
    void DecreaseRefCount(int nSlotNumber);

    bool IsUnique()
    {
        return mnRefCount == 1;
    }

    bool InitializeSlotMechanism(int nInitialSlotSize);

    void SetSlotDeallocateCallback(std::function<void(int)> aCallback)
    {
        mFunctSlotDeallocateCallback = aCallback;
    }

    void ResetSlotDeallocateCallback()
    {
        mFunctSlotDeallocateCallback = std::function<void(int)>();
    }

    GLuint AddStencil();
};

class VCL_DLLPUBLIC OpenGLTexture
{
private:
    // if the rect size doesn't match the mpImpl one, this instance
    // is a sub-area from the real OpenGL texture
    Rectangle maRect;
    ImplOpenGLTexture* mpImpl;
    int mnSlotNumber;

    inline bool GetTextureRect(const SalTwoRect& rPosAry, bool bInverted, GLfloat& x1, GLfloat& x2, GLfloat& y1, GLfloat& y2) const;

    inline bool IsValid() const
    {
        return (mpImpl && mpImpl->mnTexture != 0);
    }

public:
                    OpenGLTexture();
                    OpenGLTexture(ImplOpenGLTexture* pImpl, Rectangle aRectangle, int nSlotNumber);

                    OpenGLTexture( int nWidth, int nHeight, bool bAllocate = true );
                    OpenGLTexture( int nWidth, int nHeight, int nFormat, int nType, void const * pData );
                    OpenGLTexture( int nX, int nY, int nWidth, int nHeight );
                    OpenGLTexture( const OpenGLTexture& rTexture );
                    OpenGLTexture( const OpenGLTexture& rTexture, int nX, int nY, int nWidth, int nHeight );
    virtual         ~OpenGLTexture();

    bool            IsUnique() const;

    GLuint          Id() const;
    int             GetWidth() const;
    int             GetHeight() const;

    void            GetCoord( GLfloat* pCoord, const SalTwoRect& rPosAry, bool bInverted=false ) const;
    void            GetWholeCoord( GLfloat* pCoord ) const;
    OpenGLTexture   GetWholeTexture();
    void            Bind();
    void            Unbind();
    void            Read( GLenum nFormat, GLenum nType, sal_uInt8* pData );
    GLuint          AddStencil();
    GLuint          StencilId() const;

    bool            CopyData(int nWidth, int nHeight, int nFormat, int nType, sal_uInt8* pData);

    void            SaveToFile(const OUString& rFileName);

    GLenum          GetFilter() const;
    void            SetFilter( GLenum nFilter );

                    operator bool() const;
    OpenGLTexture&  operator=( const OpenGLTexture& rTexture );
    bool            operator==( const OpenGLTexture& rTexture ) const;
    bool            operator!=( const OpenGLTexture& rTexture ) const;

    template<GLenum type>
    void FillCoords(std::vector<GLfloat>& aCoordVector, const SalTwoRect& rPosAry, bool bInverted) const;
};

template<> void OpenGLTexture::FillCoords<GL_TRIANGLES>(
    std::vector<GLfloat>& aCoord, const SalTwoRect& rPosAry, bool bInverted)
    const;

template<> void OpenGLTexture::FillCoords<GL_TRIANGLE_FAN>(
    std::vector<GLfloat>& aCoord, const SalTwoRect& rPosAry, bool bInverted)
    const;

#endif // INCLUDED_VCL_INC_OPENGL_TEXTURE_H

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
