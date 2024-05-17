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

#include <svx/xdef.hxx>
#include <svx/xfillit0.hxx>
#include <svx/xlineit0.hxx>
#include <svx/svdobj.hxx>
#include <svl/itemset.hxx>
#include <com/sun/star/drawing/LineStyle.hpp>
#include <officecfg/Office/Draw.hxx>

using namespace com::sun::star;

namespace sd {

MorphDlg::MorphDlg(weld::Window* pParent, const SdrObject* pObj1, const SdrObject* pObj2 )
    : GenericDialogController(pParent, u"modules/sdraw/ui/crossfadedialog.ui"_ustr, u"CrossFadeDialog"_ustr)
    , m_xMtfSteps(m_xBuilder->weld_spin_button(u"increments"_ustr))
    , m_xCbxAttributes(m_xBuilder->weld_check_button(u"attributes"_ustr))
    , m_xCbxOrientation(m_xBuilder->weld_check_button(u"orientation"_ustr))
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
    SaveSettings();
}

void MorphDlg::LoadSettings()
{
    m_xMtfSteps->set_value(officecfg::Office::Draw::Misc::CrossFading::Steps::get());
    m_xCbxOrientation->set_active(officecfg::Office::Draw::Misc::CrossFading::Orientation::get());
    m_xCbxAttributes->set_active(officecfg::Office::Draw::Misc::CrossFading::Attributes::get());
}

void MorphDlg::SaveSettings() const
{
    std::shared_ptr<comphelper::ConfigurationChanges> batch(
        comphelper::ConfigurationChanges::create());
    officecfg::Office::Draw::Misc::CrossFading::Steps::set(m_xMtfSteps->get_value(),batch);
    officecfg::Office::Draw::Misc::CrossFading::Orientation::set(m_xCbxOrientation->get_active(),batch);
    officecfg::Office::Draw::Misc::CrossFading::Attributes::set(m_xCbxAttributes->get_active(),batch);
    batch->commit();
}

} // end of namespace sd

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
