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
#ifndef DBA_XMLDOCUMENTS_HXX
#define DBA_XMLDOCUMENTS_HXX

#include <xmloff/xmlictxt.hxx>
#include "xmlEnums.hxx"
#include <com/sun/star/container/XNameAccess.hpp>

namespace dbaxml
{
    class ODBFilter;
    class OXMLDocuments : public SvXMLImportContext
    {
    private:
        ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess > m_xContainer;
        OUString     m_sCollectionServiceName;
        OUString     m_sComponentServiceName;

        ODBFilter& GetOwnImport();
    public:

        // for forms and reports
        OXMLDocuments( ODBFilter& rImport
                    ,sal_uInt16 nPrfx
                    ,const OUString& rLName
                    ,const ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess >& _xContainer
                    ,const OUString& _sCollectionServiceName
                    ,const OUString& _sComponentServiceName);

        // for queries
        OXMLDocuments( ODBFilter& rImport
                    ,sal_uInt16 nPrfx
                    ,const OUString& rLName
                    ,const ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess >& _xContainer
                    ,const OUString& _sCollectionServiceName = OUString()
                    );

        virtual ~OXMLDocuments();

        virtual SvXMLImportContext *CreateChildContext( sal_uInt16 nPrefix,
                    const OUString& rLocalName,
                    const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XAttributeList > & xAttrList );
    };
} // namespace dbaxml

#endif // DBA_XMLDOCUMENTS_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
