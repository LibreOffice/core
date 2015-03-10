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

OUString SAL_CALL XMLImpressStylesImportOasis_getImplementationName() throw();
css::uno::Sequence<OUString> SAL_CALL XMLImpressStylesImportOasis_getSupportedServiceNames() throw();

OUString SAL_CALL XMLImpressContentImportOasis_getImplementationName() throw();
css::uno::Sequence<OUString> SAL_CALL XMLImpressContentImportOasis_getSupportedServiceNames() throw();

OUString SAL_CALL XMLImpressMetaImportOasis_getImplementationName() throw();
css::uno::Sequence<OUString> SAL_CALL XMLImpressMetaImportOasis_getSupportedServiceNames() throw();

OUString SAL_CALL XMLImpressSettingsImportOasis_getImplementationName() throw();
css::uno::Sequence<OUString> SAL_CALL XMLImpressSettingsImportOasis_getSupportedServiceNames() throw();


// impress oasis export
OUString SAL_CALL XMLImpressExportOasis_getImplementationName() throw();
css::uno::Sequence<OUString> SAL_CALL XMLImpressExportOasis_getSupportedServiceNames() throw();

OUString SAL_CALL XMLImpressStylesExportOasis_getImplementationName() throw();
css::uno::Sequence<OUString> SAL_CALL XMLImpressStylesExportOasis_getSupportedServiceNames() throw();

OUString SAL_CALL XMLImpressContentExportOasis_getImplementationName() throw();
css::uno::Sequence<OUString> SAL_CALL XMLImpressContentExportOasis_getSupportedServiceNames() throw();

OUString SAL_CALL XMLImpressMetaExportOasis_getImplementationName() throw();
css::uno::Sequence<OUString> SAL_CALL XMLImpressMetaExportOasis_getSupportedServiceNames() throw();

OUString SAL_CALL XMLImpressSettingsExportOasis_getImplementationName() throw();
css::uno::Sequence<OUString> SAL_CALL XMLImpressSettingsExportOasis_getSupportedServiceNames() throw();


// impress OOo import

// impress OOo export
OUString SAL_CALL XMLImpressExportOOO_getImplementationName() throw();
css::uno::Sequence<OUString> SAL_CALL XMLImpressExportOOO_getSupportedServiceNames() throw();

OUString SAL_CALL XMLImpressStylesExportOOO_getImplementationName() throw();
css::uno::Sequence<OUString> SAL_CALL XMLImpressStylesExportOOO_getSupportedServiceNames() throw();

OUString SAL_CALL XMLImpressContentExportOOO_getImplementationName() throw();
css::uno::Sequence<OUString> SAL_CALL XMLImpressContentExportOOO_getSupportedServiceNames() throw();

OUString SAL_CALL XMLImpressMetaExportOOO_getImplementationName() throw();
css::uno::Sequence<OUString> SAL_CALL XMLImpressMetaExportOOO_getSupportedServiceNames() throw();

OUString SAL_CALL XMLImpressSettingsExportOOO_getImplementationName() throw();
css::uno::Sequence<OUString> SAL_CALL XMLImpressSettingsExportOOO_getSupportedServiceNames() throw();


// draw oasis import
OUString SAL_CALL XMLDrawImportOasis_getImplementationName() throw();
css::uno::Sequence<OUString> SAL_CALL XMLDrawImportOasis_getSupportedServiceNames() throw();

OUString SAL_CALL XMLDrawStylesImportOasis_getImplementationName() throw();
css::uno::Sequence<OUString> SAL_CALL XMLDrawStylesImportOasis_getSupportedServiceNames() throw();

OUString SAL_CALL XMLDrawContentImportOasis_getImplementationName() throw();
css::uno::Sequence<OUString> SAL_CALL XMLDrawContentImportOasis_getSupportedServiceNames() throw();

