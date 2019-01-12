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
#ifndef INCLUDED_XMLOFF_XFORMSEXPORT_HXX
#define INCLUDED_XMLOFF_XFORMSEXPORT_HXX

#include <xmloff/dllapi.h>
#include <rtl/ustring.hxx>

class SvXMLExport;
namespace com { namespace sun { namespace star {
    namespace uno { template<typename T> class Reference; }
    namespace uno { template<typename T> class Sequence; }
    namespace beans { class XPropertySet; struct PropertyValue; }
    namespace container { class XNameAccess; }
} } }


/** export an XForms model. */
void exportXForms( SvXMLExport& );

OUString getXFormsBindName( const css::uno::Reference<css::beans::XPropertySet>& xBinding );

OUString getXFormsListBindName( const css::uno::Reference<css::beans::XPropertySet>& xBinding );

OUString getXFormsSubmissionName( const css::uno::Reference<css::beans::XPropertySet>& xBinding );


/** returns the settings of the given XForms container, to be exported as document specific settings
*/
void XMLOFF_DLLPUBLIC getXFormsSettings(
        const css::uno::Reference< css::container::XNameAccess >& _rXForms,
              css::uno::Sequence< css::beans::PropertyValue >& _out_rSettings
    );

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
