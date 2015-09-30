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
#ifndef INCLUDED_SW_INC_FRMFMT_HXX
#define INCLUDED_SW_INC_FRMFMT_HXX

#include <com/sun/star/text/PositionLayoutDir.hpp>
#include <cppuhelper/weakref.hxx>
#include <tools/gen.hxx>
#include <format.hxx>
#include "swdllapi.h"

class SwFlyFrm;
class SwAnchoredObject;
class Graphic;
class ImageMap;
class IMapObject;
class SwRect;
class SwContact;
class SdrObject;
namespace sw { class DocumentLayoutManager; }

/// Style of a layout element.
class SW_DLLPUBLIC SwFrameFormat: public SwFormat
{
    friend class SwDoc;
    friend class SwPageDesc;    ///< Is allowed to call protected CTor.
    friend class ::sw::DocumentLayoutManager; ///< Is allowed to call protected CTor.

    ::com::sun::star::uno::WeakReference<
        ::com::sun::star::uno::XInterface> m_wXObject;

    //UUUU DrawingLayer FillAttributes in a preprocessed form for primitive usage
    drawinglayer::attribute::SdrAllFillAttributesHelperPtr  maFillAttributes;

protected:
    SwFrameFormat(
        SwAttrPool& rPool,
        const sal_Char* pFormatNm,
        SwFrameFormat *pDrvdFrm,
        sal_uInt16 nFormatWhich = RES_FRMFMT,
        const sal_uInt16* pWhichRange = 0);

    SwFrameFormat(
        SwAttrPool& rPool,
        const OUString &rFormatNm,
        SwFrameFormat *pDrvdFrm,
        sal_uInt16 nFormatWhich = RES_FRMFMT,
        const sal_uInt16* pWhichRange = 0);

    virtual void Modify( const SfxPoolItem* pOld, const SfxPoolItem* pNewValue ) SAL_OVERRIDE;

public:
    TYPEINFO_OVERRIDE();     ///< Already in base class Client.
    virtual ~SwFrameFormat();

    /// Destroys all Frms in aDepend (Frms are identified via dynamic_cast).
    virtual void DelFrms();

    /// Creates the views.
    virtual void MakeFrms();

    virtual Graphic MakeGraphic( ImageMap* pMap = NULL );

    /**  @return the IMapObject defined at format (Fly)
        in the ImageMap at position Point.
        rPoint - test on DocPosition.
        pFly - optional FlyFrame, in case it is already known. */
    IMapObject* GetIMapObject( const Point& rPoint,
                                const SwFlyFrm *pFly = 0 ) const;

    /** @return the real size of the frame - or an empty rectangle
       if no layout exists.
       If pPoint is given, look for the frame closest to it. */
    SwRect FindLayoutRect( const bool bPrtArea = false,
                           const Point* pPoint = 0,
                           const bool bCalcFrm = false ) const;

    /** Searches SdrObject. SdrObjUserCall is client of the format.
       The UserCall knows its SdrObject. */
          SwContact *FindContactObj();
    const SwContact *FindContactObj() const
        { return const_cast<SwFrameFormat*>(this)->FindContactObj(); }

    /** @return the SdrObject, that is connected to the ContactObject.
       Only DrawFrameFormats are connected to the "real SdrObject". FlyFrameFormats
       are connected to a Master and all FlyFrms has the "real SdrObject".
       "Real SdrObject" has position and a Z-order. */
          SdrObject *FindSdrObject();
    const SdrObject *FindSdrObject() const
        { return const_cast<SwFrameFormat*>(this)->FindSdrObject(); }

          SdrObject *FindRealSdrObject();
    const SdrObject *FindRealSdrObject() const
        { return const_cast<SwFrameFormat*>(this)->FindRealSdrObject(); }

    bool IsLowerOf( const SwFrameFormat& rFormat ) const;

    enum tLayoutDir
    {
        HORI_L2R,
        HORI_R2L,
        VERT_R2L,
        VERT_L2R    ///< Not supported yet.
    };

