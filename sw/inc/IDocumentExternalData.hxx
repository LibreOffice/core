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
#ifndef INCLUDED_SW_INC_IDOCUMENTEXTERNALDATA_HXX
#define INCLUDED_SW_INC_IDOCUMENTEXTERNALDATA_HXX

#include <memory>
#include <unordered_map>

namespace sw
{

enum class tExternalDataType { FIB, STTBF_ASSOC };

struct ExternalDataTypeHash
{
    size_t operator()(tExternalDataType eType) const { return static_cast<size_t>(eType); }
};

class ExternalData
{
public:
    ExternalData() {}
    virtual ~ExternalData() {}
};

typedef std::shared_ptr<ExternalData> tExternalDataPointer;
}

class IDocumentExternalData
{
protected:
    typedef std::unordered_map<sw::tExternalDataType, sw::tExternalDataPointer, sw::ExternalDataTypeHash>
    tExternalData;

    tExternalData m_externalData;

    virtual ~IDocumentExternalData() {};

public:
    virtual void setExternalData(sw::tExternalDataType eType,
                                 sw::tExternalDataPointer pPayload) = 0;
    virtual sw::tExternalDataPointer getExternalData(sw::tExternalDataType eType) = 0;
};

#endif // INCLUDED_SW_INC_IDOCUMENTEXTERNALDATA_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
