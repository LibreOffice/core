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
#ifndef INCLUDED_FPICKER_SOURCE_OFFICE_IODLGIMP_HXX
#define INCLUDED_FPICKER_SOURCE_OFFICE_IODLGIMP_HXX

#include <PlacesListBox.hxx>

#include <vcl/fixed.hxx>
#include <vcl/lstbox.hxx>
#include <vcl/menubtn.hxx>
#include <vcl/timer.hxx>
#include <vcl/group.hxx>

#include <deque>
#include <memory>
#include <vector>

class CheckBox;
class SvtFileDialog;

#define FILEDIALOG_DEF_EXTSEP       ';'
#define FILEDIALOG_DEF_WILDCARD     '*'


// SvtFileDialogFilter_Impl


/*  [Description]

    Instances of this class represent a filter.
*/

class SvtFileDialogFilter_Impl
{
private:
    OUString m_aName;    // name of the entry
    OUString m_aType;    // filter wildcard - if empty, the entry marks a group

public:
    SvtFileDialogFilter_Impl( const OUString& rName, const OUString& rType );
    ~SvtFileDialogFilter_Impl();

    const OUString&     GetName() const     { return m_aName; }
    const OUString&     GetType() const     { return m_aType; }
    const OUString      GetExtension() const    { return m_aType.getLength() > 2 ? m_aType.copy( 2 ) : OUString(); }

    bool            isGroupSeparator() const    { return m_aType.isEmpty(); }
};

typedef std::deque<std::unique_ptr<SvtFileDialogFilter_Impl>> SvtFileDialogFilterList_Impl;

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
public:
    virtual ~SvtFileDialogURLSelector();
    virtual void dispose() override;
private:
    VclPtr<SvtFileDialog>      m_pDlg;
    PopupMenu*          m_pMenu;

protected:
    inline        SvtFileDialog*  GetDialogParent()       { return m_pDlg; }

    virtual void    FillURLMenu( PopupMenu* _pMenu ) = 0;

    SvtFileDialogURLSelector( vcl::Window* _pParent, SvtFileDialog* _pDlg, WinBits nBits, sal_uInt16 _nButtonId );

    virtual void        Activate() override;
};

class SvtUpButton_Impl : public SvtFileDialogURLSelector
{
private:
    std::vector<OUString> _aURLs;

public:
    SvtUpButton_Impl( vcl::Window* pParent, SvtFileDialog* pDlg, WinBits nBits );
    virtual ~SvtUpButton_Impl();

protected:
    virtual void        FillURLMenu( PopupMenu* _pMenu ) override;
    virtual void        Select() override;
    virtual void        Click() override;
    virtual Size        GetOptimalSize() const override;
};

typedef sal_uInt8 SvtFileDlgState;

#define FILEDLG_STATE_NONE        ((SvtFileDlgState)0x00)
#define FILEDLG_STATE_REMOTE      ((SvtFileDlgState)0x01)

class SvtURLBox;
class SvtExpFileDlg_Impl
{
private:
    const SvtFileDialogFilter_Impl* _pCurFilter;
    OUString                        m_sCurrentFilterDisplayName;    // may differ from _pCurFilter->GetName in case it is a cached entry

    css::uno::Sequence< OUString > _aBlackList;

public:
    SvtFileDialogFilterList_Impl    m_aFilter;
    SvtFileDialogFilter_Impl*       _pUserFilter;

    VclPtr<FixedText>                      _pFtFileName;
    VclPtr<SvtURLBox>                      _pEdFileName;

    VclPtr<FixedText>                      _pFtFileVersion;
    VclPtr<ListBox>                        _pLbFileVersion;

    VclPtr<FixedText>                      _pFtTemplates;
    VclPtr<ListBox>                        _pLbTemplates;

    VclPtr<FixedText>                      _pFtImageTemplates;
    VclPtr<ListBox>                        _pLbImageTemplates;

    VclPtr<FixedText>                      _pFtFileType;
    VclPtr<ListBox>                        _pLbFilter;
    VclPtr<PushButton>                     _pBtnFileOpen;
    VclPtr<PushButton>                     _pBtnCancel;
    VclPtr<HelpButton>                     _pBtnHelp;
    VclPtr<SvtUpButton_Impl>               _pBtnUp;
    VclPtr<PushButton>                     _pBtnNewFolder;
    VclPtr<CheckBox>                       _pCbPassword;
    VclPtr<SvtURLBox>                      _pEdCurrentPath;
    VclPtr<CheckBox>                       _pCbAutoExtension;
    VclPtr<CheckBox>                       _pCbOptions;

