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

    //----------------------
    //pMarkHandleProvider can be NULL; ownership is not taken
    void setMarkHandleProvider( MarkHandleProvider* pMarkHandleProvider );
    void CompleteRedraw(OutputDevice* pOut, const Region& rReg, sdr::contact::ViewObjectContactRedirector* pRedirector = 0);

    SdrObject*   getSelectedObject() const;
    SdrObject*   getTextEditObject() const;
    SdrOutliner* getOutliner() const;

    SfxItemSet   getPositionAndSizeItemSetFromMarkedObject() const;

    SdrObject* getNamedSdrObject( const rtl::OUString& rName ) const;
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

//.............................................................................
} //namespace chart
//.............................................................................
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
