/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: validationpropertyhelper.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2008-01-17 08:05:49 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
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

