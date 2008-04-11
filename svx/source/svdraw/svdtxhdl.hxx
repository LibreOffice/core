/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: svdtxhdl.hxx,v $
 * $Revision: 1.8 $
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

#ifndef _SVDTXHDL_HXX
#define _SVDTXHDL_HXX

#ifndef _VIRDEV_HXX //autogen
#include <vcl/virdev.hxx>
#endif
#include <tools/poly.hxx>
#include <tools/link.hxx>

//************************************************************
//   Vorausdeklarationen
//************************************************************

class SdrOutliner;
class DrawPortionInfo;
class SdrTextObj;
class SdrObjGroup;
class SdrModel;
class XOutputDevice;

//************************************************************
//   ImpTextPortionHandler
//************************************************************

class ImpTextPortionHandler
{
    VirtualDevice               aVDev;
    Rectangle                   aFormTextBoundRect;
    SdrOutliner&                rOutliner;
    const SdrTextObj&           rTextObj;
    XOutputDevice*          pXOut;

    // Variablen fuer ConvertToPathObj
    SdrObjGroup*                pGroup;
    SdrModel*                   pModel;
    FASTBOOL                    bToPoly;

    // Variablen fuer DrawFitText
    Point                       aPos;
    Fraction                    aXFact;
    Fraction                    aYFact;

    // Variablen fuer DrawTextToPath
    ULONG                       nParagraph;
    BOOL                        bToLastPoint;
    bool                        bDraw;
    void*                       mpRecordPortions;

private:
    // #101498#
    void SortedAddFormTextRecordPortion(DrawPortionInfo* pInfo);
    void DrawFormTextRecordPortions(Polygon aPoly);
    void ClearFormTextRecordPortions();
    sal_uInt32 GetFormTextPortionsLength(OutputDevice* pOut);

public:
    ImpTextPortionHandler(SdrOutliner& rOutln, const SdrTextObj& rTxtObj);

    void ConvertToPathObj(SdrObjGroup& rGroup, FASTBOOL bToPoly);
    void DrawFitText(XOutputDevice& rXOut, const Point& rPos, const Fraction& rXFact);
    void DrawTextToPath(XOutputDevice& rXOut, FASTBOOL bDrawEffect=TRUE);

    // wird von DrawTextToPath() gesetzt:
    const Rectangle& GetFormTextBoundRect() { return aFormTextBoundRect; }

    DECL_LINK(ConvertHdl,DrawPortionInfo*);
    DECL_LINK(FitTextDrawHdl,DrawPortionInfo*);

    // #101498#
    DECL_LINK(FormTextRecordPortionHdl, DrawPortionInfo*);
};

////////////////////////////////////////////////////////////////////////////////////////////////////

#endif //_SVDTXHDL_HXX

