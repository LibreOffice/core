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

#include <morphdlg.hxx>

#include <sdmod.hxx>
#include <sdiocmpt.hxx>
#include <svx/xfillit0.hxx>
#include <svx/xlineit0.hxx>
#include <svx/svdobj.hxx>
#include <svl/itemset.hxx>
#include <com/sun/star/drawing/LineStyle.hpp>

using namespace com::sun::star;

namespace sd {

MorphDlg::MorphDlg(weld::Window* pParent, const SdrObject* pObj1, const SdrObject* pObj2 )
    : GenericDialogController(pParent, "modules/sdraw/ui/crossfadedialog.ui", "CrossFadeDialog")
    , m_xMtfSteps(m_xBuilder->weld_spin_button("increments"))
    , m_xCbxAttributes(m_xBuilder->weld_check_button("attributes"))
    , m_xCbxOrientation(m_xBuilder->weld_check_button("orientation"))
{
    LoadSettings();

    SfxItemPool &   rPool = pObj1->GetObjectItemPool();
    SfxItemSet      aSet1( rPool );
    SfxItemSet      aSet2( rPool );

    aSet1.Put(pObj1->GetMergedItemSet());
    aSet2.Put(pObj2->GetMergedItemSet());

    const drawing::LineStyle eLineStyle1 = aSet1.Get( XATTR_LINESTYLE ).GetValue();
    const drawing::LineStyle eLineStyle2 = aSet2.Get( XATTR_LINESTYLE ).GetValue();
    const drawing::FillStyle eFillStyle1 = aSet1.Get( XATTR_FILLSTYLE ).GetValue();
    const drawing::FillStyle eFillStyle2 = aSet2.Get( XATTR_FILLSTYLE ).GetValue();

    if ( ( ( eLineStyle1 == drawing::LineStyle_NONE ) || ( eLineStyle2 == drawing::LineStyle_NONE ) ) &&
         ( ( eFillStyle1 != drawing::FillStyle_SOLID ) || ( eFillStyle2 != drawing::FillStyle_SOLID ) ) )
    {
        m_xCbxAttributes->set_sensitive(false);
    }
}

MorphDlg::~MorphDlg()
{
}

void MorphDlg::LoadSettings()
{
    tools::SvRef<SotStorageStream>  xIStm( SD_MOD()->GetOptionStream( SD_OPTION_MORPHING ,
                               SD_OPTION_LOAD ) );
    sal_uInt16              nSteps;
    bool                bOrient, bAttrib;

    if( xIStm.is() )
    {
        SdIOCompat aCompat( *xIStm, StreamMode::READ );

        xIStm->ReadUInt16( nSteps ).ReadCharAsBool( bOrient ).ReadCharAsBool( bAttrib );
    }
    else
    {
        nSteps = 16;
        bOrient = bAttrib = true;
    }

    m_xMtfSteps->set_value(nSteps);
    m_xCbxOrientation->set_active(bOrient);
    m_xCbxAttributes->set_active(bAttrib);
}

void MorphDlg::SaveSettings() const
{
    tools::SvRef<SotStorageStream> xOStm( SD_MOD()->GetOptionStream( SD_OPTION_MORPHING ,
                               SD_OPTION_STORE ) );

    if( xOStm.is() )
    {
        SdIOCompat aCompat( *xOStm, StreamMode::WRITE, 1 );

        xOStm->WriteUInt16( m_xMtfSteps->get_value() )
              .WriteBool( m_xCbxOrientation->get_active() )
              .WriteBool( m_xCbxAttributes->get_active() );
    }
}

} // end of namespace sd

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
