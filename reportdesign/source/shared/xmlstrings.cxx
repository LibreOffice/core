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

#include "xmlstrings.hrc"

namespace rptxml
{
#include "stringconstants.cxx"
IMPLEMENT_CONSTASCII_USTRING(SERVICE_SETTINGSIMPORTER           , "com.sun.star.comp.Report.XMLOasisSettingsImporter");
IMPLEMENT_CONSTASCII_USTRING(SERVICE_STYLESIMPORTER             , "com.sun.star.comp.Report.XMLOasisStylesImporter");
IMPLEMENT_CONSTASCII_USTRING(SERVICE_CONTENTIMPORTER            , "com.sun.star.comp.Report.XMLOasisContentImporter");
IMPLEMENT_CONSTASCII_USTRING(SERVICE_IMPORTFILTER               , "com.sun.star.document.ImportFilter");
IMPLEMENT_CONSTASCII_USTRING(SERVICE_METAIMPORTER               , "com.sun.star.comp.Report.XMLOasisMetaImporter");

IMPLEMENT_CONSTASCII_USTRING(PROPERTY_BORDERLEFT                , "BorderLeft");
IMPLEMENT_CONSTASCII_USTRING(PROPERTY_BORDERRIGHT               , "BorderRight");
IMPLEMENT_CONSTASCII_USTRING(PROPERTY_BORDERTOP                 , "BorderTop");
IMPLEMENT_CONSTASCII_USTRING(PROPERTY_BORDERBOTTOM              , "BorderBottom");
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
