/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */
#ifndef INCLUDED_XMLOFF_XFORMSIMPORT_HXX
#define INCLUDED_XMLOFF_XFORMSIMPORT_HXX

#include <sal/config.h>
#include <xmloff/dllapi.h>
#include <rtl/ustring.hxx>

class SvXMLImport;
class SvXMLImportContext;

namespace com { namespace sun { namespace star {
    namespace uno { template<typename T> class Reference; }
    namespace uno { template <typename > class Sequence; }
    namespace beans { class XPropertySet; struct PropertyValue; }
    namespace frame { class XModel; }
    namespace container { class XNameAccess; }
} } }

/** create import context for xforms:model element. */
XMLOFF_DLLPUBLIC SvXMLImportContext* createXFormsModelContext(
    SvXMLImport& rImport,
    sal_uInt16 nPrefix,
    const OUString& rLocalName );

/** perform the actual binding of an XForms-binding with the suitable control
 *  @param document which contains the XForms-model(s)
 *  @param pair<XForms binding ID, reference to control>
 */
void bindXFormsValueBinding(
    css::uno::Reference<css::frame::XModel> const&,
    const std::pair<css::uno::Reference<css::beans::XPropertySet>, OUString>&);

/** perform the actual binding of an XForms-binding as list source with a list control
 *  @param document which contains the XForms-model(s)
 *  @param pair<XForms binding ID, reference to control>
 */
void bindXFormsListBinding(
    css::uno::Reference<css::frame::XModel> const&,
    const std::pair<css::uno::Reference<css::beans::XPropertySet>, OUString>&);

/** perform the actual binding of an XForms submission with the suitable control
 *  @param document which contains the XForms-model(s)
 *  @param pair<XForms submission ID, reference to control>
 */
void bindXFormsSubmission(
    css::uno::Reference<css::frame::XModel> const&,
    const std::pair<css::uno::Reference<css::beans::XPropertySet>, OUString>&);

/** applies the given settings to the given XForms container
*/
void XMLOFF_DLLPUBLIC applyXFormsSettings(
        const css::uno::Reference< css::container::XNameAccess >& _rXForms,
        const css::uno::Sequence< css::beans::PropertyValue >& _rSettings
    );

#endif


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
