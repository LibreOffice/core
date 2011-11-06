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
#include <svx/xfillit.hxx>
#include <svx/svdopath.hxx>
#include <svx/svdogrp.hxx>
#include "svx/svditer.hxx"
#include <svx/svdetc.hxx>
#include <vcl/virdev.hxx>
#include <vcl/svapp.hxx>
#include <svx/svdpage.hxx>
#include <svx/svdpool.hxx>
#include <svl/style.hxx>
#include "svx/globl3d.hxx"
#include <svx/polysc3d.hxx>
#include <svx/xlnclit.hxx>
#include <svl/metitem.hxx>
#include <svx/xtable.hxx>
#include <svx/xlnwtit.hxx>

#define ITEMVALUE(ItemSet,Id,Cast)  ((const Cast&)(ItemSet).Get(Id)).GetValue()

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

TYPEINIT1(E3dPolyScene, E3dScene);

E3dPolyScene::E3dPolyScene()
:   E3dScene()
{
}

E3dPolyScene::E3dPolyScene(E3dDefaultAttributes& rDefault)
:   E3dScene(rDefault)
{
}

/*************************************************************************
|*
|* Identifier zurueckgeben
|*
\************************************************************************/

sal_uInt16 E3dPolyScene::GetObjIdentifier() const
{
    return E3D_POLYSCENE_ID;
}

// eof
