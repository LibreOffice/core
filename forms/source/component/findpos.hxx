/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: findpos.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: hr $ $Date: 2006-06-19 12:54:30 $
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

#ifndef INCLUDED_FORMS_SOURCE_COMPONENT_FINDPOS_HXX
#define INCLUDED_FORMS_SOURCE_COMPONENT_FINDPOS_HXX

#ifndef _SAL_CONFIG_H_
#include "sal/config.h"
#endif

#ifndef _COM_SUN_STAR_UNO_SEQUENCE_HXX_
#include "com/sun/star/uno/Sequence.hxx"
#endif
#ifndef _SAL_TYPES_H_
#include "sal/types.h"
#endif

namespace rtl { class OUString; }

namespace detail {

sal_Int32 findPos(
    const ::rtl::OUString& aStr,
    const ::com::sun::star::uno::Sequence< ::rtl::OUString >& rList);

}

#endif
