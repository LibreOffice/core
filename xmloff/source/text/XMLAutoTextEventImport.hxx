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

#ifndef INCLUDED_XMLOFF_SOURCE_TEXT_XMLAUTOTEXTEVENTIMPORT_HXX
#define INCLUDED_XMLOFF_SOURCE_TEXT_XMLAUTOTEXTEVENTIMPORT_HXX

#include <xmloff/xmlimp.hxx>
#include <com/sun/star/container/XNameReplace.hpp>
#include <com/sun/star/uno/Reference.hxx>


namespace com { namespace sun { namespace star {
    namespace frame { class XModel; }
    namespace text { class XAutoTextContainer; }
    namespace text { class XAutoTextGroup; }
    namespace text { class XAutoTextEntry; }
    namespace uno { template<class X> class Reference; }
    namespace uno { template<class X> class Sequence; }
    namespace xml { namespace sax { class XDocumentHandler; } }
} } }


class XMLAutoTextEventImport : public SvXMLImport
{
    css::uno::Reference<css::container::XNameReplace> xEvents;

public:
    explicit XMLAutoTextEventImport(
        const css::uno::Reference< css::uno::XComponentContext >& xContext);

    virtual ~XMLAutoTextEventImport() throw() override;

    // XInitialization
    virtual void SAL_CALL initialize(
        const css::uno::Sequence<css::uno::Any> & rArguments ) override;

protected:

    virtual SvXMLImportContext* CreateDocumentContext(
        sal_uInt16 nPrefix,
        const OUString& rLocalName,
        const css::uno::Reference< css::xml::sax::XAttributeList > & xAttrList ) override;

};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