OUString SAL_CALL XMLDrawMetaImportOasis_getImplementationName() throw();
css::uno::Sequence<OUString> SAL_CALL XMLDrawMetaImportOasis_getSupportedServiceNames() throw();

OUString SAL_CALL XMLDrawSettingsImportOasis_getImplementationName() throw();
css::uno::Sequence<OUString> SAL_CALL XMLDrawSettingsImportOasis_getSupportedServiceNames() throw();


// draw oasis export
OUString SAL_CALL XMLDrawExportOasis_getImplementationName() throw();
css::uno::Sequence<OUString> SAL_CALL XMLDrawExportOasis_getSupportedServiceNames() throw();

OUString SAL_CALL XMLDrawStylesExportOasis_getImplementationName() throw();
css::uno::Sequence<OUString> SAL_CALL XMLDrawStylesExportOasis_getSupportedServiceNames() throw();

OUString SAL_CALL XMLDrawContentExportOasis_getImplementationName() throw();
css::uno::Sequence<OUString> SAL_CALL XMLDrawContentExportOasis_getSupportedServiceNames() throw();

OUString SAL_CALL XMLDrawMetaExportOasis_getImplementationName() throw();
css::uno::Sequence<OUString> SAL_CALL XMLDrawMetaExportOasis_getSupportedServiceNames() throw();

OUString SAL_CALL XMLDrawSettingsExportOasis_getImplementationName() throw();
css::uno::Sequence<OUString> SAL_CALL XMLDrawSettingsExportOasis_getSupportedServiceNames() throw();


// draw OOo import

// draw OOo export
OUString SAL_CALL XMLDrawExportOOO_getImplementationName() throw();
css::uno::Sequence<OUString> SAL_CALL XMLDrawExportOOO_getSupportedServiceNames() throw();

OUString SAL_CALL XMLDrawStylesExportOOO_getImplementationName() throw();
css::uno::Sequence<OUString> SAL_CALL XMLDrawStylesExportOOO_getSupportedServiceNames() throw();

OUString SAL_CALL XMLDrawContentExportOOO_getImplementationName() throw();
css::uno::Sequence<OUString> SAL_CALL XMLDrawContentExportOOO_getSupportedServiceNames() throw();

OUString SAL_CALL XMLDrawMetaExportOOO_getImplementationName() throw();
css::uno::Sequence<OUString> SAL_CALL XMLDrawMetaExportOOO_getSupportedServiceNames() throw();

OUString SAL_CALL XMLDrawSettingsExportOOO_getImplementationName() throw();
css::uno::Sequence<OUString> SAL_CALL XMLDrawSettingsExportOOO_getSupportedServiceNames() throw();


// impress animation import
OUString SAL_CALL AnimationsImport_getImplementationName() throw();
css::uno::Sequence<OUString> SAL_CALL AnimationsImport_getSupportedServiceNames() throw();

// drawing layer export
OUString SAL_CALL XMLDrawingLayerExport_getImplementationName() throw();
css::uno::Sequence<OUString> SAL_CALL XMLDrawingLayerExport_getSupportedServiceNames() throw();


// impress xml clipboard export
OUString SAL_CALL XMLImpressClipboardExport_getImplementationName() throw();
css::uno::Sequence<OUString> SAL_CALL XMLImpressClipboardExport_getSupportedServiceNames() throw();


// chart oasis import
OUString SAL_CALL SchXMLImport_getImplementationName() throw();
css::uno::Sequence<OUString> SAL_CALL SchXMLImport_getSupportedServiceNames() throw();

OUString SAL_CALL SchXMLImport_Meta_getImplementationName() throw();
css::uno::Sequence<OUString> SAL_CALL SchXMLImport_Meta_getSupportedServiceNames() throw();

OUString SAL_CALL SchXMLImport_Styles_getImplementationName() throw();
css::uno::Sequence<OUString> SAL_CALL SchXMLImport_Styles_getSupportedServiceNames() throw();

