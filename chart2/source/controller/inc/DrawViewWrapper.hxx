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
#ifndef _CHART2_DRAW_VIEW_WRAPPER_HXX
#define _CHART2_DRAW_VIEW_WRAPPER_HXX

#include <svx/view3d.hxx>
#include <com/sun/star/drawing/XShape.hpp>
#include <com/sun/star/frame/XModel.hpp>

class SdrModel;

namespace chart
{

/** The DrawViewWrapper should help us to reduce effort if the underlying DrawingLayer changes.
Another task is to hide functionality we do not need, for example more than one page.
*/

class MarkHandleProvider
{
public:
    virtual bool getMarkHandles( SdrHdlList& rHdlList ) =0;
    virtual bool getFrameDragSingles() =0;

protected:
    ~MarkHandleProvider() {}
};

class DrawViewWrapper : public E3dView
{
public:
    DrawViewWrapper(SdrModel* pModel, OutputDevice* pOut, bool bPaintPageForEditMode);
    virtual ~DrawViewWrapper();

    //triggers the use of an updated first page
    void    ReInit();

    /// tries to get an OutputDevice from the XParent of the model to use as reference device
    void attachParentReferenceDevice(
        const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XModel > & xChartModel );

    //fill list of selection handles 'aHdl'
    virtual void SetMarkHandles();

    SdrPageView*    GetPageView() const;

    SdrObject* getHitObject( const Point& rPnt ) const;
    //sal_Bool PickObj(const Point& rPnt, short nTol, SdrObject*& rpObj, SdrPageView*& rpPV, sal_uLong nOptions, SdrObject** ppRootObj, sal_uLong* pnMarkNum=NULL, sal_uInt16* pnPassNum=NULL) const;
    //sal_Bool PickObj(const Point& rPnt, short nTol, SdrObject*& rpObj, SdrPageView*& rpPV, sal_uLong nOptions=0) const;
    //sal_Bool PickObj(const Point& rPnt, SdrObject*& rpObj, SdrPageView*& rpPV, sal_uLong nOptions=0) const { return PickObj(rPnt,nHitTolLog,rpObj,rpPV,nOptions); }

    //void MarkObj(SdrObject* pObj, SdrPageView* pPV, sal_Bool bUnmark=sal_False, sal_Bool bImpNoSetMarkHdl=sal_False);
    void MarkObject( SdrObject* pObj );

    //pMarkHandleProvider can be NULL; ownership is not taken
    void setMarkHandleProvider( MarkHandleProvider* pMarkHandleProvider );
    void CompleteRedraw(OutputDevice* pOut, const Region& rReg, sdr::contact::ViewObjectContactRedirector* pRedirector = 0);

    SdrObject*   getSelectedObject() const;
    SdrObject*   getTextEditObject() const;
    SdrOutliner* getOutliner() const;

    SfxItemSet   getPositionAndSizeItemSetFromMarkedObject() const;

    SdrObject* getNamedSdrObject( const OUString& rName ) const;
    bool IsObjectHit( SdrObject* pObj, const Point& rPnt ) const;

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

} //namespace chart
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
