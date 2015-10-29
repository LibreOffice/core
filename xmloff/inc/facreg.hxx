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
OUString SAL_CALL XMLImpressImportOasis_getImplementationName() throw();
css::uno::Sequence<OUString> SAL_CALL XMLImpressImportOasis_getSupportedServiceNames() throw();
css::uno::Reference<css::uno::XInterface> SAL_CALL XMLImpressImportOasis_createInstance(
    css::uno::Reference<css::lang::XMultiServiceFactory> const & rSMgr)
    throw (css::uno::Exception);
OUString SAL_CALL XMLImpressStylesImportOasis_getImplementationName() throw();
css::uno::Sequence<OUString> SAL_CALL XMLImpressStylesImportOasis_getSupportedServiceNames() throw();
css::uno::Reference<css::uno::XInterface> SAL_CALL XMLImpressStylesImportOasis_createInstance(
    css::uno::Reference<css::lang::XMultiServiceFactory> const & rSMgr)
    throw (css::uno::Exception);
OUString SAL_CALL XMLImpressContentImportOasis_getImplementationName() throw();
css::uno::Sequence<OUString> SAL_CALL XMLImpressContentImportOasis_getSupportedServiceNames() throw();
css::uno::Reference<css::uno::XInterface> SAL_CALL XMLImpressContentImportOasis_createInstance(
    css::uno::Reference<css::lang::XMultiServiceFactory> const & rSMgr)
    throw (css::uno::Exception);
OUString SAL_CALL XMLImpressMetaImportOasis_getImplementationName() throw();
css::uno::Sequence<OUString> SAL_CALL XMLImpressMetaImportOasis_getSupportedServiceNames() throw();
css::uno::Reference<css::uno::XInterface> SAL_CALL XMLImpressMetaImportOasis_createInstance(
    css::uno::Reference<css::lang::XMultiServiceFactory> const & rSMgr)
    throw (css::uno::Exception);
OUString SAL_CALL XMLImpressSettingsImportOasis_getImplementationName() throw();
css::uno::Sequence<OUString> SAL_CALL XMLImpressSettingsImportOasis_getSupportedServiceNames() throw();
css::uno::Reference<css::uno::XInterface> SAL_CALL XMLImpressSettingsImportOasis_createInstance(
    css::uno::Reference<css::lang::XMultiServiceFactory> const & rSMgr)
    throw (css::uno::Exception);

// impress oasis export
OUString SAL_CALL XMLImpressExportOasis_getImplementationName() throw();
css::uno::Sequence<OUString> SAL_CALL XMLImpressExportOasis_getSupportedServiceNames() throw();
css::uno::Reference<css::uno::XInterface> SAL_CALL XMLImpressExportOasis_createInstance(
    css::uno::Reference<css::lang::XMultiServiceFactory> const & rSMgr)
    throw (css::uno::Exception);
OUString SAL_CALL XMLImpressStylesExportOasis_getImplementationName() throw();
css::uno::Sequence<OUString> SAL_CALL XMLImpressStylesExportOasis_getSupportedServiceNames() throw();
css::uno::Reference<css::uno::XInterface> SAL_CALL XMLImpressStylesExportOasis_createInstance(
    css::uno::Reference<css::lang::XMultiServiceFactory> const & rSMgr)
    throw (css::uno::Exception);
OUString SAL_CALL XMLImpressContentExportOasis_getImplementationName() throw();
css::uno::Sequence<OUString> SAL_CALL XMLImpressContentExportOasis_getSupportedServiceNames() throw();
css::uno::Reference<css::uno::XInterface> SAL_CALL XMLImpressContentExportOasis_createInstance(
    css::uno::Reference<css::lang::XMultiServiceFactory> const & rSMgr)
    throw (css::uno::Exception);
OUString SAL_CALL XMLImpressMetaExportOasis_getImplementationName() throw();
css::uno::Sequence<OUString> SAL_CALL XMLImpressMetaExportOasis_getSupportedServiceNames() throw();
css::uno::Reference<css::uno::XInterface> SAL_CALL XMLImpressMetaExportOasis_createInstance(
    css::uno::Reference<css::lang::XMultiServiceFactory> const & rSMgr)
    throw (css::uno::Exception);
