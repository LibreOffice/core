/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: dx_linepolypolygon.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: hr $ $Date: 2007-11-01 17:56:14 $
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

#ifndef _DXCANVAS_LINEPOLYPOLYGON_HXX_
#define _DXCANVAS_LINEPOLYPOLYGON_HXX_

#include <canvas/canvastools.hxx>
#include <canvas/base/linepolypolygonbase.hxx>

#include "dx_gdiplususer.hxx"
#include "dx_impltools.hxx"


namespace dxcanvas
{
    class LinePolyPolygon : public ::canvas::LinePolyPolygonBase
    {
    public:
        explicit LinePolyPolygon( const ::basegfx::B2DPolyPolygon& );

        GraphicsPathSharedPtr getGraphicsPath() const;

    private:
        // overridden, to clear mpPath
        virtual void modifying() const;

        GDIPlusUserSharedPtr            mpGdiPlusUser;
        mutable GraphicsPathSharedPtr   mpPath;
    };
}

#endif /* _DXCANVAS_LINEPOLYPOLYGON_HXX_ */
