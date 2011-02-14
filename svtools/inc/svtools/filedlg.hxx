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

#ifndef _SVT_FILEDLG_HXX
#define _SVT_FILEDLG_HXX

#include "svtools/svtdllapi.h"

#ifndef _DIALOG_HXX
#include <vcl/dialog.hxx>
#endif

class Edit;
class ImpSvFileDlg;

// --------------
// - SvPathDialog -
// --------------

class SVT_DLLPUBLIC PathDialog : public ModalDialog
{
private:
    friend class FileDialog;    // Imp...

    ImpSvFileDlg*       pImpFileDlg;    // Implementation
    Link                aOKHdlLink;     // Link zum OK-Handler

protected:
    UniString           aDfltExt;       // Default - Extension

public:
                        PathDialog( Window* pParent, WinBits nWinStyle = 0, sal_Bool bCreateDir = sal_True );
                        ~PathDialog();

    virtual long        OK();

    void                SetPath( const UniString& rNewPath );
    void                SetPath( const Edit& rEdit );
    UniString               GetPath() const;

    void                SetOKHdl( const Link& rLink ) { aOKHdlLink = rLink; }
    const Link&         GetOKHdl() const { return aOKHdlLink; }

    virtual short       Execute();
};

// --------------
// - SvFileDialog -
// --------------

class SVT_DLLPUBLIC FileDialog : public PathDialog
{
private:
    Link                aFileHdlLink;   // Link zum FileSelect-Handler
    Link                aFilterHdlLink; // Link zum FilterSelect-Handler

public:
                        FileDialog( Window* pParent, WinBits nWinStyle );
                        ~FileDialog();

    virtual void        FileSelect();
    virtual void        FilterSelect();

    void                SetDefaultExt( const UniString& rExt ) { aDfltExt = rExt; }
    const UniString&        GetDefaultExt() const { return aDfltExt; }
    void                AddFilter( const UniString& rFilter, const UniString& rType );
    void                RemoveFilter( const UniString& rFilter );
    void                RemoveAllFilter();
    void                SetCurFilter( const UniString& rFilter );
    UniString               GetCurFilter() const;
    sal_uInt16              GetFilterCount() const;
    UniString               GetFilterName( sal_uInt16 nPos ) const;
    UniString               GetFilterType( sal_uInt16 nPos ) const;

    void                SetFileSelectHdl( const Link& rLink ) { aFileHdlLink = rLink; }
    const Link&         GetFileSelectHdl() const { return aFileHdlLink; }
    void                SetFilterSelectHdl( const Link& rLink ) { aFilterHdlLink = rLink; }
    const Link&         GetFilterSelectHdl() const { return aFilterHdlLink; }

    void                SetOkButtonText( const UniString& rText );
    void                SetCancelButtonText( const UniString& rText );
};

#endif  // _FILEDLG_HXX
