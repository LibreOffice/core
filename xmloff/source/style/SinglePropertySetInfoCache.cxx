/*************************************************************************
 *
 *  $RCSfile: SinglePropertySetInfoCache.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: hr $ $Date: 2003-03-27 18:20:33 $
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
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef _COM_SUN_STAR_LANG_XTYPEPROVIDER_HPP_
#include <com/sun/star/lang/XTypeProvider.hpp>
#endif
#ifndef _CPPUHELPER_WEAKREF_HXX_
#include <cppuhelper/weakref.hxx>
#endif

#ifndef _XMLOFF_SINGLEPROPERTYSETINFOCACHE_HXX
#include "SinglePropertySetInfoCache.hxx"
#endif

using namespace ::com::sun::star::uno;
using ::com::sun::star::lang::XTypeProvider;
using ::com::sun::star::beans::XPropertySet;
using ::com::sun::star::beans::XPropertySetInfo;

sal_Bool SinglePropertySetInfoCache::hasProperty(
        const Reference< XPropertySet >& rPropSet,
        Reference< XPropertySetInfo >& rPropSetInfo )
{
    if( !rPropSetInfo.is() )
        rPropSetInfo = rPropSet->getPropertySetInfo();
    sal_Bool bRet = sal_False, bValid = sal_False;
    Reference < XTypeProvider > xTypeProv( rPropSet, UNO_QUERY );
    Sequence< sal_Int8 > aImplId;
    if( xTypeProv.is() )
    {
        aImplId = xTypeProv->getImplementationId();
        if( aImplId.getLength() == 16 )
        {
            // The key must not be created outside this block, because it
            // keeps a reference to the property set info.
            PropertySetInfoKey aKey( rPropSetInfo, aImplId );
            iterator aIter = find( aKey );
            if( aIter != end() )
            {
                bRet = (*aIter).second;
                bValid = sal_True;
            }
        }
    }
    if( !bValid )
    {
        bRet = rPropSetInfo->hasPropertyByName( sName );
        if( xTypeProv.is() && aImplId.getLength() == 16 )
        {
            // Check whether the property set info is destroyed if it is
            // assigned to a weak reference only. If it is destroyed, then
            // every instance of getPropertySetInfo returns a new object.
            // Such property set infos must not be cached.
            WeakReference < XPropertySetInfo > xWeakInfo( rPropSetInfo );
            rPropSetInfo = 0;
            rPropSetInfo = xWeakInfo;
            if( rPropSetInfo.is() )
            {
                PropertySetInfoKey aKey( rPropSetInfo, aImplId );
                value_type aValue( aKey, bRet );
                insert( aValue );
            }
        }
    }

    return bRet;
}
