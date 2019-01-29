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

#ifndef INCLUDED_SVTOOLS_COLLATORRES_HXX
#define INCLUDED_SVTOOLS_COLLATORRES_HXX

#include <svtools/svtdllapi.h>
#include <rtl/ustring.hxx>
#include <vector>

class SVT_DLLPUBLIC CollatorResource
{
private:
    //  wrapper for locale specific translations data of collator algorithm
    class CollatorResourceData
    {
        friend class CollatorResource;
    private:
        OUString m_aName;
        OUString m_aTranslation;
    public:
        CollatorResourceData(const OUString& rAlgorithm, const OUString& rTranslation)
            : m_aName(rAlgorithm)
            , m_aTranslation(rTranslation)
        {
        }
        const OUString& GetAlgorithm() const { return m_aName; }
        const OUString& GetTranslation() const { return m_aTranslation; }
    };
    std::vector<CollatorResourceData> m_aData;
public:
    CollatorResource();
    const OUString& GetTranslation(const OUString& rAlgorithm);
};

#endif // INCLUDED_SVTOOLS_COLLATORRES_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
