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
#include "precompiled_svx.hxx"

#include <svx/AccessibleSvxFindReplaceDialog.hxx>


#ifndef _TOOLKIT_AWT_VCLXWINDOW_HXX_
#include "toolkit/awt/vclxwindow.hxx"
#endif

#ifndef _SVX_SRCHDLG_HXX
#include <svx/srchdlg.hxx>
#endif

#ifndef _UTL_ACCESSIBLERELATIONSETHELPER_HXX_
#include <unotools/accessiblerelationsethelper.hxx>
#endif

#ifndef _COM_SUN_STAR_ACCESSIBILITY_ACCESSIBLERELATIONTYPE_HPP_
#include <com/sun/star/accessibility/AccessibleRelationType.hpp>
#endif

#ifndef _COM_SUN_STAR_ACCESSIBILITY_XACCESSIBLEGETACCFLOWTO_HPP_
#include <com/sun/star/accessibility/XAccessibleGetAccFlowTo.hpp>
#endif

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;

VCLXAccessibleSvxFindReplaceDialog::VCLXAccessibleSvxFindReplaceDialog(VCLXWindow* pVCLXindow)
    :VCLXAccessibleComponent( pVCLXindow )
{

}

VCLXAccessibleSvxFindReplaceDialog::~VCLXAccessibleSvxFindReplaceDialog()
{
}

void VCLXAccessibleSvxFindReplaceDialog::FillAccessibleRelationSet( utl::AccessibleRelationSetHelper& rRelationSet )
{
    VCLXAccessibleComponent::FillAccessibleRelationSet( rRelationSet );
    Window* pDlg = GetWindow();
    if ( pDlg )
    {
        SvxSearchDialog* pSrchDlg = static_cast<SvxSearchDialog*>( pDlg );
        Window* pDocWin = pSrchDlg->GetDocWin();
        if ( !pDocWin )
        {
            return;
        }
        Reference < accessibility::XAccessible > xDocAcc = pDocWin->GetAccessible();
        if ( !xDocAcc.is() )
        {
            return;
        }
        Reference< accessibility::XAccessibleGetAccFlowTo > xGetAccFlowTo( xDocAcc, UNO_QUERY );
        if ( !xGetAccFlowTo.is() )
        {
            return;
        }
        Any aAny;
        aAny <<= ( pSrchDlg->GetSrchFlag() );

        const sal_Int32 FORFINDREPLACEFLOWTO = 2;
        uno::Sequence<uno::Any> aAnySeq = xGetAccFlowTo->get_AccFlowTo( aAny,  FORFINDREPLACEFLOWTO );

        sal_Int32 nLen = aAnySeq.getLength();
        if ( nLen )
        {
            uno::Sequence< uno::Reference< uno::XInterface > > aSequence( nLen );
            for ( sal_Int32 i = 0; i < nLen; i++ )
            {
                uno::Reference < accessibility::XAccessible > xAcc;
                aAnySeq[i] >>= xAcc;
                aSequence[i] = xAcc;
            }
            rRelationSet.AddRelation( accessibility::AccessibleRelation( accessibility::AccessibleRelationType::CONTENT_FLOWS_TO, aSequence ) );
        }
    }
}
// -----------------------------------------------------------------------------
// XServiceInfo
// -----------------------------------------------------------------------------

::rtl::OUString VCLXAccessibleSvxFindReplaceDialog::getImplementationName() throw (RuntimeException)
{
    return ::rtl::OUString::createFromAscii( "VCLXAccessibleSvxFindReplaceDialog" );
}

// -----------------------------------------------------------------------------

Sequence< ::rtl::OUString > VCLXAccessibleSvxFindReplaceDialog::getSupportedServiceNames() throw (RuntimeException)
{
    Sequence< ::rtl::OUString > aNames(1);
    aNames[0] = ::rtl::OUString::createFromAscii( "VCLXAccessibleSvxFindReplaceDialog" );
    return aNames;
}
