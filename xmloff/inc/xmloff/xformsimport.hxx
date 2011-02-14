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
#ifndef _XMLOFF_XFORMSIMPORT_HXX
#define _XMLOFF_XFORMSIMPORT_HXX

#include "sal/config.h"
#include "xmloff/dllapi.h"

#include <tools/solar.h> // for sal_uInt16
#include <com/sun/star/uno/Reference.hxx>

class SvXMLImport;
class SvXMLImportContext;
namespace rtl { class OUString; }

#if defined(WNT) && _MSC_VER < 1400
namespace std { template<typename A, typename B> struct pair; }
#endif

namespace com { namespace sun { namespace star {
    namespace uno { template<typename T> class Reference; }
    namespace beans { class XPropertySet; struct PropertyValue; }
    namespace frame { class XModel; }
    namespace container { class XNameAccess; }
} } }

/** create import context for xforms:model element. */
XMLOFF_DLLPUBLIC SvXMLImportContext* createXFormsModelContext(
    SvXMLImport& rImport,
    sal_uInt16 nPrefix,
    const rtl::OUString& rLocalName );

/** perform the actual binding of an XForms-binding with the suitable control
 *  @param document which contains the XForms-model(s)
 *  @param pair<XForms binding ID, reference to control>
 */
void bindXFormsValueBinding(
    com::sun::star::uno::Reference<com::sun::star::frame::XModel>,
    std::pair<com::sun::star::uno::Reference<com::sun::star::beans::XPropertySet>,rtl::OUString> );


/** perform the actual binding of an XForms-binding as list source with a list control
 *  @param document which contains the XForms-model(s)
 *  @param pair<XForms binding ID, reference to control>
 */
void bindXFormsListBinding(
    com::sun::star::uno::Reference<com::sun::star::frame::XModel>,
    std::pair<com::sun::star::uno::Reference<com::sun::star::beans::XPropertySet>,rtl::OUString> );

/** perform the actual binding of an XForms submission with the suitable control
 *  @param document which contains the XForms-model(s)
 *  @param pair<XForms submission ID, reference to control>
 */
void bindXFormsSubmission(
    com::sun::star::uno::Reference<com::sun::star::frame::XModel>,
    std::pair<com::sun::star::uno::Reference<com::sun::star::beans::XPropertySet>,rtl::OUString> );

/** applies the given settings to the given XForms container
*/
void XMLOFF_DLLPUBLIC applyXFormsSettings(
        const ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess >& _rXForms,
        const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& _rSettings
    );

#endif


