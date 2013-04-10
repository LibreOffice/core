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

#include "precompiled_sfx2.hxx"

#include "DeckDescriptor.hxx"

namespace sfx2 { namespace sidebar {

DeckDescriptor::DeckDescriptor (void)
    : msTitle(),
      msId(),
      msIconURL(),
      msHighContrastIconURL(),
      msHelpURL(),
      msHelpText(),
      maContextList(),
      mnOrderIndex(10000) // Default value as defined in Sidebar.xcs
{
}




DeckDescriptor::DeckDescriptor (const DeckDescriptor& rOther)
    : msTitle(rOther.msTitle),
      msId(rOther.msId),
      msIconURL(rOther.msIconURL),
      msHighContrastIconURL(rOther.msHighContrastIconURL),
      msHelpURL(rOther.msHelpURL),
      msHelpText(rOther.msHelpText),
      maContextList(rOther.maContextList),
      mnOrderIndex(rOther.mnOrderIndex)
{
}




DeckDescriptor::~DeckDescriptor (void)
{
}

} } // end of namespace sfx2::sidebar
