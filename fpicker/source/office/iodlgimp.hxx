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
#ifndef _IODLGIMP_HXX
#define _IODLGIMP_HXX

#include <tools/string.hxx>
#include <vcl/fixed.hxx>
#include <vcl/lstbox.hxx>
#include <vcl/menubtn.hxx>
#include <vcl/timer.hxx>
#include <vcl/group.hxx>

#include "svl/svarray.hxx"

#include <vector>

//*****************************************************************************

class Accelerator;
class CheckBox;
class SvtFileDialog;
class SvStringsDtor;
class SvUShorts;

//*****************************************************************************

#define FILEDIALOG_DEF_EXTSEP       ';'
#define FILEDIALOG_DEF_WILDCARD     '*'
#define FILEDIALOG_DEF_IMAGEBORDER  10
#define FILEDIALOG_DEF_TIMEOUT      250

//*****************************************************************************
// SvtFileDialogFilter_Impl
//*****************************************************************************

/*  [Description]

    Instances of this class represent a filter.
*/

class SvtFileDialogFilter_Impl
{
private:
    String  m_aName;    // name of the entry
    String  m_aType;    // filter wildcard - if empty, the entry marks a group

public:
    SvtFileDialogFilter_Impl( const String& rName, const String& rType );
    ~SvtFileDialogFilter_Impl();

    const String&       GetName() const     { return m_aName; }
    const String&       GetType() const     { return m_aType; }
    const String        GetExtension() const    { return m_aType.Copy( 2 ); }

    sal_Bool            isGroupSeparator() const    { return 0 == m_aType.Len(); }
};

//*****************************************************************************
// SvtFileDialogFilterList_Impl
//*****************************************************************************

SV_DECL_PTRARR_DEL( SvtFileDialogFilterList_Impl, SvtFileDialogFilter_Impl*, 3, 3 )

//*****************************************************************************
// SvtFileDlgMode
//*****************************************************************************

enum SvtFileDlgMode
{
    FILEDLG_MODE_OPEN = 0,
    FILEDLG_MODE_SAVE = 1
};

//*****************************************************************************
// SvtFileDlgType
//*****************************************************************************

enum SvtFileDlgType
{
    FILEDLG_TYPE_FILEDLG = 0,
    FILEDLG_TYPE_PATHDLG
};

//*****************************************************************************
// SvtFileDialogURLSelector
//*****************************************************************************
class SvtFileDialogURLSelector : public MenuButton
{
private:
    SvtFileDialog*      m_pParent;
    PopupMenu*          m_pMenu;

protected:
    inline        SvtFileDialog*  GetDialogParent()       { return m_pParent; }

protected:
            void    OpenURL( const String& rURL );

    virtual void    FillURLMenu( PopupMenu* _pMenu ) = 0;

protected:
    SvtFileDialogURLSelector( SvtFileDialog* _pParent, const ResId& _rResId, sal_uInt16 _nButtonId );
    ~SvtFileDialogURLSelector();

    virtual void        Activate();
};

//*****************************************************************************
// SvtUpButton_Impl
//*****************************************************************************

class SvtUpButton_Impl : public SvtFileDialogURLSelector
{
private:
    SvStringsDtor*      _pURLs;

public:
    SvtUpButton_Impl( SvtFileDialog* pParent, const ResId& rResId );
    ~SvtUpButton_Impl();

protected:
    virtual void        FillURLMenu( PopupMenu* _pMenu );
    virtual void        Select();
    virtual void        Click();
};

//*****************************************************************************
// SvtTravelButton_Impl
//*****************************************************************************

class SvtTravelButton_Impl : public SvtFileDialogURLSelector
{
private:
    ::std::vector< String > m_aFavourites;

public:
    SvtTravelButton_Impl( SvtFileDialog* pParent, const ResId& rResId );
    ~SvtTravelButton_Impl();

            void            SetFavouriteLocations( const ::std::vector< String >& _rLocations );

protected:
    virtual void        FillURLMenu( PopupMenu* _pMenu );
    virtual void        Select();
    virtual void        Click();
};

//*****************************************************************************
// SvtFileDlgState
//*****************************************************************************

typedef sal_uInt8 SvtFileDlgState;

#define FILEDLG_STATE_NONE        ((SvtFileDlgState)0x00)
#define FILEDLG_STATE_REMOTE      ((SvtFileDlgState)0x01)

//*****************************************************************************
// SvtExpFileDlg_Impl
//*****************************************************************************
class SvtURLBox;
class SvtExpFileDlg_Impl
{
private:
    DECL_STATIC_LINK( SvtExpFileDlg_Impl, UnClickHdl, Button* );

private:
    ListBox*                        _pLbFilter;

    const SvtFileDialogFilter_Impl* _pCurFilter;
    String                          m_sCurrentFilterDisplayName;    // may differ from _pCurFilter->GetName in case it is a cached entry

    ::com::sun::star::uno::Sequence< ::rtl::OUString > _aBlackList;

public:
    SvtFileDialogFilterList_Impl*   _pFilter;
    SvtFileDialogFilter_Impl*       _pUserFilter;

    FixedText*                      _pFtFileName;
    SvtURLBox*                      _pEdFileName;

    FixedText*                      _pFtFileVersion;
    ListBox*                        _pLbFileVersion;

    FixedText*                      _pFtTemplates;
    ListBox*                        _pLbTemplates;

    FixedText*                      _pFtImageTemplates;
    ListBox*                        _pLbImageTemplates;

