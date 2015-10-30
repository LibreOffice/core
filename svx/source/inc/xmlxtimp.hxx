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

#ifndef INCLUDED_SVX_SOURCE_INC_XMLXTIMP_HXX
#define INCLUDED_SVX_SOURCE_INC_XMLXTIMP_HXX

#include <xmloff/xmlimp.hxx>

namespace com { namespace sun { namespace star {
    namespace uno { template<class X> class Reference; }
    namespace uno { class XInterface; }
    namespace document { class XGraphicObjectResolver; }
    namespace container { class XNameContainer; }

} } }

class SvxXMLXTableImport : public SvXMLImport
{
public:
    SvxXMLXTableImport(
        const css::uno::Reference< css::uno::XComponentContext >& rContext,
        const css::uno::Reference< css::container::XNameContainer > & rTable,
        css::uno::Reference< css::document::XGraphicObjectResolver >& rGrfResolver);

    virtual ~SvxXMLXTableImport() throw ();

    static bool load( const OUString &rPath, const OUString &rReferer,
                      const css::uno::Reference < css::embed::XStorage > &xStorage,
                      const css::uno::Reference< css::container::XNameContainer >& xTable,
                      bool *bOptLoadedFromStorage ) throw();
protected:
    virtual SvXMLImportContext *CreateContext( sal_uInt16 nPrefix,
                                      const OUString& rLocalName,
                                      const css::uno::Reference< css::xml::sax::XAttributeList >& xAttrList ) override;

private:
    const css::uno::Reference< css::container::XNameContainer > & mrTable;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
