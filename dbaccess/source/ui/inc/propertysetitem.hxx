/*************************************************************************
 *
 *  $RCSfile: propertysetitem.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: hr $ $Date: 2003-03-19 17:52:42 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc..
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
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

