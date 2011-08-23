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
#ifndef _FRMFMT_HXX
#define _FRMFMT_HXX

#ifndef _FORMAT_HXX
#include <format.hxx>
#endif
class Graphic; 
class Point; 

namespace binfilter {

class ImageMap; 
class SwDrawContact;
class SwFlyFrm;
class IMapObject;
class SwRect;
class SwContact;
class SdrObject;

class SwFrmFmt: public SwFmt
{
    friend class SwDoc;
    friend class SwPageDesc;	//darf den protected CTor rufen.
    friend class SwSwgReader;	// der SW2-Reader auch!
    friend class Sw3IoImp;		// der SW3-Reader auch!

protected:
    SwFrmFmt( SwAttrPool& rPool, const sal_Char* pFmtNm,
                SwFrmFmt *pDrvdFrm, USHORT nFmtWhich = RES_FRMFMT,
                const USHORT* pWhichRange = 0 )
          : SwFmt( rPool, pFmtNm, (pWhichRange ? pWhichRange : aFrmFmtSetRange),
                pDrvdFrm, nFmtWhich )
    {}

    SwFrmFmt( SwAttrPool& rPool, const String &rFmtNm,
                SwFrmFmt *pDrvdFrm, USHORT nFmtWhich = RES_FRMFMT,
                const USHORT* pWhichRange = 0 )
          : SwFmt( rPool, rFmtNm, (pWhichRange ? pWhichRange : aFrmFmtSetRange),
                pDrvdFrm, nFmtWhich )
    {}

public:
    TYPEINFO();		//Bereits in Basisklasse Client drin.

    //Vernichtet alle Frms in aDepend (Frms werden per PTR_CAST erkannt).
    virtual void DelFrms();

    //Erzeugt die Ansichten
    virtual void MakeFrms();


    virtual void Modify( SfxPoolItem* pOldValue, SfxPoolItem* pNewValue );

    // returnt das IMapObject, das an dem Format (Fly), in der ImageMap
    // an der Point Position definiert ist.
    // 	rPoint - teste auf der DocPosition
    //	pFly - optionaler FlyFrame, falls der schon bekannt ist.

    // Gibt die tatsaechlche Groesse des Frames zurueck bzw. ein leeres
    // Rechteck, wenn kein Layout existiert. Wird pPoint angegeben, dann
    // wird der am dichtesten liegende Frame gesucht.
    SwRect FindLayoutRect( const BOOL bPrtArea = FALSE,
                            const Point* pPoint = 0,
                            const BOOL bCalcFrm = FALSE ) const;

    // Sucht das SdrObject. Der SdrObjUserCall ist Client vom Format.
    // Der UserCall kennt sein SdrObject.
          SwContact *FindContactObj();
    const SwContact *FindContactObj() const
        { return ((SwFrmFmt*)this)->FindContactObj(); }

    // returns the SdrObject, that ist connected to the ContactObject.
    // Only DrawFrmFmts are connected to the "real SdrObject". FlyFrmFmts
    // are connected to a Master and all FlyFrms has the "real SdrObject".
    // "Real SdrObject" has position and a Z-order.
          SdrObject *FindSdrObject();
    const SdrObject *FindSdrObject() const
        { return ((SwFrmFmt*)this)->FindSdrObject(); }

          SdrObject *FindRealSdrObject();
    const SdrObject *FindRealSdrObject() const
        { return ((SwFrmFmt*)this)->FindRealSdrObject(); }


    DECL_FIXEDMEMPOOL_NEWDEL(SwFrmFmt)
};

//Das FlyFrame-Format ------------------------------

class SwFlyFrmFmt: public SwFrmFmt
{
    friend class SwDoc;

    //Beide nicht vorhanden.
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

    //Erzeugt die Ansichten
    virtual void MakeFrms();

    SwFlyFrm* GetFrm( const Point* pDocPos = 0,
                        const BOOL bCalcFrm = FALSE ) const;


    virtual BOOL GetInfo( SfxPoolItem& rInfo ) const;

    /** SwFlyFrmFmt::IsBackgroundTransparent - for #99657#

        OD 22.08.2002 - overloading virtual method and its default implementation,
        because format of fly frame provides transparent backgrounds.
        Method determines, if background of fly frame is transparent.

        @author OD

        @return true, if background color is transparent, but not "no fill"
        or a existing background graphic is transparent.
    */
    virtual sal_Bool IsBackgroundTransparent() const;

    /** SwFlyFrmFmt::IsBackgroundBrushInherited - for #103898#

        OD 08.10.2002 - method to determine, if the brush for drawing the
        background is "inherited" from its parent/grandparent.
        This is the case, if no background graphic is set and the background
        color is "no fill"/"auto fill"

        @author OD

        @return true, if background brush is "inherited" from parent/grandparent
    */
    sal_Bool IsBackgroundBrushInherited() const;

    DECL_FIXEDMEMPOOL_NEWDEL(SwFlyFrmFmt)
};

//Das DrawFrame-Format -----------------------------

class SwDrawFrmFmt: public SwFrmFmt
{
    friend class SwDoc;

    //Beide nicht vorhanden.
    SwDrawFrmFmt( const SwDrawFrmFmt &rCpy );
    SwDrawFrmFmt &operator=( const SwDrawFrmFmt &rCpy );

protected:
    SwDrawFrmFmt( SwAttrPool& rPool, const sal_Char* pFmtNm,
                    SwFrmFmt *pDrvdFrm )
        : SwFrmFmt( rPool, pFmtNm, pDrvdFrm, RES_DRAWFRMFMT )
    {}
    SwDrawFrmFmt( SwAttrPool& rPool, const String &rFmtNm,
                    SwFrmFmt *pDrvdFrm )
        : SwFrmFmt( rPool, rFmtNm, pDrvdFrm, RES_DRAWFRMFMT )
    {}

public:
    TYPEINFO();
    ~SwDrawFrmFmt();

    //DrawObjecte werden aus den Arrays am Layout entfernt. Die DrawObjecte
    //werden als geloescht gekennzeichnet.
    virtual void DelFrms();

    //Anmelden der DrawObjecte in den Arrays am Layout. Loeschkennzeichen
    //werden zurueckgesetzt.
    virtual void MakeFrms();



    DECL_FIXEDMEMPOOL_NEWDEL(SwDrawFrmFmt);
};


} //namespace binfilter
#endif

