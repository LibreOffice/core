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

#undef SC_DLLIMPLEMENTATION

#include "mtrindlg.hxx"
#include "scresid.hxx"
#include "miscdlgs.hrc"

ScMetricInputDlg::ScMetricInputDlg( vcl::Window*         pParent,
                                    const OString&  sDialogName,
                                    long            nCurrent,
                                    long            nDefault,
                                    FieldUnit       eFUnit,
                                    sal_uInt16      nDecimals,
                                    long            nMaximum,
                                    long            nMinimum,
                                    long            nFirst,
                                    long            nLast )

    :   ModalDialog(pParent, OStringToOUString(sDialogName, RTL_TEXTENCODING_UTF8),
            OStringToOUString("modules/scalc/ui/" +
                sDialogName.toAsciiLowerCase() + ".ui", RTL_TEXTENCODING_UTF8))
{
    get(m_pEdValue, "value");
    get(m_pBtnDefVal, "default");

    m_pBtnDefVal->SetClickHdl ( LINK( this, ScMetricInputDlg, SetDefValHdl ) );
    m_pEdValue->SetModifyHdl( LINK( this, ScMetricInputDlg, ModifyHdl    ) );

    m_pEdValue->SetUnit            ( eFUnit );
    m_pEdValue->SetDecimalDigits   ( nDecimals );
    m_pEdValue->SetMax             ( m_pEdValue->Normalize( nMaximum ), FUNIT_TWIP );
    m_pEdValue->SetMin             ( m_pEdValue->Normalize( nMinimum ), FUNIT_TWIP );
    m_pEdValue->SetLast            ( m_pEdValue->Normalize( nLast ),    FUNIT_TWIP );
    m_pEdValue->SetFirst           ( m_pEdValue->Normalize( nFirst ),   FUNIT_TWIP );
    m_pEdValue->SetSpinSize        ( m_pEdValue->Normalize( 1 ) / 10 );
    m_pEdValue->SetValue           ( m_pEdValue->Normalize( nDefault ), FUNIT_TWIP );
    nDefaultValue = sal::static_int_cast<long>( m_pEdValue->GetValue() );
    m_pEdValue->SetValue           ( m_pEdValue->Normalize( nCurrent ), FUNIT_TWIP );
    nCurrentValue = sal::static_int_cast<long>( m_pEdValue->GetValue() );
    m_pBtnDefVal->Check( nCurrentValue == nDefaultValue );
}

ScMetricInputDlg::~ScMetricInputDlg()
{
    disposeOnce();
}

void ScMetricInputDlg::dispose()
{
    m_pEdValue.clear();
    m_pBtnDefVal.clear();
    ModalDialog::dispose();
}

long ScMetricInputDlg::GetInputValue( FieldUnit eUnit ) const
{
/*
    with decimal digits

    double  nVal    = m_pEdValue->GetValue( eUnit );
    sal_uInt16  nDecs   = m_pEdValue->GetDecimalDigits();
    double  nFactor = 0.0;

    // static long ImpPower10( sal_uInt16 nDecs )
    {
        nFactor = 1.0;

        for ( sal_uInt16 i=0; i < nDecs; i++ )
            nFactor *= 10.0;
    }

    return nVal / nFactor;
*/
    // first cut off the decimal digits - not that great...

    return sal::static_int_cast<long>( m_pEdValue->Denormalize( m_pEdValue->GetValue( eUnit ) ) );
}

// Handler:

IMPL_LINK_NOARG_TYPED(ScMetricInputDlg, SetDefValHdl, Button*, void)
{
    if ( m_pBtnDefVal->IsChecked() )
    {
        nCurrentValue = sal::static_int_cast<long>( m_pEdValue->GetValue() );
        m_pEdValue->SetValue( nDefaultValue );
    }
    else
        m_pEdValue->SetValue( nCurrentValue );
}

IMPL_LINK_NOARG_TYPED(ScMetricInputDlg, ModifyHdl, Edit&, void)
{
    m_pBtnDefVal->Check( nDefaultValue == m_pEdValue->GetValue() );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
