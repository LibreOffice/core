/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: DrawViewWrapper.hxx,v $
 *
 *  $Revision: 1.10 $
 *
 *  last change: $Author: ihi $ $Date: 2008-01-14 13:57:44 $
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
#ifndef _CHART2_DRAW_VIEW_WRAPPER_HXX
#define _CHART2_DRAW_VIEW_WRAPPER_HXX

#ifndef _E3D_VIEW3D_HXX
#include <svx/view3d.hxx>
#endif

#ifndef _COM_SUN_STAR_DRAWING_XSHAPE_HPP_
#include <com/sun/star/drawing/XShape.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_XMODEL_HPP_
#include <com/sun/star/frame/XModel.hpp>
#endif

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
    //BOOL PickObj(const Point& rPnt, short nTol, SdrObject*& rpObj, SdrPageView*& rpPV, ULONG nOptions, SdrObject** ppRootObj, ULONG* pnMarkNum=NULL, USHORT* pnPassNum=NULL) const;
    //BOOL PickObj(const Point& rPnt, short nTol, SdrObject*& rpObj, SdrPageView*& rpPV, ULONG nOptions=0) const;
    //BOOL PickObj(const Point& rPnt, SdrObject*& rpObj, SdrPageView*& rpPV, ULONG nOptions=0) const { return PickObj(rPnt,nHitTolLog,rpObj,rpPV,nOptions); }

    //void MarkObj(SdrObject* pObj, SdrPageView* pPV, BOOL bUnmark=FALSE, BOOL bImpNoSetMarkHdl=FALSE);
    void MarkObject( SdrObject* pObj );

    //----------------------
    //pMarkHandleProvider can be NULL; ownership is not taken
    void setMarkHandleProvider( MarkHandleProvider* pMarkHandleProvider );
    void CompleteRedraw( OutputDevice* pOut, const Region& rReg, USHORT nPaintMode = 0,
                         ::sdr::contact::ViewObjectContactRedirector* pRedirector = 0L );

    SdrObject*   getSelectedObject() const;
    SdrObject*   getTextEditObject() const;
    SdrOutliner* getOutliner() const;

    SfxItemSet   getPositionAndSizeItemSetFromMarkedObject() const;

    SdrObject* getNamedSdrObject( const rtl::OUString& rName ) const;
    bool IsObjectHit( SdrObject* pObj, const Point& rPnt ) const;

    virtual void SFX_NOTIFY(SfxBroadcaster& rBC, const TypeId& rBCType, const SfxHint& rHint, const TypeId& rHintType);

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

