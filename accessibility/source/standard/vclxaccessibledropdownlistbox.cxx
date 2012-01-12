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

#include <accessibility/standard/vclxaccessibledropdownlistbox.hxx>
#include <accessibility/standard/vclxaccessiblelistbox.hxx>
#include <accessibility/helper/accresmgr.hxx>
#include <accessibility/helper/accessiblestrings.hrc>

#include <com/sun/star/accessibility/AccessibleEventId.hpp>
#include <com/sun/star/accessibility/AccessibleRole.hpp>
#include <com/sun/star/accessibility/AccessibleStateType.hpp>
#include <tools/debug.hxx>
#include <vcl/svapp.hxx>
#include <vcl/lstbox.hxx>
#include <vcl/unohelp.hxx>

#include <toolkit/awt/vclxwindow.hxx>
#include <toolkit/helper/convert.hxx>

#include <comphelper/sequence.hxx>
#include <cppuhelper/typeprovider.hxx>
#include <unotools/accessiblestatesethelper.hxx>


using namespace ::com::sun::star;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::accessibility;


VCLXAccessibleDropDownListBox::VCLXAccessibleDropDownListBox (VCLXWindow* pVCLWindow)
    : VCLXAccessibleBox (pVCLWindow,  VCLXAccessibleBox::LISTBOX, true)
{
}




VCLXAccessibleDropDownListBox::~VCLXAccessibleDropDownListBox()
{
}




bool VCLXAccessibleDropDownListBox::IsValid (void) const
{
    return static_cast<ListBox*>(GetWindow()) != NULL;

}




void VCLXAccessibleDropDownListBox::ProcessWindowEvent( const VclWindowEvent& rVclWindowEvent )
{
    VCLXAccessibleBox::ProcessWindowEvent (rVclWindowEvent);
}




//=====  XServiceInfo  ========================================================

::rtl::OUString VCLXAccessibleDropDownListBox::getImplementationName()
    throw (RuntimeException)
{
    return ::rtl::OUString::createFromAscii("com.sun.star.comp.toolkit.AccessibleDropDownListBox");
}




Sequence< ::rtl::OUString > VCLXAccessibleDropDownListBox::getSupportedServiceNames (void)
    throw (RuntimeException)
{
    Sequence< ::rtl::OUString > aNames = VCLXAccessibleBox::getSupportedServiceNames();
    sal_Int32 nLength = aNames.getLength();
    aNames.realloc( nLength + 1 );
    aNames[nLength] = ::rtl::OUString::createFromAscii(
        "com.sun.star.accessibility.AccessibleDropDownListBox" );
    return aNames;
}
