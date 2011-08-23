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

#ifndef _SC_XMLTABLESHAPEIMPORTHELPER_HXX
#define _SC_XMLTABLESHAPEIMPORTHELPER_HXX

#ifndef _XMLOFF_SHAPEIMPORT_HXX_
#include <bf_xmloff/shapeimport.hxx>
#endif
#ifndef _COM_SUN_STAR_TABLE_CELLADDRESS_HPP_
#include <com/sun/star/table/CellAddress.hpp>
#endif
namespace binfilter {

class ScXMLImport;

class XMLTableShapeImportHelper : public XMLShapeImportHelper
{
    ::com::sun::star::table::CellAddress aStartCell;
    sal_Bool bOnTable : 1;

public:

    XMLTableShapeImportHelper( ScXMLImport& rImp, SvXMLImportPropertyMapper *pImpMapper=0 );
    ~XMLTableShapeImportHelper();

    void SetLayer(::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape>& rShape, sal_Int16 nLayerID, const ::rtl::OUString& sType) const;
    virtual void finishShape(::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape >& rShape,
            const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XAttributeList >& xAttrList,
            ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShapes >& rShapes);


    void SetCell (const ::com::sun::star::table::CellAddress& rAddress) { aStartCell = rAddress; }
    void SetOnTable (const sal_Bool bTempOnTable) { bOnTable = bTempOnTable; }
};


} //namespace binfilter
#endif