    VclPtr<PlacesListBox>                  _pPlaces;
    VclPtr<PushButton>                     _pBtnConnectToServer;

    SvtFileDlgMode                  _eMode;
    SvtFileDlgType                  _eDlgType;
    SvtFileDlgState                 _nState;
    WinBits                         _nStyle;

    OUString                        _aStdDir;

    // delay filter when traveling the filterbox
    Timer                           _aFilterTimer;

    // shows OpenHdl_Imp() if the open was triggered by a double click
    bool                        _bDoubleClick;
    bool                        m_bNeedDelayedFilterExecute;

    // list of the 5 most recently used filters
    // Defaultfilter for <All> oder <All ...>
    const SvtFileDialogFilter_Impl* _pDefaultFilter;

    // MultiSelection?
    bool                        _bMultiSelection;

    // remember sizes
    OUString                        _aIniKey;

                            SvtExpFileDlg_Impl( WinBits nBits );
                            ~SvtExpFileDlg_Impl();


    inline void             SetBlackList( const css::uno::Sequence< OUString >& rBlackList ) { _aBlackList = rBlackList; }
    inline const css::uno::Sequence< OUString >& GetBlackList() const { return _aBlackList; }
    void                    SetStandardDir( const OUString& _rDir );
    inline const OUString&  GetStandardDir() const          { return _aStdDir; }
    inline void             DisableFilterBoxAutoWidth()     { _pLbFilter->EnableDDAutoWidth( false ); }


    // access to the filter listbox only as Control* - we want to maintain the entries/userdata ourself
            Control*        GetFilterListControl()          { return _pLbFilter; }
            const Control*  GetFilterListControl() const    { return _pLbFilter; }
    inline  void            SetFilterListSelectHdl( const Link<ListBox&,void>& _rHandler );

    // inits the listbox for the filters from the filter list (_pFilter)
            void            ClearFilterList( );
            void            InitFilterList( );
    inline  bool        HasFilterListEntry( const OUString& _rFilterName );
    inline  void            SelectFilterListEntry( const OUString& _rFilterName );
    inline  void            SetNoFilterListSelection( );
            void            InsertFilterListEntry( const SvtFileDialogFilter_Impl* _pFilterDesc );
                                // _pFilterDesc must already have been added to _pFilter
    inline  SvtFileDialogFilter_Impl*   GetSelectedFilterEntry( OUString& /* [out] */ _rDisplayName ) const;
    inline  bool        IsFilterListTravelSelect() const;


    // access to the current filter via methods only - need to care for consistency between _pCurFilter and m_sCurrentFilterDisplayName
    inline      const SvtFileDialogFilter_Impl*     GetCurFilter( ) const;
    inline      const OUString&                     GetCurFilterDisplayName() const;
                void                                SetCurFilter( SvtFileDialogFilter_Impl* _pFilter, const OUString& _rDisplayName );
};

inline void SvtExpFileDlg_Impl::SetFilterListSelectHdl( const Link<ListBox&,void>& _rHandler )
{
    _pLbFilter->SetSelectHdl( _rHandler );
}

inline bool SvtExpFileDlg_Impl::HasFilterListEntry( const OUString& _rFilterName )
{
    return ( LISTBOX_ENTRY_NOTFOUND != _pLbFilter->GetEntryPos( _rFilterName ) );
}

inline void SvtExpFileDlg_Impl::SelectFilterListEntry( const OUString& _rFilterName )
{
    _pLbFilter->SelectEntry( _rFilterName );
}

inline  void SvtExpFileDlg_Impl::SetNoFilterListSelection( )
{
    _pLbFilter->SetNoSelection( );
}

inline SvtFileDialogFilter_Impl* SvtExpFileDlg_Impl::GetSelectedFilterEntry( OUString& _rDisplayName ) const
{
    _rDisplayName = _pLbFilter->GetSelectEntry();
    return static_cast< SvtFileDialogFilter_Impl* >( _pLbFilter->GetSelectEntryData () );
}

inline bool SvtExpFileDlg_Impl::IsFilterListTravelSelect() const
{
    return _pLbFilter->IsTravelSelect();
}

inline const SvtFileDialogFilter_Impl* SvtExpFileDlg_Impl::GetCurFilter( ) const
{
    return _pCurFilter;
}

inline const OUString& SvtExpFileDlg_Impl::GetCurFilterDisplayName() const
{
    return m_sCurrentFilterDisplayName;
}

#endif // INCLUDED_FPICKER_SOURCE_OFFICE_IODLGIMP_HXX


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
