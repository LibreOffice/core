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


#include <svtools/filedlg.hxx>
#include <filedlg2.hxx>

PathDialog::PathDialog( Window* _pParent, WinBits nStyle, sal_Bool bCreateDir ) :
    ModalDialog( _pParent, WB_STDMODAL | nStyle )
{
    pImpFileDlg = new ImpSvFileDlg;
    pImpFileDlg->CreatePathDialog(this, bCreateDir);
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

rtl::OUString PathDialog::GetPath() const
{
    return pImpFileDlg->GetDialog()->GetPath();
}

void PathDialog::SetPath( const rtl::OUString& rPath )
{
    pImpFileDlg->GetDialog()->SetPath( rPath );
}

long PathDialog::OK()
{
    if( aOKHdlLink.IsSet() )
        return aOKHdlLink.Call( this );
    else
        return sal_True;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
