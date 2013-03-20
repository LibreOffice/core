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
#if 1

#include <xmloff/xmlimp.hxx>

namespace rtl { class OUString; }
namespace com { namespace sun { namespace star {
    namespace uno { template<class X> class Reference; }
    namespace uno { class XInterface; }
    namespace document { class XGraphicObjectResolver; }
    namespace container { class XNameContainer; }

} } }

class SvxXMLXTableImport : public SvXMLImport
{
public:
    // #110680#
    SvxXMLXTableImport(
        const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext > xContext,
        const ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameContainer > & rTable,
        com::sun::star::uno::Reference< com::sun::star::document::XGraphicObjectResolver >& xGrfResolver);

    virtual ~SvxXMLXTableImport() throw ();

    static bool load( const rtl::OUString &rPath,
                      const ::com::sun::star::uno::Reference < ::com::sun::star::embed::XStorage > &xStorage,
                      const ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameContainer >& xTable,
                      bool *bOptLoadedFromStorage ) throw();
protected:
    virtual SvXMLImportContext *CreateContext( sal_uInt16 nPrefix,
                                      const ::rtl::OUString& rLocalName,
                                      const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XAttributeList >& xAttrList );

private:
    const ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameContainer > & mrTable;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
