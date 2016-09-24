/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */


#include <string.h>
#include <vcl/gdimtf.hxx>
#include <vcl/graph.hxx>
#include <vcl/virdev.hxx>
#include "dxf2mtf.hxx"
#include <math.h>

class FilterConfigItem;

//================== GraphicImport - die exportierte Funktion ================

extern "C" SAL_DLLPUBLIC_EXPORT bool SAL_CALL
idxGraphicImport( SvStream & rStream, Graphic & rGraphic, FilterConfigItem* )
{
    DXFRepresentation aDXF;
    DXF2GDIMetaFile aConverter;
    GDIMetaFile aMTF;

    if ( !aDXF.Read( rStream, 0, 60 ) )
        return false;
    if ( !aConverter.Convert( aDXF, aMTF, 60, 100 ) )
        return false;
    rGraphic = Graphic(aMTF);

    return true;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
