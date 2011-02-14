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

#include <tools/geninfo.hxx>
//#include "depapp.hxx"
#include <soldep/soldep.hxx>
#include <soldep/soldlg.hxx>
#include <soldep/soldlg.hrc>

#ifndef SOLARIS
#define SIZE( nX, nY)   \
    LogicToLogic(Size(nX,nY),&MapMode(MAP_APPFONT),&GetMapMode())
#define POS(nX, nY) \
    LogicToLogic(Point(nX,nY),&MapMode(MAP_APPFONT),&GetMapMode())
#else
#define SIZE( nX, nY)   \
        LogicToLogic(Size(nX,nY),MapMode(MAP_APPFONT),GetMapMode())
#define POS(nX, nY) \
        LogicToLogic(Point(nX,nY),MapMode(MAP_APPFONT),GetMapMode())
#endif


//
// class SolNewProjectDlg
//

/*****************************************************************************/
SolNewProjectDlg::SolNewProjectDlg( Window* pParent, const ResId& rResId )
/*****************************************************************************/
                : ModalDialog( pParent, rResId ),
                maOkButton( this, DtSodResId( BTN_OK )),
                maCancelButton( this, DtSodResId( BTN_CANCEL )),
                maFTName( this, DtSodResId( FT_PRJNAME )),
                maEName( this, DtSodResId( EDIT_PRJNAME )),
                maFTShort( this, DtSodResId( FT_PRJSHORT )),
                maEShort( this, DtSodResId( EDIT_PRJSHORT )),
                maFTDeps( this, DtSodResId( FT_PRJDEPS )),
                maEDeps( this, DtSodResId( EDIT_PRJDEPS ))
{
    FreeResource();
    maOkButton.SetClickHdl( LINK( this, SolNewProjectDlg, OkHdl ));
    maCancelButton.SetClickHdl( LINK( this, SolNewProjectDlg, CancelHdl ));
}

/*****************************************************************************/
IMPL_LINK( SolNewProjectDlg, OkHdl, Button*, pOkBtn )
/*****************************************************************************/
{
    EndDialog( 1 );
    return 0;
}

/*****************************************************************************/
IMPL_LINK( SolNewProjectDlg, CancelHdl, Button*,  pCancelBtn )
/*****************************************************************************/
{
    EndDialog( 0 );
    return 0;
}

//
// class SolNewDirectoryDlg
//

/*****************************************************************************/
SolNewDirectoryDlg::SolNewDirectoryDlg( Window* pParent, const ResId& rResId )
/*****************************************************************************/
                : ModalDialog( pParent, rResId ),
                maOkButton( this, DtSodResId( BTN_OK )),
                maCancelButton( this, DtSodResId( BTN_CANCEL )),
                maFTName( this, DtSodResId( FT_DIRNAME )),
                maEName( this, DtSodResId( EDIT_DIRNAME )),
                maFTFlag( this, DtSodResId( FT_DIRFLAG )),
                maEFlag( this, DtSodResId( EDIT_DIRFLAG )),
                maFTDeps( this, DtSodResId( FT_DIRDEPS )),
                maEDeps( this, DtSodResId( EDIT_DIRDEPS )),
                maFTAction( this, DtSodResId( FT_DIRACTION )),
                maEAction( this, DtSodResId( EDIT_DIRACTION )),
                maFTEnv( this, DtSodResId( FT_DIRENV )),
                maEEnv( this, DtSodResId( EDIT_DIRENV ))
{
    FreeResource();
    maOkButton.SetClickHdl( LINK( this, SolNewDirectoryDlg, OkHdl ));
    maCancelButton.SetClickHdl( LINK( this, SolNewDirectoryDlg, CancelHdl ));
}

/*****************************************************************************/
IMPL_LINK( SolNewDirectoryDlg, OkHdl, Button*, pOkBtn )
/*****************************************************************************/
{
    EndDialog( 1 );
    return 0;
}

/*****************************************************************************/
IMPL_LINK( SolNewDirectoryDlg, CancelHdl, Button*,  pCancelBtn )
/*****************************************************************************/
{
    EndDialog( 0 );
    return 0;
}

//
// class SolHelpDlg
//

/*****************************************************************************/
SolHelpDlg::SolHelpDlg( Window* pParent, const ResId& rResId )
/*****************************************************************************/
                : ModalDialog( pParent, rResId ),
                maOkButton( this, DtSodResId( BTN_OK )),
                maMLEHelp( this, DtSodResId( EDIT_HELP ))
{
    FreeResource();
    maOkButton.SetClickHdl( LINK( this, SolHelpDlg, OkHdl ));
}

/*****************************************************************************/
IMPL_LINK( SolHelpDlg, OkHdl, Button*, pOkBtn )
/*****************************************************************************/
{
    EndDialog( 1 );
    return 0;
}

//
// class SolSelectVersionDlg
//

/*****************************************************************************/
SolSelectVersionDlg::SolSelectVersionDlg(
    Window *pParent, GenericInformationList *pStandLst )
