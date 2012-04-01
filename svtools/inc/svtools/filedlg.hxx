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

#ifndef SVT_FILEDLG_HXX
#define SVT_FILEDLG_HXX

#include "svtools/svtdllapi.h"

#include <vcl/dialog.hxx>

class Edit;
class ImpSvFileDlg;

// --------------
// - SvPathDialog -
// --------------

class SVT_DLLPUBLIC PathDialog : public ModalDialog
{
private:
    ImpSvFileDlg*       pImpFileDlg;    // Implementation
    Link                aOKHdlLink;     // Link to OK-Handler

public:
                        PathDialog( Window* pParent, WinBits nWinStyle = 0, sal_Bool bCreateDir = sal_True );
                        ~PathDialog();

    virtual long        OK();

    void                SetPath( const rtl::OUString& rNewPath );
    rtl::OUString       GetPath() const;

    void                SetOKHdl( const Link& rLink ) { aOKHdlLink = rLink; }
    const Link&         GetOKHdl() const { return aOKHdlLink; }

    virtual short       Execute();
};

#endif  // FILEDLG_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