    virtual SwFrameFormat::tLayoutDir GetLayoutDir() const;
    virtual void SetLayoutDir( const SwFrameFormat::tLayoutDir _eLayoutDir );

    virtual sal_Int16 GetPositionLayoutDir() const;
    virtual void SetPositionLayoutDir( const sal_Int16 _nPositionLayoutDir );

    virtual OUString GetDescription() const;

    SAL_DLLPRIVATE ::com::sun::star::uno::WeakReference<
        ::com::sun::star::uno::XInterface> const& GetXObject() const
            { return m_wXObject; }
    SAL_DLLPRIVATE void SetXObject(::com::sun::star::uno::Reference<
                    ::com::sun::star::uno::XInterface> const& xObject)
            { m_wXObject = xObject; }

    DECL_FIXEDMEMPOOL_NEWDEL_DLL(SwFrameFormat)
    void RegisterToFormat( SwFormat& rFormat );

    //UUUU Access to DrawingLayer FillAttributes in a preprocessed form for primitive usage
    virtual drawinglayer::attribute::SdrAllFillAttributesHelperPtr getSdrAllFillAttributesHelper() const SAL_OVERRIDE;
    virtual bool supportsFullDrawingLayerFillAttributeSet() const SAL_OVERRIDE;

    void dumpAsXml(struct _xmlTextWriter* pWriter) const;
};

// The FlyFrame-Format

class SW_DLLPUBLIC SwFlyFrameFormat: public SwFrameFormat
{
    friend class SwDoc;
    OUString msTitle;
    OUString msDesc;

    /** Both not existent.
       it stores the previous position of Prt rectangle from RequestObjectResize
       so it can be used to move frames of non-resizable objects to align them correctly
       when they get borders (this is done in SwWrtShell::CalcAndGetScale) */
    Point   m_aLastFlyFrmPrtRectPos;

    SwFlyFrameFormat( const SwFlyFrameFormat &rCpy ) SAL_DELETED_FUNCTION;
    SwFlyFrameFormat &operator=( const SwFlyFrameFormat &rCpy ) SAL_DELETED_FUNCTION;

protected:
    SwFlyFrameFormat( SwAttrPool& rPool, const sal_Char* pFormatNm,
                    SwFrameFormat *pDrvdFrm )
        : SwFrameFormat( rPool, pFormatNm, pDrvdFrm, RES_FLYFRMFMT )
    {}
    SwFlyFrameFormat( SwAttrPool& rPool, const OUString &rFormatNm,
                    SwFrameFormat *pDrvdFrm )
        : SwFrameFormat( rPool, rFormatNm, pDrvdFrm, RES_FLYFRMFMT )
    {}

public:
    TYPEINFO_OVERRIDE();
    virtual ~SwFlyFrameFormat();

    /// Creates the views.
    virtual void MakeFrms() SAL_OVERRIDE;

    SwFlyFrm* GetFrm( const Point* pDocPos = 0,
                      const bool bCalcFrm = false ) const;

    SwAnchoredObject* GetAnchoredObj( const Point* pDocPos = 0,
                                      const bool bCalcFrm = false ) const;

    virtual Graphic MakeGraphic( ImageMap* pMap = NULL ) SAL_OVERRIDE;

    virtual bool GetInfo( SfxPoolItem& rInfo ) const SAL_OVERRIDE;

    OUString GetObjTitle() const;
    void SetObjTitle( const OUString& rTitle, bool bBroadcast = false );
    OUString GetObjDescription() const;
    void SetObjDescription( const OUString& rDescription, bool bBroadcast = false );

    /** SwFlyFrameFormat::IsBackgroundTransparent

        Override virtual method and its default implementation,
        because format of fly frame provides transparent backgrounds.
        Method determines, if background of fly frame is transparent.

        @author OD

        @return true, if background color is transparent, but not "no fill"
        or a existing background graphic is transparent.
    */
    virtual bool IsBackgroundTransparent() const SAL_OVERRIDE;