OUString SAL_CALL XMLImpressSettingsExportOasis_getImplementationName() throw();
css::uno::Sequence<OUString> SAL_CALL XMLImpressSettingsExportOasis_getSupportedServiceNames() throw();
css::uno::Reference<css::uno::XInterface> SAL_CALL XMLImpressSettingsExportOasis_createInstance(
    css::uno::Reference<css::lang::XMultiServiceFactory> const & rSMgr)
    throw (css::uno::Exception);

// impress OOo import

// impress OOo export
OUString SAL_CALL XMLImpressExportOOO_getImplementationName() throw();
css::uno::Sequence<OUString> SAL_CALL XMLImpressExportOOO_getSupportedServiceNames() throw();
css::uno::Reference<css::uno::XInterface> SAL_CALL XMLImpressExportOOO_createInstance(
    css::uno::Reference<css::lang::XMultiServiceFactory> const & rSMgr)
    throw (css::uno::Exception);
OUString SAL_CALL XMLImpressStylesExportOOO_getImplementationName() throw();
css::uno::Sequence<OUString> SAL_CALL XMLImpressStylesExportOOO_getSupportedServiceNames() throw();
css::uno::Reference<css::uno::XInterface> SAL_CALL XMLImpressStylesExportOOO_createInstance(
    css::uno::Reference<css::lang::XMultiServiceFactory> const & rSMgr)
    throw (css::uno::Exception);
OUString SAL_CALL XMLImpressContentExportOOO_getImplementationName() throw();
css::uno::Sequence<OUString> SAL_CALL XMLImpressContentExportOOO_getSupportedServiceNames() throw();
css::uno::Reference<css::uno::XInterface> SAL_CALL XMLImpressContentExportOOO_createInstance(
    css::uno::Reference<css::lang::XMultiServiceFactory> const & rSMgr)
    throw (css::uno::Exception);
OUString SAL_CALL XMLImpressMetaExportOOO_getImplementationName() throw();
css::uno::Sequence<OUString> SAL_CALL XMLImpressMetaExportOOO_getSupportedServiceNames() throw();
css::uno::Reference<css::uno::XInterface> SAL_CALL XMLImpressMetaExportOOO_createInstance(
    css::uno::Reference<css::lang::XMultiServiceFactory> const & rSMgr)
    throw (css::uno::Exception);
OUString SAL_CALL XMLImpressSettingsExportOOO_getImplementationName() throw();
css::uno::Sequence<OUString> SAL_CALL XMLImpressSettingsExportOOO_getSupportedServiceNames() throw();
css::uno::Reference<css::uno::XInterface> SAL_CALL XMLImpressSettingsExportOOO_createInstance(
    css::uno::Reference<css::lang::XMultiServiceFactory> const & rSMgr)
    throw (css::uno::Exception);

// draw oasis import
OUString SAL_CALL XMLDrawImportOasis_getImplementationName() throw();
css::uno::Sequence<OUString> SAL_CALL XMLDrawImportOasis_getSupportedServiceNames() throw();
css::uno::Reference<css::uno::XInterface> SAL_CALL XMLDrawImportOasis_createInstance(
    css::uno::Reference<css::lang::XMultiServiceFactory> const & rSMgr)
    throw (css::uno::Exception);
OUString SAL_CALL XMLDrawStylesImportOasis_getImplementationName() throw();
css::uno::Sequence<OUString> SAL_CALL XMLDrawStylesImportOasis_getSupportedServiceNames() throw();
css::uno::Reference<css::uno::XInterface> SAL_CALL XMLDrawStylesImportOasis_createInstance(
    css::uno::Reference<css::lang::XMultiServiceFactory> const & rSMgr)
    throw (css::uno::Exception);
