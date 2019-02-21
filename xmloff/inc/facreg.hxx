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

#ifndef INCLUDED_XMLOFF_INC_FACREG_HXX
#define INCLUDED_XMLOFF_INC_FACREG_HXX

#include <sal/config.h>

#include <com/sun/star/uno/Exception.hpp>
#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/uno/Sequence.hxx>
#include <rtl/ustring.hxx>
#include <sal/types.h>

namespace com { namespace sun { namespace star {
    namespace lang { class XMultiServiceFactory; }
    namespace uno { class XInterface; }
} } }

// impress oasis import
OUString XMLImpressStylesImportOasis_getImplementationName() throw();
css::uno::Sequence<OUString> XMLImpressStylesImportOasis_getSupportedServiceNames() throw();
/// @throws css::uno::Exception
css::uno::Reference<css::uno::XInterface> XMLImpressStylesImportOasis_createInstance(
    css::uno::Reference<css::lang::XMultiServiceFactory> const & rSMgr);
OUString XMLImpressContentImportOasis_getImplementationName() throw();
css::uno::Sequence<OUString> XMLImpressContentImportOasis_getSupportedServiceNames() throw();
/// @throws css::uno::Exception
css::uno::Reference<css::uno::XInterface> XMLImpressContentImportOasis_createInstance(
    css::uno::Reference<css::lang::XMultiServiceFactory> const & rSMgr);
OUString XMLImpressMetaImportOasis_getImplementationName() throw();
css::uno::Sequence<OUString> XMLImpressMetaImportOasis_getSupportedServiceNames() throw();
/// @throws css::uno::Exception
css::uno::Reference<css::uno::XInterface> XMLImpressMetaImportOasis_createInstance(
    css::uno::Reference<css::lang::XMultiServiceFactory> const & rSMgr);
OUString XMLImpressSettingsImportOasis_getImplementationName() throw();
css::uno::Sequence<OUString> XMLImpressSettingsImportOasis_getSupportedServiceNames() throw();
/// @throws css::uno::Exception
css::uno::Reference<css::uno::XInterface> XMLImpressSettingsImportOasis_createInstance(
    css::uno::Reference<css::lang::XMultiServiceFactory> const & rSMgr);

// impress oasis export
OUString XMLImpressMetaExportOasis_getImplementationName() throw();
css::uno::Sequence<OUString> XMLImpressMetaExportOasis_getSupportedServiceNames() throw();
/// @throws css::uno::Exception
css::uno::Reference<css::uno::XInterface> XMLImpressMetaExportOasis_createInstance(
    css::uno::Reference<css::lang::XMultiServiceFactory> const & rSMgr);
OUString XMLImpressSettingsExportOasis_getImplementationName() throw();
css::uno::Sequence<OUString> XMLImpressSettingsExportOasis_getSupportedServiceNames() throw();
/// @throws css::uno::Exception
css::uno::Reference<css::uno::XInterface> XMLImpressSettingsExportOasis_createInstance(
    css::uno::Reference<css::lang::XMultiServiceFactory> const & rSMgr);

// impress OOo import

// impress OOo export
OUString XMLImpressExportOOO_getImplementationName() throw();
css::uno::Sequence<OUString> XMLImpressExportOOO_getSupportedServiceNames() throw();
/// @throws css::uno::Exception
css::uno::Reference<css::uno::XInterface> XMLImpressExportOOO_createInstance(
    css::uno::Reference<css::lang::XMultiServiceFactory> const & rSMgr);
OUString XMLImpressStylesExportOOO_getImplementationName() throw();
css::uno::Sequence<OUString> XMLImpressStylesExportOOO_getSupportedServiceNames() throw();
/// @throws css::uno::Exception
css::uno::Reference<css::uno::XInterface> XMLImpressStylesExportOOO_createInstance(
    css::uno::Reference<css::lang::XMultiServiceFactory> const & rSMgr);
OUString XMLImpressContentExportOOO_getImplementationName() throw();
css::uno::Sequence<OUString> XMLImpressContentExportOOO_getSupportedServiceNames() throw();
/// @throws css::uno::Exception
css::uno::Reference<css::uno::XInterface> XMLImpressContentExportOOO_createInstance(
    css::uno::Reference<css::lang::XMultiServiceFactory> const & rSMgr);
OUString XMLImpressMetaExportOOO_getImplementationName() throw();
css::uno::Sequence<OUString> XMLImpressMetaExportOOO_getSupportedServiceNames() throw();
/// @throws css::uno::Exception
css::uno::Reference<css::uno::XInterface> XMLImpressMetaExportOOO_createInstance(
    css::uno::Reference<css::lang::XMultiServiceFactory> const & rSMgr);
