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
#include <accessibility/standard/vclxaccessiblecombobox.hxx>
#include <accessibility/standard/vclxaccessiblelist.hxx>
#include <com/sun/star/accessibility/AccessibleStateType.hpp>
#include <com/sun/star/accessibility/AccessibleEventId.hpp>
#include <vcl/svapp.hxx>
#include <vcl/combobox.hxx>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::accessibility;


VCLXAccessibleComboBox::VCLXAccessibleComboBox (VCLXWindow* pVCLWindow)
    : VCLXAccessibleBox (pVCLWindow, VCLXAccessibleBox::COMBOBOX, false)
{
}




VCLXAccessibleComboBox::~VCLXAccessibleComboBox (void)
{
}




bool VCLXAccessibleComboBox::IsValid (void) const
{
    return static_cast<ComboBox*>(GetWindow()) != NULL;

}




void VCLXAccessibleComboBox::ProcessWindowEvent (const VclWindowEvent& rVclWindowEvent)
{
    VCLXAccessibleBox::ProcessWindowEvent( rVclWindowEvent );
}




//=====  XServiceInfo  ========================================================

::rtl::OUString VCLXAccessibleComboBox::getImplementationName (void)
    throw (RuntimeException)
{
    return ::rtl::OUString::createFromAscii ("com.sun.star.comp.toolkit.AccessibleComboBox");
}




Sequence< ::rtl::OUString > VCLXAccessibleComboBox::getSupportedServiceNames (void)
    throw (RuntimeException)
{
    Sequence< ::rtl::OUString > aNames = VCLXAccessibleBox::getSupportedServiceNames();
    sal_Int32 nLength = aNames.getLength();
    aNames.realloc( nLength + 1 );
    aNames[nLength] = ::rtl::OUString::createFromAscii(
        "com.sun.star.accessibility.AccessibleComboBox" );
    return aNames;
}
