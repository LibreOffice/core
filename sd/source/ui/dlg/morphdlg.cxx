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
#include "precompiled_sd.hxx"

#ifdef SD_DLLIMPLEMENTATION
#undef SD_DLLIMPLEMENTATION
#endif

#include "morphdlg.hxx"

#include "strings.hrc"
#include "sdresid.hxx"
#include "sdmod.hxx"
#include "sdiocmpt.hxx"
#include "morphdlg.hrc"
#include <tools/config.hxx>
#include <svx/xfillit0.hxx>
#include <svx/xlineit0.hxx>
#include <svx/xenum.hxx>
#include <svx/svdobj.hxx>
#include <svl/itemset.hxx>
#include <svl/itempool.hxx>

namespace sd {


/******************************************************************************/


#define FADE_STEP   "FadeSteps"
#define FADE_ATTRIB "FadeAttributes"
#define FADE_ORIENT "FadeOrientation"
#define FADE_TRUE   "true"
#define FADE_FALSE  "false"


/******************************************************************************/


/******************************************************************************
|*
|*
|*
\******************************************************************************/

MorphDlg::MorphDlg( ::Window* pParent, const SdrObject* pObj1, const SdrObject* pObj2 ) :
            ModalDialog     ( pParent, SdResId( DLG_MORPH ) ),
            aGrpPreset      ( this, SdResId( GRP_PRESET ) ),
            aFtSteps        ( this, SdResId( FT_STEPS ) ),
            aMtfSteps       ( this, SdResId( MTF_STEPS ) ),
            aCbxAttributes  ( this, SdResId( CBX_ATTRIBUTES ) ),
            aCbxOrientation ( this, SdResId( CBX_ORIENTATION ) ),
            aBtnOK          ( this, SdResId( BTN_OK ) ),
            aBtnCancel      ( this, SdResId( BTN_CANCEL ) ),
            aBtnHelp        ( this, SdResId( BTN_HELP ) )
{
    FreeResource();
    LoadSettings();

    SfxItemSet aSet1( pObj1->GetMergedItemSet() );
    SfxItemSet aSet2( pObj2->GetMergedItemSet() );

    const XLineStyle    eLineStyle1 = ( (const XLineStyleItem&) aSet1.Get( XATTR_LINESTYLE ) ).GetValue();
    const XLineStyle    eLineStyle2 = ( (const XLineStyleItem&) aSet2.Get( XATTR_LINESTYLE ) ).GetValue();
    const XFillStyle    eFillStyle1 = ( (const XFillStyleItem&) aSet1.Get( XATTR_FILLSTYLE ) ).GetValue();
    const XFillStyle    eFillStyle2 = ( (const XFillStyleItem&) aSet2.Get( XATTR_FILLSTYLE ) ).GetValue();

    if ( ( ( eLineStyle1 == XLINE_NONE ) || ( eLineStyle2 == XLINE_NONE ) ) &&
         ( ( eFillStyle1 != XFILL_SOLID ) || ( eFillStyle2 != XFILL_SOLID ) ) )
    {
        aCbxAttributes.Disable();
    }
}


/******************************************************************************
|*
|*
|*
\******************************************************************************/

MorphDlg::~MorphDlg()
{
}


/******************************************************************************
|*
|*
|*
\******************************************************************************/

void MorphDlg::LoadSettings()
{
    SvStorageStreamRef  xIStm( SD_MOD()->GetOptionStream( UniString::CreateFromAscii(
                               RTL_CONSTASCII_STRINGPARAM( SD_OPTION_MORPHING ) ),
                               SD_OPTION_LOAD ) );
    sal_uInt16              nSteps;
    bool                bOrient, bAttrib;

    if( xIStm.Is() )
    {
        SdIOCompat aCompat( *xIStm, STREAM_READ );
        sal_Bool bTempBOOL;

        *xIStm >> nSteps;
        *xIStm >> bTempBOOL; bOrient = bTempBOOL;
        *xIStm >> bTempBOOL; bAttrib = bTempBOOL;
    }
    else
    {
        nSteps = 16;
        bOrient = bAttrib = true;
    }

    aMtfSteps.SetValue( nSteps );
    aCbxOrientation.Check( bOrient );
    aCbxAttributes.Check( bAttrib );
}

// -----------------------------------------------------------------------------

void MorphDlg::SaveSettings() const
{
    SvStorageStreamRef xOStm( SD_MOD()->GetOptionStream( UniString::CreateFromAscii(
                               RTL_CONSTASCII_STRINGPARAM( SD_OPTION_MORPHING ) ),
                               SD_OPTION_STORE ) );

    if( xOStm.Is() )
    {
        SdIOCompat aCompat( *xOStm, STREAM_WRITE, 1 );

        *xOStm << (sal_uInt16) aMtfSteps.GetValue()
               << (sal_Bool)aCbxOrientation.IsChecked()
               << (sal_Bool)aCbxAttributes.IsChecked();
    }
}

} // end of namespace sd
