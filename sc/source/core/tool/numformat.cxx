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
#include <scitems.hxx>
#include <document.hxx>

#include <svl/zforlist.hxx>
#include <svl/zformat.hxx>
#include <svl/intitem.hxx>
#include <svl/languageoptions.hxx>

namespace sc {

bool NumFmtUtil::isGeneral( sal_uLong nFormat )
{
    return (nFormat % SV_COUNTRY_LANGUAGE_OFFSET) == 0;
}

bool NumFmtUtil::isGeneral( const ScPatternAttr& rPat )
{
    const SfxPoolItem* pItem = NULL;
    if (!rPat.GetItemSet().HasItem(ATTR_VALUE_FORMAT, &pItem))
        // Assume it's 'General' when the number format is not explicitly set.
        return true;

    sal_uInt32 nNumFmt = static_cast<const SfxUInt32Item*>(pItem)->GetValue();
    return isGeneral(nNumFmt);
}

bool NumFmtUtil::isLatinScript( const ScPatternAttr& rPat, ScDocument& rDoc )
{
    SvNumberFormatter* pFormatter = rDoc.GetFormatTable();
    sal_uInt32 nKey = rPat.GetNumberFormat(pFormatter);
    const SvNumberformat* pFormat = pFormatter->GetEntry(nKey);
    if (!pFormat || !pFormat->IsStandard())
        return false;

    // The standard format is all-latin if the decimal separator doesn't
    // have a different script type

    OUString aDecSep;
    LanguageType nFormatLang = pFormat->GetLanguage();
    if (nFormatLang == LANGUAGE_SYSTEM)
        aDecSep = ScGlobal::pLocaleData->getNumDecimalSep();
    else
    {
        LocaleDataWrapper aLocaleData(
            comphelper::getProcessComponentContext(), LanguageTag(nFormatLang));
        aDecSep = aLocaleData.getNumDecimalSep();
    }

    sal_uInt8 nScript = rDoc.GetStringScriptType(aDecSep);
    return (nScript == 0 || nScript == SCRIPTTYPE_LATIN);
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