OUString XMLImpressSettingsExportOOO_getImplementationName() throw();
css::uno::Sequence<OUString> XMLImpressSettingsExportOOO_getSupportedServiceNames() throw();
/// @throws css::uno::Exception
css::uno::Reference<css::uno::XInterface> XMLImpressSettingsExportOOO_createInstance(
    css::uno::Reference<css::lang::XMultiServiceFactory> const & rSMgr);

// draw oasis import
OUString XMLDrawImportOasis_getImplementationName() throw();
css::uno::Sequence<OUString> XMLDrawImportOasis_getSupportedServiceNames() throw();
/// @throws css::uno::Exception
css::uno::Reference<css::uno::XInterface> XMLDrawImportOasis_createInstance(
    css::uno::Reference<css::lang::XMultiServiceFactory> const & rSMgr);
OUString XMLDrawStylesImportOasis_getImplementationName() throw();
css::uno::Sequence<OUString> XMLDrawStylesImportOasis_getSupportedServiceNames() throw();
/// @throws css::uno::Exception
css::uno::Reference<css::uno::XInterface> XMLDrawStylesImportOasis_createInstance(
    css::uno::Reference<css::lang::XMultiServiceFactory> const & rSMgr);
OUString XMLDrawContentImportOasis_getImplementationName() throw();
css::uno::Sequence<OUString> XMLDrawContentImportOasis_getSupportedServiceNames() throw();
/// @throws css::uno::Exception
css::uno::Reference<css::uno::XInterface> XMLDrawContentImportOasis_createInstance(
    css::uno::Reference<css::lang::XMultiServiceFactory> const & rSMgr);
OUString XMLDrawMetaImportOasis_getImplementationName() throw();
css::uno::Sequence<OUString> XMLDrawMetaImportOasis_getSupportedServiceNames() throw();
/// @throws css::uno::Exception
css::uno::Reference<css::uno::XInterface> XMLDrawMetaImportOasis_createInstance(
    css::uno::Reference<css::lang::XMultiServiceFactory> const & rSMgr);
OUString XMLDrawSettingsImportOasis_getImplementationName() throw();
css::uno::Sequence<OUString> XMLDrawSettingsImportOasis_getSupportedServiceNames() throw();
/// @throws css::uno::Exception
css::uno::Reference<css::uno::XInterface> XMLDrawSettingsImportOasis_createInstance(
    css::uno::Reference<css::lang::XMultiServiceFactory> const & rSMgr);

// draw oasis export
OUString XMLDrawExportOasis_getImplementationName() throw();
css::uno::Sequence<OUString> XMLDrawExportOasis_getSupportedServiceNames() throw();
/// @throws css::uno::Exception
css::uno::Reference<css::uno::XInterface> XMLDrawExportOasis_createInstance(
    css::uno::Reference<css::lang::XMultiServiceFactory> const & rSMgr);
OUString XMLDrawStylesExportOasis_getImplementationName() throw();
css::uno::Sequence<OUString> XMLDrawStylesExportOasis_getSupportedServiceNames() throw();
/// @throws css::uno::Exception
css::uno::Reference<css::uno::XInterface> XMLDrawStylesExportOasis_createInstance(
    css::uno::Reference<css::lang::XMultiServiceFactory> const & rSMgr);
OUString XMLDrawContentExportOasis_getImplementationName() throw();
css::uno::Sequence<OUString> XMLDrawContentExportOasis_getSupportedServiceNames() throw();
/// @throws css::uno::Exception
css::uno::Reference<css::uno::XInterface> XMLDrawContentExportOasis_createInstance(
    css::uno::Reference<css::lang::XMultiServiceFactory> const & rSMgr);
OUString XMLDrawMetaExportOasis_getImplementationName() throw();
css::uno::Sequence<OUString> XMLDrawMetaExportOasis_getSupportedServiceNames() throw();
/// @throws css::uno::Exception
css::uno::Reference<css::uno::XInterface> XMLDrawMetaExportOasis_createInstance(
    css::uno::Reference<css::lang::XMultiServiceFactory> const & rSMgr);
OUString XMLDrawSettingsExportOasis_getImplementationName() throw();
css::uno::Sequence<OUString> XMLDrawSettingsExportOasis_getSupportedServiceNames() throw();
/// @throws css::uno::Exception
css::uno::Reference<css::uno::XInterface> XMLDrawSettingsExportOasis_createInstance(
    css::uno::Reference<css::lang::XMultiServiceFactory> const & rSMgr);

// draw OOo import

