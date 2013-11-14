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



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sc.hxx"


#include "AccessibleDocumentBase.hxx"

#ifndef _COM_SUN_STAR_ACCESSIBILITY_XACCESSIBLEROLE_HPP_
#include <com/sun/star/accessibility/AccessibleRole.hpp>
#endif

using namespace ::com::sun::star;
using namespace ::com::sun::star::accessibility;

    //=====  internal  ========================================================

ScAccessibleDocumentBase::ScAccessibleDocumentBase(
        const uno::Reference<XAccessible>& rxParent)
    : ScAccessibleContextBase(rxParent, AccessibleRole::DOCUMENT)
{
}

ScAccessibleDocumentBase::~ScAccessibleDocumentBase(void)
{
}
//IAccessibility2 Implementation 2009-----
void ScAccessibleDocumentBase::SwitchViewFireFocus()
{
    CommitFocusGained();
}
//-----IAccessibility2 Implementation 2009

