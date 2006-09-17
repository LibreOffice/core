/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: filedlg.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: obo $ $Date: 2006-09-17 14:44:05 $
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
#include "precompiled_svtools.hxx"

#include <filedlg.hxx>
#include <filedlg2.hxx>

PathDialog::PathDialog( Window* _pParent, WinBits nStyle, BOOL bCreateDir ) :
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
        return TRUE;
}


FileDialog::FileDialog( Window* _pParent, WinBits nStyle ) :
    PathDialog( _pParent, WB_STDMODAL | nStyle )
{
    // Dadurch dass hier bei VCL nicht der CTOR mit ResType verwendet wird,
    // wurde im PathDialog-CTOR leider ein ImpPathDialog angelegt...
    // So zwar scheisse, aber der Dialog ist eh' nur ein Hack:
    pImpFileDlg->CreateDialog( this, nStyle, WINDOW_FILEDIALOG, FALSE );
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

USHORT FileDialog::GetFilterCount() const
{
  return ((ImpFileDialog*)pImpFileDlg->GetDialog())->GetFilterCount();
}

UniString FileDialog::GetFilterName( USHORT nPos ) const
{
  return ((ImpFileDialog*)pImpFileDlg->GetDialog())->GetFilterName( nPos );
}

UniString FileDialog::GetFilterType( USHORT nPos ) const
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
