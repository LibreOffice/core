/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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

#include "sal/config.h"

#include <comphelper/processfactory.hxx>
#include <tools/shl.hxx>
#include <vcl/msgbox.hxx>
#include <sfx2/filedlghelper.hxx>

#include <tools/urlobj.hxx>

#include "multipat.hxx"
#include "multifil.hxx"
#include <dialmgr.hxx>

#include "multipat.hrc"
#include <cuires.hrc>

// #97807# -------------
#include <com/sun/star/ucb/UniversalContentBroker.hpp>

#include "com/sun/star/ui/dialogs/TemplateDescription.hpp"

using namespace com::sun::star::ucb;
using namespace com::sun::star::uno;

// class SvxMultiFileDialog ----------------------------------------------

IMPL_LINK( SvxMultiFileDialog, AddHdl_Impl, PushButton *, pBtn )
{
    sfx2::FileDialogHelper aDlg(
        com::sun::star::ui::dialogs::TemplateDescription::FILEOPEN_SIMPLE, 0 );

    if ( IsClassPathMode() )
    {
        aDlg.SetTitle( CUI_RES( RID_SVXSTR_ARCHIVE_TITLE ) );
        aDlg.AddFilter( CUI_RES( RID_SVXSTR_ARCHIVE_HEADLINE ), rtl::OUString("*.jar;*.zip") );
    }

    if ( aDlg.Execute() == ERRCODE_NONE )
    {
        // #97807# URL content comparison of entries -----------
        INetURLObject aFile( aDlg.GetPath() );
        String sInsFile = aFile.getFSysPath( INetURLObject::FSYS_DETECT );
        ::ucbhelper::Content aContent( aFile.GetMainURL( INetURLObject::NO_DECODE ), Reference< XCommandEnvironment >(), comphelper::getProcessComponentContext() );
        Reference< XContent > xContent = aContent.get();
        OSL_ENSURE( xContent.is(), "AddHdl_Impl: invalid content interface!" );
        Reference< XContentIdentifier > xID = xContent->getIdentifier();
        OSL_ENSURE( xID.is(), "AddHdl_Impl: invalid ID interface!" );
        // ensure the content of files are valid

        sal_uInt16 nCount = aPathLB.GetEntryCount();
        sal_Bool bDuplicated = sal_False;
        try
        {
            if( nCount > 0 ) // start comparison
            {
                sal_uInt16 i;
                ::ucbhelper::Content & VContent = aContent; // temporary Content reference
                Reference< XContent > xVContent;
                Reference< XContentIdentifier > xVID;
                for( i = 0; i < nCount; i++ )
                {
                    String sVFile = aPathLB.GetEntry( i );
                    std::map< String, ::ucbhelper::Content >::iterator aCur = aFileContentMap.find( sVFile );
                    if( aCur == aFileContentMap.end() ) // look for File Content in aFileContentMap, but not find it.
                    {
                        INetURLObject aVFile( sVFile, INetURLObject::FSYS_DETECT );
                        aFileContentMap[sVFile] = ::ucbhelper::Content( aVFile.GetMainURL( INetURLObject::NO_DECODE ), Reference< XCommandEnvironment >(), comphelper::getProcessComponentContext() );
                        VContent = aFileContentMap.find( sVFile )->second;
                    }
                    else
                        VContent = aCur->second;
                    xVContent = VContent.get();
                    OSL_ENSURE( xVContent.is(), "AddHdl_Impl: invalid content interface!" );
                    xVID = xVContent->getIdentifier();
                    OSL_ENSURE( xVID.is(), "AddHdl_Impl: invalid ID interface!" );
                    if ( xID.is() && xVID.is()
                         && ( UniversalContentBroker::create(
                                  comphelper::getProcessComponentContext() )->
                              compareContentIds( xID, xVID ) == 0 ) )
                    {
                        bDuplicated = sal_True;
                        break;
                    }
                }
            } // end of if the entries are more than zero.
        } // end of try(}
        catch( const Exception& ) // catch every exception of comparison
           {
            OSL_FAIL( "AddHdl_Impl: caught an unexpected exception!" );
           }

        if ( bDuplicated ) // #97807# --------------------
        {
            String sMsg( CUI_RES( RID_SVXSTR_MULTIFILE_DBL_ERR ) );
            sMsg.SearchAndReplaceAscii( "%1", sInsFile );
            InfoBox( pBtn, sMsg ).Execute();
        }
        else
        {
            sal_uInt16 nPos = aPathLB.InsertEntry( sInsFile, LISTBOX_APPEND );
            aPathLB.SetEntryData( nPos, (void*) new String( sInsFile ) );
        }

    } // end of if ( aDlg.Execute() == ERRCODE_NONE )
    return 0;
}

// -----------------------------------------------------------------------

IMPL_LINK_NOARG(SvxMultiFileDialog, DelHdl_Impl)
{
    sal_uInt16 nPos = aPathLB.GetSelectEntryPos();
    aPathLB.RemoveEntry( nPos );
    sal_uInt16 nCnt = aPathLB.GetEntryCount();

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

SvxMultiFileDialog::SvxMultiFileDialog( Window* pParent, sal_Bool bEmptyAllowed ) :

    SvxMultiPathDialog( pParent, bEmptyAllowed )

{
    aAddBtn.SetClickHdl( LINK( this, SvxMultiFileDialog, AddHdl_Impl ) );
    aDelBtn.SetClickHdl( LINK( this, SvxMultiFileDialog, DelHdl_Impl ) );
    SetText( CUI_RES( RID_SVXSTR_FILE_TITLE ) );
    aPathFL.SetText( CUI_RES( RID_SVXSTR_FILE_HEADLINE ) );
    aDelBtn.Enable();
}

// -----------------------------------------------------------------------

SvxMultiFileDialog::~SvxMultiFileDialog()
{
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
