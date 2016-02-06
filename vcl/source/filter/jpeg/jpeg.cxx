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

VCL_DLLPUBLIC bool ImportJPEG( SvStream& rInputStream, Graphic& rGraphic, void* pCallerData, GraphicFilterImportFlags nImportFlags )
{
    ReadState   eReadState;
    bool        bReturn = true;

    JPEGReader* pJPEGReader = static_cast<JPEGReader*>( rGraphic.GetContext() );

    if( !pJPEGReader )
    {
        pJPEGReader = new JPEGReader( rInputStream, pCallerData, bool( nImportFlags & GraphicFilterImportFlags::SetLogsizeForJpeg ) );
    }

    if( nImportFlags & GraphicFilterImportFlags::ForPreview )
    {
        pJPEGReader->SetPreviewSize( Size(128,128) );
    }
    else
    {
        pJPEGReader->DisablePreviewMode();
    }

    rGraphic.SetContext( nullptr );
    eReadState = pJPEGReader->Read( rGraphic );

    if( eReadState == JPEGREAD_ERROR )
    {
        bReturn = false;
        delete pJPEGReader;
    }
    else if( eReadState == JPEGREAD_OK )
    {
        delete pJPEGReader;
    }
    else
    {
        rGraphic.SetContext( pJPEGReader );
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
