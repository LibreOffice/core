/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/
#ifndef INCLUDED_I_DOCUMENT_EXTERNAL_DATA_HXX
#define INCLUDED_I_DOCUMENT_EXTERNAL_DATA_HXX

#include <boost/unordered_map.hpp>
#include <boost/shared_ptr.hpp>

namespace sw
{
enum tExternalDataType { FIB, STTBF_ASSOC };

struct ExternalDataTypeHash
{
    size_t operator()(tExternalDataType eType) const { return eType; }
};

class ExternalData
{
public:
    ExternalData() {}
    virtual ~ExternalData() {}
};

typedef ::boost::shared_ptr<ExternalData> tExternalDataPointer;
}


class IDocumentExternalData
{
protected:
    typedef ::boost::unordered_map<sw::tExternalDataType, sw::tExternalDataPointer, sw::ExternalDataTypeHash>
    tExternalData;

    tExternalData m_externalData;

    virtual ~IDocumentExternalData() {};

public:
    virtual void setExternalData(sw::tExternalDataType eType,
                                 sw::tExternalDataPointer pPayload) = 0;
    virtual sw::tExternalDataPointer getExternalData(sw::tExternalDataType eType) = 0;
};

#endif //INCLUDED_I_DOCUMENT_EXTERNAL_DATA_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
