/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



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
        (i_rStreamName.equalsAscii("content.xml") ||
         i_rStreamName.equalsAscii("styles.xml"));
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

