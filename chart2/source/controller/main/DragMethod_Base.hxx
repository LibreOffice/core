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


#ifndef CHART2_DRAGMETHOD_BASE_HXX
#define CHART2_DRAGMETHOD_BASE_HXX

#include "DrawViewWrapper.hxx"
#include <svx/ActionDescriptionProvider.hxx>
// header for class SdrDragMethod
#include <svx/svddrgmt.hxx>

// header for class WeakReference
#include <cppuhelper/weakref.hxx>

namespace chart
{

class DragMethod_Base : public SdrDragMethod
{
public:
    DragMethod_Base( DrawViewWrapper& rDrawViewWrapper, const rtl::OUString& rObjectCID
        , const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XModel >& xChartModel
        , ActionDescriptionProvider::ActionType eActionType = ActionDescriptionProvider::MOVE );
    virtual ~DragMethod_Base();

    virtual rtl::OUString getUndoDescription() const;

    virtual void TakeSdrDragComment(String& rStr) const;
    virtual Pointer GetSdrDragPointer() const;

protected:
    ::com::sun::star::uno::Reference< ::com::sun::star::frame::XModel > getChartModel() const;

protected:
    DrawViewWrapper&    m_rDrawViewWrapper;
    rtl::OUString       m_aObjectCID;
    ActionDescriptionProvider::ActionType m_eActionType;

private:
    ::com::sun::star::uno::WeakReference< ::com::sun::star::frame::XModel > m_xChartModel;
};

} //  namespace chart

// CHART2_DRAGMETHOD_BASE_HXX
#endif
