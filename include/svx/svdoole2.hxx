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

    // #i118485# helper added
    SdrObject* createSdrGrafObjReplacement(bool bAddText, bool bUseHCGraphic) const;

protected:
    virtual sdr::contact::ViewContact* CreateObjectSpecificViewContact();
    virtual sdr::properties::BaseProperties* CreateObjectSpecificProperties();

    svt::EmbeddedObjectRef      xObjRef;
    Graphic*                    pGraphic;
    String                      aProgName;

    // wg. Kompatibilitaet erstmal am SdrTextObj
    bool                        bFrame:1;
    bool                        bInDestruction:1;
    // #i118524#
    bool                        mbSuppressSetVisAreaSize:1;
    mutable bool                m_bTypeAsked:1;
    mutable bool                m_bChart:1;

    SdrOle2ObjImpl*             mpImpl;

    SvxUnoShapeModifyListener*  pModifyListener;

protected:

    void ImpSetVisAreaSize();
    void Init();

public:
    TYPEINFO();

    SdrOle2Obj(bool bFrame_ = false);
    SdrOle2Obj(const svt::EmbeddedObjectRef& rNewObjRef, const String& rNewObjName, const Rectangle& rNewRect, bool bFrame_ = false);
    virtual ~SdrOle2Obj();

    // access to svt::EmbeddedObjectRef
    const svt::EmbeddedObjectRef& getEmbeddedObjectRef() const { return xObjRef; }

    sal_Int64 GetAspect() const { return xObjRef.GetViewAspect(); }
    bool isInplaceActive() const;
    bool isUiActive() const;
    void SetAspect( sal_Int64 nAspect );

    // Ein OLE-Zeichenobjekt kann eine StarView-Grafik beinhalten.
    // Diese wird angezeigt, wenn das OLE-Objekt leer ist.
    void        SetGraphic(const Graphic* pGrf);
    const Graphic* GetGraphic() const;
    void        GetNewReplacement();

    // the original size of the object ( size of the icon for iconified object )
    // no conversion is done if no target mode is provided
    Size        GetOrigObjSize( MapMode* pTargetMapMode = NULL ) const;

    // #i118524# Allow suppress SetVisAreaSize in changing methods when call
    // comes from OLE client
    void setSuppressSetVisAreaSize(bool bNew) { mbSuppressSetVisAreaSize = bNew; }

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
    virtual OUString TakeObjNameSingul() const;
    virtual OUString TakeObjNamePlural() const;

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
    void SetGraphicToObj( const Graphic& aGraphic, const OUString& aMediaType );
    void SetGraphicToObj( const ::com::sun::star::uno::Reference< ::com::sun::star::io::XInputStream >& xGrStream,
                          const OUString& aMediaType );

    ::com::sun::star::uno::Reference< ::com::sun::star::frame::XModel > GetParentXModel()  const;
    sal_Bool CalculateNewScaling( Fraction& aScaleWidth, Fraction& aScaleHeight, Size& aObjAreaSize );
    sal_Bool AddOwnLightClient();

    // handy to get the empty replacement graphic without accessing all the old stuff
    static Graphic GetEmptyOLEReplacementGraphic();

    void SetWindow(const com::sun::star::uno::Reference < com::sun::star::awt::XWindow >& _xWindow);

    // #i118485# missing converter added
    virtual SdrObject* DoConvertToPolyObj(sal_Bool bBezier, bool bAddText) const;
};

#endif //_SVDOOLE2_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
