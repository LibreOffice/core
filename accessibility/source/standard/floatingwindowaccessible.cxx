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
#include "precompiled_accessibility.hxx"

#include <floatingwindowaccessible.hxx>


#include <com/sun/star/accessibility/AccessibleRelationType.hpp>
#include <unotools/accessiblerelationsethelper.hxx>
#include <vcl/window.hxx>

namespace uno = ::com::sun::star::uno;

using ::com::sun::star::accessibility::AccessibleRelation;
namespace AccessibleRelationType = ::com::sun::star::accessibility::AccessibleRelationType;

//-------------------------------------------------------------------

FloatingWindowAccessible::FloatingWindowAccessible(VCLXWindow* pWindow) :
    VCLXAccessibleComponent(pWindow)
{
}

//-------------------------------------------------------------------

FloatingWindowAccessible::~FloatingWindowAccessible()
{
}

//-------------------------------------------------------------------

void FloatingWindowAccessible::FillAccessibleRelationSet(utl::AccessibleRelationSetHelper& rRelationSet)
{
    Window* pWindow = GetWindow();
    if ( pWindow )
    {
        Window* pParentWindow = pWindow->GetParent();
        if( pParentWindow )
        {
            uno::Sequence< uno::Reference< uno::XInterface > > aSequence(1);
            aSequence[0] = pParentWindow->GetAccessible();
            rRelationSet.AddRelation( AccessibleRelation( AccessibleRelationType::SUB_WINDOW_OF, aSequence ) );
        }
    }
}
