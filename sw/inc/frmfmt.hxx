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
#ifndef _FRMFMT_HXX
#define _FRMFMT_HXX

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

/// Style of a layout element.
class SW_DLLPUBLIC SwFrmFmt: public SwFmt
{
    friend class SwDoc;
    friend class SwPageDesc;    ///< Is allowed to call protected CTor.

    ::com::sun::star::uno::WeakReference<
        ::com::sun::star::uno::XInterface> m_wXObject;

protected:
    SwFrmFmt( SwAttrPool& rPool, const sal_Char* pFmtNm,
                SwFrmFmt *pDrvdFrm, sal_uInt16 nFmtWhich = RES_FRMFMT,
                const sal_uInt16* pWhichRange = 0 )
          : SwFmt( rPool, pFmtNm, (pWhichRange ? pWhichRange : aFrmFmtSetRange),
                pDrvdFrm, nFmtWhich )
    {}

    SwFrmFmt( SwAttrPool& rPool, const String &rFmtNm,
                SwFrmFmt *pDrvdFrm, sal_uInt16 nFmtWhich = RES_FRMFMT,
                const sal_uInt16* pWhichRange = 0 )
          : SwFmt( rPool, rFmtNm, (pWhichRange ? pWhichRange : aFrmFmtSetRange),
                pDrvdFrm, nFmtWhich )
    {}

   virtual void Modify( const SfxPoolItem* pOld, const SfxPoolItem* pNewValue );

public:
    TYPEINFO();     ///< Already in base class Client.

    /// Destroys all Frms in aDepend (Frms are identified via PTR_CAST).
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
    SwRect FindLayoutRect( const sal_Bool bPrtArea = sal_False,
                            const Point* pPoint = 0,
                            const sal_Bool bCalcFrm = sal_False ) const;

    /** Searches SdrObject. SdrObjUserCall is client of the format.
       The UserCall knows its SdrObject. */
          SwContact *FindContactObj();
    const SwContact *FindContactObj() const
        { return ((SwFrmFmt*)this)->FindContactObj(); }

    /** @return the SdrObject, that ist connected to the ContactObject.
       Only DrawFrmFmts are connected to the "real SdrObject". FlyFrmFmts
       are connected to a Master and all FlyFrms has the "real SdrObject".
       "Real SdrObject" has position and a Z-order. */
          SdrObject *FindSdrObject();
    const SdrObject *FindSdrObject() const
        { return ((SwFrmFmt*)this)->FindSdrObject(); }

          SdrObject *FindRealSdrObject();
    const SdrObject *FindRealSdrObject() const
        { return ((SwFrmFmt*)this)->FindRealSdrObject(); }

    sal_Bool IsLowerOf( const SwFrmFmt& rFmt ) const;

    enum tLayoutDir
    {
        HORI_L2R,
        HORI_R2L,
        VERT_R2L,
        VERT_L2R    ///< Not supported yet.
    };

    virtual SwFrmFmt::tLayoutDir GetLayoutDir() const;
    virtual void SetLayoutDir( const SwFrmFmt::tLayoutDir _eLayoutDir );

    virtual sal_Int16 GetPositionLayoutDir() const;
    virtual void SetPositionLayoutDir( const sal_Int16 _nPositionLayoutDir );

    virtual String GetDescription() const;

    SW_DLLPRIVATE ::com::sun::star::uno::WeakReference<
        ::com::sun::star::uno::XInterface> const& GetXObject() const
            { return m_wXObject; }
    SW_DLLPRIVATE void SetXObject(::com::sun::star::uno::Reference<
                    ::com::sun::star::uno::XInterface> const& xObject)
            { m_wXObject = xObject; }

    DECL_FIXEDMEMPOOL_NEWDEL_DLL(SwFrmFmt)
    void RegisterToFormat( SwFmt& rFmt );
};

// The FlyFrame-Format

class SW_DLLPUBLIC SwFlyFrmFmt: public SwFrmFmt
{
    friend class SwDoc;

    /** Both not existent.
       it stores the previous position of Prt rectangle from RequestObjectResize
       so it can be used to move frames of non-resizable objects to align them correctly
       when they get borders (this is done in SwWrtShell::CalcAndGetScale) */
    Point   m_aLastFlyFrmPrtRectPos;

    SwFlyFrmFmt( const SwFlyFrmFmt &rCpy );
    SwFlyFrmFmt &operator=( const SwFlyFrmFmt &rCpy );

protected:
    SwFlyFrmFmt( SwAttrPool& rPool, const sal_Char* pFmtNm,
                    SwFrmFmt *pDrvdFrm )
        : SwFrmFmt( rPool, pFmtNm, pDrvdFrm, RES_FLYFRMFMT )
    {}
    SwFlyFrmFmt( SwAttrPool& rPool, const String &rFmtNm,
                    SwFrmFmt *pDrvdFrm )
        : SwFrmFmt( rPool, rFmtNm, pDrvdFrm, RES_FLYFRMFMT )
    {}

public:
    TYPEINFO();
    ~SwFlyFrmFmt();

