/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: XMLTableShapeImportHelper.hxx,v $
 *
 *  $Revision: 1.10 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 19:59:29 $
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

#ifndef _SC_XMLTABLESHAPEIMPORTHELPER_HXX
#define _SC_XMLTABLESHAPEIMPORTHELPER_HXX

#ifndef _XMLOFF_SHAPEIMPORT_HXX_
#include <xmloff/shapeimport.hxx>
#endif
#ifndef _COM_SUN_STAR_TABLE_CELLADDRESS_HPP_
#include <com/sun/star/table/CellAddress.hpp>
#endif

class ScXMLImport;
class ScXMLAnnotationContext;

class XMLTableShapeImportHelper : public XMLShapeImportHelper
{
    ::com::sun::star::table::CellAddress aStartCell;
    ScXMLAnnotationContext* pAnnotationContext;
    sal_Bool bOnTable;

public:

    XMLTableShapeImportHelper( ScXMLImport& rImp, SvXMLImportPropertyMapper *pImpMapper=0 );
    ~XMLTableShapeImportHelper();

    void SetLayer(com::sun::star::uno::Reference<com::sun::star::drawing::XShape>& rShape, sal_Int16 nLayerID, const rtl::OUString& sType) const;
    virtual void finishShape(com::sun::star::uno::Reference< com::sun::star::drawing::XShape >& rShape,
            const com::sun::star::uno::Reference< com::sun::star::xml::sax::XAttributeList >& xAttrList,
            com::sun::star::uno::Reference< com::sun::star::drawing::XShapes >& rShapes);


    void SetCell (const ::com::sun::star::table::CellAddress& rAddress) { aStartCell = rAddress; }
    void SetOnTable (const sal_Bool bTempOnTable) { bOnTable = bTempOnTable; }
    void SetAnnotation(ScXMLAnnotationContext* pAnnotation) { pAnnotationContext = pAnnotation; }
};


#endif
