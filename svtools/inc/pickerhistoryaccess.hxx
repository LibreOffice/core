/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: pickerhistoryaccess.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: hr $ $Date: 2006-06-19 20:24:44 $
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

#ifndef SVTOOLS_PICKERHISTORYACCESS_HXX
#define SVTOOLS_PICKERHISTORYACCESS_HXX

#ifndef INCLUDED_SVLDLLAPI_H
#include "svtools/svldllapi.h"
#endif

#ifndef _COM_SUN_STAR_UNO_REFERENX_HXX_
#include <com/sun/star/uno/Reference.hxx>
#endif

//.........................................................................
namespace svt
{
//.........................................................................

    // --------------------------------------------------------------------
    SVL_DLLPUBLIC void addFolderPicker(
        const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >& _rxPicker );

    SVL_DLLPUBLIC void addFilePicker(
        const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >& _rxPicker );

//.........................................................................
}   // namespace svt
//.........................................................................

#endif // SVTOOLS_PICKERHISTORYACCESS_HXX

