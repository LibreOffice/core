/*************************************************************************
 *
 *  $RCSfile: multipat.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: kz $ $Date: 2005-01-21 16:41:51 $
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

#ifdef SVX_DLLIMPLEMENTATION
#undef SVX_DLLIMPLEMENTATION
#endif

// include ---------------------------------------------------------------

#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif
#ifndef _URLOBJ_HXX
#include <tools/urlobj.hxx>
#endif
#ifndef _SV_MSGBOX_HXX //autogen
#include <vcl/msgbox.hxx>
#endif
#ifndef _FILEDLGHELPER_HXX
#include <sfx2/filedlghelper.hxx>
#endif
#pragma hdrstop

#include "multipat.hxx"
#include "dialmgr.hxx"

#include "multipat.hrc"
#include "dialogs.hrc"

#ifndef _COMPHELPER_PROCESSFACTORY_HXX_
#include <comphelper/processfactory.hxx>
#endif

#ifndef  _COM_SUN_STAR_LANG_XMULTISERVICEFACTORY_HPP_
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#endif

#ifndef  _COM_SUN_STAR_UI_DIALOGS_XFOLDERPICKER_HPP_
#include <com/sun/star/ui/dialogs/XFolderPicker.hpp>
#endif
#ifndef  _COM_SUN_STAR_UI_DIALOGS_EXECUTABLEDIALOGRESULTS_HPP_
#include <com/sun/star/ui/dialogs/ExecutableDialogResults.hpp>
#endif

#include <unotools/localfilehelper.hxx>
#include <svtools/pathoptions.hxx>

using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::ui::dialogs;
using namespace ::com::sun::star::uno;

// struct MultiPath_Impl -------------------------------------------------

struct MultiPath_Impl
{
    BOOL    bEmptyAllowed;
    BOOL    bIsClassPathMode;

    MultiPath_Impl( BOOL bAllowed ) : bEmptyAllowed( bAllowed ), bIsClassPathMode( FALSE ) {}
};

// class SvxMultiPathDialog ----------------------------------------------

IMPL_LINK( SvxMultiPathDialog, SelectHdl_Impl, ListBox *, pBox )
{
    BOOL bEnable = ( pImpl->bEmptyAllowed || pBox->GetEntryCount() > 1 );
    aDelBtn.Enable( bEnable && pBox->GetSelectEntryPos() != LISTBOX_ENTRY_NOTFOUND );
    return 0;
}

// -----------------------------------------------------------------------

IMPL_LINK( SvxMultiPathDialog, AddHdl_Impl, PushButton *, EMPTYARG )
{
    rtl::OUString aService( RTL_CONSTASCII_USTRINGPARAM( FOLDER_PICKER_SERVICE_NAME ) );
    Reference < XMultiServiceFactory > xFactory( ::comphelper::getProcessServiceFactory() );
    Reference < XFolderPicker > xFolderPicker( xFactory->createInstance( aService ), UNO_QUERY );

    if ( xFolderPicker->execute() == ExecutableDialogResults::OK )
    {
        INetURLObject aPath( xFolderPicker->getDirectory() );
        aPath.removeFinalSlash();
        String aURL = aPath.GetMainURL( INetURLObject::NO_DECODE );
        String sInsPath;
        ::utl::LocalFileHelper::ConvertURLToSystemPath( aURL, sInsPath );

        if ( LISTBOX_ENTRY_NOTFOUND != aPathLB.GetEntryPos( sInsPath ) )
        {
            String sMsg( SVX_RES( RID_MULTIPATH_DBL_ERR ) );
            sMsg.SearchAndReplaceAscii( "%1", sInsPath );
            InfoBox( this, sMsg ).Execute();
        }
        else
        {
            USHORT nPos = aPathLB.InsertEntry( sInsPath, LISTBOX_APPEND );
            aPathLB.SetEntryData( nPos, (void*)new String( aURL ) );
        }
        SelectHdl_Impl( &aPathLB );
    }
    return 0;
}

// -----------------------------------------------------------------------

IMPL_LINK( SvxMultiPathDialog, DelHdl_Impl, PushButton *, EMPTYARG )
{
    USHORT nPos = aPathLB.GetSelectEntryPos();
    aPathLB.RemoveEntry( nPos );
    USHORT nCnt = aPathLB.GetEntryCount();

    if ( nCnt )
    {
        nCnt--;

        if ( nPos > nCnt )
            nPos = nCnt;
        aPathLB.SelectEntryPos( nPos );
    }
    SelectHdl_Impl( &aPathLB );
    return 0;
}

// -----------------------------------------------------------------------

SvxMultiPathDialog::SvxMultiPathDialog( Window* pParent, BOOL bEmptyAllowed ) :

    ModalDialog( pParent, SVX_RES( RID_SVXDLG_MULTIPATH ) ),

    aPathLB     ( this, ResId( LB_MULTIPATH ) ),
    aAddBtn     ( this, ResId( BTN_ADD_MULTIPATH ) ),
    aDelBtn     ( this, ResId( BTN_DEL_MULTIPATH ) ),
    aPathGB     ( this, ResId( GB_MULTIPATH) ),
    aOKBtn      ( this, ResId( BTN_MULTIPATH_OK ) ),
    aCancelBtn  ( this, ResId( BTN_MULTIPATH_CANCEL ) ),
    aHelpButton ( this, ResId( BTN_MULTIPATH_HELP ) ),
    pImpl       ( new MultiPath_Impl( bEmptyAllowed ) )

{
    FreeResource();
    aPathLB.SetSelectHdl( LINK( this, SvxMultiPathDialog, SelectHdl_Impl ) );
    aAddBtn.SetClickHdl( LINK( this, SvxMultiPathDialog, AddHdl_Impl ) );
    aDelBtn.SetClickHdl( LINK( this, SvxMultiPathDialog, DelHdl_Impl ) );
    SelectHdl_Impl( &aPathLB );
}

// -----------------------------------------------------------------------

SvxMultiPathDialog::~SvxMultiPathDialog()
{
    USHORT nPos = aPathLB.GetEntryCount();
    while(nPos--)
        delete (String*)aPathLB.GetEntryData(nPos);
    delete pImpl;
}

// -----------------------------------------------------------------------

String SvxMultiPathDialog::GetPath() const
{
    String aPath;
    sal_Unicode cDelim = pImpl->bIsClassPathMode ? CLASSPATH_DELIMITER : SVT_SEARCHPATH_DELIMITER;

    for ( USHORT i = 0; i < aPathLB.GetEntryCount(); ++i )
    {
        aPath += *(String*)aPathLB.GetEntryData(i);
        aPath += cDelim;
    }
    aPath.EraseTrailingChars( cDelim );
    return aPath;
}

// -----------------------------------------------------------------------

void SvxMultiPathDialog::SetPath( const String& rPath )
{
    sal_Unicode cDelim = pImpl->bIsClassPathMode ? CLASSPATH_DELIMITER : SVT_SEARCHPATH_DELIMITER;
    USHORT nPos;

    for ( USHORT i = 0; i < rPath.GetTokenCount( cDelim ); ++i )
    {
        String aTmp;
        if ( ::utl::LocalFileHelper::ConvertURLToSystemPath( rPath.GetToken( i, cDelim ), aTmp ) )
            nPos = aPathLB.InsertEntry( aTmp, LISTBOX_APPEND );
        else
            nPos = aPathLB.InsertEntry( rPath.GetToken( i, cDelim ), LISTBOX_APPEND );
        aPathLB.SetEntryData( nPos, (void*)new String( rPath.GetToken( i, cDelim ) ) );
    }

    SelectHdl_Impl( &aPathLB );
}

// -----------------------------------------------------------------------

void SvxMultiPathDialog::SetClassPathMode()
{
    pImpl->bIsClassPathMode = TRUE;
    SetText( SVX_RES( RID_SVXSTR_ARCHIVE_TITLE ));
    aPathGB.SetText( SVX_RES( RID_SVXSTR_ARCHIVE_HEADLINE ) );
}

// -----------------------------------------------------------------------

sal_Bool SvxMultiPathDialog::IsClassPathMode() const
{
    return pImpl->bIsClassPathMode;
}

