/*************************************************************************
 *
 *  $RCSfile: xmldlg_imexp.hxx,v $
 *
 *  $Revision: 1.8 $
 *
 *  last change: $Author: dbo $ $Date: 2001-03-28 10:50:35 $
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
#ifndef _XMLSCRIPT_XMLDLG_IMEXP_HXX_
#define _XMLSCRIPT_XMLDLG_IMEXP_HXX_

#ifndef _COM_SUN_STAR_CONTAINER_XNAMECONTAINER_HXX_
#include <com/sun/star/container/XNameContainer.hpp>
#endif
#ifndef _COM_SUN_STAR_IO_XINPUTSTREAMPROVIDER_HXX_
#include <com/sun/star/io/XInputStreamProvider.hpp>
#endif
#ifndef _COM_SUN_STAR_XML_SAX_XEXTENDEDDOCUMENTHANDLER_HXX_
#include <com/sun/star/xml/sax/XExtendedDocumentHandler.hpp>
#endif


namespace xmlscript
{

#define XMLNS_DIALOGS_URI "http://openoffice.org/2000/dialog"
#define XMLNS_DIALOGS_UID 1
#define XMLNS_DIALOGS_PREFIX "dlg"


//==================================================================================================
void SAL_CALL exportDialogModel(
    ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XExtendedDocumentHandler > const & xOut,
    ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameContainer > const & xDialogModel )
    SAL_THROW( (::com::sun::star::uno::Exception) );
//==================================================================================================
::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XDocumentHandler >
SAL_CALL importDialogModel(
    ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameContainer > const & xDialogModel )
    SAL_THROW( (::com::sun::star::uno::Exception) );

// additional functions for convenience

//==================================================================================================
::com::sun::star::uno::Reference< ::com::sun::star::io::XInputStreamProvider >
SAL_CALL exportDialogModel(
    ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameContainer > const & xDialogModel )
    SAL_THROW( (::com::sun::star::uno::Exception) );
//==================================================================================================
void SAL_CALL importDialogModel(
    ::com::sun::star::uno::Reference< ::com::sun::star::io::XInputStream > xInput,
    ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameContainer > const & xDialogModel )
    SAL_THROW( (::com::sun::star::uno::Exception) );

};

#endif
