/*************************************************************************
 *
 *  $RCSfile: svdtxhdl.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:01:26 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef _SVDTXHDL_HXX
#define _SVDTXHDL_HXX

#ifndef _VIRDEV_HXX //autogen
#include <vcl/virdev.hxx>
#endif

#ifndef _POLY_HXX //autogen
#include <vcl/poly.hxx>
#endif

#ifndef _LINK_HXX //autogen
#include <tools/link.hxx>
#endif

//************************************************************
//   Vorausdeklarationen
//************************************************************

class SdrOutliner;
class DrawPortionInfo;
class SdrTextObj;
class SdrObjGroup;
class SdrModel;
class ExtOutputDevice;

//************************************************************
//   ImpTextPortionHandler
//************************************************************

class ImpTextPortionHandler
{
    VirtualDevice               aVDev;
    Rectangle                   aFormTextBoundRect;
    SdrOutliner&                rOutliner;
    const SdrTextObj&           rTextObj;
    ExtOutputDevice*            pXOut;

    // Variablen fuer ConvertToPathObj
    SdrObjGroup*                pGroup;
    SdrModel*                   pModel;
    FASTBOOL                    bToPoly;

    // Variablen fuer DrawFitText
    Point                       aPos;
    Fraction                    aXFact;
    Fraction                    aYFact;

    // Variablen fuer DrawTextToPath
    Polygon                     aPoly;
    long                        nTextWidth;
    ULONG                       nParagraph;
    BOOL                        bToLastPoint;
    BOOL                        bDraw;

public:
    ImpTextPortionHandler(SdrOutliner& rOutln, const SdrTextObj& rTxtObj);

    void ConvertToPathObj(SdrObjGroup& rGroup, FASTBOOL bToPoly);
    void DrawFitText(ExtOutputDevice& rXOut, const Point& rPos, const Fraction& rXFact);
    void DrawTextToPath(ExtOutputDevice& rXOut, FASTBOOL bDrawEffect=TRUE);

    // wird von DrawTextToPath() gesetzt:
    const Rectangle& GetFormTextBoundRect() { return aFormTextBoundRect; }

    DECL_LINK(ConvertHdl,DrawPortionInfo*);
    DECL_LINK(FitTextDrawHdl,DrawPortionInfo*);
    DECL_LINK(FormTextWidthHdl,DrawPortionInfo*);
    DECL_LINK(FormTextDrawHdl,DrawPortionInfo*);
};

////////////////////////////////////////////////////////////////////////////////////////////////////

#endif //_SVDTXHDL_HXX

