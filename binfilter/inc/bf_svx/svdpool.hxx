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

#ifndef _SVDPOOL_HXX
#define _SVDPOOL_HXX

#ifndef _XPOOL_HXX
#include <bf_svx/xpool.hxx>
#endif

#ifndef _SVDDEF_HXX
#include <bf_svx/svddef.hxx>
#endif
namespace binfilter {

class XLineAttrSetItem;
class XFillAttrSetItem;
class XTextAttrSetItem;
class SdrShadowSetItem;
class SdrOutlinerSetItem;
class SdrMiscSetItem;

////////////////////////////////////////////////////////////////////////////////////////////////////

class SdrItemPool: public XOutdevItemPool {
//-/#ifdef SDRDEFITEMCACHE
//-/friend class SdrAttrObj;
//-/	const XLineAttrSetItem*   pDefLineAttr;
//-/	const XFillAttrSetItem*   pDefFillAttr;
//-/	const XTextAttrSetItem*   pDefTextAttr;
//-/	const SdrShadowSetItem*   pDefShadAttr;
//-/	const SdrOutlinerSetItem* pDefOutlAttr;
//-/	const SdrMiscSetItem*     pDefMiscAttr;
//-/	const XLineAttrSetItem*   pDefTextFrameLineAttr;
//-/	const XFillAttrSetItem*   pDefTextFrameFillAttr;
//-/	const XTextAttrSetItem*   pDefTextFrameTextAttr;
//-/	const SdrShadowSetItem*   pDefTextFrameShadAttr;
//-/	const SdrOutlinerSetItem* pDefTextFrameOutlAttr;
//-/	const SdrMiscSetItem*     pDefTextFrameMiscAttr;
//-/#endif
private:
    void Ctor(SfxItemPool* pMaster, USHORT nAttrStart, USHORT nAttrEnd);
//-/	void ImpMakeDefItems();
public:
    // Kontstruktion des Pools als MasterPool
    SdrItemPool(USHORT nAttrStart=SDRATTR_START, USHORT nAttrEnd=SDRATTR_END,
        FASTBOOL bLoadRefCounts = TRUE);

    // Damit meine SetItems mit dem MasterPool konstruiert werden koennen.
    // Der SdrItemPool wird dabei automatisch als Secondary an den
    // uebergebenen MasterPool bzw. an's Ende einer bestehenden Pool-Kette
    // angehaengt.
    SdrItemPool(SfxItemPool* pMaster, USHORT nAttrStart=SDRATTR_START, USHORT nAttrEnd=SDRATTR_END,
        FASTBOOL bLoadRefCounts = TRUE);

    SdrItemPool(const SdrItemPool& rPool);
    virtual ~SdrItemPool();
    virtual SfxItemPool* Clone() const;

    // liefert nur einen String, wenn die DLL mit DBG_UTIL uebersetzt ist
};

////////////////////////////////////////////////////////////////////////////////////////////////////

}//end of namespace binfilter
#endif // _SVDPOOL_HXX

