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

#ifndef FILEDLG2_HXX
#define FILEDLG2_HXX

#include <tools/fsys.hxx>
#include <vcl/button.hxx>
#include <vcl/unohelp.hxx>
#include <vector>

class FixedText;
class Edit;
class ListBox;
class ListBox;
class Button;

class PathDialog;
class ImpPathDialog;

struct ImpFilterItem
{
    String aName;
    String aMask;

    ImpFilterItem( const String & rFilter, const String & rMask )
    {
        aName = rFilter;
        aMask = rMask;
    }
};

typedef ::std::vector< ImpFilterItem* > ImpFilterList;
#include <vcl/lstbox.hxx>

class KbdListBox : public ListBox
{
public:

                KbdListBox( Window* pParent, WinBits nStyle = WB_BORDER )
                    : ListBox ( pParent, nStyle )
                {};

virtual long    PreNotify( NotifyEvent& rNEvt );

};


class ImpPathDialog
{
private:
    PathDialog*         pSvPathDialog;
    Edit*               pEdit;
    FixedText*          pDirTitel;
    KbdListBox*         pDirList;
    FixedText*          pDirPath;
    ListBox*            pDriveList;
    FixedText*          pDriveTitle;
    PushButton*         pLoadBtn;
    PushButton*         pOkBtn;
    PushButton*         pCancelBtn;
    PushButton*         pHomeBtn;
    PushButton*         pNewDirBtn;

    sal_uInt16              nOwnChildren;

    DirEntry            aPath;          // aktuell angewaehlter Pfad
    sal_uInt16              nDirCount;      // Anzahl der Verzeichnis-
                                        // Verschachtelungen
protected:

    virtual void        UpdateEntries( const sal_Bool bWithDirs );
    void                UpdateDirs( const DirEntry& rTmpPath );

    sal_Bool                IsFileOk( const DirEntry& rDirEntry );
    void                InitControls();

    DECL_LINK(          SelectHdl, ListBox * );
    DECL_LINK(          DblClickHdl, ListBox * );
    DECL_LINK(          ClickHdl, Button * );

public:
                        ImpPathDialog( PathDialog* pDlg, RESOURCE_TYPE nType, sal_Bool bCreateDir );
    virtual             ~ImpPathDialog();

    virtual void        SetPath( const String& rPath );
    virtual String      GetPath() const;

    virtual void        PreExecute();
    virtual void        PostExecute();

    PathDialog*     GetPathDialog() const   { return pSvPathDialog; }

    void            SetOkButtonText( const String& rText ) { pOkBtn->SetText( rText ); }
    void            SetCancelButtonText( const String& rText ) { pCancelBtn->SetText( rText ); }

};

class ImpSvFileDlg
{
private:
    ImpPathDialog*  pDlg;

public:
                    ImpSvFileDlg()      { pDlg = 0; }
                    ~ImpSvFileDlg()     { delete pDlg; }

    ImpPathDialog*  GetDialog() const   { return pDlg; }
    void            CreatePathDialog(PathDialog* pCreateFrom, bool bCreate);

    void            SetOkButtonText( const String& rText ) { pDlg->SetOkButtonText( rText ); }  // ihr habts ja nicht anders gewollt
    void            SetCancelButtonText( const String& rText ) { pDlg->SetCancelButtonText( rText ); }

};

#endif // FILEDLG2_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
