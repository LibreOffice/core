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
    bool                        bFrame : 1;
    bool                        bInDestruction : 1;

    // #i118524#
    bool                        mbSuppressSetVisAreaSize : 1;

    mutable bool                m_bTypeAsked;
    mutable bool                m_bChart;

    SdrOle2ObjImpl*             mpImpl;

    SvxUnoShapeModifyListener*  pModifyListener;

    void ImpSetVisAreaSize();
    void Init();
    virtual ~SdrOle2Obj();

    /// method to copy all data from given source
    virtual void copyDataFromSdrObject(const SdrObject& rSource);

public:
    /// create a copy, evtl. with a different target model (if given)
    virtual SdrObject* CloneSdrObject(SdrModel* pTargetModel = 0) const;

//IAccessibility2 Implementation 2009-----
    String GetStyleString();
//-----IAccessibility2 Implementation 2009

    // react on model/page change
    virtual void handlePageChange(SdrPage* pOldPage, SdrPage* pNewPage);
    virtual bool IsClosedObj() const;

    SdrOle2Obj(
        SdrModel& rSdrModel,
        const svt::EmbeddedObjectRef& rNewObjRef = svt::EmbeddedObjectRef(),
        const String aNewObjName = String(),
        const basegfx::B2DHomMatrix& rTransform = basegfx::B2DHomMatrix(),
        bool bFrame_ = false);

    // access to svt::EmbeddedObjectRef
    const svt::EmbeddedObjectRef& getEmbeddedObjectRef() const { return xObjRef; }

    sal_Int64 GetAspect() const { return xObjRef.GetViewAspect(); }
    bool isInplaceActive() const;
    bool isUiActive() const;
    void SetAspect( sal_Int64 nAspect );

    // Ein OLE-Zeichenobjekt kann eine StarView-Grafik beinhalten.
    // Diese wird angezeigt, wenn das OLE-Objekt leer ist.
    void        SetGraphic(const Graphic* pGrf);
    Graphic*    GetGraphic() const;
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

    // FullDrag support
    virtual SdrObject* getFullDragClone() const;

    virtual void TakeObjInfo(SdrObjTransformInfoRec& rInfo) const;
    virtual sal_uInt16 GetObjIdentifier() const;
    virtual void TakeObjNameSingul(String& rName) const;
    virtual void TakeObjNamePlural(String& rName) const;

    virtual void SetGeoData(const SdrObjGeoData& rGeo);

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
    void SetGraphicToObj( const Graphic& aGraphic, const ::rtl::OUString& aMediaType );
    void SetGraphicToObj( const ::com::sun::star::uno::Reference< ::com::sun::star::io::XInputStream >& xGrStream,
                          const ::rtl::OUString& aMediaType );

    ::com::sun::star::uno::Reference< ::com::sun::star::frame::XModel > GetParentXModel()  const;
    bool CalculateNewScaling( Fraction& aScaleWidth, Fraction& aScaleHeight, Size& aObjAreaSize );
    bool AddOwnLightClient();

    // handy to get the empty replacement bitmap without accessing all the old stuff
    static Bitmap GetEmtyOLEReplacementBitmap();

    void SetWindow(const com::sun::star::uno::Reference < com::sun::star::awt::XWindow >& _xWindow);
    virtual void setSdrObjectTransformation(const basegfx::B2DHomMatrix& rTransformation);

    // #i118485# missing converter added
    virtual SdrObject* DoConvertToPolygonObject(bool bBezier, bool bAddText) const;
};

#endif //_SVDOOLE2_HXX

