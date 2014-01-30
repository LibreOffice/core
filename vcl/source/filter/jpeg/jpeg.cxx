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

#include <vcl/bmpacc.hxx>
#include <vcl/FilterConfigItem.hxx>
#include <vcl/graphicfilter.hxx>

bool ImportJPEG( SvStream& rInputStream, Graphic& rGraphic, void* pCallerData, sal_Int32 nImportFlags )
{
    ReadState   eReadState;
    bool        bReturn = true;

    JPEGReader* pJPEGReader = (JPEGReader*) rGraphic.GetContext();

    if( !pJPEGReader )
    {
        pJPEGReader = new JPEGReader( rInputStream, pCallerData, ( nImportFlags & GRFILTER_I_FLAGS_SET_LOGSIZE_FOR_JPEG ) != 0 );
    }

    if( nImportFlags & GRFILTER_I_FLAGS_FOR_PREVIEW )
    {
        pJPEGReader->SetPreviewSize( Size(128,128) );
    }
    else
    {
        pJPEGReader->DisablePreviewMode();
    }

    rGraphic.SetContext( NULL );
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
                    const com::sun::star::uno::Sequence<com::sun::star::beans::PropertyValue>* pFilterData,
                    bool* pExportWasGrey)
{
    JPEGWriter aJPEGWriter( rOutputStream, pFilterData, pExportWasGrey );
    bool bReturn = aJPEGWriter.Write( rGraphic );
    return bReturn;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