OUString SAL_CALL XMLDrawContentImportOasis_getImplementationName() throw();
css::uno::Sequence<OUString> SAL_CALL XMLDrawContentImportOasis_getSupportedServiceNames() throw();
css::uno::Reference<css::uno::XInterface> SAL_CALL XMLDrawContentImportOasis_createInstance(
    css::uno::Reference<css::lang::XMultiServiceFactory> const & rSMgr)
    throw (css::uno::Exception);
OUString SAL_CALL XMLDrawMetaImportOasis_getImplementationName() throw();
css::uno::Sequence<OUString> SAL_CALL XMLDrawMetaImportOasis_getSupportedServiceNames() throw();
css::uno::Reference<css::uno::XInterface> SAL_CALL XMLDrawMetaImportOasis_createInstance(
    css::uno::Reference<css::lang::XMultiServiceFactory> const & rSMgr)
    throw (css::uno::Exception);
OUString SAL_CALL XMLDrawSettingsImportOasis_getImplementationName() throw();
css::uno::Sequence<OUString> SAL_CALL XMLDrawSettingsImportOasis_getSupportedServiceNames() throw();
css::uno::Reference<css::uno::XInterface> SAL_CALL XMLDrawSettingsImportOasis_createInstance(
    css::uno::Reference<css::lang::XMultiServiceFactory> const & rSMgr)
    throw (css::uno::Exception);

// draw oasis export
OUString SAL_CALL XMLDrawExportOasis_getImplementationName() throw();
css::uno::Sequence<OUString> SAL_CALL XMLDrawExportOasis_getSupportedServiceNames() throw();
css::uno::Reference<css::uno::XInterface> SAL_CALL XMLDrawExportOasis_createInstance(
    css::uno::Reference<css::lang::XMultiServiceFactory> const & rSMgr)
    throw (css::uno::Exception);
OUString SAL_CALL XMLDrawStylesExportOasis_getImplementationName() throw();
css::uno::Sequence<OUString> SAL_CALL XMLDrawStylesExportOasis_getSupportedServiceNames() throw();
css::uno::Reference<css::uno::XInterface> SAL_CALL XMLDrawStylesExportOasis_createInstance(
    css::uno::Reference<css::lang::XMultiServiceFactory> const & rSMgr)
    throw (css::uno::Exception);
OUString SAL_CALL XMLDrawContentExportOasis_getImplementationName() throw();
css::uno::Sequence<OUString> SAL_CALL XMLDrawContentExportOasis_getSupportedServiceNames() throw();
css::uno::Reference<css::uno::XInterface> SAL_CALL XMLDrawContentExportOasis_createInstance(
    css::uno::Reference<css::lang::XMultiServiceFactory> const & rSMgr)
    throw (css::uno::Exception);
OUString SAL_CALL XMLDrawMetaExportOasis_getImplementationName() throw();
css::uno::Sequence<OUString> SAL_CALL XMLDrawMetaExportOasis_getSupportedServiceNames() throw();
css::uno::Reference<css::uno::XInterface> SAL_CALL XMLDrawMetaExportOasis_createInstance(
    css::uno::Reference<css::lang::XMultiServiceFactory> const & rSMgr)
    throw (css::uno::Exception);
OUString SAL_CALL XMLDrawSettingsExportOasis_getImplementationName() throw();
css::uno::Sequence<OUString> SAL_CALL XMLDrawSettingsExportOasis_getSupportedServiceNames() throw();
css::uno::Reference<css::uno::XInterface> SAL_CALL XMLDrawSettingsExportOasis_createInstance(
    css::uno::Reference<css::lang::XMultiServiceFactory> const & rSMgr)
    throw (css::uno::Exception);

// draw OOo import

// draw OOo export
OUString SAL_CALL XMLDrawExportOOO_getImplementationName() throw();
css::uno::Sequence<OUString> SAL_CALL XMLDrawExportOOO_getSupportedServiceNames() throw();
css::uno::Reference<css::uno::XInterface> SAL_CALL XMLDrawExportOOO_createInstance(
    css::uno::Reference<css::lang::XMultiServiceFactory> const & rSMgr)
    throw (css::uno::Exception);
