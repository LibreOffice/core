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

#include <sal/types.h>

#include <vector>

class SvXMLExport;
class XMLPropertySetMapper;
struct XMLPropertyState;
namespace rtl {
    template<class X> class Reference;
}

/**
 * Export the footnote-/endnote-configuration element in section styles.
 *
 * Because this class contains only one method, and all information is
 * available during that method call, we simply make it static.
 */
class XMLSectionFootnoteConfigExport
{

public:
    static void exportXML(
        SvXMLExport& rExport,
        bool bEndnote,
        const ::std::vector<XMLPropertyState> * pProperties,
        sal_uInt32 nIdx,
        const rtl::Reference<XMLPropertySetMapper> & rMapper);    /// used only for debugging
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
