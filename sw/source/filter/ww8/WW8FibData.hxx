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
#ifndef INCLUDED_WW8_FIB_DATA_HXX
#define INCLUDED_WW8_FIB_DATA_HXX
#include <IDocumentExternalData.hxx>

namespace ww8
{
class WW8FibData : public ::sw::ExternalData
{
    bool m_bReadOnlyRecommended;
    bool m_bWriteReservation;

public:
    WW8FibData();
    virtual ~WW8FibData();

    void setReadOnlyRecommended(bool bReadOnlyRecommended);
    void setWriteReservation(bool bWriteReservation);

    bool getReadOnlyRecommended() const;
    bool getWriteReservation() const;
};
}

#endif // INCLUDED_WW8_FIB_DATA_HXX
