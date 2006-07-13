/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: multifil.cxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: obo $ $Date: 2006-07-13 11:59:59 $
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

#ifdef SVX_DLLIMPLEMENTATION
#undef SVX_DLLIMPLEMENTATION
#endif

// include ---------------------------------------------------------------

#ifndef _SHL_HXX //autogen
#include <tools/shl.hxx>
#endif
#ifndef _SV_MSGBOX_HXX //autogen
#include <vcl/msgbox.hxx>
#endif

#ifndef _FILEDLGHELPER_HXX
#include <sfx2/filedlghelper.hxx>
#endif

#include <tools/urlobj.hxx>

#include "multipat.hxx"
#include "multifil.hxx"
#include "dialmgr.hxx"

#include "multipat.hrc"
#include "dialogs.hrc"

// #97807# -------------
#ifndef _COM_SUN_STAR_UCB_XCONTENTPROVIDER_HPP_
#include <com/sun/star/ucb/XContentProvider.hpp>
#endif
#ifndef _UCBHELPER_CONTENTBROKER_HXX
#include <ucbhelper/contentbroker.hxx>
#endif

using namespace com::sun::star::ucb;
using namespace com::sun::star::uno;

// class SvxMultiFileDialog ----------------------------------------------

IMPL_LINK( SvxMultiFileDialog, AddHdl_Impl, PushButton *, pBtn )
{
    sfx2::FileDialogHelper aDlg( ::sfx2::FILEOPEN_SIMPLE, 0 );

    if ( IsClassPathMode() )
    {
        aDlg.SetTitle( SVX_RES( RID_SVXSTR_ARCHIVE_TITLE ) );
        aDlg.AddFilter( SVX_RES( RID_SVXSTR_ARCHIVE_HEADLINE ), String::CreateFromAscii("*.jar;*.zip") );
    }

    if ( aDlg.Execute() == ERRCODE_NONE )
    {
        // #97807# URL content comparison of entries -----------
        INetURLObject aFile( aDlg.GetPath() );
        String sInsFile = aFile.getFSysPath( INetURLObject::FSYS_DETECT );
        ::ucb::Content aContent( aFile.GetMainURL( INetURLObject::NO_DECODE ), Reference< XCommandEnvironment >() );
        Reference< XContent > xContent = aContent.get();
        OSL_ENSURE( xContent.is(), "AddHdl_Impl: invalid content interface!" );
        Reference< XContentIdentifier > xID = xContent->getIdentifier();
        OSL_ENSURE( xID.is(), "AddHdl_Impl: invalid ID interface!" );
        // ensure the content of files are valid

        USHORT nCount = aPathLB.GetEntryCount();
        BOOL bDuplicated = FALSE;
        try
        {
            if( nCount > 0 ) // start comparison
            {
                USHORT i;
                ::ucb::Content & VContent = aContent; // temporary Content reference
                Reference< XContent > xVContent;
                Reference< XContentIdentifier > xVID;
                for( i = 0; i < nCount; i++ )
                {
                    String sVFile = aPathLB.GetEntry( i );
                    std::map< String, ::ucb::Content >::iterator aCur = aFileContentMap.find( sVFile );
                    if( aCur == aFileContentMap.end() ) // look for File Content in aFileContentMap, but not find it.
                    {
                        INetURLObject aVFile( sVFile, INetURLObject::FSYS_DETECT );
                        aFileContentMap[sVFile] = ::ucb::Content( aVFile.GetMainURL( INetURLObject::NO_DECODE ), Reference< XCommandEnvironment >() );
                        VContent = aFileContentMap.find( sVFile )->second;
                    }
                    else
                        VContent = aCur->second;
                    xVContent = VContent.get();
                    OSL_ENSURE( xVContent.is(), "AddHdl_Impl: invalid content interface!" );
                    xVID = xVContent->getIdentifier();
                    OSL_ENSURE( xVID.is(), "AddHdl_Impl: invalid ID interface!" );
                    if ( xID.is() && xVID.is() )
                    {
                        // get a generic content provider
                        ::ucb::ContentBroker* pBroker = ::ucb::ContentBroker::get();
                        Reference< XContentProvider > xProvider;
                        if ( pBroker )
                            xProvider = pBroker->getContentProviderInterface();
                        if ( xProvider.is() )
                        {
                            if ( 0 == xProvider->compareContentIds( xID, xVID ) )
                            {
                                bDuplicated = TRUE;
                                break;
                            }
                        }
                    }
                }
            } // end of if the entries are more than zero.
        } // end of try(}
        catch( const Exception& ) // catch every exception of comparison
           {
            OSL_ENSURE( sal_False, "AddHdl_Impl: caught an unexpected exception!" );
           }

        if ( bDuplicated ) // #97807# --------------------
        {
            String sMsg( SVX_RES( RID_SVXSTR_MULTIFILE_DBL_ERR ) );
            sMsg.SearchAndReplaceAscii( "%1", sInsFile );
            InfoBox( pBtn, sMsg ).Execute();
        }
        else
        {
            USHORT nPos = aPathLB.InsertEntry( sInsFile, LISTBOX_APPEND );
            aPathLB.SetEntryData( nPos, (void*) new String( sInsFile ) );
        }

    } // end of if ( aDlg.Execute() == ERRCODE_NONE )
    return 0;
}

// -----------------------------------------------------------------------

IMPL_LINK( SvxMultiFileDialog, DelHdl_Impl, PushButton *, EMPTYARG )
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
    return 0;
}

// -----------------------------------------------------------------------

SvxMultiFileDialog::SvxMultiFileDialog( Window* pParent, BOOL bEmptyAllowed ) :

    SvxMultiPathDialog( pParent, bEmptyAllowed )

{
    aAddBtn.SetClickHdl( LINK( this, SvxMultiFileDialog, AddHdl_Impl ) );
    aDelBtn.SetClickHdl( LINK( this, SvxMultiFileDialog, DelHdl_Impl ) );
    SetText( SVX_RES( RID_SVXSTR_FILE_TITLE ) );
    aPathFL.SetText( SVX_RES( RID_SVXSTR_FILE_HEADLINE ) );
    aDelBtn.Enable();
}

// -----------------------------------------------------------------------

SvxMultiFileDialog::~SvxMultiFileDialog()
{
}


