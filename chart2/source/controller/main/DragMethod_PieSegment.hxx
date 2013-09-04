/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */
#ifndef CHART2_DRAGMETHOD_PIESEGMENT_HXX
#define CHART2_DRAGMETHOD_PIESEGMENT_HXX

#include "DragMethod_Base.hxx"
#include <basegfx/vector/b2dvector.hxx>

namespace chart
{

class DragMethod_PieSegment : public DragMethod_Base
{
public:
    DragMethod_PieSegment( DrawViewWrapper& rDrawViewWrapper, const OUString& rObjectCID
        , const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XModel >& xChartModel );
    virtual ~DragMethod_PieSegment();

    virtual void TakeSdrDragComment(OUString& rStr) const;
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
