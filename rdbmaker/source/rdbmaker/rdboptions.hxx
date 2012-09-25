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

#ifndef _RDBMAKER_RDBOPTIONS_HXX_
#define _RDBMAKER_RDBOPTIONS_HXX_

#include    <codemaker/options.hxx>

class RdbOptions : public Options
{
public:
    RdbOptions()
        : Options()
        , m_generateTypeList(false)
        {}

    ~RdbOptions() {}

    sal_Bool initOptions(int ac, char* av[], sal_Bool bCmdFile=sal_False)
            throw( IllegalArgument );

    ::rtl::OString  prepareHelp();

    ::rtl::OString  prepareVersion();

    bool generateTypeList()
        { return m_generateTypeList; }

protected:
    bool m_generateTypeList;
};

#endif // _RDBMAKER_RDBOPTIONS_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
