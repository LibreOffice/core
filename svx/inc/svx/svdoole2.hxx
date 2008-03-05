/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: svdoole2.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: kz $ $Date: 2008-03-05 17:00:19 $
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

#ifndef _SVDOOLE2_HXX
#define _SVDOOLE2_HXX

#include <svtools/embedhlp.hxx>

#ifndef _COM_SUN_STAR_UNO_REFERENCE_H_
#include <com/sun/star/uno/Reference.h>
#endif
#ifndef _COM_SUN_STAR_FRAME_XMODEL_HPP_
#include <com/sun/star/frame/XModel.hpp>
#endif
#ifndef _COM_SUN_STAR_DATATRANSFER_XTRANSFERABLE_HPP_
#include <com/sun/star/datatransfer/XTransferable.hpp>
#endif

#ifndef _SVDORECT_HXX
#include <svx/svdorect.hxx>
#endif
#ifndef _GRAPH_HXX //autogen
#include <vcl/graph.hxx>
#endif
#ifndef _GDIMTF_HXX //autogen
#include <vcl/gdimtf.hxx>
#endif
#ifndef _SOT_STORAGE_HXX
#include <sot/storage.hxx>
#endif

#ifndef INCLUDED_SVXDLLAPI_H
#include "svx/svxdllapi.h"
#endif

//************************************************************
//   SdrOle2Obj
//************************************************************

class SvxUnoShapeModifyListener;
class SdrOle2ObjImpl;

class SVX_DLLPUBLIC SdrOle2Obj :  public SdrRectObj
{
private:

    SVX_DLLPRIVATE void Connect_Impl();
    SVX_DLLPRIVATE void Disconnect_Impl();
    SVX_DLLPRIVATE void Reconnect_Impl();
    SVX_DLLPRIVATE void AddListeners_Impl();
    SVX_DLLPRIVATE void RemoveListeners_Impl();
    SVX_DLLPRIVATE ::com::sun::star::uno::Reference < ::com::sun::star::datatransfer::XTransferable > GetTransferable_Impl() const;
    SVX_DLLPRIVATE void GetObjRef_Impl();
    SVX_DLLPRIVATE void PaintGraphic_Impl(XOutputDevice& rOut, const SdrPaintInfoRec& rInfoRec, sal_Bool bActive = sal_False ) const;
    SVX_DLLPRIVATE void SetGraphic_Impl(const Graphic* pGrf);

protected:
    svt::EmbeddedObjectRef      xObjRef;
    Graphic*                    pGraphic;
    String                      aProgName;

    // wg. Kompatibilitaet erstmal am SdrTextObj
    BOOL                        bFrame : 1;
    BOOL                        bInDestruction : 1;

    SdrOle2ObjImpl*             mpImpl;

    SvxUnoShapeModifyListener*  pModifyListener;

protected:

    void ImpSetVisAreaSize();
    void Init();

public:
    TYPEINFO();

    SdrOle2Obj(FASTBOOL bFrame_=FALSE);
    SdrOle2Obj(const svt::EmbeddedObjectRef& rNewObjRef, FASTBOOL bFrame_=FALSE);
    SdrOle2Obj(const svt::EmbeddedObjectRef& rNewObjRef, const String& rNewObjName, FASTBOOL bFrame_=FALSE);
    SdrOle2Obj(const svt::EmbeddedObjectRef& rNewObjRef, const String& rNewObjName, const Rectangle& rNewRect, FASTBOOL bFrame_=FALSE);
    virtual ~SdrOle2Obj();

    sal_Int64 GetAspect() const { return xObjRef.GetViewAspect(); }
    void SetAspect( sal_Int64 nAspect );

    // Ein OLE-Zeichenobjekt kann eine StarView-Grafik beinhalten.
    // Diese wird angezeigt, wenn das OLE-Objekt leer ist.
    void        SetGraphic(const Graphic* pGrf);
    Graphic*    GetGraphic() const;
    void        GetNewReplacement();

    // the original size of the object ( size of the icon for iconified object )
    // no conversion is done if no target mode is provided
    Size        GetOrigObjSize( MapMode* pTargetMapMode = NULL ) const;


    // OLE object has got a separate PersistName member now;
    // !!! use ::SetPersistName( ... ) only, if you know what you do !!!
    String      GetPersistName() const;
    void        SetPersistName( const String& rPersistName );

    // Einem SdrOle2Obj kann man ein Applikationsnamen verpassen, den man
    // spaeter wieder abfragen kann (SD braucht das fuer Praesentationsobjekte).
    void SetProgName(const String& rNam) { aProgName=rNam; }
    const String& GetProgName() const { return aProgName; }
    FASTBOOL IsEmpty() const;

    void SetObjRef(const com::sun::star::uno::Reference < com::sun::star::embed::XEmbeddedObject >& rNewObjRef);
    com::sun::star::uno::Reference < com::sun::star::embed::XEmbeddedObject > GetObjRef() const;

    void AbandonObject();

    virtual void SetPage(SdrPage* pNewPage);
    virtual void SetModel(SdrModel* pModel);

    /** Change the IsClosedObj attribute

        @param bIsClosed
        Whether the OLE object is closed, i.e. has opaque background
     */
    void SetClosedObj( bool bIsClosed );

    virtual void TakeObjInfo(SdrObjTransformInfoRec& rInfo) const;
    virtual UINT16 GetObjIdentifier() const;
    virtual sal_Bool DoPaintObject(XOutputDevice& rOut, const SdrPaintInfoRec& rInfoRec) const;
    virtual SdrObject* CheckHit(const Point& rPnt, USHORT nTol, const SetOfByte* pVisiLayer) const;
    virtual void TakeObjNameSingul(String& rName) const;
    virtual void TakeObjNamePlural(String& rName) const;

    virtual void operator=(const SdrObject& rObj);
    virtual FASTBOOL HasSpecialDrag() const;

    virtual void NbcMove(const Size& rSize);
    virtual void NbcResize(const Point& rRef, const Fraction& xFact, const Fraction& yFact);
    virtual void NbcSetSnapRect(const Rectangle& rRect);
    virtual void NbcSetLogicRect(const Rectangle& rRect);
    virtual void SetGeoData(const SdrObjGeoData& rGeo);

    static sal_Bool Unload( const ::com::sun::star::uno::Reference< ::com::sun::star::embed::XEmbeddedObject >& xObj, sal_Int64 nAspect );
    BOOL Unload();
    void Connect();
    void Disconnect();
    void ObjectLoaded();

    ::com::sun::star::uno::Reference< ::com::sun::star::frame::XModel > getXModel() const;

    // #109985#
    sal_Bool IsChart() const;
    sal_Bool IsCalc() const;

    sal_Bool UpdateLinkURL_Impl();
    void BreakFileLink_Impl();
    void DisconnectFileLink_Impl();
    void CheckFileLink_Impl();

    // allows to transfer the graphics to the object helper
    void SetGraphicToObj( const Graphic& aGraphic, const ::rtl::OUString& aMediaType );
    void SetGraphicToObj( const ::com::sun::star::uno::Reference< ::com::sun::star::io::XInputStream >& xGrStream,
                          const ::rtl::OUString& aMediaType );

    ::com::sun::star::uno::Reference< ::com::sun::star::frame::XModel > GetParentXModel();
    sal_Bool CalculateNewScaling( Fraction& aScaleWidth, Fraction& aScaleHeight, Size& aObjAreaSize );
    sal_Bool AddOwnLightClient();
};

#endif //_SVDOOLE2_HXX

