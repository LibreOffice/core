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

#include "PanelDescriptor.hxx"


namespace sfx2 { namespace sidebar {

PanelDescriptor::PanelDescriptor (void)
    : msTitle(),
      mbIsTitleBarOptional(false),
      msId(),
      msDeckId(),
      msHelpURL(),
      maContextList(),
      msImplementationURL(),
      mnOrderIndex(10000), // Default value as defined in Sidebar.xcs
      mbWantsCanvas(false)
{
}




PanelDescriptor::PanelDescriptor (const PanelDescriptor& rOther)
    : msTitle(rOther.msTitle),
      mbIsTitleBarOptional(rOther.mbIsTitleBarOptional),
      msId(rOther.msId),
      msDeckId(rOther.msDeckId),
      msHelpURL(rOther.msHelpURL),
      maContextList(rOther.maContextList),
      msImplementationURL(rOther.msImplementationURL),
      mnOrderIndex(rOther.mnOrderIndex),
      mbWantsCanvas(rOther.mbWantsCanvas)
{
}



PanelDescriptor::~PanelDescriptor (void)
{
}


} } // end of namespace sfx2::sidebar
