/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: XPropertyTable.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-05-22 15:14:32 $
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

#ifndef _SVX_XPROPERTYTABLE_HXX
#define _SVX_XPROPERTYTABLE_HXX

#ifndef _COM_SUN_STAR_UNO_XINTERFACE_HPP_
#include <com/sun/star/uno/XInterface.hpp>
#endif

#ifndef INCLUDED_SVXDLLAPI_H
#include "svx/svxdllapi.h"
#endif

#ifndef _XTABLE_HXX
#include "xtable.hxx"
#endif

SVX_DLLPUBLIC ::com::sun::star::uno::Reference< com::sun::star::uno::XInterface > SAL_CALL SvxUnoXColorTable_createInstance( XPropertyTable* pTable ) throw();
SVX_DLLPUBLIC ::com::sun::star::uno::Reference< com::sun::star::uno::XInterface > SAL_CALL SvxUnoXLineEndTable_createInstance( XPropertyList* pList ) throw();
SVX_DLLPUBLIC ::com::sun::star::uno::Reference< com::sun::star::uno::XInterface > SAL_CALL SvxUnoXDashTable_createInstance( XPropertyList* pList ) throw();
SVX_DLLPUBLIC ::com::sun::star::uno::Reference< com::sun::star::uno::XInterface > SAL_CALL SvxUnoXHatchTable_createInstance( XPropertyList* pList ) throw();
SVX_DLLPUBLIC ::com::sun::star::uno::Reference< com::sun::star::uno::XInterface > SAL_CALL SvxUnoXGradientTable_createInstance( XPropertyList* pList ) throw();
SVX_DLLPUBLIC ::com::sun::star::uno::Reference< com::sun::star::uno::XInterface > SAL_CALL SvxUnoXBitmapTable_createInstance( XPropertyList* pList ) throw();

#endif


