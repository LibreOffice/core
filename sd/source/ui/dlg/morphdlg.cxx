/*************************************************************************
 *
 *  $RCSfile: morphdlg.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: ka $ $Date: 2000-11-10 16:48:33 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/


#include "strings.hrc"
#include "sdresid.hxx"
#include "sdmod.hxx"
#include "sdiocmpt.hxx"
#include "morphdlg.hxx"
#include "morphdlg.hrc"

#ifndef _SV_CONFIG_HXX //autogen
#include <vcl/config.hxx>
#endif
#ifndef SVX_XFILLIT0_HXX //autogen
#include <svx/xfillit0.hxx>
#endif
#ifndef _SVX_XLINEIT0_HXX //autogen
#include <svx/xlineit0.hxx>
#endif
#ifndef _XENUM_HXX //autogen
#include <svx/xenum.hxx>
#endif
#ifndef _SVDOBJ_HXX //autogen
#include <svx/svdobj.hxx>
#endif
#ifndef _SFXITEMSET_HXX //autogen
#include <svtools/itemset.hxx>
#endif
#ifndef _SFXITEMPOOL_HXX //autogen
#include <svtools/itempool.hxx>
#endif


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

SdMorphDlg::SdMorphDlg( Window* pParent, const SdrObject* pObj1, const SdrObject* pObj2 ) :
            ModalDialog     ( pParent, SdResId( DLG_MORPH ) ),
            aBtnOK          ( this, SdResId( BTN_OK ) ),
            aBtnCancel      ( this, SdResId( BTN_CANCEL ) ),
            aBtnHelp        ( this, SdResId( BTN_HELP ) ),
            aGrpPreset      ( this, SdResId( GRP_PRESET ) ),
            aFtSteps        ( this, SdResId( FT_STEPS ) ),
            aMtfSteps       ( this, SdResId( MTF_STEPS ) ),
            aCbxAttributes  ( this, SdResId( CBX_ATTRIBUTES ) ),
            aCbxOrientation ( this, SdResId( CBX_ORIENTATION ) )
{
    FreeResource();
    LoadSettings();

    SfxItemPool*    pPool = (SfxItemPool*) pObj1->GetItemPool();
    SfxItemSet      aSet1( *pPool );
    SfxItemSet      aSet2( *pPool );

//-/    pObj1->TakeAttributes( aSet1, TRUE, FALSE );
    aSet1.Put(pObj1->GetItemSet());

//-/    pObj2->TakeAttributes( aSet2, TRUE, FALSE );
    aSet2.Put(pObj2->GetItemSet());

    const XLineStyle    eLineStyle1 = ( (const XLineStyleItem&) aSet1.Get( XATTR_LINESTYLE ) ).GetValue();
    const XLineStyle    eLineStyle2 = ( (const XLineStyleItem&) aSet2.Get( XATTR_LINESTYLE ) ).GetValue();
    const XFillStyle    eFillStyle1 = ( (const XFillStyleItem&) aSet1.Get( XATTR_FILLSTYLE ) ).GetValue();
    const XFillStyle    eFillStyle2 = ( (const XFillStyleItem&) aSet2.Get( XATTR_FILLSTYLE ) ).GetValue();

    if ( ( ( eLineStyle1 == XLINE_NONE ) || ( eLineStyle2 == XLINE_NONE ) ) &&
         ( ( eFillStyle1 != XFILL_SOLID ) || ( eFillStyle2 == XFILL_SOLID ) ) )
    {
        aCbxAttributes.Disable();
    }
}


/******************************************************************************
|*
|*
|*
\******************************************************************************/

SdMorphDlg::~SdMorphDlg()
{
}


/******************************************************************************
|*
|*
|*
\******************************************************************************/

void SdMorphDlg::LoadSettings()
{
    SvStorageStreamRef  xIStm( SD_MOD()->GetOptionStream( UniString::CreateFromAscii(
                               RTL_CONSTASCII_STRINGPARAM( SD_OPTION_MORPHING ) ),
                               SD_OPTION_LOAD ) );
    UINT16              nSteps;
    BOOL                bOrient, bAttrib;

    if( xIStm.Is() )
    {
        SdIOCompat aCompat( *xIStm, STREAM_READ );

        *xIStm >> nSteps >> bOrient >> bAttrib;
    }
    else
    {
        nSteps = 16;
        bOrient = bAttrib = TRUE;
    }

    aMtfSteps.SetValue( nSteps );
    aCbxOrientation.Check( bOrient );
    aCbxAttributes.Check( bAttrib );
}

// -----------------------------------------------------------------------------

void SdMorphDlg::SaveSettings() const
{
    SvStorageStreamRef xOStm( SD_MOD()->GetOptionStream( UniString::CreateFromAscii(
                               RTL_CONSTASCII_STRINGPARAM( SD_OPTION_MORPHING ) ),
                               SD_OPTION_STORE ) );

    if( xOStm.Is() )
    {
        SdIOCompat aCompat( *xOStm, STREAM_WRITE, 1 );

        *xOStm << (UINT16) aMtfSteps.GetValue()
               << aCbxOrientation.IsChecked()
               << aCbxAttributes.IsChecked();
    }
}

