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

//IAccessibility2 Implementation 2009-----

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_svtools.hxx"

#ifndef _SVTOOLS_VCLXACCESSIBLEHEADERBAR_HXX_
#include <svtools/vclxaccessibleheaderbar.hxx>
#endif
#ifndef _SVTOOLS_VCLACCESSIBLEHEADBARITEM_HXX_
#include <svtools/vclxaccessibleheaderbaritem.hxx>
#endif

#ifndef _TOOLKIT_AWT_VCLXWINDOWS_HXX_
#include <toolkit/awt/vclxwindows.hxx>
#endif
#ifndef _HEADBAR_HXX
#include <headbar.hxx>
#endif
#ifndef _UTL_ACCESSIBLESTATESETHELPER_HXX_
#include <unotools/accessiblestatesethelper.hxx>
#endif
#ifndef _COM_SUN_STAR_ACCESSIBILITY_ACCESSIBLESTATETYPE_HPP_
#include <com/sun/star/accessibility/AccessibleStateType.hpp>
#endif
#ifndef _COM_SUN_STAR_ACCESSIBILITY_ACCESSIBLEEVENTID_HPP_
#include <com/sun/star/accessibility/AccessibleEventId.hpp>
#endif
#ifndef _CPPUHELPER_TYPEPROVIDER_HXX_
#include <cppuhelper/typeprovider.hxx>
#endif
#ifndef _COMPHELPER_SEQUENCE_HXX_
#include <comphelper/sequence.hxx>
#endif
#ifndef _COM_SUN_STAR_ACCESSIBILITY_ACCESSIBLEROLE_HPP_
#include <com/sun/star/accessibility/AccessibleRole.hpp>
#endif

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::awt;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::accessibility;
using namespace ::comphelper;

VCLXHeaderBar::VCLXHeaderBar(Window* pHeaderBar)
{
    SetWindow(pHeaderBar);
}

VCLXHeaderBar::~VCLXHeaderBar()
{
}

::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessibleContext > VCLXHeaderBar::CreateAccessibleContext()
{
    return new VCLXAccessibleHeaderBar(this);
}


VCLXAccessibleHeaderBar::VCLXAccessibleHeaderBar( VCLXWindow* pVCLWindow )
    :VCLXAccessibleComponent( pVCLWindow )
    ,m_pHeadBar(NULL)
{
    m_pHeadBar = static_cast< HeaderBar* >( GetWindow() );
}

// -----------------------------------------------------------------------------

VCLXAccessibleHeaderBar::~VCLXAccessibleHeaderBar()
{
}

// -----------------------------------------------------------------------------

void VCLXAccessibleHeaderBar::ProcessWindowEvent( const VclWindowEvent& rVclWindowEvent )
{
    VCLXAccessibleComponent::ProcessWindowEvent( rVclWindowEvent );
}

// -----------------------------------------------------------------------------

void VCLXAccessibleHeaderBar::FillAccessibleStateSet( utl::AccessibleStateSetHelper& rStateSet )
{
    VCLXAccessibleComponent::FillAccessibleStateSet( rStateSet );
}

// -----------------------------------------------------------------------------
// XServiceInfo
// -----------------------------------------------------------------------------

::rtl::OUString VCLXAccessibleHeaderBar::getImplementationName() throw (RuntimeException)
{
    return ::rtl::OUString::createFromAscii( "com.sun.star.comp.toolkit.AccessibleHeaderBar" );
}

// -----------------------------------------------------------------------------

Sequence< ::rtl::OUString > VCLXAccessibleHeaderBar::getSupportedServiceNames() throw (RuntimeException)
{
    Sequence< ::rtl::OUString > aNames(1);
    aNames[0] = ::rtl::OUString::createFromAscii( "com.sun.star.awt.AccessibleHeaderBar" );
    return aNames;
}

// =======XAccessibleContext=======

sal_Int32 SAL_CALL VCLXAccessibleHeaderBar::getAccessibleChildCount(  )
        throw (::com::sun::star::uno::RuntimeException)
{
    sal_Int32 nCount = 0;
    if ( m_pHeadBar )
        nCount = m_pHeadBar->GetItemCount();

    return nCount;
}
::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible > SAL_CALL
        VCLXAccessibleHeaderBar::getAccessibleChild( sal_Int32 i )  throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException)
{
    if ( i < 0 || i >= getAccessibleChildCount() )
        throw IndexOutOfBoundsException();

    Reference< XAccessible > xChild;
    // search for the child
    if ( static_cast<sal_uInt16>(i) >= m_aAccessibleChildren.size() )
        xChild = CreateChild (i);
    else
    {
        xChild = m_aAccessibleChildren[i];
        if ( !xChild.is() )
            xChild = CreateChild (i);
    }
    return xChild;
}

sal_Int16 SAL_CALL VCLXAccessibleHeaderBar::getAccessibleRole(  ) throw (::com::sun::star::uno::RuntimeException)
{
    return com::sun::star::accessibility::AccessibleRole::LIST;
}

void SAL_CALL VCLXAccessibleHeaderBar::disposing (void)
{
    ListItems().swap(m_aAccessibleChildren);
    VCLXAccessibleComponent::disposing();
}

::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible > VCLXAccessibleHeaderBar::CreateChild (sal_Int32 i)
{
    Reference<XAccessible> xChild;

    sal_uInt16 nPos = static_cast<sal_uInt16>(i);
    if ( nPos >= m_aAccessibleChildren.size() )
    {
        m_aAccessibleChildren.resize(nPos + 1);

        // insert into the container
        xChild = new VCLXAccessibleHeaderBarItem(m_pHeadBar, i);
        m_aAccessibleChildren[nPos] = xChild;
    }
    else
    {
        xChild = m_aAccessibleChildren[nPos];
        // check if position is empty and can be used else we have to adjust all entries behind this
        if ( !xChild.is() )
        {
            xChild = new VCLXAccessibleHeaderBarItem(m_pHeadBar, i);
            m_aAccessibleChildren[nPos] = xChild;
        }
    }
    return xChild;
}
//-----IAccessibility2 Implementation 2009
