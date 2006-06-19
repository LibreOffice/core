/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: filedlg2.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: hr $ $Date: 2006-06-19 20:59:47 $
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

#ifndef _FILEDLG2_HXX
#define _FILEDLG2_HXX

#ifndef _TOOLS_DEBUG_HXX //autogen
#include <tools/debug.hxx>
#endif
#ifndef _FSYS_HXX //autogen
#include <tools/fsys.hxx>
#endif
#ifndef _SV_BUTTON_HXX //autogen wg. PushButton
#include <vcl/button.hxx>
#endif
#ifndef _VCL_UNOHELP_HXX
#include <vcl/unohelp.hxx>
#endif

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

#ifndef _SV_LSTBOX_HXX
#include <vcl/lstbox.hxx>
#endif // _SV_LSTBOX_HXX

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

    USHORT              nOwnChilds;

    DirEntry            aPath;          // aktuell angewaehlter Pfad
    USHORT              nDirCount;      // Anzahl der Verzeichnis-
                                        // Verschachtelungen

    ::com::sun::star::uno::Reference< ::com::sun::star::i18n::XCollator >
                        xCollator;

protected:

    virtual void        UpdateEntries( const BOOL bWithDirs );
    void                UpdateDirs( const DirEntry& rTmpPath );

    BOOL                IsFileOk( const DirEntry& rDirEntry );
    void                InitControls();

    DECL_LINK(          SelectHdl, ListBox * );
    DECL_LINK(          DblClickHdl, ListBox * );
    DECL_LINK(          ClickHdl, Button * );

public:
                        ImpPathDialog( PathDialog* pDlg, RESOURCE_TYPE nType, BOOL bCreateDir );
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
    USHORT              nCurFilter;     // aktueller Filter

    BOOL                bOpen;          // TRUE = Open; FALSE = SAVEAS

protected:
    void                InitControls();

    String              ExtendFileName( DirEntry aEntry ) const;

    DECL_LINK(          SelectHdl, ListBox * );
    DECL_LINK(          DblClickHdl, ListBox * );
    DECL_LINK(          ClickHdl, Button * );

    virtual void        UpdateEntries( const BOOL bWithDirs );
    BOOL                IsFileOk( const DirEntry& rDirEntry );

public:
                        ImpFileDialog( PathDialog* pDlg, WinBits nStyle, RESOURCE_TYPE nType );
    virtual             ~ImpFileDialog();

    void                AddFilter( const String& rFilter, const String& rMask );
    void                RemoveFilter( const String& rFilter );
    void                RemoveAllFilter();
    void                SetCurFilter( const String& rFilter );
    String              GetCurFilter() const;

    USHORT              GetFilterCount() const  { return (USHORT)aFilterList.Count(); }
    inline String       GetFilterName( USHORT nPos ) const;
    inline String       GetFilterType( USHORT nPos ) const;

    virtual void        SetPath( const String& rPath );
    virtual void        SetPath( const Edit& rEdit );
    virtual String      GetPath() const;

    virtual void        PreExecute();

    FileDialog*     GetFileDialog() const { return (FileDialog*)GetPathDialog(); }
};

inline String ImpFileDialog::GetFilterName( USHORT nPos ) const
{
    String aName;
    ImpFilterItem* pItem = aFilterList.GetObject( nPos );
    if ( pItem )
        aName = pItem->aName;
    return aName;
}

inline String ImpFileDialog::GetFilterType( USHORT nPos ) const
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
    void            CreateDialog( PathDialog* pCreateFrom, WinBits nStyle, RESOURCE_TYPE nType, BOOL bCreate );

    void            SetOkButtonText( const String& rText ) { pDlg->SetOkButtonText( rText ); }  // ihr habts ja nicht anders gewollt
    void            SetCancelButtonText( const String& rText ) { pDlg->SetCancelButtonText( rText ); }

};

#endif // _FILEDLG2_HXX
