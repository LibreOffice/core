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

#ifndef SC_FILTOPT_HXX
#define SC_FILTOPT_HXX

#include <unotools/configitem.hxx>
#include <tools/solar.h>
#include "scdllapi.h"

// filter options

class SC_DLLPUBLIC ScFilterOptions : public utl::ConfigItem
{
    sal_Bool        bWK3Flag;
    double      fExcelColScale;
    double      fExcelRowScale;

    com::sun::star::uno::Sequence<OUString> GetPropertyNames();

public:
            ScFilterOptions();

    virtual void    Notify( const com::sun::star::uno::Sequence<OUString>& aPropertyNames );
    virtual void    Commit();

    sal_Bool        GetWK3Flag() const          { return bWK3Flag; }
    double      GetExcelColScale() const    { return fExcelColScale; }
    double      GetExcelRowScale() const    { return fExcelRowScale; }

    //  values are never modified by office
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
