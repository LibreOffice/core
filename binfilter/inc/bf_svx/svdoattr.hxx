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

#ifndef _SVDOATTR_HXX
#define _SVDOATTR_HXX

#ifndef SVX_XFILLIT0_HXX //autogen
#include <bf_svx/xfillit0.hxx>
#endif

#ifndef _SVX_XFLASIT_HXX //autogen
#include <bf_svx/xflasit.hxx>
#endif

#ifndef _SVX_XLINEIT0_HXX //autogen
#include <bf_svx/xlineit0.hxx>
#endif

#ifndef _SVX_XLNASIT_HXX //autogen
#include <bf_svx/xlnasit.hxx>
#endif

#ifndef _SVDOBJ_HXX
#include <bf_svx/svdobj.hxx>
#endif

#ifndef _SVDATTR_HXX
#include <bf_svx/svdattr.hxx>
#endif
namespace binfilter {

//************************************************************
//   Vorausdeklarationen
//************************************************************

class SfxPoolItem; 
class SfxSetItem;
class SfxItemSet;
class SfxItemPool;

class SdrOutliner;

//************************************************************
//   SdrAttrObj
//************************************************************

class SdrAttrObj : public SdrObject
{
    friend class				SdrOutliner;

protected:
    Rectangle					maSnapRect;
    SfxStyleSheet				*mpStyleSheet;
    SfxItemSet					*mpObjectItemSet;

    void ImpDeleteItemSet();
    void ImpForceItemSet();

protected:
    // Strichstaerke ermitteln. Keine Linie -> 0.
    INT32 ImpGetLineWdt() const;

    // Feststellen, wieviel wegen der Linienenden auf's BoundRect draufaddiert werden muss.
    INT32 ImpGetLineEndAdd() const;

    // HitTest auf's Linienende
    // ueber nSin/nCos wird der Winkel vorgegeben

    // Schattenabstand ermitteln. FALSE=Kein Schatten.
    FASTBOOL ImpGetShadowDist(sal_Int32& nXDist, sal_Int32& nYDist) const;

    // ggf. Schattenversatz zum BoundRect draufaddieren
    void ImpAddShadowToBoundRect();

    // Line und Fill Attribute fuer Schatten setzen.
    // Return=FALSE: kein Schatten attributiert.
    FASTBOOL ImpSetShadowAttributes( const SfxItemSet& rSet, SfxItemSet& rShadowSet ) const;

    // Zuhoeren, ob sich ein StyleSheet aendert
    virtual void SFX_NOTIFY(SfxBroadcaster& rBC, const TypeId& rBCType, const SfxHint& rHint, const TypeId& rHintType);
    virtual void RemoveStyleSheet();
    virtual void AddStyleSheet(SfxStyleSheet* pNewStyleSheet, FASTBOOL bDontRemoveHardAttr);

    // aus NULL-AttrPointern Pointer auf defaults machen
    virtual void ForceDefaultAttr();
    SdrAttrObj();
    virtual ~SdrAttrObj();

public:
    TYPEINFO();

    // Feststellen, ob bFilledObj && Fuellung!=FillNone
    BOOL HasFill() const;

    // Feststellen, ob Linie!=LineNone
    BOOL HasLine() const;

    virtual const Rectangle& GetSnapRect() const;
    virtual void operator=(const SdrObject& rObj);
    virtual void WriteData(SvStream& rOut) const;
    virtual void ReadData(const SdrObjIOHeader& rHead, SvStream& rIn);
    virtual void SetModel(SdrModel* pNewModel);

    // pre- and postprocessing for objects for saving
    virtual void PreSave();
    virtual void PostSave();

    // ItemSet access
    virtual const SfxItemSet& GetItemSet() const;
    virtual SfxItemSet* CreateNewItemSet(SfxItemPool& rPool);

    // syntactical sugar for ItemSet accesses
    const SfxItemSet& GetUnmergedItemSet() const;

    // t support routines for ItemSet access. NULL pointer means clear item.
    virtual void ItemChange(const sal_uInt16 nWhich, const SfxPoolItem* pNewItem = 0);
    virtual void ItemSetChanged(const SfxItemSet& rSet);

    virtual SfxStyleSheet* GetStyleSheet() const;
    virtual void SetStyleSheet(SfxStyleSheet* pNewStyleSheet, FASTBOOL bDontRemoveHardAttr);
    virtual void NbcSetStyleSheet(SfxStyleSheet* pNewStyleSheet, FASTBOOL bDontRemoveHardAttr);


    // ItemPool fuer dieses Objekt wechseln
    virtual void MigrateItemPool(SfxItemPool* pSrcPool, SfxItemPool* pDestPool, SdrModel* pNewModel = NULL );
};

////////////////////////////////////////////////////////////////////////////////////////////////////

}//end of namespace binfilter
#endif //_SVDOATTR_HXX

