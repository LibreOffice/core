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

#include "morphdlg.hxx"

#include "strings.hrc"
#include "sdresid.hxx"
#include "sdmod.hxx"
#include "sdiocmpt.hxx"
#include <svx/xfillit0.hxx>
#include <svx/xlineit0.hxx>
#include <svx/xenum.hxx>
#include <svx/svdobj.hxx>
#include <svl/itemset.hxx>
#include <svl/itempool.hxx>

namespace sd {

MorphDlg::MorphDlg( ::Window* pParent, const SdrObject* pObj1, const SdrObject* pObj2 )
    : ModalDialog(pParent, "CrossFadeDialog",
        "modules/sdraw/ui/crossfadedialog.ui")
{
    get(m_pMtfSteps, "increments");
    get(m_pCbxAttributes, "attributes");
    get(m_pCbxOrientation, "orientation");

    LoadSettings();

    SfxItemPool*    pPool = (SfxItemPool*) pObj1->GetObjectItemPool();
    SfxItemSet      aSet1( *pPool );
    SfxItemSet      aSet2( *pPool );

    aSet1.Put(pObj1->GetMergedItemSet());
    aSet2.Put(pObj2->GetMergedItemSet());

    const XLineStyle    eLineStyle1 = ( (const XLineStyleItem&) aSet1.Get( XATTR_LINESTYLE ) ).GetValue();
    const XLineStyle    eLineStyle2 = ( (const XLineStyleItem&) aSet2.Get( XATTR_LINESTYLE ) ).GetValue();
    const XFillStyle    eFillStyle1 = ( (const XFillStyleItem&) aSet1.Get( XATTR_FILLSTYLE ) ).GetValue();
    const XFillStyle    eFillStyle2 = ( (const XFillStyleItem&) aSet2.Get( XATTR_FILLSTYLE ) ).GetValue();

    if ( ( ( eLineStyle1 == XLINE_NONE ) || ( eLineStyle2 == XLINE_NONE ) ) &&
         ( ( eFillStyle1 != XFILL_SOLID ) || ( eFillStyle2 != XFILL_SOLID ) ) )
    {
        m_pCbxAttributes->Disable();
    }
}

MorphDlg::~MorphDlg()
{
}

void MorphDlg::LoadSettings()
{
    SvStorageStreamRef  xIStm( SD_MOD()->GetOptionStream( OUString(SD_OPTION_MORPHING) ,
                               SD_OPTION_LOAD ) );
    sal_uInt16              nSteps;
    sal_Bool                bOrient, bAttrib;

    if( xIStm.Is() )
    {
        SdIOCompat aCompat( *xIStm, STREAM_READ );

        xIStm->ReadUInt16( nSteps ).ReadUChar( bOrient ).ReadUChar( bAttrib );
    }
    else
    {
        nSteps = 16;
        bOrient = bAttrib = sal_True;
    }

    m_pMtfSteps->SetValue( nSteps );
    m_pCbxOrientation->Check( bOrient );
    m_pCbxAttributes->Check( bAttrib );
}



void MorphDlg::SaveSettings() const
{
    SvStorageStreamRef xOStm( SD_MOD()->GetOptionStream( OUString(SD_OPTION_MORPHING) ,
                               SD_OPTION_STORE ) );

    if( xOStm.Is() )
    {
        SdIOCompat aCompat( *xOStm, STREAM_WRITE, 1 );

        xOStm->WriteUInt16( (sal_uInt16) m_pMtfSteps->GetValue() )
              .WriteUChar( m_pCbxOrientation->IsChecked() )
              .WriteUChar( m_pCbxAttributes->IsChecked() );
    }
}

} // end of namespace sd

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
