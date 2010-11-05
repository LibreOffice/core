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

#ifndef _FILEDLG2_HXX
#define _FILEDLG2_HXX

#include <tools/debug.hxx>
#include <tools/fsys.hxx>
#ifndef _SV_BUTTON_HXX //autogen wg. PushButton
#include <vcl/button.hxx>
#endif
#include <vcl/unohelp.hxx>

class FixedText;
class Edit;
class ListBox;
class ListBox;
class Button;

class PathDialog;
class FileDialog;
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

DECLARE_LIST( ImpFilterList, ImpFilterItem* )
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
    friend class ImpFileDialog;

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

    sal_uInt16              nOwnChilds;

    DirEntry            aPath;          // aktuell angewaehlter Pfad
    sal_uInt16              nDirCount;      // Anzahl der Verzeichnis-
                                        // Verschachtelungen

    ::com::sun::star::uno::Reference< ::com::sun::star::i18n::XCollator >
                        xCollator;

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
    virtual void        SetPath( const Edit& rEdit );
    virtual String      GetPath() const;

    virtual void        PreExecute();
    virtual void        PostExecute();

    PathDialog*     GetPathDialog() const   { return pSvPathDialog; }

    void            SetOkButtonText( const String& rText ) { pOkBtn->SetText( rText ); }
    void            SetCancelButtonText( const String& rText ) { pCancelBtn->SetText( rText ); }

};


class ImpFileDialog : public ImpPathDialog
{
private:
    FixedText*          pFileTitel;
    ListBox*            pFileList;
    FixedText*          pTypeTitel;
    ListBox*            pTypeList;

    WildCard            aMask;          // aktuelle Maske

    ImpFilterList       aFilterList;    // Filterliste
    sal_uInt16              nCurFilter;     // aktueller Filter

    sal_Bool                bOpen;          // sal_True = Open; sal_False = SAVEAS

protected:
    void                InitControls();

    String              ExtendFileName( DirEntry aEntry ) const;

    DECL_LINK(          SelectHdl, ListBox * );
    DECL_LINK(          DblClickHdl, ListBox * );
    DECL_LINK(          ClickHdl, Button * );

    virtual void        UpdateEntries( const sal_Bool bWithDirs );
    sal_Bool                IsFileOk( const DirEntry& rDirEntry );

public:
                        ImpFileDialog( PathDialog* pDlg, WinBits nStyle, RESOURCE_TYPE nType );
    virtual             ~ImpFileDialog();

    void                AddFilter( const String& rFilter, const String& rMask );
    void                RemoveFilter( const String& rFilter );
    void                RemoveAllFilter();
    void                SetCurFilter( const String& rFilter );
    String              GetCurFilter() const;

    sal_uInt16              GetFilterCount() const  { return (sal_uInt16)aFilterList.Count(); }
    inline String       GetFilterName( sal_uInt16 nPos ) const;
    inline String       GetFilterType( sal_uInt16 nPos ) const;

    virtual void        SetPath( const String& rPath );
    virtual void        SetPath( const Edit& rEdit );
    virtual String      GetPath() const;

    virtual void        PreExecute();

    FileDialog*     GetFileDialog() const { return (FileDialog*)GetPathDialog(); }
};

inline String ImpFileDialog::GetFilterName( sal_uInt16 nPos ) const
{
    String aName;
    ImpFilterItem* pItem = aFilterList.GetObject( nPos );
    if ( pItem )
        aName = pItem->aName;
    return aName;
}

inline String ImpFileDialog::GetFilterType( sal_uInt16 nPos ) const
{
    String aFilterMask;
    ImpFilterItem* pItem = aFilterList.GetObject( nPos );
    if ( pItem )
        aFilterMask = pItem->aMask;
    return aFilterMask;
}

class ImpSvFileDlg
{
private:
    ImpPathDialog*  pDlg;

public:
                    ImpSvFileDlg()      { pDlg = 0; }
                    ~ImpSvFileDlg()     { delete pDlg; }

    ImpPathDialog*  GetDialog() const   { return pDlg; }
    void            CreateDialog( PathDialog* pCreateFrom, WinBits nStyle, RESOURCE_TYPE nType, sal_Bool bCreate );

    void            SetOkButtonText( const String& rText ) { pDlg->SetOkButtonText( rText ); }  // ihr habts ja nicht anders gewollt
    void            SetCancelButtonText( const String& rText ) { pDlg->SetCancelButtonText( rText ); }

};

#endif // _FILEDLG2_HXX
