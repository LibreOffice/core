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

#ifdef PCH
#endif

#ifdef _MSC_VER
#pragma hdrstop
#endif

// INCLUDE ---------------------------------------------------------------

#ifndef _SC_XMLTABLESHAPESCONTEXT_HXX
#include "XMLTableShapesContext.hxx"
#endif
#ifndef _SC_XMLTABLESHAPEIMPORTHELPER_HXX
#include "XMLTableShapeImportHelper.hxx"
#endif
#ifndef SC_XMLIMPRT_HXX
#include "xmlimprt.hxx"
#endif

namespace binfilter {

using namespace ::com::sun::star;

//------------------------------------------------------------------

ScXMLTableShapesContext::ScXMLTableShapesContext( ScXMLImport& rImport,
                                      USHORT nPrfx,
                                      const ::rtl::OUString& rLName,
                                      const ::com::sun::star::uno::Reference<
                                      ::com::sun::star::xml::sax::XAttributeList>& xAttrList) :
    SvXMLImportContext( rImport, nPrfx, rLName )
{
    // here are no attributes
}

ScXMLTableShapesContext::~ScXMLTableShapesContext()
{
}

SvXMLImportContext *ScXMLTableShapesContext::CreateChildContext( USHORT nPrefix,
                                            const ::rtl::OUString& rLName,
                                            const ::com::sun::star::uno::Reference<
                                          ::com::sun::star::xml::sax::XAttributeList>& xAttrList )
{
    SvXMLImportContext *pContext = 0;

    if (!pContext)
    {
        ScXMLImport& rXMLImport = GetScImport();
        uno::Reference<drawing::XShapes> xShapes (rXMLImport.GetTables().GetCurrentXShapes());
        if (xShapes.is())
        {
            XMLTableShapeImportHelper* pTableShapeImport = (XMLTableShapeImportHelper*)rXMLImport.GetShapeImport().get();
            pTableShapeImport->SetOnTable(sal_True);
            pContext = rXMLImport.GetShapeImport()->CreateGroupChildContext(
                rXMLImport, nPrefix, rLName, xAttrList, xShapes);
        }
    }

    if( !pContext )
        pContext = new SvXMLImportContext( GetImport(), nPrefix, rLName );

    return pContext;
}

void ScXMLTableShapesContext::EndElement()
{
}

}
