/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * Copyright the Collabora Online contributors.
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

/*
 * Dummy COKit implementation for testing.
 * Classes: LibLODocument_Impl, LibCOKit_Impl - Mock COKit interfaces
 */

#include <config.h>

#include "DummyCOKit.hpp"

#include <COKit/COKit.hxx>

#include <cstdlib>
#include <cstring>
#include <memory>

struct LibLODocument_Impl : public _COKitDocument
{
private:
    std::shared_ptr< COKitDocumentClass > m_pDocumentClass;

public:
    LibLODocument_Impl();
};

struct LibCOKit_Impl : public _COKit
{
private:
    std::shared_ptr< COKitClass > m_pOfficeClass;

public:
    LibCOKit_Impl();
};

static LibCOKit_Impl *gImpl = nullptr;
static std::weak_ptr< COKitClass > gOfficeClass;
static std::weak_ptr< COKitDocumentClass > gDocumentClass;

extern "C"
{

static void doc_destroy(COKitDocument* self);
static int doc_saveAs(COKitDocument* self, const char* url, const char* format, const char* filterOptions);
static int doc_getDocumentType(COKitDocument* self);
static int doc_getParts(COKitDocument* self);
static char* doc_getPartPageRectangles(COKitDocument* self);
static int doc_getPart(COKitDocument* self);
static void doc_setPart(COKitDocument* self, int part);
static char* doc_getPartName(COKitDocument* self, int part);
static void doc_setPartMode(COKitDocument* self, COKitPartMode partMode);
static int doc_getEditMode(COKitDocument* self);
static void doc_paintTile(COKitDocument* self,
                          unsigned char* buffer,
                          const int canvasWidth, const int canvasHeight,
                          const int tilePosX, const int tilePosY,
                          const int tileWidth, const int tileHeight);
static void doc_paintPartTile(COKitDocument* self,
                              unsigned char* buffer,
                              const int part,
                              const int canvasWidth, const int canvasHeight,
                              const int tilePosX, const int tilePosY,
                              const int tileWidth, const int tileHeight);
static COKitTileMode doc_getTileMode(COKitDocument* self);
static void doc_getDocumentSize(COKitDocument* self,
                                long* width,
                                long* height);
static void doc_getDataArea(COKitDocument* self,
                            long part,
                            long* col,
                            long* row);
static void doc_initializeForRendering(COKitDocument* self,
                                       const char* arguments);

static void doc_registerCallback(COKitDocument* self,
                                COKitCallback callback,
                                void* data);
static void doc_postKeyEvent(COKitDocument* self,
                             int type,
                             int charCode,
                             int keyCode);
static void doc_postMouseEvent (COKitDocument* self,
                                int type,
                                int x,
                                int y,
                                int count,
                                int buttons,
                                int modifier);
static void doc_postUnoCommand(COKitDocument* self,
                               const char* command,
                               const char* arguments,
                               bool notifyWhenFinished);
static void doc_setTextSelection (COKitDocument* self,
                                  COKitSetTextSelectionType type,
                                  int x,
                                  int y);
static char* doc_getTextSelection(COKitDocument* self,
                                  const char* mimeType,
                                  char** usedMimeType);
static bool doc_paste(COKitDocument* self,
                      const char* mimeType,
                      const char* data,
                      size_t size);
static void doc_setGraphicSelection (COKitDocument* self,
                                  COKitSetGraphicSelectionType type,
                                  int x,
                                  int y);
static void doc_resetSelection (COKitDocument* self);
static char* doc_getCommandValues(COKitDocument* self, const char* command);
static void doc_setClientZoom(COKitDocument* self,
                                    int tilePixelWidth,
                                    int tilePixelHeight,
                                    int tileTwipWidth,
                                    int tileTwipHeight);
static void doc_setClientVisibleArea(COKitDocument* self, int x, int y, int width, int height);
static void doc_setOutlineState(COKitDocument* self, bool column, int level, int index, bool hidden);
static int doc_createView(COKitDocument* self);
static void doc_destroyView(COKitDocument* self, int id);
static void doc_setView(COKitDocument* self, int id);
static int doc_getView(COKitDocument* self);
static int doc_getViewsCount(COKitDocument* self);
static bool doc_getViewIds(COKitDocument* self, int* array, size_t size);
static unsigned char* doc_renderFontOrientation(COKitDocument* self,
                          const char *fontName,
                          const char *character,
                          int* fontWidth,
                          int* fontHeight,
                          int orientation);
static char* doc_getPartHash(COKitDocument* self, int part);

static size_t doc_renderShapeSelection(COKitDocument* self, char** output);

LibLODocument_Impl::LibLODocument_Impl()
{
    if (!(m_pDocumentClass = gDocumentClass.lock()))
    {
        m_pDocumentClass = std::make_shared<COKitDocumentClass>();

        m_pDocumentClass->size = sizeof(COKitDocument);

        m_pDocumentClass->destroy = doc_destroy;
        m_pDocumentClass->saveAs = doc_saveAs;
        m_pDocumentClass->getDocumentType = doc_getDocumentType;
        m_pDocumentClass->getParts = doc_getParts;
        m_pDocumentClass->getPartPageRectangles = doc_getPartPageRectangles;
        m_pDocumentClass->getPart = doc_getPart;
        m_pDocumentClass->setPart = doc_setPart;
        m_pDocumentClass->getPartName = doc_getPartName;
        m_pDocumentClass->setPartMode = doc_setPartMode;
        m_pDocumentClass->getEditMode = doc_getEditMode;
        m_pDocumentClass->paintTile = doc_paintTile;
        m_pDocumentClass->paintPartTile = doc_paintPartTile;
        m_pDocumentClass->getTileMode = doc_getTileMode;
        m_pDocumentClass->getDocumentSize = doc_getDocumentSize;
        m_pDocumentClass->getDataArea = doc_getDataArea;
        m_pDocumentClass->initializeForRendering = doc_initializeForRendering;
        m_pDocumentClass->registerCallback = doc_registerCallback;
        m_pDocumentClass->postKeyEvent = doc_postKeyEvent;
        m_pDocumentClass->postMouseEvent = doc_postMouseEvent;
        m_pDocumentClass->postUnoCommand = doc_postUnoCommand;
        m_pDocumentClass->setTextSelection = doc_setTextSelection;
        m_pDocumentClass->getTextSelection = doc_getTextSelection;
        m_pDocumentClass->paste = doc_paste;
        m_pDocumentClass->setGraphicSelection = doc_setGraphicSelection;
        m_pDocumentClass->resetSelection = doc_resetSelection;
        m_pDocumentClass->getCommandValues = doc_getCommandValues;
        m_pDocumentClass->setClientZoom = doc_setClientZoom;
        m_pDocumentClass->setClientVisibleArea = doc_setClientVisibleArea;
        m_pDocumentClass->setOutlineState = doc_setOutlineState;

        m_pDocumentClass->createView = doc_createView;
        m_pDocumentClass->destroyView = doc_destroyView;
        m_pDocumentClass->setView = doc_setView;
        m_pDocumentClass->getView = doc_getView;
        m_pDocumentClass->getViewsCount = doc_getViewsCount;
        m_pDocumentClass->getViewIds = doc_getViewIds;

        m_pDocumentClass->renderFontOrientation = doc_renderFontOrientation;
        m_pDocumentClass->getPartHash = doc_getPartHash;

        m_pDocumentClass->renderShapeSelection = doc_renderShapeSelection;

        gDocumentClass = m_pDocumentClass;
    }
    classPointer = m_pDocumentClass.get();
}

static void                    lo_destroy       (COKit* self);
static COKitDocument* lo_documentLoad  (COKit* self, const char* url);
static char *                  lo_getError      (COKit* self);
static void                    lo_freeError     (char* freePointer);
static COKitDocument* lo_documentLoadWithOptions  (COKit* self,
                                                           const char* url,
                                                           const char* options);
static void                    lo_registerCallback (COKit* self,
                                                    COKitCallback callback,
                                                    void* data);
static char* lo_getFilterTypes(COKit* self);
static void lo_setOptionalFeatures(COKit* self, unsigned long long features);
static void                    lo_setDocumentPassword(COKit* self,
                                                       const char* url,
                                                       const char* password);
static char*                   lo_getVersionInfo(COKit* self);

LibCOKit_Impl::LibCOKit_Impl()
{
    if(!m_pOfficeClass) {
        m_pOfficeClass.reset(new COKitClass);
        m_pOfficeClass->size = sizeof(COKitClass);

        m_pOfficeClass->destroy = lo_destroy;
        m_pOfficeClass->documentLoad = lo_documentLoad;
        m_pOfficeClass->getError = lo_getError;
        m_pOfficeClass->freeError = lo_freeError;
        m_pOfficeClass->documentLoadWithOptions = lo_documentLoadWithOptions;
        m_pOfficeClass->registerCallback = lo_registerCallback;
        m_pOfficeClass->getFilterTypes = lo_getFilterTypes;
        m_pOfficeClass->setOptionalFeatures = lo_setOptionalFeatures;
        m_pOfficeClass->setDocumentPassword = lo_setDocumentPassword;
        m_pOfficeClass->getVersionInfo = lo_getVersionInfo;

        gOfficeClass = m_pOfficeClass;
    }

    classPointer = m_pOfficeClass.get();
}

static COKitDocument* lo_documentLoad(COKit* self, const char* url)
{
    return lo_documentLoadWithOptions(self, url, nullptr);
}

static COKitDocument* lo_documentLoadWithOptions(COKit* self, const char* url, const char* options)
{
    (void) self;
    (void) url;
    (void) options;

    return new LibLODocument_Impl();
}

static void lo_registerCallback (COKit* self,
                                 COKitCallback callback,
                                 void* data)
{
    (void) self;
    (void) callback;
    (void) data;
}

static int doc_saveAs(COKitDocument* self, const char* url, const char* format, const char* filterOptions)
{
    (void) self;
    (void) url;
    (void) format;
    (void) filterOptions;

    return true;
}

static int doc_getDocumentType (COKitDocument* self)
{
    (void) self;
    return KIT_DOCTYPE_TEXT;
}

static int doc_getParts (COKitDocument* self)
{
    (void) self;
    return 1;
}

static int doc_getPart (COKitDocument* self)
{
    (void) self;
    return 0;
}

static void doc_setPart(COKitDocument* self, int part)
{
    (void) self;
    (void) part;
}

static char* doc_getPartPageRectangles(COKitDocument* self)
{
    (void) self;
    return nullptr;
}

static char* doc_getPartName(COKitDocument* self, int part)
{
    (void) self;
    (void) part;

    char* memory = strdup("Dummy part");
    return memory;

}

static char* doc_getPartHash(COKitDocument* self, int part)
{
    (void) self;
    (void) part;
    return nullptr;
}

static void doc_setPartMode(COKitDocument* self,
                            COKitPartMode partMode)
{
    (void) self;
    (void) partMode;
}

static int doc_getEditMode(COKitDocument* self)
{
    (void) self;
}

static void doc_paintTile(COKitDocument* self,
                          unsigned char* buffer,
                          const int canvasWidth, const int canvasHeight,
                          const int tilePosX, const int tilePosY,
                          const int tileWidth, const int tileHeight)
{
    (void) self;
    (void) buffer;
    (void) canvasWidth;
    (void) canvasHeight;
    (void) tilePosX;
    (void) tilePosY;
    (void) tileWidth;
    (void) tileHeight;

    // TODO maybe at least clean the buffer?
}


static void doc_paintPartTile(COKitDocument* self,
                              unsigned char* buffer,
                              const int part,
                              const int canvasWidth, const int canvasHeight,
                              const int tilePosX, const int tilePosY,
                              const int tileWidth, const int tileHeight)
{
    (void) part;

    doc_paintTile(self, buffer, canvasWidth, canvasHeight, tilePosX, tilePosY, tileWidth, tileHeight);
}

static COKitTileMode doc_getTileMode(COKitDocument* /*self*/)
{
    return COKitTileMode::RGBA;
}

static void doc_getDocumentSize(COKitDocument* self,
                                long* width,
                                long* height)
{
    (void) self;
    // TODO better values here maybe?
    *width = 10000;
    *height = 10000;
}

static void doc_getDataArea(COKitDocument* self,
                            long part,
                            long* col,
                            long* row)
{
    (void) self;
    (void) part;

    *col = 999;
    *row = 999;
}

static void doc_initializeForRendering(COKitDocument* self,
                                       const char* arguments)
{
    (void) self;
    (void) arguments;
}

static void doc_registerCallback(COKitDocument* self,
                                 COKitCallback callback,
                                 void* data)
{
    (void) self;
    (void) callback;
    (void) data;
}

static void doc_postKeyEvent(COKitDocument* self, int type, int charCode, int keyCode)
{
    (void) self;
    (void) type;
    (void) charCode;
    (void) keyCode;
}

static void doc_postUnoCommand(COKitDocument* self, const char* command, const char* arguments, bool notifyWhenFinished)
{
    (void) self;
    (void) command;
    (void) arguments;
    (void) notifyWhenFinished;
}

static void doc_postMouseEvent(COKitDocument* self, int type, int x, int y, int count, int buttons, int modifier)
{
    (void) self;
    (void) type;
    (void) x;
    (void) y;
    (void) count;
    (void) buttons;
    (void) modifier;
}

static void doc_setTextSelection(COKitDocument* self, COKitSetTextSelectionType type, int x, int y)
{
    (void) self;
    (void) type;
    (void) x;
    (void) y;
}

static char* doc_getTextSelection(COKitDocument* self, const char* mimeType, char** usedMimeType)
{
    (void) self;
    (void) mimeType;
    (void) usedMimeType;

    char* memory = strdup("Dummy text");

    if (usedMimeType)
    {
        *usedMimeType = strdup("text/plain;charset=utf-8");
    }

    return memory;
}

static bool doc_paste(COKitDocument* self, const char* mimeType, const char* data, size_t size)
{
    (void) self;
    (void) mimeType;
    (void) data;
    (void) size;

    return true;
}

static void doc_setGraphicSelection(COKitDocument* self, COKitSetGraphicSelectionType type,
                                    int x, int y)
{
    (void) self;
    (void) type;
    (void) x;
    (void) y;
}

static void doc_resetSelection(COKitDocument* self)
{
    (void) self;
}

static char* doc_getCommandValues(COKitDocument* self, const char* command)
{
    (void) self;
    (void) command;

    char* memory = strdup("");
    return memory;
}

static void doc_setClientZoom(COKitDocument* self, int tilePixelWidth, int tilePixelHeight,
        int tileTwipWidth, int tileTwipHeight)
{
    (void) self;
    (void) tilePixelWidth;
    (void) tilePixelHeight;
    (void) tileTwipWidth;
    (void) tileTwipHeight;
}

static void doc_setClientVisibleArea(COKitDocument* self, int x, int y, int width, int height)
{
    (void) self;
    (void) x;
    (void) y;
    (void) width;
    (void) height;
}

static void doc_setOutlineState(COKitDocument* self, bool column, int level, int index, bool hidden)
{
    (void) self;
    (void) column;
    (void) level;
    (void) index;
    (void) hidden;
}

static int doc_createView(COKitDocument* /*self*/)
{
    return 1;
}

static void doc_destroyView(COKitDocument* /*self*/, int id)
{
    (void) id;
}

static void doc_setView(COKitDocument* /*self*/, int id)
{
    (void) id;
}

static int doc_getView(COKitDocument* /*self*/)
{
    return 1;
}

static int doc_getViewsCount(COKitDocument* /*self*/)
{
    return 1;
}

static bool doc_getViewIds(COKitDocument* /*self*/, int* array, size_t size)
{
    (void) array;
    (void) size;

    // TODO Should we return something here?
    return true;
}

unsigned char* doc_renderFontOrientation(COKitDocument* /*self*/,
                    const char* fontName,
                    const char* character,
                    int* fontWidth,
                    int* fontHeight,
                    int orientation)
{
    (void) fontName;
    (void) character;
    (void) fontWidth;
    (void) fontHeight;
    (void) orientation;

    return nullptr;
}

static size_t doc_renderShapeSelection(COKitDocument* self, char** output)
{
    (void) self;
    (void) output;
    return 0;
}

static char* lo_getError (COKit *self)
{
    (void) self;

    char* memory = strdup("Dummy error");
    return memory;
}

static void lo_freeError(char* freePointer)
{
    free(freePointer);
}

static char* lo_getFilterTypes(COKit* self)
{
    (void) self;

    // TODO aything more here?
    return nullptr;
}

static void lo_setOptionalFeatures(COKit* self, unsigned long long const features)
{
    (void) self;
    (void) features;
}

static void lo_setDocumentPassword(COKit* self,
        const char* url, const char* password)
{
    (void) self;
    (void) url;
    (void) password;
}

static char* lo_getVersionInfo(COKit* /*self*/)
{
    const char version[] =
        "{ "
        "\"ProductName\": \"Dummy\", "
        "\"ProductVersion\": \"5.3\", "
        "\"ProductExtension\": \"Dummy\", "
        "\"BuildId\": \"1\" "
        "}";

    char* version = strdup(version);
    return version;
}

COKit* dummy_cok_init_2(const char *install_path,  const char *user_profile_url)
{
    (void) install_path;
    (void) user_profile_url;

    if (!gImpl)
    {
        gImpl = new LibCOKit_Impl();
    }
    return static_cast<COKit*>(gImpl);
}

static void doc_destroy(COKitDocument *self)
{
    LibLODocument_Impl *document = static_cast<LibLODocument_Impl*>(self);
    delete document;
}

static void lo_destroy(COKit* self)
{
    LibCOKit_Impl* lib = static_cast<LibCOKit_Impl*>(self);
    gImpl = nullptr;

    delete lib;
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
