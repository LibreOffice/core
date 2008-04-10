/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: unoipset.hxx,v $
 * $Revision: 1.3 $
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

#ifndef _SVX_UNOIPSET_HXX_
#define _SVX_UNOIPSET_HXX_

#include <com/sun/star/beans/XPropertySetInfo.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include "svx/svxdllapi.h"

class SvxIDPropertyCombineList;
class SdrItemPool;
struct SfxItemPropertyMap;
class SfxItemSet;
class SvxShape;

class SVX_DLLPUBLIC SvxItemPropertySet
{
    const SfxItemPropertyMap*   _pMap;
    const SfxItemPropertyMap*   mpLastMap;
    SvxIDPropertyCombineList*   pCombiList;
    sal_Bool                    mbConvertTwips;

public:
    SvxItemPropertySet( const SfxItemPropertyMap *pMap, sal_Bool bConvertTwips = sal_False );
    ~SvxItemPropertySet();

    // Methoden, die direkt mit dem ItemSet arbeiten
    ::com::sun::star::uno::Any getPropertyValue( const SfxItemPropertyMap* pMap, const SfxItemSet& rSet ) const;
    void setPropertyValue( const SfxItemPropertyMap* pMap, const ::com::sun::star::uno::Any& rVal, SfxItemSet& rSet ) const;

    // Methoden, die stattdessen Any benutzen
    ::com::sun::star::uno::Any getPropertyValue( const SfxItemPropertyMap* pMap ) const;
    void setPropertyValue( const SfxItemPropertyMap* pMap, const ::com::sun::star::uno::Any& rVal ) const;

    // Properties von einem anderen Set uebernehmen
    void ObtainSettingsFromPropertySet(SvxItemPropertySet& rPropSet,  SfxItemSet& rSet, ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet > xSet );
    sal_Bool AreThereOwnUsrAnys() { return (pCombiList ? sal_True : sal_False); }
    ::com::sun::star::uno::Any* GetUsrAnyForID(sal_uInt16 nWID) const;
    void AddUsrAnyForID(const ::com::sun::star::uno::Any& rAny, sal_uInt16 nWID);

    com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo > getPropertySetInfo() const;
    const SfxItemPropertyMap* getPropertyMap() const {return _pMap;}
    void setPropertyMap( const SfxItemPropertyMap *pMap ) { _pMap = pMap; }
    const SfxItemPropertyMap* getPropertyMapEntry(const ::rtl::OUString &rName) const;

    static com::sun::star::uno::Reference< com::sun::star::beans::XPropertySetInfo > getPropertySetInfo( const SfxItemPropertyMap* pMap );
};

#endif // _SVX_UNOIPSET_HXX_

