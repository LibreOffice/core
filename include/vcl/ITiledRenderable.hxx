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

#define LOK_USE_UNSTABLE_API
#include <LibreOfficeKit/LibreOfficeKit.h>
#include <tools/gen.hxx>
#include <vcl/virdev.hxx>

namespace vcl
{

class VCL_DLLPUBLIC ITiledRenderable
{
public:
    virtual ~ITiledRenderable();

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

    /**
     * Set the document "part", i.e. slide for a slideshow, and
     * tab for a spreadsheet.
     */
    virtual void setPart( int nPart )
    {
        (void) nPart;
    }

    /**
     * Get the number of parts -- see setPart for further details.
     */
    virtual int getParts()
    {
        return 1;
    }

    /**
     * Get the currently displayed/selected part -- see setPart for further
     * details.
     */
    virtual int getPart()
    {
        return 0;
    }

    /**
     * Get the name of the currently displayed part, i.e. sheet in a spreadsheet
     * or slide in a presentation.
     */
    virtual OUString getPartName(int nPart)
    {
        (void) nPart;
        return OUString("");
    }

    virtual void setPartMode(LibreOfficeKitPartMode ePartMode)
    {
        (void) ePartMode;
    }

    virtual void initializeForTiledRendering()
    {
    }
};

} // namespace vcl

#endif // INCLUDED_VCL_ITILEDRENDERABLE_HXX
