/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: xformsimport.hxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: vg $ $Date: 2006-09-25 13:21:15 $
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
#ifndef _XMLOFF_XFORMSIMPORT_HXX
#define _XMLOFF_XFORMSIMPORT_HXX

#ifndef _SAL_CONFIG_H_
#include "sal/config.h"
#endif

#ifndef INCLUDED_XMLOFF_DLLAPI_H
#include "xmloff/dllapi.h"
#endif

#include <tools/solar.h> // for USHORT
#include <com/sun/star/uno/Reference.hxx>

class SvXMLImport;
class SvXMLImportContext;
namespace rtl { class OUString; }

#if defined(WNT) && _MSC_VER < 1400
namespace std { template<typename A, typename B> struct pair; }
#endif

namespace com { namespace sun { namespace star {
    namespace uno { template<typename T> class Reference; }
    namespace beans { class XPropertySet; }
    namespace frame { class XModel; }
} } }

/** create import context for xforms:model element. */
XMLOFF_DLLPUBLIC SvXMLImportContext* createXFormsModelContext(
    SvXMLImport& rImport,
    USHORT nPrefix,
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


#endif


