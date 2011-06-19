/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#ifndef _SFX_XMLIDREGISTRY_HXX_
#define _SFX_XMLIDREGISTRY_HXX_

#include <sal/config.h>

#include <sfx2/dllapi.h>

#include <com/sun/star/beans/StringPair.hpp>


namespace com { namespace sun { namespace star { namespace rdf {
    class XMetadatable;
} } } }

namespace sfx2 {

// XML ID utilities --------------------------------------------------

/** is i_rIdref a valid NCName ? */
bool SFX2_DLLPUBLIC isValidNCName(::rtl::OUString const & i_rIdref);

extern inline bool
isValidXmlId(::rtl::OUString const & i_rStreamName,
    ::rtl::OUString const & i_rIdref)
{
    return isValidNCName(i_rIdref) &&
        (i_rStreamName.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM("content.xml")) ||
         i_rStreamName.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM("styles.xml")));
}


// XML ID handling ---------------------------------------------------

/** interface for getElementByMetadataReference;
    for use by sfx2::DocumentMetadataAccess
 */
class SFX2_DLLPUBLIC IXmlIdRegistry
{

public:
    virtual ~IXmlIdRegistry() { }

    virtual ::com::sun::star::uno::Reference<
            ::com::sun::star::rdf::XMetadatable > SAL_CALL
        GetElementByMetadataReference(
            const ::com::sun::star::beans::StringPair & i_rXmlId) const = 0;

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
    virtual const IXmlIdRegistry* GetXmlIdRegistry() const { return 0; }

};

} // namespace sfx2

#endif // _SFX_XMLIDREGISTRY_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
