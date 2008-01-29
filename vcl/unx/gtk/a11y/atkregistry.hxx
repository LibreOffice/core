/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: atkregistry.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2008-01-29 16:19:47 $
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

#ifndef __ATK_REGISTRY_HXX__
#define __ATK_REGISTRY_HXX__

#include <com/sun/star/accessibility/XAccessible.hpp>
#include <atk/atk.h>

AtkObject * ooo_wrapper_registry_get(const ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible >& rxAccessible);

void ooo_wrapper_registry_add(const ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible >& rxAccessible, AtkObject *obj);

void ooo_wrapper_registry_remove(::com::sun::star::accessibility::XAccessible *pAccessible);

#endif // __ATK_REGISTRY_HXX_
