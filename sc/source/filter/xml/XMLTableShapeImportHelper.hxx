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

#pragma once

#include <xmloff/shapeimport.hxx>
#include <svx/svdtypes.hxx>
#include <address.hxx>

class ScXMLImport;
class ScXMLAnnotationContext;

class XMLTableShapeImportHelper : public XMLShapeImportHelper
{
    ScAddress aStartCell;
    ScXMLAnnotationContext* pAnnotationContext;
    bool bOnTable;

public:

    explicit XMLTableShapeImportHelper( ScXMLImport& rImp );
    virtual ~XMLTableShapeImportHelper() override;

    static void SetLayer(const css::uno::Reference<css::drawing::XShape>& rShape, SdrLayerID nLayerID, std::u16string_view sType);
    virtual void finishShape(css::uno::Reference< css::drawing::XShape >& rShape,
            const css::uno::Reference< css::xml::sax::XFastAttributeList >& xAttrList,
            css::uno::Reference< css::drawing::XShapes >& rShapes) override;

    void SetCell (const ScAddress& rAddress) { aStartCell = rAddress; }
    void SetOnTable (const bool bTempOnTable) { bOnTable = bTempOnTable; }
    void SetAnnotation(ScXMLAnnotationContext* pAnnotation) { pAnnotationContext = pAnnotation; }

    ScXMLAnnotationContext* GetAnnotationContext() const    { return pAnnotationContext; }
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
