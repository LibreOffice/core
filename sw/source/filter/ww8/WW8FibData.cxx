/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/


/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil -*- */

#include "WW8FibData.hxx"

namespace ww8
{
WW8FibData::WW8FibData()
  : m_bReadOnlyRecommended(false),
    m_bWriteReservation(false)
{
}

WW8FibData::~WW8FibData()
{
}

void WW8FibData::setReadOnlyRecommended(bool bReadOnlyRecommended)
{
    m_bReadOnlyRecommended = bReadOnlyRecommended;
}

void WW8FibData::setWriteReservation(bool bWriteReservation)
{
    m_bWriteReservation = bWriteReservation;
}

bool WW8FibData::getReadOnlyRecommended() const
{
    return m_bReadOnlyRecommended;
}

bool WW8FibData::getWriteReservation() const
{
    return m_bWriteReservation;
}
}
