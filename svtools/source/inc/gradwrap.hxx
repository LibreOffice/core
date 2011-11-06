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




#ifndef _SVGEN_HXX
#include <svgen.hxx>
#endif


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
