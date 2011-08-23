/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 * 
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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


#include <hintids.hxx>

#ifndef _COM_SUN_STAR_FRAME_XMODEL_HPP_
#include <com/sun/star/frame/XModel.hpp>
#endif
#ifndef _COM_SUN_STAR_DOCUMENT_XDOCUMENTINFOSUPPLIER_HPP_
#include <com/sun/star/document/XDocumentInfoSupplier.hpp>
#endif

#ifndef _XMLOFF_XMLNMSPE_HXX
#include <bf_xmloff/xmlnmspe.hxx>
#endif

#ifndef _XMLOFF_XMLMETAI_HXX
#include <bf_xmloff/xmlscripti.hxx>
#endif

#ifndef _SVX_LANGITEM_HXX
#include <bf_svx/langitem.hxx>
#endif

#ifndef _SWDOCSH_HXX
#include "docsh.hxx"
#endif

#ifndef _HORIORNT_HXX
#include <horiornt.hxx>
#endif

#ifndef _DOC_HXX //autogen wg. SwDoc
#include <doc.hxx>
#endif

#ifndef _XMLIMP_HXX
#include "xmlimp.hxx"
#endif
#ifndef _XMLEXP_HXX
#include "xmlexp.hxx"
#endif
namespace binfilter {

using namespace ::rtl;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;

// ---------------------------------------------------------------------

SvXMLImportContext *SwXMLImport::CreateScriptContext(
                                       const OUString& rLocalName )
{
    SvXMLImportContext *pContext = 0;

    if( !(IsStylesOnlyMode() || IsInsertMode()) )
    {
        pContext = new XMLScriptContext( *this,
                                    XML_NAMESPACE_OFFICE, rLocalName,
                                    GetModel() );
    }
    
    if( !pContext )
        pContext = new SvXMLImportContext( *this, XML_NAMESPACE_OFFICE,
                                              rLocalName );

    return pContext;
}

}
