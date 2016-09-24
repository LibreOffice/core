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

#ifndef INCLUDED_XMLOFF_SOURCE_STYLE_XMLFOOTNOTESEPARATOREXPORT_HXX
#define INCLUDED_XMLOFF_SOURCE_STYLE_XMLFOOTNOTESEPARATOREXPORT_HXX

#include <sal/types.h>
#include <vector>

class SvXMLExport;
class XMLPropertySetMapper;
struct XMLPropertyState;
namespace rtl {
    template<class X> class Reference;
}

/**
 * export footnote separator element in page styles
 */
class XMLFootnoteSeparatorExport
{
    SvXMLExport& rExport;

public:

    explicit XMLFootnoteSeparatorExport(SvXMLExport& rExp);

    ~XMLFootnoteSeparatorExport();

    void exportXML(
        const ::std::vector<XMLPropertyState> * pProperties,
        sal_uInt32 nIdx,
        /// used only for debugging
        const rtl::Reference<XMLPropertySetMapper> & rMapper);
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
