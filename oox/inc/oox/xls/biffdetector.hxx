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



#ifndef OOX_XLS_BIFFDETECTOR_HXX
#define OOX_XLS_BIFFDETECTOR_HXX

#include <com/sun/star/document/XExtendedFilterDetection.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <cppuhelper/implbase2.hxx>
#include "oox/helper/storagebase.hxx"
#include "oox/xls/biffhelper.hxx"

namespace com { namespace sun { namespace star {
    namespace beans { struct PropertyValue; }
    namespace uno { class XComponentContext; }
} } }

namespace oox { class BinaryInputStream; }

namespace oox {
namespace xls {

// ============================================================================

/** Detection service for BIFF streams or storages. */
class BiffDetector : public ::cppu::WeakImplHelper2<
            ::com::sun::star::lang::XServiceInfo,
            ::com::sun::star::document::XExtendedFilterDetection >
{
public:
    explicit            BiffDetector(
                            const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >& rxContext )
                            throw( ::com::sun::star::uno::RuntimeException );

    virtual             ~BiffDetector();

    /** Detects the BIFF version of the passed stream. */
    static BiffType     detectStreamBiffVersion( BinaryInputStream& rInStream );

    /** Detects the BIFF version and workbook stream name of the passed storage. */
    static BiffType     detectStorageBiffVersion(
                            ::rtl::OUString& orWorkbookStreamName,
                            const StorageRef& rxStorage );

    // com.sun.star.lang.XServiceInfo interface -------------------------------

    virtual ::rtl::OUString SAL_CALL
                        getImplementationName() throw( ::com::sun::star::uno::RuntimeException );

    virtual sal_Bool SAL_CALL
                        supportsService( const ::rtl::OUString& rService )
                            throw( ::com::sun::star::uno::RuntimeException );

    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL
                        getSupportedServiceNames() throw( ::com::sun::star::uno::RuntimeException );

    // com.sun.star.document.XExtendedFilterDetect interface ------------------

    virtual ::rtl::OUString SAL_CALL
                        detect( ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& orDescriptor )
                            throw( ::com::sun::star::uno::RuntimeException );

private:
    ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >
                        mxContext;
};

// ============================================================================

} // namespace xls
} // namespace oox

#endif
