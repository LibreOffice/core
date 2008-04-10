/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: querytablefragment.cxx,v $
 * $Revision: 1.4 $
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

#include "oox/xls/querytablefragment.hxx"
#include "oox/xls/webquerybuffer.hxx"

using ::rtl::OUString;
using ::com::sun::star::uno::Reference;

namespace oox {
namespace xls {

OoxQueryTableFragment::OoxQueryTableFragment(
        const WorkbookHelper& rHelper, const OUString& rFragmentPath ) :
    OoxWorkbookFragmentBase( rHelper, rFragmentPath )
{
}

ContextWrapper OoxQueryTableFragment::onCreateContext( sal_Int32 nElement, const AttributeList& )
{
    switch( getCurrentElement() )
    {
        case XML_ROOT_CONTEXT:
            return  (nElement == XLS_TOKEN( queryTable ));
    }
    return false;
}

void OoxQueryTableFragment::onStartElement( const AttributeList& rAttribs )
{
    switch ( getCurrentElement() )
    {
        case XLS_TOKEN( queryTable ):
            importQueryTable( rAttribs );
        break;
    }
}

void OoxQueryTableFragment::importQueryTable( const AttributeList& rAttribs )
{
    getWebQueries().importQueryTable( rAttribs );
}

} // namespace xls
} // namespace oox

