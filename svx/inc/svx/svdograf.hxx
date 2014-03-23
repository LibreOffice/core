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



#ifndef _SVDOGRAF_HXX
#define _SVDOGRAF_HXX

#include <com/sun/star/io/XInputStream.hpp>
#include <vcl/graph.hxx>
#include <svx/svdorect.hxx>
#include <svtools/grfmgr.hxx>
#include "svx/svxdllapi.h"

//////////////////////////////////////////////////////////////////////////////
// predeclarations

namespace sdr
{
    namespace properties
    {
        class GraphicProperties;
    } // end of namespace properties
    namespace contact
    {
        class ViewObjectContactOfGraphic;
    } // end of namespace contact
} // end of namespace sdr

// -----------
// - Defines -
// -----------

#define SDRGRAFOBJ_TRANSFORMATTR_NONE       0x00000000UL
#define SDRGRAFOBJ_TRANSFORMATTR_COLOR      0x00000001UL
#define SDRGRAFOBJ_TRANSFORMATTR_MIRROR     0x00000002UL
#define SDRGRAFOBJ_TRANSFORMATTR_ROTATE     0x00000004UL
#define SDRGRAFOBJ_TRANSFORMATTR_ALL        0xffffffffUL

// --------------
// - SdrGrafObj -
// --------------

class GraphicObject;
class SdrGraphicLink;

class SVX_DLLPUBLIC SdrGrafObj : public SdrRectObj
{
private:
    // to allow sdr::properties::GraphicProperties access to SetXPolyDirty()
    friend class sdr::properties::GraphicProperties;

    // to allow sdr::contact::ViewObjectContactOfGraphic access to ImpUpdateGraphicLink()
    friend class sdr::contact::ViewObjectContactOfGraphic;
    friend class SdrGraphicLink;

protected:
    virtual sdr::contact::ViewContact* CreateObjectSpecificViewContact();
    virtual sdr::properties::BaseProperties* CreateObjectSpecificProperties();

    void                    ImpSetAttrToGrafInfo(); // Werte vom Pool kopieren
    void                    ImpSetGrafInfoToAttr(); // Werte in den Pool kopieren
    GraphicAttr             aGrafInfo;

    Rectangle               aCropRect;          // Wenn aCropRect nicht Empty ist, dann enthaelt es den sichtbaren
                                                // Ausschnitt der Grafik in logischen Eingeiten der Grafik! Also Bitmap->=Pixel
    String                  aFileName;          // Wenn es sich um einen Link handelt, steht hier der Dateiname drin.
    String                  aFilterName;
    GraphicObject*          pGraphic;           // Zur Beschleunigung von Bitmapausgaben, besonders von gedrehten.
    GraphicObject*          mpReplacementGraphic;
    SdrGraphicLink*         pGraphicLink;       // Und hier noch ein Pointer fuer gelinkte Grafiken

    // #111096#
    // Flag for allowing text animation. Default is sal_true.
    bool                        mbGrafAnimationAllowed : 1;

    // #i25616#
    bool                        mbInsidePaint : 1;
    bool                        mbIsPreview   : 1;

    void                    ImpLinkAnmeldung();
    void                    ImpLinkAbmeldung();
    bool                    ImpUpdateGraphicLink( bool bAsynchron = true ) const;
    void                    ImpSetLinkedGraphic( const Graphic& rGraphic );
                            DECL_LINK( ImpSwapHdl, GraphicObject* );
    void onGraphicChanged();

    virtual                 ~SdrGrafObj();

    /// method to copy all data from given source
    virtual void copyDataFromSdrObject(const SdrObject& rSource);

public:
    /// create a copy, evtl. with a different target model (if given)
    virtual SdrObject* CloneSdrObject(SdrModel* pTargetModel = 0) const;

    // react on model/page change
    virtual void handlePageChange(SdrPage* pOldPage, SdrPage* pNewPage);

    SdrGrafObj(
        SdrModel& rSdrModel,
        const Graphic& rGrf,
        const basegfx::B2DHomMatrix& rTransform = basegfx::B2DHomMatrix());

//  virtual bool IsSdrGrafObj() const;
    virtual bool DoesSupportTextIndentingOnLineWidthChange() const;

    void                    SetGraphicObject( const GraphicObject& rGrfObj );
    const GraphicObject&    GetGraphicObject( bool bForceSwapIn = false) const;
    const GraphicObject*    GetReplacementGraphicObject() const;

    //void                  NbcSetGraphic(const Graphic& rGrf);
    void                    SetGraphic(const Graphic& rGrf);
    const Graphic&          GetGraphic() const;

    Graphic                 GetTransformedGraphic( sal_uIntPtr nTransformFlags = SDRGRAFOBJ_TRANSFORMATTR_ALL ) const;

    GraphicType             GetGraphicType() const;

    // #111096#
    // Keep ATM for SD.
    bool IsAnimated() const;
    bool IsEPS() const;
    bool IsSwappedOut() const;

    const MapMode&          GetGrafPrefMapMode() const;
    const Size&             GetGrafPrefSize() const;

    void                    SetGrafStreamURL( const String& rGraphicStreamURL );
    String                  GetGrafStreamURL() const;

    void                    ForceSwapIn() const;
    void                    ForceSwapOut() const;

    void                    SetGraphicLink(const String& rFileName, const String& rFilterName);
    void                    ReleaseGraphicLink();
    bool IsLinkedGraphic() const { return (bool)aFileName.Len(); }

    void                    SetFileName(const String& rFileName);
    const String&           GetFileName() const { return aFileName; }
    void                    SetFilterName(const String& rFilterName);
    const String&           GetFilterName() const { return aFilterName; }

    virtual void            TakeObjInfo(SdrObjTransformInfoRec& rInfo) const;
    virtual sal_uInt16          GetObjIdentifier() const;

    virtual void            TakeObjNameSingul(String& rName) const;
    virtual void            TakeObjNamePlural(String& rName) const;

    // #i25616#
    virtual basegfx::B2DPolyPolygon TakeXorPoly() const;

    virtual void AddToHdlList(SdrHdlList& rHdlList) const;

    bool                HasGDIMetaFile() const;
    const GDIMetaFile*      GetGDIMetaFile() const;

    virtual SdrObject*      DoConvertToPolygonObject(bool bBezier, bool bAddText) const;

    virtual void            AdjustToMaxRange( const basegfx::B2DRange& rMaxRange, bool bShrinkOnly = false );
    bool isEmbeddedSvg() const;
    GDIMetaFile getMetafileFromEmbeddedSvg() const;

    virtual void            Notify( SfxBroadcaster& rBC, const SfxHint& rHint );

    // #111096#
    // Access to GrafAnimationAllowed flag
    bool IsGrafAnimationAllowed() const;
    void SetGrafAnimationAllowed(bool bNew);

    // #i25616#
    bool IsObjectTransparent() const;

    ::com::sun::star::uno::Reference< ::com::sun::star::io::XInputStream > getInputStream();

    // #i103116# FullDrag support
    virtual SdrObject* getFullDragClone() const;

    virtual void setSdrObjectTransformation(const basegfx::B2DHomMatrix& rTransformation);

    // add handles for crop mode when selected
    void addCropHandles(SdrHdlList& rTarget) const;
};

#endif //_SVDOGRAF_HXX
