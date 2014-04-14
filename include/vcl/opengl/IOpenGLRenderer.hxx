/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef VCL_IOPENGLRENDER_HXX
#define VCL_IOPENGLRENDER_HXX

class IOpenGLInfoProvider
{
public:
    virtual ~IOpenGLInfoProvider() {}

    virtual bool isOpenGLInitialized() = 0;
};

class IOpenGLRenderer
{
public:
    IOpenGLRenderer():
        mpInfoProvider(NULL) {}
    virtual ~IOpenGLRenderer() {}

    bool isOpenGLInitialized()
    {
        if(mpInfoProvider)
            return mpInfoProvider->isOpenGLInitialized();

        return false;
    }

    void setInfoProvider(IOpenGLInfoProvider* pInfo)
    {
        mpInfoProvider = pInfo;
    }

private:
    IOpenGLInfoProvider* mpInfoProvider;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
