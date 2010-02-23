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

#ifndef OOX_XLS_BIFFDETECTOR_HXX
#define OOX_XLS_BIFFDETECTOR_HXX

#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/document/XExtendedFilterDetection.hpp>
#include <cppuhelper/implbase2.hxx>
#include "oox/helper/storagebase.hxx"
#include "oox/xls/biffhelper.hxx"

namespace com { namespace sun { namespace star {
    namespace lang { class XMultiServiceFactory; }
    namespace beans { struct PropertyValue; }
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
                            const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& rxFactory );

    virtual             ~BiffDetector();

    /** Detects the BIFF version of the passed stream. */
    static BiffType     detectStreamBiffVersion( BinaryInputStream& rInStream );

    /** Detects the BIFF version and workbook stream name of the passed storage. */
    static BiffType     detectStorageBiffVersion(
                            ::rtl::OUString& orWorkbookStreamName,
                            StorageRef xStorage );

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
    ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >
                        mxFactory;
};

// ============================================================================

} // namespace xls
} // namespace oox

#endif

