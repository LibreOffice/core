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

#include <svx/svdorect.hxx>
#include <svx/svxdllapi.h>

#include <com/sun/star/uno/Reference.h>

namespace com { namespace sun { namespace star {

namespace awt { class XWindow; }

namespace datatransfer { class XTransferable; }

namespace embed { class XEmbeddedObject; }

namespace frame { class XModel; }

}}}

namespace svt { class EmbeddedObjectRef; }

class SdrOle2ObjImpl;

class SVX_DLLPUBLIC SdrOle2Obj : public SdrRectObj
{
private:
    SdrOle2ObjImpl* mpImpl;

private:
    SVX_DLLPRIVATE void Connect_Impl();
    SVX_DLLPRIVATE void Disconnect_Impl();
    SVX_DLLPRIVATE void Reconnect_Impl();
    SVX_DLLPRIVATE void AddListeners_Impl();
    SVX_DLLPRIVATE void RemoveListeners_Impl();
    SVX_DLLPRIVATE void GetObjRef_Impl();
    SVX_DLLPRIVATE void SetGraphic_Impl(const Graphic* pGrf);

    // #i118485# helper added
    SVX_DLLPRIVATE SdrObject* createSdrGrafObjReplacement(bool bAddText, bool bUseHCGraphic) const;
    SVX_DLLPRIVATE void ImpSetVisAreaSize();

protected:
    virtual sdr::contact::ViewContact* CreateObjectSpecificViewContact() override;
    virtual sdr::properties::BaseProperties* CreateObjectSpecificProperties() override;

public:
    OUString GetStyleString();

    SdrOle2Obj( bool bFrame_ = false );
    SdrOle2Obj( const svt::EmbeddedObjectRef& rNewObjRef, const OUString& rNewObjName, const Rectangle& rNewRect );
    virtual ~SdrOle2Obj();

    const svt::EmbeddedObjectRef& getEmbeddedObjectRef() const;

    sal_Int64 GetAspect() const;
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
    Size        GetOrigObjSize( MapMode* pTargetMapMode = nullptr ) const;

    // #i118524# Allow suppress SetVisAreaSize in changing methods when call
    // comes from OLE client
    void setSuppressSetVisAreaSize( bool bNew );

    // OLE object has got a separate PersistName member now;
    // !!! use ::SetPersistName( ... ) only, if you know what you do !!!
    const OUString& GetPersistName() const;
    void        SetPersistName( const OUString& rPersistName );

    // One can add an application name to a SdrOle2Obj, which can be queried for
    // later on (SD needs this for presentation objects).
    void SetProgName( const OUString& rName );
    const OUString& GetProgName() const;
    bool IsEmpty() const;

    void SetObjRef(const css::uno::Reference < css::embed::XEmbeddedObject >& rNewObjRef);
    css::uno::Reference < css::embed::XEmbeddedObject > GetObjRef() const;

    SVX_DLLPRIVATE css::uno::Reference < css::embed::XEmbeddedObject > GetObjRef_NoInit() const;

    void AbandonObject();

    virtual void SetPage(SdrPage* pNewPage) override;
    virtual void SetModel(SdrModel* pModel) override;

    /** Change the IsClosedObj attribute

        @param bIsClosed
        Whether the OLE object is closed, i.e. has opaque background
     */
    void SetClosedObj( bool bIsClosed );

    // FullDrag support
    virtual SdrObject* getFullDragClone() const override;

    virtual void TakeObjInfo(SdrObjTransformInfoRec& rInfo) const override;
    virtual sal_uInt16 GetObjIdentifier() const override;
    virtual OUString TakeObjNameSingul() const override;
    virtual OUString TakeObjNamePlural() const override;

    virtual SdrOle2Obj* Clone() const override;

    SdrOle2Obj& assignFrom(const SdrOle2Obj& rObj);
    SdrOle2Obj& operator=(const SdrOle2Obj& rObj);

    virtual void NbcMove(const Size& rSize) override;
    virtual void NbcResize(const Point& rRef, const Fraction& xFact, const Fraction& yFact) override;
    virtual void NbcSetSnapRect(const Rectangle& rRect) override;
    virtual void NbcSetLogicRect(const Rectangle& rRect) override;
    virtual void SetGeoData(const SdrObjGeoData& rGeo) override;

    static bool CanUnloadRunningObj( const css::uno::Reference< css::embed::XEmbeddedObject >& xObj,
                                         sal_Int64 nAspect );
    static bool Unload( const css::uno::Reference< css::embed::XEmbeddedObject >& xObj, sal_Int64 nAspect );
    bool Unload();
    void Connect();
    void Disconnect();
    void ObjectLoaded();

    css::uno::Reference< css::frame::XModel > getXModel() const;

    bool IsChart() const;
    bool IsReal3DChart() const;
    bool IsCalc() const;

    bool UpdateLinkURL_Impl();
    void BreakFileLink_Impl();
    void DisconnectFileLink_Impl();
    void CheckFileLink_Impl();

    // allows to transfer the graphics to the object helper
    void SetGraphicToObj( const Graphic& aGraphic, const OUString& aMediaType );
    void SetGraphicToObj( const css::uno::Reference< css::io::XInputStream >& xGrStream,
                          const OUString& aMediaType );

    css::uno::Reference< css::frame::XModel > GetParentXModel()  const;
    bool CalculateNewScaling( Fraction& aScaleWidth, Fraction& aScaleHeight, Size& aObjAreaSize );
    bool AddOwnLightClient();

    // handy to get the empty replacement graphic without accessing all the old stuff
    static Graphic GetEmptyOLEReplacementGraphic();

    void SetWindow(const css::uno::Reference < css::awt::XWindow >& _xWindow);

    // #i118485# missing converter added
    virtual SdrObject* DoConvertToPolyObj(bool bBezier, bool bAddText) const override;
};

#endif // INCLUDED_SVX_SVDOOLE2_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
