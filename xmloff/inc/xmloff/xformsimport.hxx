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