OUString SAL_CALL XMLDrawStylesExportOOO_getImplementationName() throw();
css::uno::Sequence<OUString> SAL_CALL XMLDrawStylesExportOOO_getSupportedServiceNames() throw();
css::uno::Reference<css::uno::XInterface> SAL_CALL XMLDrawStylesExportOOO_createInstance(
    css::uno::Reference<css::lang::XMultiServiceFactory> const & rSMgr)
    throw (css::uno::Exception);
OUString SAL_CALL XMLDrawContentExportOOO_getImplementationName() throw();
css::uno::Sequence<OUString> SAL_CALL XMLDrawContentExportOOO_getSupportedServiceNames() throw();
css::uno::Reference<css::uno::XInterface> SAL_CALL XMLDrawContentExportOOO_createInstance(
    css::uno::Reference<css::lang::XMultiServiceFactory> const & rSMgr)
    throw (css::uno::Exception);
OUString SAL_CALL XMLDrawMetaExportOOO_getImplementationName() throw();
css::uno::Sequence<OUString> SAL_CALL XMLDrawMetaExportOOO_getSupportedServiceNames() throw();
css::uno::Reference<css::uno::XInterface> SAL_CALL XMLDrawMetaExportOOO_createInstance(
    css::uno::Reference<css::lang::XMultiServiceFactory> const & rSMgr)
    throw (css::uno::Exception);
OUString SAL_CALL XMLDrawSettingsExportOOO_getImplementationName() throw();
css::uno::Sequence<OUString> SAL_CALL XMLDrawSettingsExportOOO_getSupportedServiceNames() throw();
css::uno::Reference<css::uno::XInterface> SAL_CALL XMLDrawSettingsExportOOO_createInstance(
    css::uno::Reference<css::lang::XMultiServiceFactory> const & rSMgr)
    throw (css::uno::Exception);

// impress animation import
OUString SAL_CALL AnimationsImport_getImplementationName() throw();
css::uno::Sequence<OUString> SAL_CALL AnimationsImport_getSupportedServiceNames() throw();
css::uno::Reference<css::uno::XInterface> SAL_CALL AnimationsImport_createInstance(
    css::uno::Reference<css::lang::XMultiServiceFactory> const & rSMgr)
    throw (css::uno::Exception);

// drawing layer export
OUString SAL_CALL XMLDrawingLayerExport_getImplementationName() throw();
css::uno::Sequence<OUString> SAL_CALL XMLDrawingLayerExport_getSupportedServiceNames() throw();
css::uno::Reference<css::uno::XInterface> SAL_CALL XMLDrawingLayerExport_createInstance(
    css::uno::Reference<css::lang::XMultiServiceFactory> const & rSMgr)
    throw (css::uno::Exception);

// impress xml clipboard export
OUString SAL_CALL XMLImpressClipboardExport_getImplementationName() throw();
css::uno::Sequence<OUString> SAL_CALL XMLImpressClipboardExport_getSupportedServiceNames() throw();
css::uno::Reference<css::uno::XInterface> SAL_CALL XMLImpressClipboardExport_createInstance(
    css::uno::Reference<css::lang::XMultiServiceFactory> const & rSMgr)
    throw (css::uno::Exception);

// chart oasis import
OUString SAL_CALL SchXMLImport_getImplementationName() throw();
css::uno::Sequence<OUString> SAL_CALL SchXMLImport_getSupportedServiceNames() throw();
css::uno::Reference<css::uno::XInterface> SAL_CALL SchXMLImport_createInstance(
    css::uno::Reference<css::lang::XMultiServiceFactory> const & rSMgr)
    throw (css::uno::Exception);
OUString SAL_CALL SchXMLImport_Meta_getImplementationName() throw();
css::uno::Sequence<OUString> SAL_CALL SchXMLImport_Meta_getSupportedServiceNames() throw();
css::uno::Reference<css::uno::XInterface> SAL_CALL SchXMLImport_Meta_createInstance(
    css::uno::Reference<css::lang::XMultiServiceFactory> const & rSMgr)
    throw (css::uno::Exception);
