/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: xmlstrings.cxx,v $
 * $Revision: 1.4 $
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
#include "precompiled_reportdesign.hxx"

#include "xmlstrings.hrc"

namespace rptxml
{
#include "stringconstants.cxx"
IMPLEMENT_CONSTASCII_USTRING(SERVICE_SETTINGSIMPORTER           , "com.sun.star.comp.Report.XMLOasisSettingsImporter");
IMPLEMENT_CONSTASCII_USTRING(SERVICE_STYLESIMPORTER             , "com.sun.star.comp.Report.XMLOasisStylesImporter");
IMPLEMENT_CONSTASCII_USTRING(SERVICE_CONTENTIMPORTER            , "com.sun.star.comp.Report.XMLOasisContentImporter");
IMPLEMENT_CONSTASCII_USTRING(SERVICE_IMPORTFILTER               , "com.sun.star.comp.document.ImportFilter");
IMPLEMENT_CONSTASCII_USTRING(SERVICE_METAIMPORTER               , "com.sun.star.comp.Report.XMLOasisMetaImporter");

IMPLEMENT_CONSTASCII_USTRING(PROPERTY_BORDERLEFT                , "BorderLeft");
IMPLEMENT_CONSTASCII_USTRING(PROPERTY_BORDERRIGHT               , "BorderRight");
IMPLEMENT_CONSTASCII_USTRING(PROPERTY_BORDERTOP                 , "BorderTop");
IMPLEMENT_CONSTASCII_USTRING(PROPERTY_BORDERBOTTOM              , "BorderBottom");
}

