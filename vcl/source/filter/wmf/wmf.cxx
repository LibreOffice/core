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

#include "emfwr.hxx"
#include "wmfwr.hxx"
#include <vcl/wmf.hxx>
#include <vcl/gdimetafiletools.hxx>
#include <comphelper/scopeguard.hxx>

bool ReadWindowMetafile( SvStream& rStream, GDIMetaFile& rMTF )
{
    // Use new method to import Metafile. First, read binary data to mem array
    const sal_uInt32 nStreamLength(rStream.Seek(STREAM_SEEK_TO_END));
    VectorGraphicDataArray aNewData(nStreamLength);
    rStream.Seek(0);
    rStream.ReadBytes(aNewData.begin(), nStreamLength);
    rStream.Seek(0);

    if (rStream.good())
    {
        // Throw into VectorGraphicData to get the import. Do not care
        // too much for type, this will be checked there. Also no path
        // needed, it is a temporary object
        VectorGraphicDataPtr aVectorGraphicDataPtr(
            new VectorGraphicData(
                aNewData,
                OUString(),
                VectorGraphicDataType::Emf));

        // create a Graphic and grep Metafile from it
        const Graphic aGraphic(aVectorGraphicDataPtr);

        // get the Metafile from it, done
        rMTF = aGraphic.GetGDIMetaFile();
        return true;
    }

    return rStream.good();
}

bool ConvertGDIMetaFileToWMF( const GDIMetaFile & rMTF, SvStream & rTargetStream,
                              FilterConfigItem* pConfigItem, bool bPlaceable)
{
    WMFWriter aWMFWriter;
    GDIMetaFile aGdiMetaFile(rMTF);

    if(usesClipActions(aGdiMetaFile))
    {
        // #i121267# It is necessary to prepare the metafile since the export does *not* support
        // clip regions. This tooling method clips the geometry content of the metafile internally
        // against its own clip regions, so that the export is safe to ignore clip regions
        clipMetafileContentAgainstOwnRegions(aGdiMetaFile);
    }

    return aWMFWriter.WriteWMF( aGdiMetaFile, rTargetStream, pConfigItem, bPlaceable );
}

bool ConvertGDIMetaFileToEMF(const GDIMetaFile & rMTF, SvStream & rTargetStream)
{
    EMFWriter aEMFWriter(rTargetStream);
    GDIMetaFile aGdiMetaFile(rMTF);

    if(usesClipActions(aGdiMetaFile))
    {
        // #i121267# It is necessary to prepare the metafile since the export does *not* support
        // clip regions. This tooling method clips the geometry content of the metafile internally
        // against its own clip regions, so that the export is safe to ignore clip regions
        clipMetafileContentAgainstOwnRegions(aGdiMetaFile);
    }

    return aEMFWriter.WriteEMF(aGdiMetaFile);
}

bool WriteWindowMetafileBits( SvStream& rStream, const GDIMetaFile& rMTF )
{
    return WMFWriter().WriteWMF( rMTF, rStream, nullptr, false );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
