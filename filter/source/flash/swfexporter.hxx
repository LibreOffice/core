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
#ifndef INCLUDED_FILTER_SOURCE_FLASH_SWFEXPORTER_HXX
#define INCLUDED_FILTER_SOURCE_FLASH_SWFEXPORTER_HXX

#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/task/XStatusIndicator.hpp>
#include <com/sun/star/drawing/XDrawPage.hpp>
#include <com/sun/star/drawing/XGraphicExportFilter.hpp>
#include <com/sun/star/io/XOutputStream.hpp>
#include <vcl/checksum.hxx>

#include <map>
#include <memory>

typedef ::std::map<BitmapChecksum, sal_uInt16> ChecksumCache;

class GDIMetaFile;

namespace swf
{
class Writer;

class ShapeInfo
{
public:
    sal_uInt16 mnID; // the character id for the sprite definition of this shape

    sal_Int32 mnX;
    sal_Int32 mnY;

    sal_Int32 mnWidth;
    sal_Int32 mnHeight;

    ShapeInfo()
        : mnID(0)
        , mnX(0)
        , mnY(0)
        , mnWidth(0)
        , mnHeight(0)
    {
    }
};

struct PageInfo
{
    sal_uInt16 mnBackgroundID;
    sal_uInt16 mnObjectsID;
    sal_uInt16 mnForegroundID;

    PageInfo();
};

class FlashExporter
{
public:
    FlashExporter(const css::uno::Reference<css::uno::XComponentContext>& rxContext,

                  // #i56084# variables for selection export
                  const css::uno::Reference<css::drawing::XShapes>& rxSelectedShapes,
                  const css::uno::Reference<css::drawing::XDrawPage>& rxSelectedDrawPage,

                  sal_Int32 nJPEGCompressMode, bool bExportOLEAsJPEG);
    ~FlashExporter();

    void Flush();

    bool exportAll(const css::uno::Reference<css::lang::XComponent>& xDoc,
                   css::uno::Reference<css::io::XOutputStream> const& xOutputStream,
                   css::uno::Reference<css::task::XStatusIndicator> const& xStatusIndicator);
    bool exportSlides(const css::uno::Reference<css::drawing::XDrawPage>& xDrawPage,
                      css::uno::Reference<css::io::XOutputStream> const& xOutputStream);
    sal_uInt16 exportBackgrounds(const css::uno::Reference<css::drawing::XDrawPage>& xDrawPage,
                                 css::uno::Reference<css::io::XOutputStream> const& xOutputStream,
                                 sal_uInt16 nPage, bool bExportObjects);
    sal_uInt16 exportBackgrounds(css::uno::Reference<css::drawing::XDrawPage> const& xDrawPage,
                                 sal_uInt16 nPage, bool bExportObjects);

    ChecksumCache gMasterCache;
    ChecksumCache gPrivateCache;
    ChecksumCache gObjectCache;
    ChecksumCache gMetafileCache;

private:
    css::uno::Reference<css::uno::XComponentContext> mxContext;

    // #i56084# variables for selection export
    const css::uno::Reference<css::drawing::XShapes> mxSelectedShapes;
    const css::uno::Reference<css::drawing::XDrawPage> mxSelectedDrawPage;
    bool mbExportSelection;

    css::uno::Reference<css::drawing::XGraphicExportFilter> mxGraphicExporter;

    ::std::map<sal_uInt32, PageInfo> maPagesMap;

    sal_uInt16 exportDrawPageBackground(sal_uInt16 nPage,
                                        css::uno::Reference<css::drawing::XDrawPage> const& xPage);
    sal_uInt16
    exportMasterPageObjects(sal_uInt16 nPage,
                            css::uno::Reference<css::drawing::XDrawPage> const& xMasterPage);

    void exportDrawPageContents(const css::uno::Reference<css::drawing::XDrawPage>& xPage,
                                bool bStream, bool bMaster);
    void exportShapes(const css::uno::Reference<css::drawing::XShapes>& xShapes, bool bStream,
                      bool bMaster);
    void exportShape(const css::uno::Reference<css::drawing::XShape>& xShape, bool bMaster);

    BitmapChecksum ActionSummer(css::uno::Reference<css::drawing::XShape> const& xShape);
    BitmapChecksum ActionSummer(css::uno::Reference<css::drawing::XShapes> const& xShapes);

    bool getMetaFile(css::uno::Reference<css::lang::XComponent> const& xComponent,
                     GDIMetaFile& rMtf, bool bOnlyBackground = false, bool bExportAsJPEG = false);

    std::unique_ptr<Writer> mpWriter;

    sal_Int32 mnDocWidth;
    sal_Int32 mnDocHeight;

    sal_Int32 mnJPEGcompressMode;

    bool mbExportOLEAsJPEG;

    bool mbPresentation;

    sal_Int32 mnPageNumber;
};
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
