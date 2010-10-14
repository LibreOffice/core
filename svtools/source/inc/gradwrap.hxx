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


#include <svgen.hxx>


/******************************************************************************
|*
|*  class GradientWrapper
|*
|*  Ersterstellung:     KA 24.11.95
|*  letzte Aenderung:   KA 24.11.95
|*
|*  Zeck:   dient beim MetaFile-Export dazu, die eigentliche Berechungs-
|*          funktionalitaet zu kapseln. Das Schreiben der Records fuer
|*          die unterschiedlichen File-Formate geschieht ueber LinkHandler.
|*
|*          Klassen, die diesen Wrapper benutzen, muessen drei Linkhandler
|*          zur Verfuegung stellen, die im Ctor uebergeben werden:
|*
|*              1. Linkhandler zum Schreiben eines Records fuer Polygonausgabe
|*              2. Linkhandler zum Schreiben eines Records fuer PolyPolygonausgabe
|*              3. Linkhandler zum Schreiben eines Records fuer Setzen der Brush
|*
\******************************************************************************/


class GradientWrapper
{
    Link        aDrawPolyRecordHdl;
    Link        aDrawPolyPolyRecordHdl;
    Link        aSetFillInBrushRecordHdl;

                GradientWrapper() {};


public:
                GradientWrapper(const Link& rDrawPolyRecordHdl,
                                const Link& rDrawPolyPolyRecordHdl,
                                const Link& rSetFillInBrushHdl);
                ~GradientWrapper();


    void        WriteLinearGradient(const Rectangle& rRect,
                                    const Gradient& rGradient);
    void        WriteRadialGradient(const Rectangle& rRect,
                                    const Gradient& rGradient);
    void        WriteRectGradient(const Rectangle& rRect,
                                  const Gradient& rGradient);
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
