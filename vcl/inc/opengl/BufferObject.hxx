/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#ifndef INCLUDED_VCL_INC_OPENGL_BUFFEROBJECT_H
#define INCLUDED_VCL_INC_OPENGL_BUFFEROBJECT_H

namespace vcl
{
template <typename TYPE, GLenum BUFFER_TYPE> class BufferObject
{
private:
    GLuint mId;

public:
    BufferObject()
        : mId(0)
    {
        glGenBuffers(1, &mId);
        CHECK_GL_ERROR();
    }

    virtual ~BufferObject()
    {
        if (mId)
        {
            glDeleteBuffers(1, &mId);
            CHECK_GL_ERROR();
            mId = 0;
        }
    }

    void bind()
    {
        if (mId)
        {
            glBindBuffer(BUFFER_TYPE, mId);
            CHECK_GL_ERROR();
        }
    }

    void unbind()
    {
        if (mId)
        {
            glBindBuffer(BUFFER_TYPE, 0);
            CHECK_GL_ERROR();
        }
    }

    void upload(const std::vector<TYPE>& rData)
    {
        if (mId)
        {
            bind();
            glBufferData(BUFFER_TYPE, sizeof(TYPE) * rData.size(), rData.data(), GL_STATIC_DRAW);
            CHECK_GL_ERROR();
        }
    }
};

template <typename TYPE> class VertexBufferObject final : public BufferObject<TYPE, GL_ARRAY_BUFFER>
{
};

class IndexBufferObject final : public BufferObject<GLuint, GL_ELEMENT_ARRAY_BUFFER>
{
};

} // end vcl

#endif // INCLUDED_VCL_INC_OPENGL_BUFFEROBJECT_H

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
