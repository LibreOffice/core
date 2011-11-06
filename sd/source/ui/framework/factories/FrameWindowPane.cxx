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



#include "precompiled_sd.hxx"

#include "FrameWindowPane.hxx"

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::drawing::framework;

namespace sd { namespace framework {

FrameWindowPane::FrameWindowPane (
    const Reference<XResourceId>& rxPaneId,
    ::Window* pWindow)
    : Pane(rxPaneId,pWindow)
{
}




FrameWindowPane::~FrameWindowPane (void) throw()
{
}




sal_Bool SAL_CALL FrameWindowPane::isAnchorOnly (void)
    throw (RuntimeException)
{
    return false;
}


} } // end of namespace sd::framework