OUString SAL_CALL SchXMLImport_Styles_getImplementationName() throw();
css::uno::Sequence<OUString> SAL_CALL SchXMLImport_Styles_getSupportedServiceNames() throw();
css::uno::Reference<css::uno::XInterface> SAL_CALL SchXMLImport_Styles_createInstance(
    css::uno::Reference<css::lang::XMultiServiceFactory> const & rSMgr)
    throw (css::uno::Exception);
OUString SAL_CALL SchXMLImport_Content_getImplementationName() throw();
css::uno::Sequence<OUString> SAL_CALL SchXMLImport_Content_getSupportedServiceNames() throw();
css::uno::Reference<css::uno::XInterface> SAL_CALL SchXMLImport_Content_createInstance(
    css::uno::Reference<css::lang::XMultiServiceFactory> const & rSMgr)
    throw (css::uno::Exception);

// chart oasis export
OUString SAL_CALL SchXMLExport_Oasis_getImplementationName() throw();
css::uno::Sequence<OUString> SAL_CALL SchXMLExport_Oasis_getSupportedServiceNames() throw();
css::uno::Reference<css::uno::XInterface> SAL_CALL SchXMLExport_Oasis_createInstance(
    css::uno::Reference<css::lang::XMultiServiceFactory> const & rSMgr)
    throw (css::uno::Exception);
OUString SAL_CALL SchXMLExport_Oasis_Meta_getImplementationName() throw();
css::uno::Sequence<OUString> SAL_CALL SchXMLExport_Oasis_Meta_getSupportedServiceNames() throw();
css::uno::Reference<css::uno::XInterface> SAL_CALL SchXMLExport_Oasis_Meta_createInstance(
    css::uno::Reference<css::lang::XMultiServiceFactory> const & rSMgr)
    throw (css::uno::Exception);
OUString SAL_CALL SchXMLExport_Oasis_Styles_getImplementationName() throw();
css::uno::Sequence<OUString> SAL_CALL SchXMLExport_Oasis_Styles_getSupportedServiceNames() throw();
css::uno::Reference<css::uno::XInterface> SAL_CALL SchXMLExport_Oasis_Styles_createInstance(
    css::uno::Reference<css::lang::XMultiServiceFactory> const & rSMgr)
    throw (css::uno::Exception);
OUString SAL_CALL SchXMLExport_Oasis_Content_getImplementationName() throw();
css::uno::Sequence<OUString> SAL_CALL SchXMLExport_Oasis_Content_getSupportedServiceNames() throw();
css::uno::Reference<css::uno::XInterface> SAL_CALL SchXMLExport_Oasis_Content_createInstance(
    css::uno::Reference<css::lang::XMultiServiceFactory> const & rSMgr)
    throw (css::uno::Exception);

// chart OOo import

// chart OOo export
OUString SAL_CALL SchXMLExport_getImplementationName() throw();
css::uno::Sequence<OUString> SAL_CALL SchXMLExport_getSupportedServiceNames() throw();
css::uno::Reference<css::uno::XInterface> SAL_CALL SchXMLExport_createInstance(
    css::uno::Reference<css::lang::XMultiServiceFactory> const & rSMgr)
    throw (css::uno::Exception);
OUString SAL_CALL SchXMLExport_Styles_getImplementationName() throw();
css::uno::Sequence<OUString> SAL_CALL SchXMLExport_Styles_getSupportedServiceNames() throw();
css::uno::Reference<css::uno::XInterface> SAL_CALL SchXMLExport_Styles_createInstance(
    css::uno::Reference<css::lang::XMultiServiceFactory> const & rSMgr)
    throw (css::uno::Exception);
OUString SAL_CALL SchXMLExport_Content_getImplementationName() throw();
css::uno::Sequence<OUString> SAL_CALL SchXMLExport_Content_getSupportedServiceNames() throw();
css::uno::Reference<css::uno::XInterface> SAL_CALL SchXMLExport_Content_createInstance(
    css::uno::Reference<css::lang::XMultiServiceFactory> const & rSMgr)
    throw (css::uno::Exception);

