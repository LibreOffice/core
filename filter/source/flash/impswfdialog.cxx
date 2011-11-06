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
#include "precompiled_filter.hxx"

#include "impswfdialog.hxx"
#include "impswfdialog.hrc"

using namespace rtl;
using namespace com::sun::star::uno;
using namespace com::sun::star::beans;

// ----------------
// - ImpPDFDialog -
// ----------------

ImpSWFDialog::ImpSWFDialog( Window* pParent, ResMgr& rResMgr, Sequence< PropertyValue >& rFilterData ) :
    ModalDialog( pParent, ResId( DLG_OPTIONS, rResMgr ) ),
    maFiDescr( this, ResId( FI_DESCR, rResMgr ) ),
    maNumFldQuality( this, ResId( NUM_FLD_QUALITY, rResMgr ) ),
    maFiExportAllDescr( this, ResId( FI_EXPORT_ALL_DESCR, rResMgr ) ),
    maCheckExportAll( this, ResId( BOOL_EXPORT_ALL, rResMgr ) ),
    maFiExportBackgroundsDescr( this, ResId( FI_EXPORT_BACKGROUNDS_DESCR, rResMgr ) ),
    maCheckExportBackgrounds( this, ResId( BOOL_EXPORT_BACKGROUNDS, rResMgr ) ),
    maFiExportBackgroundObjectsDescr( this, ResId( FI_EXPORT_BACKGROUND_OBJECTS_DESCR, rResMgr ) ),
    maCheckExportBackgroundObjects( this, ResId( BOOL_EXPORT_BACKGROUND_OBJECTS, rResMgr ) ),
    maFiExportSlideContentsDescr( this, ResId( FI_EXPORT_SLIDE_CONTENTS_DESCR, rResMgr ) ),
    maCheckExportSlideContents( this, ResId( BOOL_EXPORT_SLIDE_CONTENTS, rResMgr ) ),
    maFiExportSoundDescr( this, ResId( FI_EXPORT_SOUND_DESCR, rResMgr ) ),
    maCheckExportSound( this, ResId( BOOL_EXPORT_SOUND, rResMgr ) ),
    maFiExportOLEAsJPEGDescr( this, ResId( FI_EXPORT_OLE_AS_JPEG_DESCR, rResMgr ) ),
    maCheckExportOLEAsJPEG( this, ResId( BOOL_EXPORT_OLE_AS_JPEG, rResMgr ) ),
    maFiExportMultipleFilesDescr( this, ResId( FI_EXPORT_MULTIPLE_FILES_DESCR, rResMgr ) ),
    maCheckExportMultipleFiles( this, ResId( BOOL_EXPORT_MULTIPLE_FILES, rResMgr ) ),

    maBtnOK( this, ResId( BTN_OK, rResMgr ) ),
    maBtnCancel( this, ResId( BTN_CANCEL, rResMgr ) ),
    maBtnHelp( this, ResId( BTN_HELP, rResMgr ) ),
    maConfigItem( String( RTL_CONSTASCII_USTRINGPARAM( "Office.Common/Filter/Flash/Export/" ) ), &rFilterData )
{
    const sal_uLong nCompressMode = maConfigItem.ReadInt32( String( RTL_CONSTASCII_USTRINGPARAM( "CompressMode" ) ), 75 );
    maNumFldQuality.SetValue( nCompressMode );

    maCheckExportAll.Check();
    maCheckExportSlideContents.Check();
    maCheckExportSound.Check();

    maCheckExportAll.SetToggleHdl( LINK( this, ImpSWFDialog, OnToggleCheckbox ) );

    maCheckExportBackgrounds.Disable(); maFiExportBackgroundsDescr.Disable();
    maCheckExportBackgroundObjects.Disable(); maFiExportBackgroundObjectsDescr.Disable();
    maCheckExportSlideContents.Disable(); maFiExportSlideContentsDescr.Disable();

#ifdef AUGUSTUS
    maCheckExportMultipleFiles.Check();
#endif

    FreeResource();
}

// -----------------------------------------------------------------------------

ImpSWFDialog::~ImpSWFDialog()
{
}

// -----------------------------------------------------------------------------

Sequence< PropertyValue > ImpSWFDialog::GetFilterData()
{
    sal_Int32 nCompressMode = (sal_Int32)maNumFldQuality.GetValue();
    maConfigItem.WriteInt32( OUString( RTL_CONSTASCII_USTRINGPARAM( "CompressMode" ) ), nCompressMode );
    maConfigItem.WriteBool( OUString( RTL_CONSTASCII_USTRINGPARAM( "ExportAll" ) ), maCheckExportAll.IsChecked() );
    maConfigItem.WriteBool( OUString( RTL_CONSTASCII_USTRINGPARAM( "ExportBackgrounds" ) ), maCheckExportBackgrounds.IsChecked() );
    maConfigItem.WriteBool( OUString( RTL_CONSTASCII_USTRINGPARAM( "ExportBackgroundObjects" ) ), maCheckExportBackgroundObjects.IsChecked() );
    maConfigItem.WriteBool( OUString( RTL_CONSTASCII_USTRINGPARAM( "ExportSlideContents" ) ), maCheckExportSlideContents.IsChecked() );
    maConfigItem.WriteBool( OUString( RTL_CONSTASCII_USTRINGPARAM( "ExportSound" ) ), maCheckExportSound.IsChecked() );
    maConfigItem.WriteBool( OUString( RTL_CONSTASCII_USTRINGPARAM( "ExportOLEAsJPEG" ) ), maCheckExportOLEAsJPEG.IsChecked() );
    maConfigItem.WriteBool( OUString( RTL_CONSTASCII_USTRINGPARAM( "ExportMultipleFiles" ) ), maCheckExportMultipleFiles.IsChecked() );

    Sequence< PropertyValue > aRet( maConfigItem.GetFilterData() );

    return aRet;
}

// AS: This is called whenever the user toggles one of the checkboxes
IMPL_LINK( ImpSWFDialog, OnToggleCheckbox, CheckBox*, pBox )
{
    if (pBox == &maCheckExportAll)
    {
        maCheckExportBackgrounds.Enable(!maCheckExportBackgrounds.IsEnabled());
        maFiExportBackgroundsDescr.Enable(!maFiExportBackgroundsDescr.IsEnabled());
        maCheckExportBackgroundObjects.Enable(!maCheckExportBackgroundObjects.IsEnabled());
        maFiExportBackgroundObjectsDescr.Enable(!maFiExportBackgroundObjectsDescr.IsEnabled());
        maCheckExportSlideContents.Enable(!maCheckExportSlideContents.IsEnabled());
        maFiExportSlideContentsDescr.Enable(!maFiExportSlideContentsDescr.IsEnabled());
    }

    return 0;
}
