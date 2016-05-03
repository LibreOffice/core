/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */
#include <AccessibleSvxFindReplaceDialog.hxx>
#include <svx/srchdlg.hxx>
#include <toolkit/awt/vclxwindow.hxx>
#include <unotools/accessiblerelationsethelper.hxx>
#include <com/sun/star/accessibility/AccessibleRelationType.hpp>
#include <com/sun/star/accessibility/XAccessibleGetAccFlowTo.hpp>

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
    vcl::Window* pDlg = GetWindow();
    if ( pDlg )
    {
        SvxSearchDialog* pSrchDlg = static_cast<SvxSearchDialog*>( pDlg );
        vcl::Window* pDocWin = pSrchDlg->GetDocWin();
        if ( !pDocWin )
        {
            return;
        }
        Reference < css::accessibility::XAccessible > xDocAcc = pDocWin->GetAccessible();
        if ( !xDocAcc.is() )
        {
            return;
        }
        Reference< css::accessibility::XAccessibleGetAccFlowTo > xGetAccFlowTo( xDocAcc, UNO_QUERY );
        if ( !xGetAccFlowTo.is() )
        {
            return;
        }

        const sal_Int32 FORFINDREPLACEFLOWTO = 2;
        uno::Sequence<uno::Any> aAnySeq = xGetAccFlowTo->getAccFlowTo( Any(pSrchDlg->GetSrchFlag()),  FORFINDREPLACEFLOWTO );

        sal_Int32 nLen = aAnySeq.getLength();
        if ( nLen )
        {
            uno::Sequence< uno::Reference< uno::XInterface > > aSequence( nLen );
            for ( sal_Int32 i = 0; i < nLen; i++ )
            {
                uno::Reference < css::accessibility::XAccessible > xAcc;
                aAnySeq[i] >>= xAcc;
                aSequence[i] = xAcc;
            }
            rRelationSet.AddRelation( css::accessibility::AccessibleRelation( css::accessibility::AccessibleRelationType::CONTENT_FLOWS_TO, aSequence ) );
        }
    }
}

// XServiceInfo


OUString VCLXAccessibleSvxFindReplaceDialog::getImplementationName() throw (RuntimeException, std::exception)
{
    return OUString( "VCLXAccessibleSvxFindReplaceDialog" );
}


Sequence< OUString > VCLXAccessibleSvxFindReplaceDialog::getSupportedServiceNames() throw (RuntimeException, std::exception)
{
    Sequence< OUString > aNames { "VCLXAccessibleSvxFindReplaceDialog" };
    return aNames;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
