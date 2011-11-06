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



#ifndef OOX_CORE_FASTTOKENHANDLER_HXX
#define OOX_CORE_FASTTOKENHANDLER_HXX

#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/xml/sax/XFastTokenHandler.hpp>
#include <cppuhelper/implbase2.hxx>

namespace oox { class TokenMap; }

namespace oox {
namespace core {

// ============================================================================

typedef ::cppu::WeakImplHelper2< ::com::sun::star::lang::XServiceInfo, ::com::sun::star::xml::sax::XFastTokenHandler > FastTokenHandler_BASE;

/** Wrapper implementing the com.sun.star.xml.sax.XFastTokenHandler API interface
    that provides access to the tokens generated from the internal token name list.
 */
class FastTokenHandler : public FastTokenHandler_BASE
{
public:
    explicit            FastTokenHandler();
    virtual             ~FastTokenHandler();

    // XServiceInfo
    virtual ::rtl::OUString SAL_CALL getImplementationName() throw (::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL supportsService( const ::rtl::OUString& rServiceName ) throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames() throw (::com::sun::star::uno::RuntimeException);

    // XFastTokenHandler
    virtual sal_Int32 SAL_CALL getToken( const ::rtl::OUString& rIdentifier ) throw (::com::sun::star::uno::RuntimeException);
    virtual ::rtl::OUString SAL_CALL getIdentifier( sal_Int32 nToken ) throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< sal_Int8 > SAL_CALL getUTF8Identifier( sal_Int32 nToken ) throw (::com::sun::star::uno::RuntimeException);
    virtual sal_Int32 SAL_CALL getTokenFromUTF8( const ::com::sun::star::uno::Sequence< sal_Int8 >& Identifier ) throw (::com::sun::star::uno::RuntimeException);

private:
    const TokenMap&     mrTokenMap;     /// Reference to global token map singleton.
};

// ============================================================================

} // namespace core
} // namespace oox

#endif
