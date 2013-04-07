/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#ifndef SC_XMLTABLESHAPEIMPORTHELPER_HXX
#define SC_XMLTABLESHAPEIMPORTHELPER_HXX

#include <xmloff/shapeimport.hxx>
#include <com/sun/star/table/CellAddress.hpp>

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

    void SetLayer(com::sun::star::uno::Reference<com::sun::star::drawing::XShape>& rShape, sal_Int16 nLayerID, const OUString& sType) const;
    virtual void finishShape(com::sun::star::uno::Reference< com::sun::star::drawing::XShape >& rShape,
            const com::sun::star::uno::Reference< com::sun::star::xml::sax::XAttributeList >& xAttrList,
            com::sun::star::uno::Reference< com::sun::star::drawing::XShapes >& rShapes);


    void SetCell (const ::com::sun::star::table::CellAddress& rAddress) { aStartCell = rAddress; }
    void SetOnTable (const sal_Bool bTempOnTable) { bOnTable = bTempOnTable; }
    void SetAnnotation(ScXMLAnnotationContext* pAnnotation) { pAnnotationContext = pAnnotation; }

    ScXMLAnnotationContext* GetAnnotationContext() const    { return pAnnotationContext; }
};


#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
