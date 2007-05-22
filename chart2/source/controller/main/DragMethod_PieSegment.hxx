/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: DragMethod_PieSegment.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-05-22 18:07:47 $
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
#ifndef CHART2_DRAGMETHOD_PIESEGMENT_HXX
#define CHART2_DRAGMETHOD_PIESEGMENT_HXX

#include "DragMethod_Base.hxx"

#ifndef _BGFX_VECTOR_B2DVECTOR_HXX
#include <basegfx/vector/b2dvector.hxx>
#endif

namespace chart
{

class DragMethod_PieSegment : public DragMethod_Base
{
public:
    DragMethod_PieSegment( DrawViewWrapper& rDrawViewWrapper, const rtl::OUString& rObjectCID
        , const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XModel >& xChartModel );
    virtual ~DragMethod_PieSegment();

    virtual void TakeComment(String& rStr) const;
    virtual void MovPoint(Point& rPnt);
    virtual FASTBOOL Beg();
    virtual void Mov(const Point& rPnt);
    virtual FASTBOOL End(FASTBOOL bCopy);

private:
    ::basegfx::B2DVector    m_aStartVector;
    double                  m_fInitialOffset;
    double                  m_fAdditionalOffset;
    ::basegfx::B2DVector    m_aDragDirection;
    double                  m_fDragRange;
};

} //  namespace chart

// CHART2_DRAGMETHOD_PIESEGMENT_HXX
#endif
