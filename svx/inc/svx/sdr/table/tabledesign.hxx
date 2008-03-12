/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: tabledesign.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2008-03-12 09:34:27 $
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

#ifndef _SDR_TABLE_DESIGN_HXX
#define _SDR_TABLE_DESIGN_HXX

#ifndef INCLUDED_SVXDLLAPI_H
#include "svx/svxdllapi.h"
#endif

#include <com/sun/star/container/XNameAccess.hpp>

namespace sdr { namespace table {

const sal_Int32 first_row_style = 0;
const sal_Int32 last_row_style = 1;
const sal_Int32 first_column_style = 2;
const sal_Int32 last_column_style = 3;
const sal_Int32 even_rows_style = 4;
const sal_Int32 odd_rows_style = 5;
const sal_Int32 even_columns_style = 6;
const sal_Int32 odd_columns_style = 7;
const sal_Int32 body_style = 8;
const sal_Int32 background_style = 9;
const sal_Int32 style_count = 10;

extern SVX_DLLPUBLIC ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess > CreateTableDesignFamily();

} }


#endif
