/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: vclhelperbitmaprender.hxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: aw $ $Date: 2007-03-06 12:34:16 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#ifndef INCLUDED_DRAWINGLAYER_PROCESSOR2D_VCLHELPERBITMAPRENDER_HXX
#define INCLUDED_DRAWINGLAYER_PROCESSOR2D_VCLHELPERBITMAPRENDER_HXX

#ifndef _SAL_TYPES_H_
#include <sal/types.h>
#endif

//////////////////////////////////////////////////////////////////////////////
// predefines

class OutputDevice;
class BitmapEx;
namespace basegfx { class B2DHomMatrix; }

//////////////////////////////////////////////////////////////////////////////
// support methods for vcl direct gradient renderering

namespace drawinglayer
{
    void RenderBitmapPrimitive2D_GraphicManager(
        OutputDevice& rOutDev,
        const BitmapEx& rBitmapEx,
        const basegfx::B2DHomMatrix& rTransform);

    void RenderBitmapPrimitive2D_BitmapEx(
        OutputDevice& rOutDev,
        const BitmapEx& rBitmapEx,
        const basegfx::B2DHomMatrix& rTransform);

    void RenderBitmapPrimitive2D_self(
        OutputDevice& rOutDev,
        const BitmapEx& rBitmapEx,
        const basegfx::B2DHomMatrix& rTransform);

} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////

#endif // INCLUDED_DRAWINGLAYER_PROCESSOR2D_VCLHELPERBITMAPRENDER_HXX

// eof
