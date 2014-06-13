/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#ifndef INCLUDED_VCL_ITILEDRENDERABLE_HXX
#define INCLUDED_VCL_ITILEDRENDERABLE_HXX

#include <tools/gen.hxx>
#include <vcl/virdev.hxx>

namespace vcl
{

class VCL_DLLPUBLIC ITiledRenderable
{
public:
    virtual ~ITiledRenderable() {};

    /**
     * Paint a tile to a given VirtualDevice.
     *
     * Output parameters are measured in pixels, tile parameters are in
     * twips.
     */
    virtual void paintTile( VirtualDevice &rDevice,
                            int nOutputWidth,
                            int nOutputHeight,
                            int nTilePosX,
                            int nTilePosY,
                            long nTileWidth,
                            long nTileHeight ) = 0;

    /**
     * Get the document size in twips.
     */
    virtual Size getDocumentSize() = 0;

};

} // namespace vcl

#endif // INCLUDED_VCL_ITILEDRENDERABLE_HXX
