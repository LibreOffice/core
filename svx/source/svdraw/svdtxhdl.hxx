/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: svdtxhdl.hxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: ihi $ $Date: 2006-11-14 13:50:29 $
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

#ifndef _SVDTXHDL_HXX
#define _SVDTXHDL_HXX

#ifndef _VIRDEV_HXX //autogen
#include <vcl/virdev.hxx>
#endif

#ifndef _TL_POLY_HXX
#include <tools/poly.hxx>
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

