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



#ifdef SVX_DLLIMPLEMENTATION
#undef SVX_DLLIMPLEMENTATION
#endif

#if TEST_LAYOUT
#include <cstdio>
#endif /* TEST_LAYOUT */
#include <com/sun/star/awt/XDialog2.hpp>
#include <tools/shl.hxx>
#include <svtools/itemset.hxx>
#include <svtools/itempool.hxx>
#include <sfx2/objsh.hxx>
#include <vcl/msgbox.hxx>
#include <toolkit/awt/vclxwindow.hxx>


#include <com/sun/star/awt/PosSize.hpp> //redrawAlready

using namespace com::sun::star;

#define _SVX_RECOVER_CXX

#include "recover.hxx"

#include <layout/layout-pre.hxx>

#if ENABLE_LAYOUT
#undef SVX_RES
#define SVX_RES(x) #x
#undef SfxModalDialog
#define SfxModalDialog( parent, id ) Dialog( parent, "recover.xml", id )
#endif /* ENABLE_LAYOUT */

#if TEST_LAYOUT
SvxRecoverDialog::SvxRecoverDialog( Window* pParent )
#else /* !TEST_LAYOUT */
SvxRecoverDialog::SvxRecoverDialog( Window* pParent, const SfxItemSet& rCoreSet )
#endif /* !TEST_LAYOUT */
: SfxModalDialog( pParent, SVX_RES( RID_SVXDLG_RECOVER ) )

    , aHeaderImage( this, SVX_RES( FI_HEADER ) )
    , aHeaderText( this, SVX_RES( FT_HEADER ) )
    , aHeaderLine( this, SVX_RES( FL_HEADER ) )
    , aRecoverText( this, SVX_RES( FT_RECOVER ) )
    , aTextAdvanced( this, SVX_RES( FT_ADVANCED ) )

    , aCheckBoxDoc( this, SVX_RES( CB_DOC ) )
    , aImageDoc( this, SVX_RES( FI_DOC ) )
    , aTextDoc( this, SVX_RES( FT_DOC ) )

    , aCheckBoxSheet( this, SVX_RES( CB_SHEET ) )
    , aImageSheet( this, SVX_RES( FI_SHEET ) )
    , aTextSheet( this, SVX_RES( FT_SHEET ) )

    , aCheckBoxDraw( this, SVX_RES( CB_DRAW ) )
    , aImageDraw( this, SVX_RES( FI_DRAW ) )
    , aTextDraw( this, SVX_RES( FT_DRAW ) )

    , aCheckBoxPresent( this, SVX_RES( CB_PRESENT ) )
    , aImagePresent( this, SVX_RES( FI_PRESENT ) )
    , aTextPresent( this, SVX_RES( FT_PRESENT ) )

    , aButtonAdvanced( this, SVX_RES( PB_ADVANCED ) )

    , aProgressText( this, SVX_RES( FT_PROGRESS ) )
    , aProgressBar( this, SVX_RES( PB_RECOVER ) )
    , aCheckBoxLogFile( this, SVX_RES( CH_LOGFILE ) )
    , aOKBtn( this, SVX_RES( BTN_OK ) )
    , aCancelBtn( this, SVX_RES( BTN_CANCEL ) )
    , aHelpBtn( this, SVX_RES( BTN_HELP ) )
{
    aButtonAdvanced.AddAdvanced( &aTextAdvanced );
    aButtonAdvanced.AddAdvanced( &aCheckBoxDoc );
    aButtonAdvanced.AddAdvanced( &aCheckBoxSheet );
    aButtonAdvanced.AddAdvanced( &aCheckBoxDraw );
    aButtonAdvanced.AddAdvanced( &aCheckBoxPresent );
    aButtonAdvanced.AddAdvanced( &aCheckBoxLogFile );
}


SvxRecoverDialog::~SvxRecoverDialog()
{
}