// meta export
OUString SAL_CALL XMLMetaExportComponent_getImplementationName() throw();
css::uno::Sequence<OUString> SAL_CALL XMLMetaExportComponent_getSupportedServiceNames() throw();
css::uno::Reference<css::uno::XInterface> SAL_CALL XMLMetaExportComponent_createInstance(
    css::uno::Reference<css::lang::XMultiServiceFactory> const & rSMgr)
    throw (css::uno::Exception);

// meta export OOo
OUString SAL_CALL XMLMetaExportOOO_getImplementationName() throw();
css::uno::Sequence<OUString> SAL_CALL XMLMetaExportOOO_getSupportedServiceNames() throw();
css::uno::Reference<css::uno::XInterface> SAL_CALL XMLMetaExportOOO_createInstance(
    css::uno::Reference<css::lang::XMultiServiceFactory> const & rSMgr)
    throw (css::uno::Exception);

// writer autotext event export
OUString SAL_CALL XMLAutoTextEventExport_getImplementationName() throw();
css::uno::Sequence<OUString> SAL_CALL XMLAutoTextEventExport_getSupportedServiceNames() throw();
css::uno::Reference<css::uno::XInterface> SAL_CALL XMLAutoTextEventExport_createInstance(
    css::uno::Reference<css::lang::XMultiServiceFactory> const & rSMgr)
    throw (css::uno::Exception);

// writer autotext event import
OUString SAL_CALL XMLAutoTextEventImport_getImplementationName() throw();
css::uno::Sequence<OUString> SAL_CALL XMLAutoTextEventImport_getSupportedServiceNames() throw();
css::uno::Reference<css::uno::XInterface> SAL_CALL XMLAutoTextEventImport_createInstance(
    css::uno::Reference<css::lang::XMultiServiceFactory> const & rSMgr)
    throw (css::uno::Exception);

// writer autotext event export OOo
OUString SAL_CALL XMLAutoTextEventExportOOO_getImplementationName() throw();
css::uno::Sequence<OUString> SAL_CALL XMLAutoTextEventExportOOO_getSupportedServiceNames() throw();
css::uno::Reference<css::uno::XInterface> SAL_CALL XMLAutoTextEventExportOOO_createInstance(
    css::uno::Reference<css::lang::XMultiServiceFactory> const & rSMgr)
    throw (css::uno::Exception);

// writer autotext event import OOo
OUString SAL_CALL XMLAutoTextEventImportOOO_getImplementationName() throw();
css::uno::Sequence<OUString> SAL_CALL XMLAutoTextEventImportOOO_getSupportedServiceNames() throw();
css::uno::Reference<css::uno::XInterface> SAL_CALL XMLAutoTextEventImportOOO_createInstance(
    css::uno::Reference<css::lang::XMultiServiceFactory> const & rSMgr)
    throw (css::uno::Exception);

// Oasis2OOo transformer
OUString SAL_CALL Oasis2OOoTransformer_getImplementationName() throw();
css::uno::Sequence<OUString> SAL_CALL Oasis2OOoTransformer_getSupportedServiceNames() throw();
css::uno::Reference<css::uno::XInterface> SAL_CALL Oasis2OOoTransformer_createInstance(
    css::uno::Reference<css::lang::XMultiServiceFactory> const & rSMgr)
    throw (css::uno::Exception);

// OOo2Oasis transformer
OUString SAL_CALL OOo2OasisTransformer_getImplementationName() throw();
css::uno::Sequence<OUString> SAL_CALL OOo2OasisTransformer_getSupportedServiceNames() throw();
css::uno::Reference<css::uno::XInterface> SAL_CALL OOo2OasisTransformer_createInstance(
    css::uno::Reference<css::lang::XMultiServiceFactory> const & rSMgr)
    throw (css::uno::Exception);

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
