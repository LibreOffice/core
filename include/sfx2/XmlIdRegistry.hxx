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

#ifndef INCLUDED_SFX2_XMLIDREGISTRY_HXX
#define INCLUDED_SFX2_XMLIDREGISTRY_HXX

#include <sal/config.h>

#include <sfx2/dllapi.h>

#include <com/sun/star/beans/StringPair.hpp>


namespace com { namespace sun { namespace star { namespace rdf {
    class XMetadatable;
} } } }

namespace sfx2 {

// XML ID utilities --------------------------------------------------

/** is i_rIdref a valid NCName ? */
bool SFX2_DLLPUBLIC isValidNCName(OUString const & i_rIdref);

bool SFX2_DLLPUBLIC isValidXmlId(OUString const & i_rStreamName,
    OUString const & i_rIdref);

// XML ID handling ---------------------------------------------------

/** interface for getElementByMetadataReference;
    for use by sfx2::DocumentMetadataAccess
 */
class SFX2_DLLPUBLIC IXmlIdRegistry
{

public:
    virtual ~IXmlIdRegistry() { }

    virtual css::uno::Reference< css::rdf::XMetadatable >
        GetElementByMetadataReference(
            const css::beans::StringPair & i_rXmlId) const = 0;

};

/** supplier interface for the registry.

    This indirection is unfortunately necessary, because the SwDocShell
    is not always connected to a SwDoc, so we cannot guarantee that a
    registry given to a SfxBaseModel/DocumentMetadataAccess remains valid;
    it has to be retrieved from this supplier interface on access.
 */
class SFX2_DLLPUBLIC IXmlIdRegistrySupplier
{

public:
    virtual ~IXmlIdRegistrySupplier() { }

    /** override this if you have a XmlIdRegistry. */
    virtual const IXmlIdRegistry* GetXmlIdRegistry() const { return nullptr; }

};

} // namespace sfx2

#endif // INCLUDED_SFX2_XMLIDREGISTRY_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
