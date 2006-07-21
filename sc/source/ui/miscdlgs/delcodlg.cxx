/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: delcodlg.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: kz $ $Date: 2006-07-21 14:04:53 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sc.hxx"

#undef SC_DLLIMPLEMENTATION



//------------------------------------------------------------------

#include "delcodlg.hxx"
#include "scresid.hxx"
#include "miscdlgs.hrc"


//==================================================================

BOOL   ScDeleteContentsDlg::bPreviousAllCheck = FALSE;
USHORT ScDeleteContentsDlg::nPreviousChecks   = (IDF_DATETIME | IDF_STRING  |
                                                 IDF_NOTE     | IDF_FORMULA |
                                                 IDF_VALUE);

//-----------------------------------------------------------------------

ScDeleteContentsDlg::ScDeleteContentsDlg( Window* pParent,
                                          USHORT  nCheckDefaults ) :
    ModalDialog     ( pParent, ScResId( RID_SCDLG_DELCONT ) ),
    //
    aBtnOk          ( this, ScResId( BTN_OK ) ),
    aBtnCancel      ( this, ScResId( BTN_CANCEL ) ),
    aBtnHelp        ( this, ScResId( BTN_HELP ) ),
    aBtnDelAll      ( this, ScResId( BTN_DELALL ) ),
    aBtnDelStrings  ( this, ScResId( BTN_DELSTRINGS ) ),
    aBtnDelNumbers  ( this, ScResId( BTN_DELNUMBERS ) ),
    aBtnDelDateTime ( this, ScResId( BTN_DELDATETIME ) ),
    aBtnDelFormulas ( this, ScResId( BTN_DELFORMULAS ) ),
    aBtnDelNotes    ( this, ScResId( BTN_DELNOTES ) ),
    aBtnDelAttrs    ( this, ScResId( BTN_DELATTRS ) ),
    aBtnDelObjects  ( this, ScResId( BTN_DELOBJECTS ) ),
    aFlFrame        ( this, ScResId( FL_FRAME ) ),
    bObjectsDisabled( FALSE )
{
    if ( nCheckDefaults != 0 )
    {
        ScDeleteContentsDlg::nPreviousChecks = nCheckDefaults;
        ScDeleteContentsDlg::bPreviousAllCheck = FALSE;
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

USHORT ScDeleteContentsDlg::GetDelContentsCmdBits() const
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

void ScDeleteContentsDlg::DisableChecks( BOOL bDelAllChecked )
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
    bObjectsDisabled = TRUE;
    aBtnDelObjects.Check(FALSE);
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



