/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

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

sal_uInt16 E3dPolyScene::GetObjIdentifier() const
{
    return E3D_POLYSCENE_ID;
}

E3dPolyScene* E3dPolyScene::Clone() const
{
    return CloneHelper< E3dPolyScene >();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
