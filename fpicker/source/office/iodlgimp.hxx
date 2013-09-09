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
#ifndef _IODLGIMP_HXX
#define _IODLGIMP_HXX

#include <PlacesListBox.hxx>

#include <tools/string.hxx>
#include <vcl/fixed.hxx>
#include <vcl/lstbox.hxx>
#include <vcl/menubtn.hxx>
#include <vcl/timer.hxx>
#include <vcl/group.hxx>

#include <vector>
#include <boost/ptr_container/ptr_deque.hpp>

class CheckBox;
class SvtFileDialog;

#define FILEDIALOG_DEF_EXTSEP       ';'
#define FILEDIALOG_DEF_WILDCARD     '*'

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
    OUString m_aType;    // filter wildcard - if empty, the entry marks a group

public:
    SvtFileDialogFilter_Impl( const String& rName, const String& rType );
    ~SvtFileDialogFilter_Impl();

    const String&       GetName() const     { return m_aName; }
    const OUString&     GetType() const     { return m_aType; }
    const String        GetExtension() const    { return m_aType.copy( 2 ); }

    sal_Bool            isGroupSeparator() const    { return m_aType.isEmpty(); }
};

typedef boost::ptr_deque<SvtFileDialogFilter_Impl> SvtFileDialogFilterList_Impl;

enum SvtFileDlgMode
{
    FILEDLG_MODE_OPEN = 0,
    FILEDLG_MODE_SAVE = 1
};

enum SvtFileDlgType
{
    FILEDLG_TYPE_FILEDLG = 0,
    FILEDLG_TYPE_PATHDLG
};

class SvtFileDialogURLSelector : public MenuButton
{
private:
    SvtFileDialog*      m_pParent;
    PopupMenu*          m_pMenu;

protected:
    inline        SvtFileDialog*  GetDialogParent()       { return m_pParent; }

protected:

    virtual void    FillURLMenu( PopupMenu* _pMenu ) = 0;

protected:
    SvtFileDialogURLSelector( SvtFileDialog* _pParent, const ResId& _rResId, sal_uInt16 _nButtonId );
    ~SvtFileDialogURLSelector();

    virtual void        Activate();
};

class SvtUpButton_Impl : public SvtFileDialogURLSelector
{
private:
    std::vector<OUString> _aURLs;

public:
    SvtUpButton_Impl( SvtFileDialog* pParent, const ResId& rResId );
    ~SvtUpButton_Impl();

protected:
    virtual void        FillURLMenu( PopupMenu* _pMenu );
    virtual void        Select();
    virtual void        Click();
};

typedef sal_uInt8 SvtFileDlgState;

#define FILEDLG_STATE_NONE        ((SvtFileDlgState)0x00)
#define FILEDLG_STATE_REMOTE      ((SvtFileDlgState)0x01)

class SvtURLBox;
class SvtExpFileDlg_Impl
{
private:
    DECL_STATIC_LINK( SvtExpFileDlg_Impl, UnClickHdl, Button* );

private:
    ListBox*                        _pLbFilter;

    const SvtFileDialogFilter_Impl* _pCurFilter;
    String                          m_sCurrentFilterDisplayName;    // may differ from _pCurFilter->GetName in case it is a cached entry

    ::com::sun::star::uno::Sequence< OUString > _aBlackList;

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
    CheckBox*                       _pCbPassword;
    SvtURLBox*                      _pEdCurrentPath;
    CheckBox*                       _pCbAutoExtension;
    CheckBox*                       _pCbOptions;

    PlacesListBox*                  _pPlaces;
    PushButton*                     _pBtnConnectToServer;

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


    inline void             SetBlackList( const ::com::sun::star::uno::Sequence< OUString >& rBlackList ) { _aBlackList = rBlackList; }
    inline const ::com::sun::star::uno::Sequence< OUString >& GetBlackList() const { return _aBlackList; }
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
