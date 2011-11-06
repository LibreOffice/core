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
