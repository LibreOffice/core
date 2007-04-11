/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: unoipset.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-04-11 16:44:51 $
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

#ifndef _SVX_UNOIPSET_HXX_
#define _SVX_UNOIPSET_HXX_

#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSETINFO_HPP_
#include <com/sun/star/beans/XPropertySetInfo.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSET_HPP_
#include <com/sun/star/beans/XPropertySet.hpp>
#endif

#ifndef INCLUDED_SVXDLLAPI_H
#include "svx/svxdllapi.h"
#endif

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

