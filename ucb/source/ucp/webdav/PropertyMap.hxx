/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: PropertyMap.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 16:14:21 $
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

#ifndef _WEBDAV_UCP_PROPERTYMAP_HXX
#define _WEBDAV_UCP_PROPERTYMAP_HXX

#include <hash_set>

#ifndef _COM_SUN_STAR_BEANS_PROPERTY_HPP_
#include <com/sun/star/beans/Property.hpp>
#endif

namespace webdav_ucp {

//=========================================================================

struct equalPropertyName
{
  bool operator()( const ::com::sun::star::beans::Property & p1,
                     const ::com::sun::star::beans::Property & p2 ) const
  {
        return !!( p1.Name == p2.Name );
  }
};

struct hashPropertyName
{
    size_t operator()( const ::com::sun::star::beans::Property & p ) const
    {
        return p.Name.hashCode();
    }
};

typedef std::hash_set
<
    ::com::sun::star::beans::Property,
    hashPropertyName,
    equalPropertyName
>
PropertyMap;

}

#endif
