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
#include "precompiled_sc.hxx"

#undef SC_DLLIMPLEMENTATION



//------------------------------------------------------------------

#include "delcodlg.hxx"
#include "scresid.hxx"
#include "miscdlgs.hrc"


//==================================================================

sal_Bool   ScDeleteContentsDlg::bPreviousAllCheck = sal_False;
sal_uInt16 ScDeleteContentsDlg::nPreviousChecks   = (IDF_DATETIME | IDF_STRING  |
                                                 IDF_NOTE     | IDF_FORMULA |
                                                 IDF_VALUE);

//-----------------------------------------------------------------------

ScDeleteContentsDlg::ScDeleteContentsDlg( Window* pParent,
                                          sal_uInt16  nCheckDefaults ) :
    ModalDialog     ( pParent, ScResId( RID_SCDLG_DELCONT ) ),
    //
    aFlFrame        ( this, ScResId( FL_FRAME ) ),
    aBtnDelAll      ( this, ScResId( BTN_DELALL ) ),
    aBtnDelStrings  ( this, ScResId( BTN_DELSTRINGS ) ),
    aBtnDelNumbers  ( this, ScResId( BTN_DELNUMBERS ) ),
    aBtnDelDateTime ( this, ScResId( BTN_DELDATETIME ) ),
    aBtnDelFormulas ( this, ScResId( BTN_DELFORMULAS ) ),
    aBtnDelNotes    ( this, ScResId( BTN_DELNOTES ) ),
    aBtnDelAttrs    ( this, ScResId( BTN_DELATTRS ) ),
    aBtnDelObjects  ( this, ScResId( BTN_DELOBJECTS ) ),
    aBtnOk          ( this, ScResId( BTN_OK ) ),
    aBtnCancel      ( this, ScResId( BTN_CANCEL ) ),
    aBtnHelp        ( this, ScResId( BTN_HELP ) ),
    bObjectsDisabled( sal_False )
{
    if ( nCheckDefaults != 0 )
    {
        ScDeleteContentsDlg::nPreviousChecks = nCheckDefaults;
        ScDeleteContentsDlg::bPreviousAllCheck = sal_False;
    }

    aBtnDelAll.Check     ( ScDeleteContentsDlg::bPreviousAllCheck );
    aBtnDelStrings.Check ( IS_SET( IDF_STRING,
                                   ScDeleteContentsDlg::nPreviousChecks ) );
    aBtnDelNumbers.Check ( IS_SET( IDF_VALUE,
                                   ScDeleteContentsDlg::nPreviousChecks ) );
    aBtnDelDateTime.Check( IS_SET( IDF_DATETIME,
                                   ScDeleteContentsDlg::nPreviousChecks ) );
    aBtnDelFormulas.Check( IS_SET( IDF_FORMULA,
                                   ScDeleteContentsDlg::nPreviousChecks ) );
    aBtnDelNotes.Check   ( IS_SET( IDF_NOTE,
                                   ScDeleteContentsDlg::nPreviousChecks ) );
    aBtnDelAttrs.Check   ( IS_SET( IDF_ATTRIB,
                                   ScDeleteContentsDlg::nPreviousChecks ) );
    aBtnDelObjects.Check ( IS_SET( IDF_OBJECTS,
                                   ScDeleteContentsDlg::nPreviousChecks ) );

    DisableChecks( aBtnDelAll.IsChecked() );

    aBtnDelAll.SetClickHdl( LINK( this, ScDeleteContentsDlg, DelAllHdl ) );

    //-------------
    FreeResource();
}

//------------------------------------------------------------------------

sal_uInt16 ScDeleteContentsDlg::GetDelContentsCmdBits() const
{
    ScDeleteContentsDlg::nPreviousChecks = 0;

    if ( aBtnDelStrings.IsChecked() )
        ScDeleteContentsDlg::nPreviousChecks  = IDF_STRING;
    if ( aBtnDelNumbers.IsChecked() )
        ScDeleteContentsDlg::nPreviousChecks |= IDF_VALUE;
    if ( aBtnDelDateTime.IsChecked())
        ScDeleteContentsDlg::nPreviousChecks |= IDF_DATETIME;
    if ( aBtnDelFormulas.IsChecked())
        ScDeleteContentsDlg::nPreviousChecks |= IDF_FORMULA;
    if ( aBtnDelNotes.IsChecked()   )
        ScDeleteContentsDlg::nPreviousChecks |= IDF_NOTE;
    if ( aBtnDelAttrs.IsChecked()   )
        ScDeleteContentsDlg::nPreviousChecks |= IDF_ATTRIB;
    if ( aBtnDelObjects.IsChecked() )
        ScDeleteContentsDlg::nPreviousChecks |= IDF_OBJECTS;

    ScDeleteContentsDlg::bPreviousAllCheck = aBtnDelAll.IsChecked();

    return ( (ScDeleteContentsDlg::bPreviousAllCheck)
                ? IDF_ALL
                : ScDeleteContentsDlg::nPreviousChecks );
}

//------------------------------------------------------------------------

void ScDeleteContentsDlg::DisableChecks( sal_Bool bDelAllChecked )
{
    if ( bDelAllChecked )
    {
        aBtnDelStrings.Disable();
        aBtnDelNumbers.Disable();
        aBtnDelDateTime.Disable();
        aBtnDelFormulas.Disable();
        aBtnDelNotes.Disable();
        aBtnDelAttrs.Disable();
        aBtnDelObjects.Disable();
    }
    else
    {
        aBtnDelStrings.Enable();
        aBtnDelNumbers.Enable();
        aBtnDelDateTime.Enable();
        aBtnDelFormulas.Enable();
        aBtnDelNotes.Enable();
        aBtnDelAttrs.Enable();
        if (bObjectsDisabled)
            aBtnDelObjects.Disable();
        else
            aBtnDelObjects.Enable();
    }
}

//------------------------------------------------------------------------

void ScDeleteContentsDlg::DisableObjects()
{
    bObjectsDisabled = sal_True;
    aBtnDelObjects.Check(sal_False);
    aBtnDelObjects.Disable();
}

//------------------------------------------------------------------------

IMPL_LINK_INLINE_START( ScDeleteContentsDlg, DelAllHdl, void *, EMPTYARG )
{
    DisableChecks( aBtnDelAll.IsChecked() );

    return 0;
}
IMPL_LINK_INLINE_END( ScDeleteContentsDlg, DelAllHdl, void *, EMPTYARG )

__EXPORT ScDeleteContentsDlg::~ScDeleteContentsDlg()
{
}



