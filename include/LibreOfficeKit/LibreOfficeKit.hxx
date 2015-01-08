/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_DESKTOP_INC_LIBREOFFICEKIT_HXX
#define INCLUDED_DESKTOP_INC_LIBREOFFICEKIT_HXX

#include "LibreOfficeKit.h"

/*
 * The reasons this C++ code is not as pretty as it could be are:
 *  a) provide a pure C API - that's useful for some people
 *  b) allow ABI stability - C++ vtables are not good for that.
 *  c) avoid C++ types as part of the API.
 */
namespace lok
{

class Document
{
private:
    LibreOfficeKitDocument* mpDoc;

public:
    inline Document(LibreOfficeKitDocument* pDoc) :
        mpDoc(pDoc)
    {}

    inline ~Document()
    {
        mpDoc->pClass->destroy(mpDoc);
    }

    inline bool saveAs(const char* pUrl, const char* pFormat = NULL, const char* pFilterOptions = NULL)
    {
        return mpDoc->pClass->saveAs(mpDoc, pUrl, pFormat, pFilterOptions) != 0;
    }

    inline LibreOfficeKitDocument *get() { return mpDoc; }

#ifdef LOK_USE_UNSTABLE_API
    inline LibreOfficeKitDocumentType getDocumentType()
    {
        return mpDoc->pClass->getDocumentType(mpDoc);
    }

    inline int getParts()
    {
        return mpDoc->pClass->getParts(mpDoc);
    }

    inline int getPart()
    {
        return mpDoc->pClass->getPart(mpDoc);
    }

    inline void setPart(int nPart)
    {
        mpDoc->pClass->setPart(mpDoc, nPart);
    }

    inline char* getPartName(int nPart)
    {
        return mpDoc->pClass->getPartName(mpDoc, nPart);
    }

    inline void paintTile(
                          unsigned char* pBuffer,
                          const int nCanvasWidth,
                          const int nCanvasHeight,
                          int* pRowStride,
                          const int nTilePosX,
                          const int nTilePosY,
                          const int nTileWidth,
                          const int nTileHeight)
    {
        return mpDoc->pClass->paintTile(mpDoc, pBuffer, nCanvasWidth, nCanvasHeight, pRowStride,
                                nTilePosX, nTilePosY, nTileWidth, nTileHeight);
    }

    inline void getDocumentSize(long* pWidth, long* pHeight)
    {
        mpDoc->pClass->getDocumentSize(mpDoc, pWidth, pHeight);
    }

    inline void initializeForRendering()
    {
        mpDoc->pClass->initializeForRendering(mpDoc);
    }

    /**
     * Registers a callback. LOK will invoke this function when it wants to
     * inform the client about events.
     *
     * @param pCallback the callback to invoke
     * @param pData the user data, will be passed to the callback on invocation
     */
    inline void registerCallback(LibreOfficeKitCallback pCallback, void* pData)
    {
        mpDoc->pClass->registerCallback(mpDoc, pCallback, pData);
    }
#endif // LOK_USE_UNSTABLE_API
};

class Office
{
private:
    LibreOfficeKit* mpThis;

public:
    inline Office(LibreOfficeKit* pThis) :
        mpThis(pThis)
    {}

    inline ~Office()
    {
        mpThis->pClass->destroy(mpThis);
    }

    inline Document* documentLoad(const char* pUrl)
    {
        LibreOfficeKitDocument* pDoc = mpThis->pClass->documentLoad(mpThis, pUrl);
        if (pDoc == NULL)
            return NULL;
        return new Document(pDoc);
    }

    // return the last error as a string, free me.
    inline char* getError()
    {
        return mpThis->pClass->getError(mpThis);
    }

    /**
     * Posts a keyboard event to the focused frame.
     *
     * @param nType Event type, like press or release.
     * @param nCode Code of the key.
     */
    inline void postKeyEvent(int nType, int nCode)
    {
        mpThis->pClass->postKeyEvent(mpThis, nType, nCode);
    }
};

inline Office* lok_cpp_init(const char* pInstallPath)
{
    LibreOfficeKit* pThis = lok_init(pInstallPath);
    if (pThis == NULL || pThis->pClass->nSize == 0)
        return NULL;
    return new ::lok::Office(pThis);
}

}
#endif
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