/*****************************************************************************/
                : ModalDialog( pParent, DtSodResId( DLG_VERSIONSELECT )),
                maVersionListBox( this, DtSodResId( DLG_VERSIONSELECT_LISTBOX )),
                maVersionGroupBox( this, DtSodResId( DLG_VERSIONSELECT_GROUP )),
                maMinorEditBox( this, DtSodResId( DLG_MINORSELECT_EDIT )),
                maMinorGroupBox( this, DtSodResId( DLG_MINORSELECT_GROUP )),
                maOKButton( this, DtSodResId( DLG_VERSIONSELECT_OK  )),
                maCancelButton( this, DtSodResId( DLG_VERSIONSELECT_CANCEL ))
{
    FreeResource();
    //Fill the ListBox with MWS versions (e.g. SRC680) from "stand.lst"
    for ( sal_uIntPtr i = 0; i < pStandLst->Count(); i++ ) {
        String sVersion( *pStandLst->GetObject( i ), RTL_TEXTENCODING_ASCII_US );
        maVersionListBox.InsertEntry( sVersion );
    }

    if ( pStandLst->Count())
        maVersionListBox.SelectEntryPos( 0 );

    maVersionListBox.SetDoubleClickHdl(
        LINK( this, SolSelectVersionDlg, DoubleClickHdl ));

}

/*****************************************************************************/
ByteString SolSelectVersionDlg::GetVersionMajor()
/*****************************************************************************/
{
    //Returns the selected version
    return ByteString(maVersionListBox.GetSelectEntry(), RTL_TEXTENCODING_ASCII_US );
}

/*****************************************************************************/
ByteString SolSelectVersionDlg::GetVersionMinor()
/*****************************************************************************/
{
    //Returns the minor
    ByteString minor = ByteString(maMinorEditBox.GetText(), RTL_TEXTENCODING_ASCII_US );
    //check for correctness (format: "m1234")
    //"m123s8" is unsupported because 'steps' aren't used anymore
    minor.EraseLeadingAndTrailingChars();
    int check = minor.SearchChar("m");
    if (check == 0)
    {
        ByteString check2 = minor.Copy(1,(minor.Len()-1));
        if (check2.IsNumericAscii()) return minor;
    }
    return ByteString("");
}

/*****************************************************************************/
IMPL_LINK( SolSelectVersionDlg, DoubleClickHdl, ListBox *, pBox )
/*****************************************************************************/
{
    EndDialog( RET_OK );
    return 0;
}

//
// class SolAutoarrangeDlg
//

/*****************************************************************************/
SolAutoarrangeDlg::SolAutoarrangeDlg( Window *pParent )
/*****************************************************************************/
                : ModelessDialog( pParent, DtSodResId( DLG_AUTOARRANGE )),
                maGroupBox( this, DtSodResId( DLG_AUTOARRANGE_GROUP )),
                maModuleText( this, DtSodResId( DLG_AUTOARRANGE_TEXT_MODULE )),
                maOverallText( this, DtSodResId( DLG_AUTOARRANGE_TEXT_OVERALL )),
                maModuleBar( this ),
                maOverallBar( this )
{
    FreeResource();
/* Mac Porting..... taking address of temporary (warning)
    maModuleBar.SetPosPixel( POS( 8, 28 ));
    maOverallBar.SetPosPixel( POS( 8, 60 ));
    maModuleBar.SetSizePixel( SIZE( 208,12 ));
    maOverallBar.SetSizePixel( SIZE( 208,12 ));
*/
    maModuleBar.Show();
    maOverallBar.Show();

       maModuleText.Show();
    maOverallText.Show();
}

/**********************************************************************************/

SolFindProjectDlg::SolFindProjectDlg( Window *pParent, ObjWinList* pObjList )
                : ModalDialog( pParent, DtSodResId( DLG_FIND_PROJECT )),
                maCombobox( this, DtSodResId( DLG_FIND_PROJECT_COMBOBOX )),
                maOKButton( this, DtSodResId( DLG_FIND_PROJECT_OK )),
                maCancelButton( this, DtSodResId( DLG_FIND_PROJECT_CANCEL ))
{
    FreeResource();
    maCombobox.SetDropDownLineCount(15);
//    SolDep* pSolDep = ((MyApp*)GetpApp())->GetSolDep();
//    ObjWinList* pObjList = pSolDep->GetObjectList();
    sal_uIntPtr n = pObjList->Count();
    //Fill combobox
    for (sal_uIntPtr i=0; i<n; i++) {
        ByteString prjname = pObjList->GetObject( i )->GetBodyText();
        if (prjname != ByteString("null"))                                 //null_project
            maCombobox.InsertEntry( String(prjname,RTL_TEXTENCODING_UTF8) );
        }
}

ByteString SolFindProjectDlg::GetProject()
{
    return ByteString(maCombobox.GetText(),RTL_TEXTENCODING_UTF8);
}
