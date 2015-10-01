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

#ifndef INCLUDED_FORMS_SOURCE_XFORMS_XMLHELPER_HXX
#define INCLUDED_FORMS_SOURCE_XFORMS_XMLHELPER_HXX

#include <sal/types.h>
#include <rtl/ustring.hxx>

namespace com { namespace sun { namespace star {
    namespace uno { template<typename T> class Reference; }
    namespace container { class XNameContainer; }
    namespace xml { namespace dom { class XDocumentBuilder; } }
} } }


bool isValidQName( const OUString& sName,
                   const css::uno::Reference<css::container::XNameContainer>& xNamespaces );

bool isValidPrefixName( const OUString& sName,
                        const css::uno::Reference<css::container::XNameContainer>& xNamespaces );

css::uno::Reference<css::xml::dom::XDocumentBuilder> getDocumentBuilder();


#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
