/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: DragMethod_RotateDiagram.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-05-22 18:08:07 $
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
#ifndef CHART2_DRAGMETHOD_ROTATEDIAGRAM_HXX
#define CHART2_DRAGMETHOD_ROTATEDIAGRAM_HXX

#include "DragMethod_Base.hxx"

// header for class ::basegfx::B3DPolygon
#ifndef _BGFX_POLYGON_B3DPOLYGON_HXX
#include <basegfx/polygon/b3dpolygon.hxx>
#endif

namespace chart
{

class DragMethod_RotateDiagram : public DragMethod_Base
{
public:
    enum RotationDirection
    {
        ROTATIONDIRECTION_FREE,
        ROTATIONDIRECTION_X,
        ROTATIONDIRECTION_Y,
        ROTATIONDIRECTION_Z
    };

    DragMethod_RotateDiagram( DrawViewWrapper& rDrawViewWrapper
        , const rtl::OUString& rObjectCID
        , const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XModel >& xChartModel
        , RotationDirection eRotationDirection
        );
    virtual ~DragMethod_RotateDiagram();

    virtual void TakeComment(String& rStr) const;
    virtual FASTBOOL Beg();
    virtual void Mov(const Point& rPnt);
    virtual FASTBOOL End(FASTBOOL bCopy);

    virtual void CreateOverlayGeometry(::sdr::overlay::OverlayManager& rOverlayManager, ::sdr::overlay::OverlayObjectList& rOverlayList);

private:
    E3dScene*   m_pScene;

    Rectangle   m_aReferenceRect;
    Point       m_aStartPos;
    ::basegfx::B3DPolygon m_aWireframePoly;

    double      m_fInitialXAngleRad;
    double      m_fInitialYAngleRad;
    double      m_fInitialZAngleRad;

    double      m_fAdditionalXAngleRad;
    double      m_fAdditionalYAngleRad;
    double      m_fAdditionalZAngleRad;

    RotationDirection m_eRotationDirection;
};

} //  namespace chart

// CHART2_DRAGMETHOD_ROTATEDIAGRAM_HXX
#endif
