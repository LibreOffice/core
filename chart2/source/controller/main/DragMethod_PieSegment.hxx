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
#ifndef CHART2_DRAGMETHOD_PIESEGMENT_HXX
#define CHART2_DRAGMETHOD_PIESEGMENT_HXX

#include "DragMethod_Base.hxx"
#include <basegfx/vector/b2dvector.hxx>

namespace chart
{

class DragMethod_PieSegment : public DragMethod_Base
{
public:
    DragMethod_PieSegment( DrawViewWrapper& rDrawViewWrapper, const rtl::OUString& rObjectCID
        , const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XModel >& xChartModel );
    virtual ~DragMethod_PieSegment();

    virtual void TakeSdrDragComment(String& rStr) const;
    virtual bool BeginSdrDrag();
    virtual void MoveSdrDrag(const Point& rPnt);
    virtual bool EndSdrDrag(bool bCopy);

    virtual basegfx::B2DHomMatrix getCurrentTransformation();

protected:
    virtual void createSdrDragEntries();

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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
