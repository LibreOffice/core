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
#include "precompiled_svtools.hxx"

#include <svtools/filedlg.hxx>
#include <filedlg2.hxx>

PathDialog::PathDialog( Window* _pParent, WinBits nStyle, sal_Bool bCreateDir ) :
    ModalDialog( _pParent, WB_STDMODAL | nStyle )
{
    pImpFileDlg = new ImpSvFileDlg;
    pImpFileDlg->CreateDialog( this, nStyle, WINDOW_PATHDIALOG, bCreateDir );
}

PathDialog::~PathDialog()
{
    delete pImpFileDlg;
}

short PathDialog::Execute()
{
    pImpFileDlg->GetDialog()->PreExecute();
    short n = ModalDialog::Execute();
    return n;
}

UniString PathDialog::GetPath() const
{
    return pImpFileDlg->GetDialog()->GetPath();
}

void PathDialog::SetPath( const UniString& rPath )
{
    pImpFileDlg->GetDialog()->SetPath( rPath );
}

void PathDialog::SetPath( const Edit& rEdit )
{
  pImpFileDlg->GetDialog()->SetPath( rEdit );
}

long PathDialog::OK()
{
    if( aOKHdlLink.IsSet() )
        return aOKHdlLink.Call( this );
    else
        return sal_True;
}


FileDialog::FileDialog( Window* _pParent, WinBits nStyle ) :
    PathDialog( _pParent, WB_STDMODAL | nStyle )
{
    // Dadurch dass hier bei VCL nicht der CTOR mit ResType verwendet wird,
    // wurde im PathDialog-CTOR leider ein ImpPathDialog angelegt...
    // So zwar scheisse, aber der Dialog ist eh' nur ein Hack:
    pImpFileDlg->CreateDialog( this, nStyle, WINDOW_FILEDIALOG, sal_False );
}

FileDialog::~FileDialog()
{
}

void FileDialog::AddFilter( const UniString& rFilter, const UniString& rMask )
{
    ((ImpFileDialog*)pImpFileDlg->GetDialog())->AddFilter( rFilter, rMask );
}

void FileDialog::RemoveFilter( const UniString& rFilter )
{
    ((ImpFileDialog*)pImpFileDlg->GetDialog())->RemoveFilter( rFilter );
}

void FileDialog::RemoveAllFilter()
{
    ((ImpFileDialog*)pImpFileDlg->GetDialog())->RemoveAllFilter();
}

void FileDialog::SetCurFilter( const UniString& rFilter )
{
    ((ImpFileDialog*)pImpFileDlg->GetDialog())->SetCurFilter( rFilter );
}

UniString FileDialog::GetCurFilter() const
{
    return ((ImpFileDialog*)pImpFileDlg->GetDialog())->GetCurFilter();
}

void FileDialog::FileSelect()
{
    aFileHdlLink.Call( this );
}

void FileDialog::FilterSelect()
{
    aFilterHdlLink.Call( this );
}

sal_uInt16 FileDialog::GetFilterCount() const
{
  return ((ImpFileDialog*)pImpFileDlg->GetDialog())->GetFilterCount();
}

UniString FileDialog::GetFilterName( sal_uInt16 nPos ) const
{
  return ((ImpFileDialog*)pImpFileDlg->GetDialog())->GetFilterName( nPos );
}

UniString FileDialog::GetFilterType( sal_uInt16 nPos ) const
{
  return ((ImpFileDialog*)pImpFileDlg->GetDialog())->GetFilterType( nPos );
}

void FileDialog::SetOkButtonText( const UniString& rText )
{
    pImpFileDlg->SetOkButtonText( rText );
}

void FileDialog::SetCancelButtonText( const UniString& rText )
{
    pImpFileDlg->SetCancelButtonText( rText );
}
