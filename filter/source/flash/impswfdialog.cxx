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


#include "impswfdialog.hxx"

using namespace com::sun::star::uno;
using namespace com::sun::star::beans;



// - ImpPDFDialog -


ImpSWFDialog::ImpSWFDialog( vcl::Window* pParent, Sequence< PropertyValue >& rFilterData ) :
    ModalDialog( pParent, "ImpSWFDialog", "filter/ui/impswfdialog.ui" ),

    maConfigItem( "Office.Common/Filter/Flash/Export/", &rFilterData )
{
    get(mpNumFldQuality,"quality");
    get(mpCheckExportAll,"exportall");
    get(mpCheckExportMultipleFiles,"exportmultiplefiles");
    get(mpCheckExportBackgrounds,"exportbackgrounds");
    get(mpCheckExportBackgroundObjects,"exportbackgroundobjects");
    get(mpCheckExportSlideContents,"exportslidecontents");
    get(mpCheckExportSound,"exportsound");
    get(mpCheckExportOLEAsJPEG,"exportoleasjpeg");

    const sal_uLong nCompressMode = maConfigItem.ReadInt32( "CompressMode", 75 );
    mpNumFldQuality->SetValue( nCompressMode );

    mpCheckExportAll->Check();
    mpCheckExportSlideContents->Check();
    mpCheckExportSound->Check();

    mpCheckExportAll->SetToggleHdl( LINK( this, ImpSWFDialog, OnToggleCheckbox ) );

    mpCheckExportBackgrounds->Disable();
    mpCheckExportBackgroundObjects->Disable();
    mpCheckExportSlideContents->Disable();
}



ImpSWFDialog::~ImpSWFDialog()
{
    disposeOnce();
}

void ImpSWFDialog::dispose()
{
    mpNumFldQuality.clear();
    mpCheckExportAll.clear();
    mpCheckExportBackgrounds.clear();
    mpCheckExportBackgroundObjects.clear();
    mpCheckExportSlideContents.clear();
    mpCheckExportSound.clear();
    mpCheckExportOLEAsJPEG.clear();
    mpCheckExportMultipleFiles.clear();
    maConfigItem.WriteModifiedConfig();
    ModalDialog::dispose();
}


Sequence< PropertyValue > ImpSWFDialog::GetFilterData()
{
    sal_Int32 nCompressMode = (sal_Int32)mpNumFldQuality->GetValue();
    maConfigItem.WriteInt32( "CompressMode" , nCompressMode );
    maConfigItem.WriteBool( "ExportAll", mpCheckExportAll->IsChecked() );
    maConfigItem.WriteBool( "ExportBackgrounds", mpCheckExportBackgrounds->IsChecked() );
    maConfigItem.WriteBool( "ExportBackgroundObjects", mpCheckExportBackgroundObjects->IsChecked() );
    maConfigItem.WriteBool( "ExportSlideContents", mpCheckExportSlideContents->IsChecked() );
    maConfigItem.WriteBool( "ExportSound", mpCheckExportSound->IsChecked() );
    maConfigItem.WriteBool( "ExportOLEAsJPEG", mpCheckExportOLEAsJPEG->IsChecked() );
    maConfigItem.WriteBool( "ExportMultipleFiles", mpCheckExportMultipleFiles->IsChecked() );

    Sequence< PropertyValue > aRet( maConfigItem.GetFilterData() );

    return aRet;
}

// AS: This is called whenever the user toggles one of the checkboxes
IMPL_LINK_TYPED( ImpSWFDialog, OnToggleCheckbox, CheckBox&, rBox, void )
{
    if (&rBox == mpCheckExportAll)
    {
        mpCheckExportBackgrounds->Enable(!mpCheckExportBackgrounds->IsEnabled());
        mpCheckExportBackgroundObjects->Enable(!mpCheckExportBackgroundObjects->IsEnabled());
        mpCheckExportSlideContents->Enable(!mpCheckExportSlideContents->IsEnabled());
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
