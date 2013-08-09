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

#ifndef _XMLOFF_DESCRIPTIONIMP_HXX
#define _XMLOFF_DESCRIPTIONIMP_HXX

#include <xmloff/xmlictxt.hxx>
#include <com/sun/star/drawing/XShape.hpp>

// office:events inside a shape

class SdXMLDescriptionContext : public SvXMLImportContext
{
private:
    com::sun::star::uno::Reference< com::sun::star::drawing::XShape > mxShape;
    OUString msText;
public:
    TYPEINFO();

    SdXMLDescriptionContext( SvXMLImport& rImport,
        sal_uInt16 nPrfx,
        const OUString& rLocalName,
        const com::sun::star::uno::Reference< com::sun::star::xml::sax::XAttributeList>& xAttrList,
        const com::sun::star::uno::Reference< com::sun::star::drawing::XShape >& rxShape );
    virtual ~SdXMLDescriptionContext();

    virtual void EndElement();

    // This method is called for all characters that are contained in the
    // current element. The default is to ignore them.
    virtual void Characters( const OUString& rChars );
};

#endif  //  _XMLOFF_EVENTIMP_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
