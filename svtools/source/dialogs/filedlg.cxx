/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */


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
