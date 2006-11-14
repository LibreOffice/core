/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: objfac3d.cxx,v $
 *
 *  $Revision: 1.8 $
 *
 *  last change: $Author: ihi $ $Date: 2006-11-14 13:20:20 $
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

#ifndef _SVDPAGE_HXX
#include "svdpage.hxx"
#endif

#ifndef _E3D_GLOBL3D_HXX
#include "globl3d.hxx"
#endif

#ifndef _E3D_POLYSC3D_HXX
#include "polysc3d.hxx"
#endif

#ifndef _E3D_CUBE3D_HXX
#include "cube3d.hxx"
#endif

#ifndef _E3D_SPHERE3D_HXX
#include "sphere3d.hxx"
#endif

#ifndef _E3D_EXTRUD3D_HXX
#include "extrud3d.hxx"
#endif

#ifndef _E3D_LATHE3D_HXX
#include "lathe3d.hxx"
#endif

#ifndef _E3D_POLYGON3D_HXX
#include "polygn3d.hxx"
#endif

#ifndef _OBJFAC3D_HXX
#include "objfac3d.hxx"
#endif

#ifndef _SVDOBJ_HXX
#include "svdobj.hxx"
#endif

/*************************************************************************
|*
|* Konstruktor
|*
\************************************************************************/

static BOOL bInit = FALSE;

E3dObjFactory::E3dObjFactory()
{
    if ( !bInit )
    {
        SdrObjFactory::InsertMakeObjectHdl(LINK(this, E3dObjFactory, MakeObject));
        bInit = TRUE;
    }
}

/*************************************************************************
|*
|* Destruktor
|*
\************************************************************************/

E3dObjFactory::~E3dObjFactory()
{
}

/*************************************************************************
|*
|* Chart-interne Objekte erzeugen
|*
\************************************************************************/

IMPL_LINK( E3dObjFactory, MakeObject, SdrObjFactory*, pObjFactory)
{
    if ( pObjFactory->nInventor == E3dInventor )
    {
        switch ( pObjFactory->nIdentifier )
        {
            case E3D_POLYSCENE_ID:
                pObjFactory->pNewObj = new E3dPolyScene();
                break;
            case E3D_OBJECT_ID:
                pObjFactory->pNewObj = new E3dObject();
                break;
            case E3D_POLYGONOBJ_ID  :
                pObjFactory->pNewObj = new E3dPolygonObj();
                break;
            case E3D_CUBEOBJ_ID :
                pObjFactory->pNewObj = new E3dCubeObj();
                break;
            case E3D_SPHEREOBJ_ID:
                    // FG: ruft den dummy constructor, da dieser Aufruf nur beim Laden von Dokumenten erfolgt.
                    //     die wirkliche Anzahkl Segmente wird aber erst nach dem Laden der Member festgelegt.
                    //     dies hat zur Folge das die erste Kugel gleich wieder zerstoert wird, obwohl sie nie
                    //     gebraucht worden ist.
                pObjFactory->pNewObj = new E3dSphereObj(123);
                break;
            case E3D_POINTOBJ_ID:
                pObjFactory->pNewObj = new E3dPointObj();
                break;
            case E3D_EXTRUDEOBJ_ID:
                pObjFactory->pNewObj = new E3dExtrudeObj();
                break;
            case E3D_LATHEOBJ_ID:
                pObjFactory->pNewObj = new E3dLatheObj();
                break;
            case E3D_LABELOBJ_ID:
                pObjFactory->pNewObj = new E3dLabelObj();
                break;
            case E3D_COMPOUNDOBJ_ID:
                pObjFactory->pNewObj = new E3dCompoundObject();
                break;
        }
    }
    return 0;
}


