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



#ifndef ACCESSIBILITY_STANDARD_VCLXACCESSIBLEDROPDOWNLISTBOX_HXX
#define ACCESSIBILITY_STANDARD_VCLXACCESSIBLEDROPDOWNLISTBOX_HXX

#include <accessibility/standard/vclxaccessiblebox.hxx>
#include <com/sun/star/accessibility/XAccessibleAction.hpp>

#ifndef _CPPUHELPER_IMPLBASE1_HXX
#include <cppuhelper/implbase1.hxx>
#endif
#ifndef _CPPUHELPER_WEAKREF_HXX
#include <cppuhelper/weakref.hxx>
#endif


/** The accessible drop down combobox has two children.  The first is the
    text field represented by an object of the
    <type>VCLXAccessibleTextField</type> class which can not be edited.  The
    second is the list containing all items and is represented by an object
    of the <type>VCLXAccessibleListBoxList</type> class which does support
    selection.
*/
class VCLXAccessibleDropDownListBox : public VCLXAccessibleBox
{
public:
    VCLXAccessibleDropDownListBox (VCLXWindow* pVCLXindow);

    // XServiceInfo
    virtual ::rtl::OUString SAL_CALL getImplementationName (void)
        throw (::com::sun::star::uno::RuntimeException);
    // Return drop down list box specific services.
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL
        getSupportedServiceNames (void)
        throw (::com::sun::star::uno::RuntimeException);

protected:
    virtual ~VCLXAccessibleDropDownListBox (void);

    virtual bool IsValid (void) const;
    virtual void ProcessWindowEvent (const VclWindowEvent& rVclWindowEvent);

};

#endif // ACCESSIBILITY_STANDARD_VCLXACCESSIBLEDROPDOWNLISTBOX_HXX

