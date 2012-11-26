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


#ifndef _CHART2_DRAW_VIEW_WRAPPER_HXX
#define _CHART2_DRAW_VIEW_WRAPPER_HXX

#include <svx/view3d.hxx>
#include <com/sun/star/drawing/XShape.hpp>
#include <com/sun/star/frame/XModel.hpp>

class SdrModel;

//.............................................................................
namespace chart
{
//.............................................................................

//-----------------------------------------------------------------------------
/** The DrawViewWrapper should help us to reduce effort if the underlying DrawingLayer changes.
Another task is to hide functionality we do not need, for example more than one page.
*/

class MarkHandleProvider
{
public:
    virtual bool getMarkHandles( SdrHdlList& rHdlList ) =0;
    virtual bool getFrameDragSingles() =0;
};

class DrawViewWrapper : public E3dView
{
public:
    DrawViewWrapper(SdrModel& rModel, OutputDevice* pOut, bool bPaintPageForEditMode);
    virtual ~DrawViewWrapper();

    //triggers the use of an updated first page
    void    ReInit();

    /// tries to get an OutputDevice from the XParent of the model to use as reference device
    void attachParentReferenceDevice(
        const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XModel > & xChartModel );

    //fill list of selection handles 'maViewHandleList'
    virtual void SetMarkHandles();

    SdrPageView*    GetPageView() const;

    SdrObject* getHitObject( const basegfx::B2DPoint& rPnt ) const;
    void MarkObject( SdrObject* pObj );

    //----------------------
    //pMarkHandleProvider can be NULL; ownership is not taken
    void setMarkHandleProvider( MarkHandleProvider* pMarkHandleProvider );
    void CompleteRedraw(OutputDevice* pOut, const Region& rReg, sdr::contact::ViewObjectContactRedirector* pRedirector = 0);

    SdrObject*   getTextEditObject() const;
    SdrOutliner* getOutliner() const;

    SfxItemSet   getPositionAndSizeItemSetFromMarkedObject() const;

    SdrObject* getNamedSdrObject( const rtl::OUString& rName ) const;
    bool IsObjectHit( SdrObject* pObj, const basegfx::B2DPoint& rPnt ) const;

    virtual void Notify(SfxBroadcaster& rBC, const SfxHint& rHint);

    static SdrObject* getSdrObject( const ::com::sun::star::uno::Reference<
                    ::com::sun::star::drawing::XShape >& xShape );

private:
    mutable MarkHandleProvider*     m_pMarkHandleProvider;

    ::std::auto_ptr< SdrOutliner >  m_apOutliner;

    // #i79965# scroll back view when ending text edit
    bool m_bRestoreMapMode;
    MapMode m_aMapModeToRestore;
};

//.............................................................................
} //namespace chart
//.............................................................................
#endif