// draw OOo export
OUString XMLDrawExportOOO_getImplementationName() throw();
css::uno::Sequence<OUString> XMLDrawExportOOO_getSupportedServiceNames() throw();
/// @throws css::uno::Exception
css::uno::Reference<css::uno::XInterface> XMLDrawExportOOO_createInstance(
    css::uno::Reference<css::lang::XMultiServiceFactory> const & rSMgr);
OUString XMLDrawStylesExportOOO_getImplementationName() throw();
css::uno::Sequence<OUString> XMLDrawStylesExportOOO_getSupportedServiceNames() throw();
/// @throws css::uno::Exception
css::uno::Reference<css::uno::XInterface> XMLDrawStylesExportOOO_createInstance(
    css::uno::Reference<css::lang::XMultiServiceFactory> const & rSMgr);
OUString XMLDrawContentExportOOO_getImplementationName() throw();
css::uno::Sequence<OUString> XMLDrawContentExportOOO_getSupportedServiceNames() throw();
/// @throws css::uno::Exception
css::uno::Reference<css::uno::XInterface> XMLDrawContentExportOOO_createInstance(
    css::uno::Reference<css::lang::XMultiServiceFactory> const & rSMgr);
OUString XMLDrawMetaExportOOO_getImplementationName() throw();
css::uno::Sequence<OUString> XMLDrawMetaExportOOO_getSupportedServiceNames() throw();
/// @throws css::uno::Exception
css::uno::Reference<css::uno::XInterface> XMLDrawMetaExportOOO_createInstance(
    css::uno::Reference<css::lang::XMultiServiceFactory> const & rSMgr);
OUString XMLDrawSettingsExportOOO_getImplementationName() throw();
css::uno::Sequence<OUString> XMLDrawSettingsExportOOO_getSupportedServiceNames() throw();
/// @throws css::uno::Exception
css::uno::Reference<css::uno::XInterface> XMLDrawSettingsExportOOO_createInstance(
    css::uno::Reference<css::lang::XMultiServiceFactory> const & rSMgr);

// impress animation import
OUString AnimationsImport_getImplementationName() throw();
css::uno::Sequence<OUString> AnimationsImport_getSupportedServiceNames() throw();
/// @throws css::uno::Exception
css::uno::Reference<css::uno::XInterface> AnimationsImport_createInstance(
    css::uno::Reference<css::lang::XMultiServiceFactory> const & rSMgr);

// drawing layer export
OUString XMLDrawingLayerExport_getImplementationName() throw();
css::uno::Sequence<OUString> XMLDrawingLayerExport_getSupportedServiceNames() throw();
/// @throws css::uno::Exception
css::uno::Reference<css::uno::XInterface> XMLDrawingLayerExport_createInstance(
    css::uno::Reference<css::lang::XMultiServiceFactory> const & rSMgr);

// impress xml clipboard export
OUString XMLImpressClipboardExport_getImplementationName() throw();
css::uno::Sequence<OUString> XMLImpressClipboardExport_getSupportedServiceNames() throw();
/// @throws css::uno::Exception
css::uno::Reference<css::uno::XInterface> XMLImpressClipboardExport_createInstance(
    css::uno::Reference<css::lang::XMultiServiceFactory> const & rSMgr);

// chart oasis import
OUString SchXMLImport_getImplementationName() throw();
css::uno::Sequence<OUString> SchXMLImport_getSupportedServiceNames() throw();
/// @throws css::uno::Exception
css::uno::Reference<css::uno::XInterface> SchXMLImport_createInstance(
    css::uno::Reference<css::lang::XMultiServiceFactory> const & rSMgr);
OUString SchXMLImport_Meta_getImplementationName() throw();
css::uno::Sequence<OUString> SchXMLImport_Meta_getSupportedServiceNames() throw();
/// @throws css::uno::Exception
css::uno::Reference<css::uno::XInterface> SchXMLImport_Meta_createInstance(
    css::uno::Reference<css::lang::XMultiServiceFactory> const & rSMgr);
OUString SchXMLImport_Styles_getImplementationName() throw();
css::uno::Sequence<OUString> SchXMLImport_Styles_getSupportedServiceNames() throw();
/// @throws css::uno::Exception
css::uno::Reference<css::uno::XInterface> SchXMLImport_Styles_createInstance(
    css::uno::Reference<css::lang::XMultiServiceFactory> const & rSMgr);
OUString SchXMLImport_Content_getImplementationName() throw();
css::uno::Sequence<OUString> SchXMLImport_Content_getSupportedServiceNames() throw();
/// @throws css::uno::Exception
css::uno::Reference<css::uno::XInterface> SchXMLImport_Content_createInstance(
    css::uno::Reference<css::lang::XMultiServiceFactory> const & rSMgr);

