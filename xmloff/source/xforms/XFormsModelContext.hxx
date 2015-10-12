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

#ifndef INCLUDED_XMLOFF_SOURCE_XFORMS_XFORMSMODELCONTEXT_HXX
#define INCLUDED_XMLOFF_SOURCE_XFORMS_XFORMSMODELCONTEXT_HXX

#include "TokenContext.hxx"
#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/xforms/XModel2.hpp>

namespace com { namespace sun { namespace star {
    namespace xml { namespace sax { class XAttributeList; } }
    namespace beans { class XPropertySet; }
} } }

class SvXMLImport;
class SvXMLImportContext;

/** import the xforms:model element */
class XFormsModelContext : public TokenContext
{
    com::sun::star::uno::Reference<com::sun::star::xforms::XModel2> mxModel;

public:
    XFormsModelContext( SvXMLImport& rImport,
                        sal_uInt16 nPrfx,
                        const OUString& rLName );
    virtual ~XFormsModelContext();

    com::sun::star::uno::Reference<com::sun::star::xforms::XModel2> getModel() { return mxModel;}

    // implement SvXMLImportContext & TokenContext methods:

    virtual void EndElement() override;

protected:
    virtual void HandleAttribute(
        sal_uInt16 nToken,
        const OUString& rValue ) override;

    virtual SvXMLImportContext* HandleChild(
        sal_uInt16 nToken,
        sal_uInt16 nPrefix,
        const OUString& rLocalName,
        const com::sun::star::uno::Reference<com::sun::star::xml::sax::XAttributeList>& xAttrList ) override;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
