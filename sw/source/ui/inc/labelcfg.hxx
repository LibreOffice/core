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
#ifndef _LABELCFG_HXX
#define _LABELCFG_HXX

#include <unotools/configitem.hxx>
#include "swdllapi.h"

#include <map>
#include <vector>

class SwLabRecs;
class SwLabRec;

struct SwLabelMeasure
{
    OUString m_aMeasure;     // string contains the label dimensions
    bool     m_bPredefined;  // used to distinguish predefined from user-defined labels
};

class SW_DLLPUBLIC SwLabelConfig : public utl::ConfigItem
{
    std::vector<rtl::OUString> m_aManufacturers;
    std::map< OUString, std::map<OUString, SwLabelMeasure> > m_aLabels;

public:
    SwLabelConfig();
    virtual ~SwLabelConfig();

    virtual void Commit();
    virtual void Notify( const ::com::sun::star::uno::Sequence< rtl::OUString >& aPropertyNames );

    void    FillLabels(const rtl::OUString& rManufacturer, SwLabRecs& rLabArr);
    const std::vector<rtl::OUString>& GetManufacturers() const {return m_aManufacturers;}

    sal_Bool    HasLabel(const rtl::OUString& rManufacturer, const rtl::OUString& rType);
    bool        IsPredefinedLabel(const rtl::OUString& rManufacturer, const rtl::OUString& rType)
                  { return m_aLabels[rManufacturer][rType].m_bPredefined; };
    void        SaveLabel(const rtl::OUString& rManufacturer, const rtl::OUString& rType,
                            const SwLabRec& rRec);
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