    /// Creates the views.
    virtual void MakeFrms();

    SwFlyFrm* GetFrm( const Point* pDocPos = 0,
                        const sal_Bool bCalcFrm = sal_False ) const;

    SwAnchoredObject* GetAnchoredObj( const Point* pDocPos = 0,
                                      const sal_Bool bCalcFrm = sal_False ) const;

    virtual Graphic MakeGraphic( ImageMap* pMap = NULL );

    virtual bool GetInfo( SfxPoolItem& rInfo ) const;

    OUString GetObjTitle() const;
    void SetObjTitle( const OUString& rTitle, bool bBroadcast = false );
    OUString GetObjDescription() const;
    void SetObjDescription( const OUString& rDescription, bool bBroadcast = false );

    /** SwFlyFrmFmt::IsBackgroundTransparent

        Overloading virtual method and its default implementation,
        because format of fly frame provides transparent backgrounds.
        Method determines, if background of fly frame is transparent.

        @author OD

        @return true, if background color is transparent, but not "no fill"
        or a existing background graphic is transparent.
    */
    virtual sal_Bool IsBackgroundTransparent() const;

    /** SwFlyFrmFmt::IsBackgroundBrushInherited

        Method to determine, if the brush for drawing the
        background is "inherited" from its parent/grandparent.
        This is the case, if no background graphic is set and the background
        color is "no fill"/"auto fill"

        @author OD

        @return true, if background brush is "inherited" from parent/grandparent
    */
    sal_Bool IsBackgroundBrushInherited() const;

    const Point & GetLastFlyFrmPrtRectPos() const       { return m_aLastFlyFrmPrtRectPos; }
    void SetLastFlyFrmPrtRectPos( const Point &rPoint ) { m_aLastFlyFrmPrtRectPos = rPoint; }

    DECL_FIXEDMEMPOOL_NEWDEL(SwFlyFrmFmt)
};

//The DrawFrame-Format

class SW_DLLPUBLIC SwDrawFrmFmt: public SwFrmFmt
{
    friend class SwDoc;

    mutable const SdrObject * pSdrObjCached;
    mutable String sSdrObjCachedComment;

    /// Both not existent.
    SwDrawFrmFmt( const SwDrawFrmFmt &rCpy );
    SwDrawFrmFmt &operator=( const SwDrawFrmFmt &rCpy );

    SwFrmFmt::tLayoutDir meLayoutDir;

    sal_Int16 mnPositionLayoutDir;

    bool mbPosAttrSet;

protected:
    SwDrawFrmFmt( SwAttrPool& rPool, const sal_Char* pFmtNm,
                    SwFrmFmt *pDrvdFrm )
        : SwFrmFmt( rPool, pFmtNm, pDrvdFrm, RES_DRAWFRMFMT ),
          pSdrObjCached(NULL),

          meLayoutDir( SwFrmFmt::HORI_L2R ),

          mnPositionLayoutDir( com::sun::star::text::PositionLayoutDir::PositionInLayoutDirOfAnchor ),

          mbPosAttrSet( false )

    {}
    SwDrawFrmFmt( SwAttrPool& rPool, const String &rFmtNm,
                    SwFrmFmt *pDrvdFrm )
        : SwFrmFmt( rPool, rFmtNm, pDrvdFrm, RES_DRAWFRMFMT ),
          pSdrObjCached(NULL),
          meLayoutDir( SwFrmFmt::HORI_L2R ),

          mnPositionLayoutDir( com::sun::star::text::PositionLayoutDir::PositionInLayoutDirOfAnchor ),

          mbPosAttrSet( false )
    {}

public:
    TYPEINFO();
    ~SwDrawFrmFmt();

    /** DrawObjects are removed from the arrays at the layout.
     The DrawObjects are marked as deleted. */
    virtual void DelFrms();

    /** Register DrawObjects in the arrays at layout.
     Reset delete marks. */
    virtual void MakeFrms();

    virtual Graphic MakeGraphic( ImageMap* pMap = NULL );

    virtual SwFrmFmt::tLayoutDir GetLayoutDir() const;
    virtual void SetLayoutDir( const SwFrmFmt::tLayoutDir _eLayoutDir );

    virtual sal_Int16 GetPositionLayoutDir() const;
    virtual void SetPositionLayoutDir( const sal_Int16 _nPositionLayoutDir );

    inline bool IsPosAttrSet() const { return mbPosAttrSet; }
    inline void PosAttrSet() { mbPosAttrSet = true; }

    inline void ResetPosAttr()
    {
        mbPosAttrSet = false;
    }

    virtual String GetDescription() const;

    DECL_FIXEDMEMPOOL_NEWDEL(SwDrawFrmFmt);
};


#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