    /** SwFlyFrameFormat::IsBackgroundBrushInherited

        Method to determine, if the brush for drawing the
        background is "inherited" from its parent/grandparent.
        This is the case, if no background graphic is set and the background
        color is "no fill"/"auto fill"

        @author OD

        @return true, if background brush is "inherited" from parent/grandparent
    */
    bool IsBackgroundBrushInherited() const;

    const Point & GetLastFlyFrmPrtRectPos() const       { return m_aLastFlyFrmPrtRectPos; }
    void SetLastFlyFrmPrtRectPos( const Point &rPoint ) { m_aLastFlyFrmPrtRectPos = rPoint; }

    DECL_FIXEDMEMPOOL_NEWDEL(SwFlyFrameFormat)
};

//The DrawFrame-Format

class SW_DLLPUBLIC SwDrawFrameFormat: public SwFrameFormat
{
    friend class SwDoc;

    mutable const SdrObject * pSdrObjCached;
    mutable OUString sSdrObjCachedComment;

    SwDrawFrameFormat( const SwDrawFrameFormat &rCpy ) SAL_DELETED_FUNCTION;
    SwDrawFrameFormat &operator=( const SwDrawFrameFormat &rCpy ) SAL_DELETED_FUNCTION;

    SwFrameFormat::tLayoutDir meLayoutDir;

    sal_Int16 mnPositionLayoutDir;

    bool mbPosAttrSet;

protected:
    SwDrawFrameFormat( SwAttrPool& rPool, const sal_Char* pFormatNm,
                    SwFrameFormat *pDrvdFrm )
        : SwFrameFormat( rPool, pFormatNm, pDrvdFrm, RES_DRAWFRMFMT ),
          pSdrObjCached(NULL),

          meLayoutDir( SwFrameFormat::HORI_L2R ),

          mnPositionLayoutDir( com::sun::star::text::PositionLayoutDir::PositionInLayoutDirOfAnchor ),

          mbPosAttrSet( false )

    {}
    SwDrawFrameFormat( SwAttrPool& rPool, const OUString &rFormatNm,
                    SwFrameFormat *pDrvdFrm )
        : SwFrameFormat( rPool, rFormatNm, pDrvdFrm, RES_DRAWFRMFMT ),
          pSdrObjCached(NULL),
          meLayoutDir( SwFrameFormat::HORI_L2R ),

          mnPositionLayoutDir( com::sun::star::text::PositionLayoutDir::PositionInLayoutDirOfAnchor ),

          mbPosAttrSet( false )
    {}

public:
    TYPEINFO_OVERRIDE();
    virtual ~SwDrawFrameFormat();

    /** DrawObjects are removed from the arrays at the layout.
     The DrawObjects are marked as deleted. */
    virtual void DelFrms() SAL_OVERRIDE;

    /** Register DrawObjects in the arrays at layout.
     Reset delete marks. */
    virtual void MakeFrms() SAL_OVERRIDE;

    virtual Graphic MakeGraphic( ImageMap* pMap = NULL ) SAL_OVERRIDE;

    virtual SwFrameFormat::tLayoutDir GetLayoutDir() const SAL_OVERRIDE;
    virtual void SetLayoutDir( const SwFrameFormat::tLayoutDir _eLayoutDir ) SAL_OVERRIDE;

    virtual sal_Int16 GetPositionLayoutDir() const SAL_OVERRIDE;
    virtual void SetPositionLayoutDir( const sal_Int16 _nPositionLayoutDir ) SAL_OVERRIDE;

    inline bool IsPosAttrSet() const { return mbPosAttrSet; }
    inline void PosAttrSet() { mbPosAttrSet = true; }

    virtual OUString GetDescription() const SAL_OVERRIDE;

    DECL_FIXEDMEMPOOL_NEWDEL(SwDrawFrameFormat);
};

namespace sw {

SW_DLLPUBLIC bool IsFlyFrameFormatInHeader(const SwFrameFormat& rFormat);

void CheckAnchoredFlyConsistency(SwDoc const& rDoc);

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
