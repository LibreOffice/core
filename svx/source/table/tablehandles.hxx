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



#ifndef _SVX_TABLEHANDLES_HXX
#define _SVX_TABLEHANDLES_HXX

#include <svx/sdr/overlay/overlayobject.hxx>

#include <svx/svdhdl.hxx>

// --------------------------------------------------------------------

namespace sdr { namespace table {

enum TableEdgeState { Empty, Invisible, Visible };

struct TableEdge
{
    sal_Int32 mnStart;
    sal_Int32 mnEnd;
    TableEdgeState meState;

    TableEdge() : mnStart(0), mnEnd(0), meState(Empty) {}
};

typedef std::vector< TableEdge > TableEdgeVector;

class TableEdgeHdl : public SdrHdl
{
public:
    TableEdgeHdl(
        SdrHdlList& rHdlList,
        const SdrObject& rSdrHdlObject,
        const basegfx::B2DPoint& rPnt,
        bool bHorizontal,
        sal_Int32 nMin,
        sal_Int32 nMax,
        sal_Int32 nEdges );

    sal_Int32 GetValidDragOffset( const SdrDragStat& rDrag ) const;

    virtual Pointer GetPointer() const;

    void SetEdge( sal_Int32 nEdge, sal_Int32 nStart, sal_Int32 nEnd, TableEdgeState nState );

    bool IsHorizontalEdge() const { return mbHorizontal; }

    basegfx::B2DPolyPolygon getSpecialDragPoly(const SdrDragStat& rDrag) const;
    void getPolyPolygon(basegfx::B2DPolyPolygon& rVisible, basegfx::B2DPolyPolygon& rInvisible, const SdrDragStat* pDrag) const;

protected:
    // create marker for this kind
    virtual void CreateB2dIAObject(::sdr::overlay::OverlayManager& rOverlayManager);

    virtual ~TableEdgeHdl();

private:
    bool mbHorizontal;
    sal_Int32 mnMin, mnMax;
    TableEdgeVector maEdges;
};

class TableBorderHdl : public SdrHdl
{
public:
    TableBorderHdl(
        SdrHdlList& rHdlList,
        const SdrObject& rSdrHdlObject,
        const basegfx::B2DRange& rRange );

    virtual Pointer GetPointer() const;

protected:
    // create marker for this kind
    virtual void CreateB2dIAObject(::sdr::overlay::OverlayManager& rOverlayManager);

    virtual ~TableBorderHdl();

private:
    basegfx::B2DRange maRange;
};

} // end of namespace table
} // end of namespace sdr

#endif