OUString SAL_CALL SchXMLImport_Content_getImplementationName() throw();
css::uno::Sequence<OUString> SAL_CALL SchXMLImport_Content_getSupportedServiceNames() throw();


// chart oasis export
OUString SAL_CALL SchXMLExport_Oasis_getImplementationName() throw();
css::uno::Sequence<OUString> SAL_CALL SchXMLExport_Oasis_getSupportedServiceNames() throw();

OUString SAL_CALL SchXMLExport_Oasis_Meta_getImplementationName() throw();
css::uno::Sequence<OUString> SAL_CALL SchXMLExport_Oasis_Meta_getSupportedServiceNames() throw();

OUString SAL_CALL SchXMLExport_Oasis_Styles_getImplementationName() throw();
css::uno::Sequence<OUString> SAL_CALL SchXMLExport_Oasis_Styles_getSupportedServiceNames() throw();

OUString SAL_CALL SchXMLExport_Oasis_Content_getImplementationName() throw();
css::uno::Sequence<OUString> SAL_CALL SchXMLExport_Oasis_Content_getSupportedServiceNames() throw();

// chart OOo import

// chart OOo export
OUString SAL_CALL SchXMLExport_getImplementationName() throw();
css::uno::Sequence<OUString> SAL_CALL SchXMLExport_getSupportedServiceNames() throw();

OUString SAL_CALL SchXMLExport_Styles_getImplementationName() throw();
css::uno::Sequence<OUString> SAL_CALL SchXMLExport_Styles_getSupportedServiceNames() throw();

OUString SAL_CALL SchXMLExport_Content_getImplementationName() throw();
css::uno::Sequence<OUString> SAL_CALL SchXMLExport_Content_getSupportedServiceNames() throw();

// meta export
OUString SAL_CALL XMLMetaExportComponent_getImplementationName() throw();
css::uno::Sequence<OUString> SAL_CALL XMLMetaExportComponent_getSupportedServiceNames() throw();

// meta import
OUString SAL_CALL XMLMetaImportComponent_getImplementationName() throw();
css::uno::Sequence<OUString> SAL_CALL XMLMetaImportComponent_getSupportedServiceNames() throw();

// meta export OOo
OUString SAL_CALL XMLMetaExportOOO_getImplementationName() throw();
css::uno::Sequence<OUString> SAL_CALL XMLMetaExportOOO_getSupportedServiceNames() throw();

// writer autotext event export
OUString SAL_CALL XMLAutoTextEventExport_getImplementationName() throw();
css::uno::Sequence<OUString> SAL_CALL XMLAutoTextEventExport_getSupportedServiceNames() throw();

// writer autotext event import
OUString SAL_CALL XMLAutoTextEventImport_getImplementationName() throw();
css::uno::Sequence<OUString> SAL_CALL XMLAutoTextEventImport_getSupportedServiceNames() throw();

// writer autotext event export OOo
OUString SAL_CALL XMLAutoTextEventExportOOO_getImplementationName() throw();
css::uno::Sequence<OUString> SAL_CALL XMLAutoTextEventExportOOO_getSupportedServiceNames() throw();

// writer autotext event import OOo
OUString SAL_CALL XMLAutoTextEventImportOOO_getImplementationName() throw();
css::uno::Sequence<OUString> SAL_CALL XMLAutoTextEventImportOOO_getSupportedServiceNames() throw();

// Oasis2OOo transformer
OUString SAL_CALL Oasis2OOoTransformer_getImplementationName() throw();
css::uno::Sequence<OUString> SAL_CALL Oasis2OOoTransformer_getSupportedServiceNames() throw();

// OOo2Oasis transformer
OUString SAL_CALL OOo2OasisTransformer_getImplementationName() throw();
css::uno::Sequence<OUString> SAL_CALL OOo2OasisTransformer_getSupportedServiceNames() throw();

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
