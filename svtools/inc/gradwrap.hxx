/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: gradwrap.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 09:34:42 $
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
