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

#include <numformat.hxx>
#include <patattr.hxx>
#include <document.hxx>

#include <comphelper/processfactory.hxx>
#include <svl/numformat.hxx>
#include <svl/zformat.hxx>
#include <svl/languageoptions.hxx>
#include <optional>

namespace
{
    const OUString& getNumDecimalSep(const SvNumberformat& rFormat)
    {
        LanguageType nFormatLang = rFormat.GetLanguage();
        if (nFormatLang == LANGUAGE_SYSTEM)
            return ScGlobal::getLocaleData().getNumDecimalSep();
        // LocaleDataWrapper can be expensive to construct, so cache the result for
        // repeated calls
        static const LocaleDataWrapper* localeCache { nullptr };
        if (!localeCache || localeCache->getLanguageTag().getLanguageType() != nFormatLang)
            localeCache = LocaleDataWrapper::get(LanguageTag(nFormatLang));
        return localeCache->getNumDecimalSep();
    }
}

namespace sc {

bool NumFmtUtil::isLatinScript( const ScPatternAttr& rPat, ScDocument& rDoc )
{
    SvNumberFormatter* pFormatter = rDoc.GetFormatTable();
    sal_uInt32 nKey = rPat.GetNumberFormat(pFormatter);
    return isLatinScript(nKey, rDoc);
}

bool NumFmtUtil::isLatinScript( sal_uLong nFormat, ScDocument& rDoc )
{
    SvNumberFormatter* pFormatter = rDoc.GetFormatTable();
    const SvNumberformat* pFormat = pFormatter->GetEntry(nFormat);
    if (!pFormat || !pFormat->IsStandard())
        return false;

    // The standard format is all-latin if the decimal separator doesn't
    // have a different script type
    SvtScriptType nScript = rDoc.GetStringScriptType(getNumDecimalSep(*pFormat));
    return (nScript == SvtScriptType::NONE || nScript == SvtScriptType::LATIN);
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
