/*************************************************************************
 *
 *  $RCSfile: filedlg.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:58:58 $
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

#include <filedlg.hxx>
#include <filedlg2.hxx>
#include <vcl/rcid.h>

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

void FileDialog::AddFilter( const UniString& rFilter, const UniString& rMask, const UniString& rSysType )
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
