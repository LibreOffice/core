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

// includes --------------------------------------------------------------
#include <accessibility/standard/vclxaccessiblefixedhyperlink.hxx>

using namespace ::com::sun::star;

// -----------------------------------------------------------------------------
// VCLXAccessibleFixedHyperlink
// -----------------------------------------------------------------------------

VCLXAccessibleFixedHyperlink::VCLXAccessibleFixedHyperlink( VCLXWindow* pVCLWindow )
    :VCLXAccessibleTextComponent( pVCLWindow )
{
}

// -----------------------------------------------------------------------------

VCLXAccessibleFixedHyperlink::~VCLXAccessibleFixedHyperlink()
{
}

// -----------------------------------------------------------------------------

void VCLXAccessibleFixedHyperlink::FillAccessibleStateSet( utl::AccessibleStateSetHelper& rStateSet )
{
    VCLXAccessibleTextComponent::FillAccessibleStateSet( rStateSet );
}

// -----------------------------------------------------------------------------

void VCLXAccessibleFixedHyperlink::implGetLineBoundary( i18n::Boundary& rBoundary, sal_Int32 nIndex )
{
    // TODO
    OCommonAccessibleText::implGetLineBoundary( rBoundary, nIndex );
}

// -----------------------------------------------------------------------------
// XServiceInfo
// -----------------------------------------------------------------------------

::rtl::OUString VCLXAccessibleFixedHyperlink::getImplementationName() throw (uno::RuntimeException)
{
    return ::rtl::OUString::createFromAscii( "com.sun.star.comp.toolkit.AccessibleFixedHyperlink" );
}

// -----------------------------------------------------------------------------

uno::Sequence< ::rtl::OUString > VCLXAccessibleFixedHyperlink::getSupportedServiceNames() throw (uno::RuntimeException)
{
    uno::Sequence< ::rtl::OUString > aNames(1);
    aNames[0] = ::rtl::OUString::createFromAscii( "com.sun.star.awt.AccessibleFixedHyperlink" );
    return aNames;
}

// -----------------------------------------------------------------------------

