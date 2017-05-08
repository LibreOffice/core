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

#include "JpegReader.hxx"
#include "JpegWriter.hxx"
#include "jpeg.hxx"

#include <vcl/bitmapaccess.hxx>
#include <vcl/FilterConfigItem.hxx>
#include <vcl/graphicfilter.hxx>

VCL_DLLPUBLIC bool ImportJPEG( SvStream& rInputStream, Graphic& rGraphic, GraphicFilterImportFlags nImportFlags, Bitmap::ScopedWriteAccess* ppAccess )
{
    bool        bReturn = true;

    std::shared_ptr<GraphicReader> pContext = rGraphic.GetContext();
    rGraphic.SetContext(nullptr);
    JPEGReader* pJPEGReader = dynamic_cast<JPEGReader*>( pContext.get() );
    if (!pJPEGReader)
    {
        pContext = std::make_shared<JPEGReader>( rInputStream, nImportFlags );
        pJPEGReader = static_cast<JPEGReader*>( pContext.get() );
    }

    if( nImportFlags & GraphicFilterImportFlags::ForPreview )
    {
        pJPEGReader->SetPreviewSize( Size(128,128) );
    }
    else
    {
        pJPEGReader->DisablePreviewMode();
    }

    ReadState eReadState = pJPEGReader->Read( rGraphic, nImportFlags, ppAccess );

    if( eReadState == JPEGREAD_ERROR )
    {
        bReturn = false;
    }
    else if( eReadState == JPEGREAD_NEED_MORE )
    {
        rGraphic.SetContext( pContext );
    }

    return bReturn;
}

bool ExportJPEG(SvStream& rOutputStream, const Graphic& rGraphic,
                    const css::uno::Sequence<css::beans::PropertyValue>* pFilterData,
                    bool* pExportWasGrey)
{
    JPEGWriter aJPEGWriter( rOutputStream, pFilterData, pExportWasGrey );
    bool bReturn = aJPEGWriter.Write( rGraphic );
    return bReturn;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
