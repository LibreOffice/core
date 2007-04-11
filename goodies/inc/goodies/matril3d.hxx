/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: matril3d.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-04-11 20:33:32 $
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

#ifndef _B3D_MATRIL3D_HXX
#define _B3D_MATRIL3D_HXX

#ifndef _B3D_BUCKET_HXX
#include <goodies/bucket.hxx>
#endif

#ifndef _TOOLS_COLOR_HXX
#include <tools/color.hxx>
#endif

#ifndef _STREAM_HXX
#include <tools/stream.hxx>
#endif

/*************************************************************************
|*
|* Moegliche MaterialModes fuer Polygone
|*
\************************************************************************/

enum Base3DMaterialMode
{
    Base3DMaterialFront = 0,
    Base3DMaterialBack,
    Base3DMaterialFrontAndBack
};

/*************************************************************************
|*
|* Moegliche MaterialValues fuer Polygone
|*
\************************************************************************/

enum Base3DMaterialValue
{
    Base3DMaterialAmbient = 0,
    Base3DMaterialDiffuse,
    Base3DMaterialSpecular,
    Base3DMaterialEmission
};

/*************************************************************************
|*
|* Basisklasse fuer Materialparameter
|*
\************************************************************************/

class B3dMaterial
{
private:
    Color                   aAmbient;
    Color                   aDiffuse;
    Color                   aSpecular;
    Color                   aEmission;
    UINT16                  nExponent;

public:
    B3dMaterial();

    // Zugriffsfunktionen
    void SetMaterial(Color rNew,
        Base3DMaterialValue=Base3DMaterialAmbient);
    Color GetMaterial(Base3DMaterialValue=Base3DMaterialAmbient) const;
    void SetShininess(UINT16 nNew);
    UINT16 GetShininess() const;

    // Vergleichsoperator
    BOOL operator==(const B3dMaterial&);
    BOOL operator!=(const B3dMaterial& rMat) { return (!((*this) == rMat)); }
protected:
};

/*************************************************************************
|*
|* Bucket fuer Materialeigenschaften
|*
\************************************************************************/

BASE3D_DECL_BUCKET(B3dMaterial, Bucket)

#endif          // _B3D_MATRIL3D_HXX
