/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: svdhlpln.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-04-11 16:21:21 $
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

#ifndef _SVDHLPLN_HXX
#define _SVDHLPLN_HXX

#ifndef _SAL_TYPES_H_
#include <sal/types.h>
#endif

#ifndef _TOOLS_COLOR_HXX
#include <tools/color.hxx>
#endif

#ifndef _GEN_HXX //autogen
#include <tools/gen.hxx>
#endif

#ifndef _POINTR_HXX //autogen
#include <vcl/pointr.hxx>
#endif

#ifndef _CONTNR_HXX //autogen
#include <tools/contnr.hxx>
#endif

#ifndef INCLUDED_SVXDLLAPI_H
#include "svx/svxdllapi.h"
#endif

class OutputDevice;
////////////////////////////////////////////////////////////////////////////////////////////////////

enum SdrHelpLineKind {SDRHELPLINE_POINT,SDRHELPLINE_VERTICAL,SDRHELPLINE_HORIZONTAL};
#define SDRHELPLINE_MIN SDRHELPLINE_POINT
#define SDRHELPLINE_MAX SDRHELPLINE_HORIZONTAL

#define SDRHELPLINE_POINT_PIXELSIZE 15 /* Tatsaechliche Groesse= PIXELSIZE*2+1 */

class SdrHelpLine {
    Point            aPos; // je nach Wert von eKind ist X oder Y evtl. belanglos
    SdrHelpLineKind  eKind;

    // #i27493#
    // Helper method to draw a hor or ver two-colored dashed line
    void ImpDrawDashedTwoColorLine(OutputDevice& rOut, sal_Int32 nStart, sal_Int32 nEnd, sal_Int32 nFixPos,
        sal_Int32 nStepWidth, Color aColA, Color aColB, sal_Bool bHorizontal) const;

public:
    SdrHelpLine(SdrHelpLineKind eNewKind=SDRHELPLINE_POINT): eKind(eNewKind) {}
    SdrHelpLine(SdrHelpLineKind eNewKind, const Point& rNewPos): aPos(rNewPos), eKind(eNewKind) {}
    bool operator==(const SdrHelpLine& rCmp) const { return aPos==rCmp.aPos && eKind==rCmp.eKind; }
    bool operator!=(const SdrHelpLine& rCmp) const { return !operator==(rCmp); }

    void            SetKind(SdrHelpLineKind eNewKind) { eKind=eNewKind; }
    SdrHelpLineKind GetKind() const                   { return eKind; }
    void            SetPos(const Point& rPnt)         { aPos=rPnt; }
    const Point&    GetPos() const                    { return aPos; }

    Pointer         GetPointer() const;
    void            Draw(OutputDevice& rOut, const Point& rOfs) const;
    FASTBOOL        IsHit(const Point& rPnt, USHORT nTolLog, const OutputDevice& rOut) const;
    // OutputDevice wird benoetigt, da Fangpunkte eine feste Pixelgroesse haben
    Rectangle       GetBoundRect(const OutputDevice& rOut) const;

    /* returns true if this and the given help line would be rendered at the same pixel position
        of the given OutputDevice. This can be used to avoid drawing multiple help lines with xor
        on same position which could render them invisible */
    bool            IsVisibleEqual( const SdrHelpLine& rHelpLine, const OutputDevice& rOut ) const;
};

#define SDRHELPLINE_NOTFOUND 0xFFFF

class SVX_DLLPUBLIC SdrHelpLineList {
    Container aList;
protected:
    SdrHelpLine* GetObject(USHORT i) const { return (SdrHelpLine*)(aList.GetObject(i)); }
public:
    SdrHelpLineList(): aList(1024,4,4) {}
    SdrHelpLineList(const SdrHelpLineList& rSrcList): aList(1024,4,4)      { *this=rSrcList; }
    ~SdrHelpLineList()                                                     { Clear(); }
    void               Clear();
    void               operator=(const SdrHelpLineList& rSrcList);
    bool operator==(const SdrHelpLineList& rCmp) const;
    bool operator!=(const SdrHelpLineList& rCmp) const                 { return !operator==(rCmp); }
    USHORT             GetCount() const                                    { return USHORT(aList.Count()); }
    void               Insert(const SdrHelpLine& rHL, USHORT nPos=0xFFFF)  { aList.Insert(new SdrHelpLine(rHL),nPos); }
    void               Delete(USHORT nPos)                                 { delete (SdrHelpLine*)aList.Remove(nPos); } // #i24900#
    void               Move(USHORT nPos, USHORT nNewPos)                   { aList.Insert(aList.Remove(nPos),nNewPos); }
    SdrHelpLine&       operator[](USHORT nPos)                             { return *GetObject(nPos); }
    const SdrHelpLine& operator[](USHORT nPos) const                       { return *GetObject(nPos); }
    void               DrawAll(OutputDevice& rOut, const Point& rOfs) const;
    USHORT             HitTest(const Point& rPnt, USHORT nTolLog, const OutputDevice& rOut) const;
};

////////////////////////////////////////////////////////////////////////////////////////////////////

#endif //_SVDHLPLN_HXX

