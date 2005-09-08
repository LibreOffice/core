/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: propertysetitem.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 16:00:04 $
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

#ifndef _DBAUI_PROPERTYSETITEM_HXX_
#define _DBAUI_PROPERTYSETITEM_HXX_

#ifndef _SFXPOOLITEM_HXX
#include <svtools/poolitem.hxx>
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSET_HPP_
#include <com/sun/star/beans/XPropertySet.hpp>
#endif

//.........................................................................
namespace dbaui
{
//.........................................................................

    //=========================================================================
    //= OPropertySetItem
    //=========================================================================
    /** <type>SfxPoolItem</type> which transports a XPropertySet
    */
    class OPropertySetItem : public SfxPoolItem
    {
        ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >   m_xSet;

    public:
        TYPEINFO();
        OPropertySetItem(sal_Int16 nWhich);
        OPropertySetItem(sal_Int16 nWhich,
            const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& _rxSet);
        OPropertySetItem(const OPropertySetItem& _rSource);

        virtual int              operator==(const SfxPoolItem& _rItem) const;
        virtual SfxPoolItem*     Clone(SfxItemPool* _pPool = NULL) const;

        ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >
                getPropertySet() const { return m_xSet; }
    };

//.........................................................................
}   // namespace dbaui
//.........................................................................

#endif // _DBAUI_PROPERTYSETITEM_HXX_