// chart oasis export
OUString SchXMLExport_Oasis_getImplementationName() throw();
css::uno::Sequence<OUString> SchXMLExport_Oasis_getSupportedServiceNames() throw();
/// @throws css::uno::Exception
css::uno::Reference<css::uno::XInterface> SchXMLExport_Oasis_createInstance(
    css::uno::Reference<css::lang::XMultiServiceFactory> const & rSMgr);
OUString SchXMLExport_Oasis_Meta_getImplementationName() throw();
css::uno::Sequence<OUString> SchXMLExport_Oasis_Meta_getSupportedServiceNames() throw();
/// @throws css::uno::Exception
css::uno::Reference<css::uno::XInterface> SchXMLExport_Oasis_Meta_createInstance(
    css::uno::Reference<css::lang::XMultiServiceFactory> const & rSMgr);
OUString SchXMLExport_Oasis_Styles_getImplementationName() throw();
css::uno::Sequence<OUString> SchXMLExport_Oasis_Styles_getSupportedServiceNames() throw();
/// @throws css::uno::Exception
css::uno::Reference<css::uno::XInterface> SchXMLExport_Oasis_Styles_createInstance(
    css::uno::Reference<css::lang::XMultiServiceFactory> const & rSMgr);
OUString SchXMLExport_Oasis_Content_getImplementationName() throw();
css::uno::Sequence<OUString> SchXMLExport_Oasis_Content_getSupportedServiceNames() throw();
/// @throws css::uno::Exception
css::uno::Reference<css::uno::XInterface> SchXMLExport_Oasis_Content_createInstance(
    css::uno::Reference<css::lang::XMultiServiceFactory> const & rSMgr);

// chart OOo import

// chart OOo export
OUString SchXMLExport_getImplementationName() throw();
css::uno::Sequence<OUString> SchXMLExport_getSupportedServiceNames() throw();
/// @throws css::uno::Exception
css::uno::Reference<css::uno::XInterface> SchXMLExport_createInstance(
    css::uno::Reference<css::lang::XMultiServiceFactory> const & rSMgr);
OUString SchXMLExport_Styles_getImplementationName() throw();
css::uno::Sequence<OUString> SchXMLExport_Styles_getSupportedServiceNames() throw();
/// @throws css::uno::Exception
css::uno::Reference<css::uno::XInterface> SchXMLExport_Styles_createInstance(
    css::uno::Reference<css::lang::XMultiServiceFactory> const & rSMgr);
OUString SchXMLExport_Content_getImplementationName() throw();
css::uno::Sequence<OUString> SchXMLExport_Content_getSupportedServiceNames() throw();
/// @throws css::uno::Exception
css::uno::Reference<css::uno::XInterface> SchXMLExport_Content_createInstance(
    css::uno::Reference<css::lang::XMultiServiceFactory> const & rSMgr);

// meta export OOo
OUString XMLMetaExportOOO_getImplementationName() throw();
css::uno::Sequence<OUString> XMLMetaExportOOO_getSupportedServiceNames() throw();
/// @throws css::uno::Exception
css::uno::Reference<css::uno::XInterface> XMLMetaExportOOO_createInstance(
    css::uno::Reference<css::lang::XMultiServiceFactory> const & rSMgr);

// writer autotext event export
OUString XMLAutoTextEventExport_getImplementationName() throw();
css::uno::Sequence<OUString> XMLAutoTextEventExport_getSupportedServiceNames() throw();
/// @throws css::uno::Exception
css::uno::Reference<css::uno::XInterface> XMLAutoTextEventExport_createInstance(
    css::uno::Reference<css::lang::XMultiServiceFactory> const & rSMgr);

// writer autotext event import
OUString XMLAutoTextEventImport_getImplementationName() throw();
css::uno::Sequence<OUString> XMLAutoTextEventImport_getSupportedServiceNames() throw();
/// @throws css::uno::Exception
css::uno::Reference<css::uno::XInterface> XMLAutoTextEventImport_createInstance(
    css::uno::Reference<css::lang::XMultiServiceFactory> const & rSMgr);

// writer autotext event export OOo
OUString XMLAutoTextEventExportOOO_getImplementationName() throw();
css::uno::Sequence<OUString> XMLAutoTextEventExportOOO_getSupportedServiceNames() throw();
/// @throws css::uno::Exception
css::uno::Reference<css::uno::XInterface> XMLAutoTextEventExportOOO_createInstance(
    css::uno::Reference<css::lang::XMultiServiceFactory> const & rSMgr);

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
