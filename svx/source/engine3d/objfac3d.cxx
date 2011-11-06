/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_svx.hxx"
#include <svx/svdpage.hxx>
#include "svx/globl3d.hxx"
#include <svx/polysc3d.hxx>
#include <svx/cube3d.hxx>
#include <svx/sphere3d.hxx>
#include <svx/extrud3d.hxx>
#include <svx/lathe3d.hxx>
#include <svx/polygn3d.hxx>
#include "svx/objfac3d.hxx"
#include <svx/svdobj.hxx>

/*************************************************************************
|*
|* Konstruktor
|*
\************************************************************************/

static sal_Bool bInit = sal_False;

E3dObjFactory::E3dObjFactory()
{
    if ( !bInit )
    {
        SdrObjFactory::InsertMakeObjectHdl(LINK(this, E3dObjFactory, MakeObject));
        bInit = sal_True;
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
            case E3D_EXTRUDEOBJ_ID:
                pObjFactory->pNewObj = new E3dExtrudeObj();
                break;
            case E3D_LATHEOBJ_ID:
                pObjFactory->pNewObj = new E3dLatheObj();
                break;
            case E3D_COMPOUNDOBJ_ID:
                pObjFactory->pNewObj = new E3dCompoundObject();
                break;
        }
    }
    return 0;
}


