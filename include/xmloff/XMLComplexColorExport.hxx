/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#pragma once

#include <xmloff/dllapi.h>
#include <sal/types.h>
#include <rtl/ustring.hxx>

namespace com::sun::star::uno
{
class Any;
}
class SvXMLExport;

class XMLOFF_DLLPUBLIC XMLComplexColorExport
{
    SvXMLExport& mrExport;

public:
    XMLComplexColorExport(SvXMLExport& rExport);

    void exportXML(const css::uno::Any& rAny, sal_uInt16 nPrefix, const OUString& rLocalName);
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
