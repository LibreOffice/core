/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

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
    sal_Bool                bOrient, bAttrib;

    if( xIStm.Is() )
    {
        SdIOCompat aCompat( *xIStm, STREAM_READ );

        *xIStm >> nSteps >> bOrient >> bAttrib;
    }
    else
    {
        nSteps = 16;
        bOrient = bAttrib = sal_True;
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
               << aCbxOrientation.IsChecked()
               << aCbxAttributes.IsChecked();
    }
}

} // end of namespace sd
