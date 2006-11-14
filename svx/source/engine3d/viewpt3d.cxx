/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: viewpt3d.cxx,v $
 *
 *  $Revision: 1.8 $
 *
 *  last change: $Author: ihi $ $Date: 2006-11-14 13:22:59 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_svx.hxx"

#ifndef _VIEWPT3D_HXX
#include "viewpt3d.hxx"
#endif

#ifndef _VOLUME3D_HXX
#include "volume3d.hxx"
#endif

/*************************************************************************
|*
|* View-Window genau um das mit rTransform transformierte Volumen legen
|*
\************************************************************************/

void Viewport3D::FitViewToVolume(const basegfx::B3DRange& rVolume, const basegfx::B3DHomMatrix& rTransform)
{
    basegfx::B3DHomMatrix aTransform(rTransform);
    basegfx::B3DPoint aTfVec;
    Volume3D aFitVol;

    aTransform *= GetViewTransform();
    Vol3DPointIterator aIter(rVolume, &aTransform);

    while ( aIter.Next(aTfVec) )
    {
        aTfVec = DoProjection(aTfVec);
        aFitVol.expand(aTfVec);
    }
    SetViewWindow(aFitVol.getMinX(), aFitVol.getMinY(), aFitVol.getWidth(), aFitVol.getHeight());
}

// eof
