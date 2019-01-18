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

#ifndef INCLUDED_VCL_PNGREAD_HXX
#define INCLUDED_VCL_PNGREAD_HXX

#include <vcl/dllapi.h>
#include <vcl/bitmapex.hxx>
#include <memory>
#include <vector>

namespace vcl
{

class PNGReaderImpl;

class VCL_DLLPUBLIC PNGReader
{
    std::unique_ptr<PNGReaderImpl> mpImpl;

public:

    /* the PNG chunks are read within the c'tor, so the stream will
    be positioned at the end of the PNG */
    explicit PNGReader(SvStream& rStream);
    ~PNGReader();

    /* an empty preview size hint (=default) will read the whole image
    */
    BitmapEx Read(const Size& i_rPreviewHint = Size());

    // retrieve every chunk that resides inside the PNG
    struct ChunkData
    {
        sal_uInt32 nType = 0;
        std::vector<sal_uInt8> aData;
    };
    const std::vector<ChunkData>& GetChunks() const;

    void SetIgnoreGammaChunk(bool bIgnoreGammaChunk);
};

} // end namespace vcl

#endif // INCLUDED_VCL_PNGREAD_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
