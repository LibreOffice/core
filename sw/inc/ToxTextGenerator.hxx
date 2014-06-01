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

#ifndef SW_TOXTEXTGENERATOR_HXX_
#define SW_TOXTEXTGENERATOR_HXX_

#include "sal/types.h"
#include "swdllapi.h"
#include <vector>

class SwDoc;
class SwForm;
class SwPageDesc;
class SwTOXSortTabBase;

namespace sw {

class SW_DLLPUBLIC ToxTextGenerator
{
public:
    ToxTextGenerator(const SwForm& toxForm)
        : mToxForm(toxForm)
    {}
    /** Generate the text for an entry of a table of X (X is, e.g., content).
     *
     * This method will process the entries in @p entries, starting at @p indexOfEntryToProcess and
     * process @p numberOfEntriesToProcess entries.
     */
    void
    GenerateText(SwDoc *doc, const std::vector<SwTOXSortTabBase*>& entries,
                      sal_uInt16 indexOfEntryToProcess, sal_uInt16 numberOfEntriesToProcess,
                      sal_uInt32 _nTOXSectNdIdx, const SwPageDesc* _pDefaultPageDesc);


private:
    const SwForm& mToxForm;
};

}

#endif /* SW_TOXTEXTGENERATOR_HXX_ */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
