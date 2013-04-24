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
#include <vcl/layout.hxx>

#include "strings.hrc"
#include "sdattr.hxx"
#include "sdresid.hxx"
#include "layeroptionsdlg.hxx"

SdInsertLayerDlg::SdInsertLayerDlg( Window* pWindow, const SfxItemSet& rInAttrs,
    bool bDeletable, const OUString& rStr )
    : ModalDialog(pWindow, "InsertLayerDialog", "modules/sdraw/ui/insertlayer.ui")
    , mrOutAttrs(rInAttrs)
{
    SetText(rStr);

    get(m_pEdtName, "name");
    get(m_pEdtTitle, "title");
    get(m_pEdtDesc, "textview");
    get(m_pCbxVisible, "visible");
    get(m_pCbxPrintable, "printable");
    get(m_pCbxLocked, "locked");

    m_pEdtName->SetText( ( ( const SdAttrLayerName& ) mrOutAttrs.Get( ATTR_LAYER_NAME ) ).GetValue() );
    m_pEdtTitle->SetText( ( ( const SdAttrLayerTitle& ) mrOutAttrs.Get( ATTR_LAYER_TITLE ) ).GetValue() );
    m_pEdtDesc->SetText( ( ( const SdAttrLayerDesc& ) mrOutAttrs.Get( ATTR_LAYER_DESC ) ).GetValue() );
    m_pEdtDesc->set_height_request(4 * m_pEdtDesc->GetTextHeight());
    m_pCbxVisible->Check( ( ( const SdAttrLayerVisible& ) mrOutAttrs.Get( ATTR_LAYER_VISIBLE ) ).GetValue() );
    m_pCbxPrintable->Check( ( ( const SdAttrLayerPrintable& ) mrOutAttrs.Get( ATTR_LAYER_PRINTABLE ) ).GetValue() );
    m_pCbxLocked->Check( ( ( const SdAttrLayerLocked& ) mrOutAttrs.Get( ATTR_LAYER_LOCKED ) ).GetValue() );

    get<VclContainer>("nameframe")->Enable(bDeletable);
}

void SdInsertLayerDlg::GetAttr( SfxItemSet& rAttrs )
{
    rAttrs.Put( SdAttrLayerName( m_pEdtName->GetText() ) );
    rAttrs.Put( SdAttrLayerTitle( m_pEdtTitle->GetText() ) );
    rAttrs.Put( SdAttrLayerDesc( m_pEdtDesc->GetText() ) );
    rAttrs.Put( SdAttrLayerVisible( m_pCbxVisible->IsChecked() ) );
    rAttrs.Put( SdAttrLayerPrintable( m_pCbxPrintable->IsChecked() ) );
    rAttrs.Put( SdAttrLayerLocked( m_pCbxLocked->IsChecked() ) );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
