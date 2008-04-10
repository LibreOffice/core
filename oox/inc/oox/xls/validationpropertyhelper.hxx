/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: validationpropertyhelper.hxx,v $
 * $Revision: 1.3 $
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

#ifndef OOX_XLS_VALIDATIONPROPERTYHELPER_HXX
#define OOX_XLS_VALIDATIONPROPERTYHELPER_HXX

#include <com/sun/star/sheet/ConditionOperator.hpp>
#include "oox/helper/propertysequence.hxx"
#include "oox/xls/workbookhelper.hxx"

namespace oox {
namespace xls {

// ============================================================================

struct OoxValidationData;

/** Helper for data validation related properties. */
class ValidationPropertyHelper : public WorkbookHelper
{
public:
    explicit            ValidationPropertyHelper( const WorkbookHelper& rHelper );

    /** Writes data validation properties to the passed property set. */
    void                writeValidationProperties(
                            PropertySet& rPropSet,
                            const OoxValidationData& rValData );

    /** Converts an OOXML condition operator token to the API constant. */
    static ::com::sun::star::sheet::ConditionOperator
                        convertToApiOperator( sal_Int32 nToken );

private:
    PropertySequence    maValProps;
    const ::rtl::OUString maValidationProp;     /// Property name for data validation settings.
};

// ============================================================================

} // namespace xls
} // namespace oox

#endif

