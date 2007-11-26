/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: svdograf.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: ihi $ $Date: 2007-11-26 14:48:56 $
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

#ifndef _SVDOGRAF_HXX
#define _SVDOGRAF_HXX

#ifndef _COM_SUN_STAR_IO_XINPUTSTREAM_HPP_
#include <com/sun/star/io/XInputStream.hpp>
#endif

#ifndef _SV_GRAPH_HXX //autogen
#include <vcl/graph.hxx>
#endif
#ifndef _SVDORECT_HXX
#include <svx/svdorect.hxx>
#endif
#ifndef _GRFMGR_HXX
#include <goodies/grfmgr.hxx>
#endif

#ifndef INCLUDED_SVXDLLAPI_H
#include "svx/svxdllapi.h"
#endif

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
        class ViewContactOfGraphic;
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

// ---------------------
// - SdrGrafObjGeoData -
// ---------------------

// #109872#
class SdrGrafObjGeoData : public SdrTextObjGeoData
{
public:
    sal_Bool                    bMirrored;

    SdrGrafObjGeoData()
    :   bMirrored(sal_False)
    {
    }
};

// --------------
// - SdrGrafObj -
// --------------

class GraphicObject;
class SdrGraphicLink;

class SVX_DLLPUBLIC SdrGrafObj: public SdrRectObj
{
private:
    // BaseProperties section
    SVX_DLLPRIVATE virtual sdr::properties::BaseProperties* CreateObjectSpecificProperties();

    // DrawContact section
private:
    virtual sdr::contact::ViewContact* CreateObjectSpecificViewContact();

    // to allow sdr::properties::GraphicProperties access to SetXPolyDirty()
    friend class sdr::properties::GraphicProperties;

    // to allow sdr::contact::ViewContactOfGraphic access to ImpUpdateGraphicLink()
    friend class sdr::contact::ViewContactOfGraphic;

    friend class SdrGraphicLink;

protected:

    void                    ImpSetAttrToGrafInfo(); // Werte vom Pool kopieren
    void                    ImpSetGrafInfoToAttr(); // Werte in den Pool kopieren
    GraphicAttr             aGrafInfo;

    Rectangle               aCropRect;          // Wenn aCropRect nicht Empty ist, dann enthaelt es den sichtbaren
                                                // Ausschnitt der Grafik in logischen Eingeiten der Grafik! Also Bitmap->=Pixel
    String                  aFileName;          // Wenn es sich um einen Link handelt, steht hier der Dateiname drin.
    String                  aFilterName;
    GraphicObject*          pGraphic;           // Zur Beschleunigung von Bitmapausgaben, besonders von gedrehten.
    SdrGraphicLink*         pGraphicLink;       // Und hier noch ein Pointer fuer gelinkte Grafiken
    bool                    bMirrored;          // True bedeutet, die Grafik ist horizontal, d.h. ueber die Y-Achse gespiegelt auszugeben.

    // #111096#
    // Flag for allowing text animation. Default is sal_true.
    unsigned                    mbGrafAnimationAllowed : 1;

    // #i25616#
    unsigned                    mbInsidePaint : 1;
    unsigned                    mbIsPreview   : 1;

protected:

    void                    ImpLinkAnmeldung();
    void                    ImpLinkAbmeldung();
    sal_Bool                ImpUpdateGraphicLink() const;
                            DECL_LINK( ImpSwapHdl, GraphicObject* );

    // #i25616#
    void ImpDoPaintGrafObjShadow(XOutputDevice& rOut) const;
    void ImpDoPaintGrafObj(XOutputDevice& rOut) const;

public:

                            TYPEINFO();

                            SdrGrafObj();
                            SdrGrafObj(const Graphic& rGrf);
                            SdrGrafObj(const Graphic& rGrf, const Rectangle& rRect);
    virtual                 ~SdrGrafObj();

    void                    SetGraphicObject( const GraphicObject& rGrfObj );
    const GraphicObject&    GetGraphicObject() const;

    void                    SetGraphic(const Graphic& rGrf);
    const Graphic&          GetGraphic() const;

    Graphic                 GetTransformedGraphic( ULONG nTransformFlags = SDRGRAFOBJ_TRANSFORMATTR_ALL ) const;

    GraphicType             GetGraphicType() const;