    FixedText*                      _pFtFileType;
    PushButton*                     _pBtnFileOpen;
    PushButton*                     _pBtnCancel;
    HelpButton*                     _pBtnHelp;
    SvtUpButton_Impl*               _pBtnUp;
    ImageButton*                    _pBtnNewFolder;
    SvtTravelButton_Impl*           _pBtnStandard;
    CheckBox*                       _pCbPassword;
    FixedText*                      _pFtCurrentPath;
    CheckBox*                       _pCbAutoExtension;
    CheckBox*                       _pCbOptions;

    SvtFileDlgMode                  _eMode;
    SvtFileDlgType                  _eDlgType;
    SvtFileDlgState                 _nState;
    WinBits                         _nStyle;

    String                          _aStdDir;

    // delay filter when traveling the filterbox
    Timer                           _aFilterTimer;

    // shows OpenHdl_Imp() if the open was triggered by a double click
    sal_Bool                        _bDoubleClick;
    sal_Bool                        m_bNeedDelayedFilterExecute;

    // list of the 5 most recently used filters
    // Defaultfilter for <All> oder <All ...>
    const SvtFileDialogFilter_Impl* _pDefaultFilter;

    // MultiSelection?
    sal_Bool                        _bMultiSelection;

    // remember fixsizes for resize
    long                            _nFixDeltaHeight;
    Size                            _a6Size;
    Size                            _aDlgSize;
    String                          _aIniKey;

    sal_Bool                        _bFolderHasOpened;

                            SvtExpFileDlg_Impl( WinBits nBits );
                            ~SvtExpFileDlg_Impl();


    inline void             SetBlackList( const ::com::sun::star::uno::Sequence< ::rtl::OUString >& rBlackList ) { _aBlackList = rBlackList; }
    inline const ::com::sun::star::uno::Sequence< ::rtl::OUString >& GetBlackList() const { return _aBlackList; }
    void                    SetStandardDir( const String& _rDir );
    inline const String&    GetStandardDir() const          { return _aStdDir; }
    inline void             DisableFilterBoxAutoWidth()     { _pLbFilter->EnableDDAutoWidth( sal_False ); }

    // ------------------------------------------
    // access to the filter listbox only as Control* - we want to maintain the entries/userdata ourself
            Control*        GetFilterListControl()          { return _pLbFilter; }
            const Control*  GetFilterListControl() const    { return _pLbFilter; }
            void            CreateFilterListControl( Window* _pParent, const ResId& _rId );
    inline  void            SetFilterListSelectHdl( const Link& _rHandler );

    // inits the listbox for the filters from the filter list (_pFilter)
            void            ClearFilterList( );
            void            InitFilterList( );
    inline  sal_Bool        HasFilterListEntry( const String& _rFilterName );
    inline  void            SelectFilterListEntry( const String& _rFilterName );
    inline  void            SetNoFilterListSelection( );
            void            InsertFilterListEntry( const SvtFileDialogFilter_Impl* _pFilterDesc );
                                // _pFilterDesc must already have been added to _pFilter
    inline  SvtFileDialogFilter_Impl*   GetSelectedFilterEntry( String& /* [out] */ _rDisplayName ) const;
    inline  sal_Bool        IsFilterListTravelSelect() const;

    // ------------------------------------------
    // access to the current filter via methods only - need to care for consistency between _pCurFilter and m_sCurrentFilterDisplayName
    inline      const SvtFileDialogFilter_Impl*     GetCurFilter( ) const;
    inline      const String&                       GetCurFilterDisplayName() const;
                void                                SetCurFilter( SvtFileDialogFilter_Impl* _pFilter, const String& _rDisplayName );
    inline      void                                SetCurFilter( SvtFileDialogFilter_Impl* _pFilter );
};

inline void SvtExpFileDlg_Impl::SetFilterListSelectHdl( const Link& _rHandler )
{
    _pLbFilter->SetSelectHdl( _rHandler );
}

inline sal_Bool SvtExpFileDlg_Impl::HasFilterListEntry( const String& _rFilterName )
{
    return ( LISTBOX_ENTRY_NOTFOUND != _pLbFilter->GetEntryPos( _rFilterName ) );
}

inline void SvtExpFileDlg_Impl::SelectFilterListEntry( const String& _rFilterName )
{
    _pLbFilter->SelectEntry( _rFilterName );
}

inline  void SvtExpFileDlg_Impl::SetNoFilterListSelection( )
{
    _pLbFilter->SetNoSelection( );
}

inline SvtFileDialogFilter_Impl* SvtExpFileDlg_Impl::GetSelectedFilterEntry( String& _rDisplayName ) const
{
    _rDisplayName = _pLbFilter->GetSelectEntry();
    return static_cast< SvtFileDialogFilter_Impl* >( _pLbFilter->GetEntryData ( _pLbFilter->GetSelectEntryPos() ) );
}

inline sal_Bool SvtExpFileDlg_Impl::IsFilterListTravelSelect() const
{
    return _pLbFilter->IsTravelSelect();
}

inline const SvtFileDialogFilter_Impl* SvtExpFileDlg_Impl::GetCurFilter( ) const
{
    return _pCurFilter;
}

inline const String& SvtExpFileDlg_Impl::GetCurFilterDisplayName() const
{
    return m_sCurrentFilterDisplayName;
}

inline  void SvtExpFileDlg_Impl::SetCurFilter( SvtFileDialogFilter_Impl* pFilter )
{
    SetCurFilter( pFilter, pFilter->GetName() );
}

#endif // #ifndef _IODLGIMP_HXX


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
