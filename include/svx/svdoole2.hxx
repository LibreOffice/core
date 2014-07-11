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

#ifndef INCLUDED_SVX_SVDOOLE2_HXX
#define INCLUDED_SVX_SVDOOLE2_HXX

#include <svtools/embedhlp.hxx>
#include <com/sun/star/uno/Reference.h>
#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/datatransfer/XTransferable.hpp>
#include <com/sun/star/awt/XWindow.hpp>
#include <svx/svdorect.hxx>
#include <vcl/graph.hxx>
#include <vcl/gdimtf.hxx>
#include <sot/storage.hxx>
#include <svx/svxdllapi.h>


//   SdrOle2Obj


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
    virtual sdr::contact::ViewContact* CreateObjectSpecificViewContact() SAL_OVERRIDE;
    virtual sdr::properties::BaseProperties* CreateObjectSpecificProperties() SAL_OVERRIDE;

    svt::EmbeddedObjectRef      xObjRef;
    Graphic*                    pGraphic;
    OUString                    aProgName;

    // Due to compatibility at SdrTextObj for now
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
    OUString GetStyleString();
    TYPEINFO_OVERRIDE();

    SdrOle2Obj(bool bFrame_ = false);
    SdrOle2Obj(const svt::EmbeddedObjectRef& rNewObjRef, const OUString& rNewObjName, const Rectangle& rNewRect, bool bFrame_ = false);
    virtual ~SdrOle2Obj();

    // access to svt::EmbeddedObjectRef
    const svt::EmbeddedObjectRef& getEmbeddedObjectRef() const { return xObjRef; }

    sal_Int64 GetAspect() const { return xObjRef.GetViewAspect(); }
    bool isInplaceActive() const;
    bool isUiActive() const;
    void SetAspect( sal_Int64 nAspect );

    // An OLE graphic object can contain a StarView graphic,
    // which will be displayed if the OLE object is empty.
    void        SetGraphic(const Graphic* pGrf);
    const       Graphic* GetGraphic() const;
    void        GetNewReplacement();

    // The original size of the object (size of the icon for iconified object)
    // no conversion is done if no target mode is provided
    Size        GetOrigObjSize( MapMode* pTargetMapMode = NULL ) const;

    // #i118524# Allow suppress SetVisAreaSize in changing methods when call
    // comes from OLE client
    void setSuppressSetVisAreaSize(bool bNew) { mbSuppressSetVisAreaSize = bNew; }

    // OLE object has got a separate PersistName member now;
    // !!! use ::SetPersistName( ... ) only, if you know what you do !!!
    OUString      GetPersistName() const;
    void        SetPersistName( const OUString& rPersistName );

    // One can add an application name to a SdrOle2Obj, which can be queried for
    // later on (SD needs this for presentation objects).
    void SetProgName(const OUString& rNam) { aProgName=rNam; }
    const OUString& GetProgName() const { return aProgName; }
    bool IsEmpty() const;

    void SetObjRef(const com::sun::star::uno::Reference < com::sun::star::embed::XEmbeddedObject >& rNewObjRef);
    com::sun::star::uno::Reference < com::sun::star::embed::XEmbeddedObject > GetObjRef() const;

    SVX_DLLPRIVATE com::sun::star::uno::Reference < com::sun::star::embed::XEmbeddedObject > GetObjRef_NoInit() const;

    void AbandonObject();

    virtual void SetPage(SdrPage* pNewPage) SAL_OVERRIDE;
    virtual void SetModel(SdrModel* pModel) SAL_OVERRIDE;

    /** Change the IsClosedObj attribute

        @param bIsClosed
        Whether the OLE object is closed, i.e. has opaque background
     */
    void SetClosedObj( bool bIsClosed );

    // FullDrag support
    virtual SdrObject* getFullDragClone() const SAL_OVERRIDE;

    virtual void TakeObjInfo(SdrObjTransformInfoRec& rInfo) const SAL_OVERRIDE;
    virtual sal_uInt16 GetObjIdentifier() const SAL_OVERRIDE;
    virtual OUString TakeObjNameSingul() const SAL_OVERRIDE;
    virtual OUString TakeObjNamePlural() const SAL_OVERRIDE;

    virtual SdrOle2Obj* Clone() const SAL_OVERRIDE;
    virtual SdrOle2Obj* CloneWithShellIDs( const OUString& rSrcShellID, const OUString& rDestShellID ) const SAL_OVERRIDE;

    SdrOle2Obj& assignFrom( const SdrOle2Obj& rObj, const OUString& rSrcShellID, const OUString& rDestShellID );
    SdrOle2Obj& operator=(const SdrOle2Obj& rObj);

    virtual void NbcMove(const Size& rSize) SAL_OVERRIDE;
    virtual void NbcResize(const Point& rRef, const Fraction& xFact, const Fraction& yFact) SAL_OVERRIDE;
    virtual void NbcSetSnapRect(const Rectangle& rRect) SAL_OVERRIDE;
    virtual void NbcSetLogicRect(const Rectangle& rRect) SAL_OVERRIDE;
    virtual void SetGeoData(const SdrObjGeoData& rGeo) SAL_OVERRIDE;

    static bool CanUnloadRunningObj( const ::com::sun::star::uno::Reference< ::com::sun::star::embed::XEmbeddedObject >& xObj,
                                         sal_Int64 nAspect );
    static bool Unload( const ::com::sun::star::uno::Reference< ::com::sun::star::embed::XEmbeddedObject >& xObj, sal_Int64 nAspect );
    bool Unload();
    void Connect();
    void Disconnect();
    void ObjectLoaded();

    ::com::sun::star::uno::Reference< ::com::sun::star::frame::XModel > getXModel() const;

    // #109985#
    bool IsChart() const;
    bool IsCalc() const;

    bool UpdateLinkURL_Impl();
    void BreakFileLink_Impl();
    void DisconnectFileLink_Impl();
    void CheckFileLink_Impl();

    // allows to transfer the graphics to the object helper
    void SetGraphicToObj( const Graphic& aGraphic, const OUString& aMediaType );
    void SetGraphicToObj( const ::com::sun::star::uno::Reference< ::com::sun::star::io::XInputStream >& xGrStream,
                          const OUString& aMediaType );

    ::com::sun::star::uno::Reference< ::com::sun::star::frame::XModel > GetParentXModel()  const;
    bool CalculateNewScaling( Fraction& aScaleWidth, Fraction& aScaleHeight, Size& aObjAreaSize );
    bool AddOwnLightClient();

    // handy to get the empty replacement graphic without accessing all the old stuff
    static Graphic GetEmptyOLEReplacementGraphic();

    void SetWindow(const com::sun::star::uno::Reference < com::sun::star::awt::XWindow >& _xWindow);

    // #i118485# missing converter added
    virtual SdrObject* DoConvertToPolyObj(bool bBezier, bool bAddText) const SAL_OVERRIDE;
};

#endif // INCLUDED_SVX_SVDOOLE2_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
