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
    virtual void MoveSdrDrag(const basegfx::B2DPoint& rPnt);
    virtual bool EndSdrDrag(bool bCopy);

    virtual basegfx::B2DHomMatrix getCurrentTransformation();

protected:
    virtual void createSdrDragEntries();

private:
    ::basegfx::B2DPoint     m_aStartPoint;
    double                  m_fInitialOffset;
    double                  m_fAdditionalOffset;
    ::basegfx::B2DVector    m_aDragDirection;
    double                  m_fDragRange;
};

} //  namespace chart

// CHART2_DRAGMETHOD_PIESEGMENT_HXX
#endif
