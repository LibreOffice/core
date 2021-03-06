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

#pragma once

#include "richstring.hxx"

namespace oox::xls {

/** Collects all strings from the shared strings substream. */
class SharedStringsBuffer : public WorkbookHelper
{
public:
    explicit            SharedStringsBuffer( const WorkbookHelper& rHelper );

    /** Creates and returns a new string entry. */
    RichStringRef       createRichString();

    /** Final processing after import of all strings. */
    void                finalizeImport();

    /** Returns the specified string. */
    RichStringRef       getString( sal_Int32 nStringId ) const;

private:
    typedef RefVector< RichString > StringVector;
    StringVector        maStrings;
};

} // namespace oox::xls

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
