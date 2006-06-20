/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: xmldlg_imexp.hxx,v $
 *
 *  $Revision: 1.13 $
 *
 *  last change: $Author: hr $ $Date: 2006-06-20 05:10:43 $
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
#ifndef _COM_SUN_STAR_UNO_XCOMPONENTCONTEXT_HXX_
#include <com/sun/star/uno/XComponentContext.hpp>
#endif

#include "xmlscript/xmlns.h"


namespace xmlscript
{

//==============================================================================
void SAL_CALL exportDialogModel(
    ::com::sun::star::uno::Reference<
    ::com::sun::star::xml::sax::XExtendedDocumentHandler > const & xOut,
    ::com::sun::star::uno::Reference<
    ::com::sun::star::container::XNameContainer > const & xDialogModel )
    SAL_THROW( (::com::sun::star::uno::Exception) );

//==============================================================================
::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XDocumentHandler >
SAL_CALL importDialogModel(
    ::com::sun::star::uno::Reference<
    ::com::sun::star::container::XNameContainer > const & xDialogModel,
    ::com::sun::star::uno::Reference<
    ::com::sun::star::uno::XComponentContext > const & xContext )
    SAL_THROW( (::com::sun::star::uno::Exception) );

// additional functions for convenience

//==============================================================================
::com::sun::star::uno::Reference< ::com::sun::star::io::XInputStreamProvider >
SAL_CALL exportDialogModel(
    ::com::sun::star::uno::Reference<
    ::com::sun::star::container::XNameContainer > const & xDialogModel,
    ::com::sun::star::uno::Reference<
    ::com::sun::star::uno::XComponentContext > const & xContext )
    SAL_THROW( (::com::sun::star::uno::Exception) );

//==============================================================================
void SAL_CALL importDialogModel(
    ::com::sun::star::uno::Reference<
    ::com::sun::star::io::XInputStream > xInput,
    ::com::sun::star::uno::Reference<
    ::com::sun::star::container::XNameContainer > const & xDialogModel,
    ::com::sun::star::uno::Reference<
    ::com::sun::star::uno::XComponentContext > const & xContext )
    SAL_THROW( (::com::sun::star::uno::Exception) );

}

#endif
