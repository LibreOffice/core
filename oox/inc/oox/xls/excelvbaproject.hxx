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

#ifndef OOX_XLS_EXCELVBAPROJECT_HXX
#define OOX_XLS_EXCELVBAPROJECT_HXX

#include "oox/ole/vbaproject.hxx"
#include "oox/dllapi.h"

namespace com { namespace sun { namespace star {
        namespace sheet { class XSpreadsheetDocument; }
} } }

namespace oox {
namespace xls {

// ============================================================================

/** Special implementation of the VBA project for the Excel filters. */
class OOX_DLLPUBLIC VbaProject : public ::oox::ole::VbaProject
{
public:
    explicit            VbaProject(
                            const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& rxGlobalFactory,
                            const ::com::sun::star::uno::Reference< ::com::sun::star::sheet::XSpreadsheetDocument >& rxDocument );

    /** Attaches all document and sheet events to existing VBA macros. */
    void                attachToEvents();

private:
    /** Attaches VBA macros to all supported document events. */
    void                attachToDocumentEvents( const ::rtl::OUString& rCodeName );

    /** Attaches VBA macros to all supported sheet events. */
    void                attachToSheetEvents(
                            const ::com::sun::star::uno::Reference< ::com::sun::star::document::XEventsSupplier >& rxEventsSupp,
                            const ::rtl::OUString& rCodeName );

private:
    ::com::sun::star::uno::Reference< ::com::sun::star::sheet::XSpreadsheetDocument >
                        mxDocument;
};

// ============================================================================

} // namespace xls
} // namespace oox

#endif
