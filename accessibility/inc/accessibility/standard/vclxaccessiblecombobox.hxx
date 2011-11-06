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



#ifndef ACCESSIBILITY_STANDARD_VCLXACCESSIBLECOMBOBOX_HXX
#define ACCESSIBILITY_STANDARD_VCLXACCESSIBLECOMBOBOX_HXX

#include <map>
#include <accessibility/standard/vclxaccessiblebox.hxx>
#ifndef _COM_SUN_STAR_ACCESSIBILITY_STANDARD_ACCESSIBLEROLE_HPP_
#include <com/sun/star/accessibility/AccessibleRole.hpp>
#endif
#ifndef _CPPUHELPER_IMPLBASE1_HXX
#include <cppuhelper/implbase1.hxx>
#endif


/** The accessible combobox has two children.  The first is the text field
    represented by an object of the <type>VCLXAccessibleEdit</type> class.
    The second is the list containing all items and is represented by an
    object of the <type>VCLXAccessibleList</type> class which does not
    support selection at the moment.
*/
class VCLXAccessibleComboBox
    : public VCLXAccessibleBox
{
public:
    VCLXAccessibleComboBox (VCLXWindow* pVCLXindow);

    // XServiceInfo
    virtual ::rtl::OUString SAL_CALL getImplementationName (void)
        throw (::com::sun::star::uno::RuntimeException);
    // Return combo box specific services.
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL
        getSupportedServiceNames (void)
        throw (::com::sun::star::uno::RuntimeException);

protected:
    virtual ~VCLXAccessibleComboBox (void);

    virtual bool IsValid (void) const;
    virtual void ProcessWindowEvent (const VclWindowEvent& rVclWindowEvent);
};

#endif // ACCESSIBILITY_STANDARD_VCLXACCESSIBLECHECKBOX_HXX

