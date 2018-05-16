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

#include <svl/itemset.hxx>

#include <sdattr.hxx>
#include <layeroptionsdlg.hxx>

SdInsertLayerDlg::SdInsertLayerDlg(weld::Window* pWindow, const SfxItemSet& rInAttrs,
    bool bDeletable, const OUString& rStr)
    : GenericDialogController(pWindow, "modules/sdraw/ui/insertlayer.ui", "InsertLayerDialog")
    , m_xEdtName(m_xBuilder->weld_entry("name"))
    , m_xEdtTitle(m_xBuilder->weld_entry("title"))
    , m_xEdtDesc(m_xBuilder->weld_text_view("textview"))
    , m_xCbxVisible(m_xBuilder->weld_check_button("visible"))
    , m_xCbxPrintable(m_xBuilder->weld_check_button("printable"))
    , m_xCbxLocked(m_xBuilder->weld_check_button("locked"))
    , m_xNameFrame(m_xBuilder->weld_widget("nameframe"))
{
    m_xDialog->set_title(rStr);

    m_xEdtName->set_text( static_cast<const SfxStringItem&>( rInAttrs.Get( ATTR_LAYER_NAME ) ).GetValue() );
    m_xEdtTitle->set_text( static_cast<const SfxStringItem&>( rInAttrs.Get( ATTR_LAYER_TITLE ) ).GetValue() );
    m_xEdtDesc->set_text( static_cast<const SfxStringItem&>( rInAttrs.Get( ATTR_LAYER_DESC ) ).GetValue() );
    m_xEdtDesc->set_size_request(-1, m_xEdtDesc->get_height_rows(4));
    m_xCbxVisible->set_active( static_cast<const SfxBoolItem&>( rInAttrs.Get( ATTR_LAYER_VISIBLE ) ).GetValue() );
    m_xCbxPrintable->set_active( static_cast<const SfxBoolItem&>( rInAttrs.Get( ATTR_LAYER_PRINTABLE ) ).GetValue() );
    m_xCbxLocked->set_active( static_cast<const SfxBoolItem&>( rInAttrs.Get( ATTR_LAYER_LOCKED ) ).GetValue() );
    m_xNameFrame->set_sensitive(bDeletable);
}

SdInsertLayerDlg::~SdInsertLayerDlg()
{
}

void SdInsertLayerDlg::GetAttr( SfxItemSet& rAttrs )
{
    rAttrs.Put( makeSdAttrLayerName( m_xEdtName->get_text() ) );
    rAttrs.Put( makeSdAttrLayerTitle( m_xEdtTitle->get_text() ) );
    rAttrs.Put( makeSdAttrLayerDesc( m_xEdtDesc->get_text() ) );
    rAttrs.Put( makeSdAttrLayerVisible( m_xCbxVisible->get_active() ) );
    rAttrs.Put( makeSdAttrLayerPrintable( m_xCbxPrintable->get_active() ) );
    rAttrs.Put( makeSdAttrLayerLocked( m_xCbxLocked->get_active() ) );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
