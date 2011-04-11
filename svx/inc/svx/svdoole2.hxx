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

#ifndef _SVDOOLE2_HXX
#define _SVDOOLE2_HXX

#include <svtools/embedhlp.hxx>
#include <com/sun/star/uno/Reference.h>
#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/datatransfer/XTransferable.hpp>
#include "com/sun/star/awt/XWindow.hpp"
#include <svx/svdorect.hxx>
#include <vcl/graph.hxx>
#include <vcl/gdimtf.hxx>
#include <sot/storage.hxx>
#include "svx/svxdllapi.h"

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
    SVX_DLLPRIVATE void SetGraphic_Impl(const Graphic* pGrf);

protected:
    virtual sdr::contact::ViewContact* CreateObjectSpecificViewContact();
    virtual sdr::properties::BaseProperties* CreateObjectSpecificProperties();

    svt::EmbeddedObjectRef      xObjRef;
    Graphic*                    pGraphic;
    String                      aProgName;

    // wg. Kompatibilitaet erstmal am SdrTextObj
    sal_Bool                        bFrame : 1;
    sal_Bool                        bInDestruction : 1;
    mutable bool                m_bTypeAsked;
    mutable bool                m_bChart;

    SdrOle2ObjImpl*             mpImpl;

    SvxUnoShapeModifyListener*  pModifyListener;

protected:

    void ImpSetVisAreaSize();
    void Init();

public:
    TYPEINFO();

    SdrOle2Obj(bool bFrame_ = false);
    SdrOle2Obj(const svt::EmbeddedObjectRef& rNewObjRef, bool bFrame_ = false);
    SdrOle2Obj(const svt::EmbeddedObjectRef& rNewObjRef, const String& rNewObjName, bool bFrame_ = false);
    SdrOle2Obj(const svt::EmbeddedObjectRef& rNewObjRef, const String& rNewObjName, const Rectangle& rNewRect, bool bFrame_ = false);
    virtual ~SdrOle2Obj();

    // access to svt::EmbeddedObjectRef
    const svt::EmbeddedObjectRef& getEmbeddedObjectRef() const { return xObjRef; }

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
    bool IsEmpty() const;

    void SetObjRef(const com::sun::star::uno::Reference < com::sun::star::embed::XEmbeddedObject >& rNewObjRef);
    com::sun::star::uno::Reference < com::sun::star::embed::XEmbeddedObject > GetObjRef() const;

    SVX_DLLPRIVATE com::sun::star::uno::Reference < com::sun::star::embed::XEmbeddedObject > GetObjRef_NoInit() const;

    void AbandonObject();

    virtual void SetPage(SdrPage* pNewPage);
    virtual void SetModel(SdrModel* pModel);

    /** Change the IsClosedObj attribute

        @param bIsClosed
        Whether the OLE object is closed, i.e. has opaque background
     */
    void SetClosedObj( bool bIsClosed );

    // FullDrag support
    virtual SdrObject* getFullDragClone() const;

    virtual void TakeObjInfo(SdrObjTransformInfoRec& rInfo) const;
    virtual sal_uInt16 GetObjIdentifier() const;
    virtual void TakeObjNameSingul(String& rName) const;
    virtual void TakeObjNamePlural(String& rName) const;

    SdrOle2Obj* Clone() const;
    SdrOle2Obj& operator=(const SdrOle2Obj& rObj);

    virtual void NbcMove(const Size& rSize);
    virtual void NbcResize(const Point& rRef, const Fraction& xFact, const Fraction& yFact);
    virtual void NbcSetSnapRect(const Rectangle& rRect);
    virtual void NbcSetLogicRect(const Rectangle& rRect);
    virtual void SetGeoData(const SdrObjGeoData& rGeo);

    static sal_Bool CanUnloadRunningObj( const ::com::sun::star::uno::Reference< ::com::sun::star::embed::XEmbeddedObject >& xObj,
                                         sal_Int64 nAspect );
    static sal_Bool Unload( const ::com::sun::star::uno::Reference< ::com::sun::star::embed::XEmbeddedObject >& xObj, sal_Int64 nAspect );
    sal_Bool Unload();
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

    ::com::sun::star::uno::Reference< ::com::sun::star::frame::XModel > GetParentXModel()  const;
    sal_Bool CalculateNewScaling( Fraction& aScaleWidth, Fraction& aScaleHeight, Size& aObjAreaSize );
    sal_Bool AddOwnLightClient();

    // handy to get the empty replacement bitmap without accessing all the old stuff
    static Bitmap GetEmtyOLEReplacementBitmap();

    void SetWindow(const com::sun::star::uno::Reference < com::sun::star::awt::XWindow >& _xWindow);
};

#endif //_SVDOOLE2_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
