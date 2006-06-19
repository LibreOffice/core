/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: filedlg.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: hr $ $Date: 2006-06-19 20:16:32 $
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

#ifndef _SVT_FILEDLG_HXX
#define _SVT_FILEDLG_HXX

#ifndef INCLUDED_SVTDLLAPI_H
#include "svtools/svtdllapi.h"
#endif

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
                        PathDialog( Window* pParent, WinBits nWinStyle = 0, BOOL bCreateDir = TRUE );
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
    USHORT              GetFilterCount() const;
    UniString               GetFilterName( USHORT nPos ) const;
    UniString               GetFilterType( USHORT nPos ) const;

    void                SetFileSelectHdl( const Link& rLink ) { aFileHdlLink = rLink; }
    const Link&         GetFileSelectHdl() const { return aFileHdlLink; }
    void                SetFilterSelectHdl( const Link& rLink ) { aFilterHdlLink = rLink; }
    const Link&         GetFilterSelectHdl() const { return aFilterHdlLink; }

    void                SetOkButtonText( const UniString& rText );
    void                SetCancelButtonText( const UniString& rText );
};

#endif  // _FILEDLG_HXX
