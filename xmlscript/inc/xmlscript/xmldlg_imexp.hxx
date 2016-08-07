/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/


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
#include "xmlscript/xcrdllapi.h"

namespace xmlscript
{

//==============================================================================
XCR_DLLPUBLIC void SAL_CALL exportDialogModel(
    ::com::sun::star::uno::Reference<
    ::com::sun::star::xml::sax::XExtendedDocumentHandler > const & xOut,
    ::com::sun::star::uno::Reference<
    ::com::sun::star::container::XNameContainer > const & xDialogModel )
    SAL_THROW( (::com::sun::star::uno::Exception) );

//==============================================================================
XCR_DLLPUBLIC ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XDocumentHandler >
SAL_CALL importDialogModel(
    ::com::sun::star::uno::Reference<
    ::com::sun::star::container::XNameContainer > const & xDialogModel,
    ::com::sun::star::uno::Reference<
    ::com::sun::star::uno::XComponentContext > const & xContext )
    SAL_THROW( (::com::sun::star::uno::Exception) );

// additional functions for convenience

//==============================================================================
XCR_DLLPUBLIC ::com::sun::star::uno::Reference< ::com::sun::star::io::XInputStreamProvider >
SAL_CALL exportDialogModel(
    ::com::sun::star::uno::Reference<
    ::com::sun::star::container::XNameContainer > const & xDialogModel,
    ::com::sun::star::uno::Reference<
    ::com::sun::star::uno::XComponentContext > const & xContext )
    SAL_THROW( (::com::sun::star::uno::Exception) );

//==============================================================================
XCR_DLLPUBLIC void SAL_CALL importDialogModel(
    ::com::sun::star::uno::Reference<
    ::com::sun::star::io::XInputStream > xInput,
    ::com::sun::star::uno::Reference<
    ::com::sun::star::container::XNameContainer > const & xDialogModel,
    ::com::sun::star::uno::Reference<
    ::com::sun::star::uno::XComponentContext > const & xContext )
    SAL_THROW( (::com::sun::star::uno::Exception) );

}

#endif
