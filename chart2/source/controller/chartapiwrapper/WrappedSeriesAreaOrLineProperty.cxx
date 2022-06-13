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

#include <utility>

#include "WrappedSeriesAreaOrLineProperty.hxx"
#include "DataSeriesPointWrapper.hxx"

namespace chart::wrapper
{

WrappedSeriesAreaOrLineProperty::WrappedSeriesAreaOrLineProperty(
      const OUString& rOuterName
    , OUString aInnerAreaTypeName
    , OUString aInnerLineTypeName
    , DataSeriesPointWrapper* pDataSeriesPointWrapper )
    : WrappedProperty( rOuterName, OUString() )
    , m_pDataSeriesPointWrapper( pDataSeriesPointWrapper )
    , m_aInnerAreaTypeName(std::move( aInnerAreaTypeName ))
    , m_aInnerLineTypeName(std::move( aInnerLineTypeName ))
{
}
WrappedSeriesAreaOrLineProperty::~WrappedSeriesAreaOrLineProperty()
{
}

//virtual
OUString WrappedSeriesAreaOrLineProperty::getInnerName() const
{
    if( m_pDataSeriesPointWrapper && !m_pDataSeriesPointWrapper->isSupportingAreaProperties() )
        return m_aInnerLineTypeName;
    return m_aInnerAreaTypeName;
}

} //namespace chart::wrapper

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
