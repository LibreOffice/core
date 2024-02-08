/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <config_options.h>
#include <xmloff/dllapi.h>
#include <xmloff/xmlictxt.hxx>

class XMLTextImportHelper;

/// Handles <text:line-break loext:clear="..."> when the attribute is present.
class UNLESS_MERGELIBS(XMLOFF_DLLPUBLIC) SvXMLLineBreakContext : public SvXMLImportContext
{
    XMLTextImportHelper& m_rHelper;

public:
    SvXMLLineBreakContext(SvXMLImport& rImport, XMLTextImportHelper& rHelper);

protected:
    void SAL_CALL startFastElement(
        sal_Int32 nElement,
        const css::uno::Reference<css::xml::sax::XFastAttributeList>& xAttrList) override;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