    // #111096#
    // Keep ATM for SD.
    sal_Bool IsAnimated() const;
    // sal_Bool IsTransparent() const; -> warning SdrObject::IsTransparent, removed. Replace with <pGraphic->IsTransparent()>
    sal_Bool IsEPS() const;
    sal_Bool IsSwappedOut() const;

    const MapMode&          GetGrafPrefMapMode() const;
    const Size&             GetGrafPrefSize() const;

    sal_Bool DrawGraphic(OutputDevice* pOut, const Point& rPt, const Size& rSz,
        const GraphicAttr* pAttr = 0L, sal_uInt32 nFlags = GRFMGR_DRAW_STANDARD) const;

    void                    SetGrafStreamURL( const String& rGraphicStreamURL );
    String                  GetGrafStreamURL() const;

    void                    ForceSwapIn() const;
    void                    ForceSwapOut() const;

    void                    SetGraphicLink(const String& rFileName, const String& rFilterName);
    void                    ReleaseGraphicLink();
    sal_Bool IsLinkedGraphic() const { return (BOOL)aFileName.Len(); }

    void                    SetFileName(const String& rFileName);
    const String&           GetFileName() const { return aFileName; }
    void                    SetFilterName(const String& rFilterName);
    const String&           GetFilterName() const { return aFilterName; }

    void                    StartAnimation(OutputDevice* pOutDev, const Point& rPoint, const Size& rSize, long nExtraData=0L);
    void                    StopAnimation(OutputDevice* pOutDev=NULL, long nExtraData=0L);

    virtual void            TakeObjInfo(SdrObjTransformInfoRec& rInfo) const;
    virtual UINT16          GetObjIdentifier() const;
    virtual sal_Bool DoPaintObject(XOutputDevice& rOut, const SdrPaintInfoRec& rInfoRec) const;
    virtual SdrObject*      CheckHit(const Point& rPnt, USHORT nTol, const SetOfByte* pVisiLayer) const;

    virtual void            TakeObjNameSingul(String& rName) const;
    virtual void            TakeObjNamePlural(String& rName) const;

    // #i25616#
    virtual basegfx::B2DPolyPolygon TakeXorPoly(sal_Bool bDetail) const;

    virtual void            operator=(const SdrObject& rObj);
    virtual FASTBOOL        HasSpecialDrag() const;
    virtual sal_uInt32 GetHdlCount() const;
    virtual SdrHdl*         GetHdl(sal_uInt32 nHdlNum) const;

    virtual void            NbcResize(const Point& rRef, const Fraction& xFact, const Fraction& yFact);
    virtual void            NbcRotate(const Point& rRef, long nWink, double sn, double cs);
    virtual void            NbcMirror(const Point& rRef1, const Point& rRef2);
    virtual void            NbcShear (const Point& rRef, long nWink, double tn, FASTBOOL bVShear);
    virtual void            NbcSetSnapRect(const Rectangle& rRect);
    virtual void            NbcSetLogicRect(const Rectangle& rRect);
    virtual SdrObjGeoData*  NewGeoData() const;
    virtual void            SaveGeoData(SdrObjGeoData& rGeo) const;
    virtual void            RestGeoData(const SdrObjGeoData& rGeo);

    FASTBOOL                HasGDIMetaFile() const;
    const GDIMetaFile*      GetGDIMetaFile() const;

    virtual void            SetPage(SdrPage* pNewPage);
    virtual void            SetModel(SdrModel* pNewModel);

    virtual SdrObject*      DoConvertToPolyObj(BOOL bBezier) const;

    void                    AdjustToMaxRect( const Rectangle& rMaxRect, BOOL bShrinkOnly );

    virtual void            SFX_NOTIFY( SfxBroadcaster& rBC, const TypeId& rBCType,
                                        const SfxHint& rHint, const TypeId& rHintType );

    bool                    IsMirrored() { return bMirrored; }
    void                    SetMirrored( bool _bMirrored ) { bMirrored = _bMirrored; }

    // #111096#
    // Access to GrafAnimationAllowed flag
    sal_Bool IsGrafAnimationAllowed() const;
    void SetGrafAnimationAllowed(sal_Bool bNew);

    // #i25616#
    sal_Bool IsObjectTransparent() const;

    ::com::sun::star::uno::Reference< ::com::sun::star::io::XInputStream > getInputStream();
};

#endif //_SVDOGRAF_HXX

