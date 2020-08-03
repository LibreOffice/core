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
#pragma once

#include <vcl/weld.hxx>
#include <com/sun/star/beans/StringPair.hpp>
#include <com/sun/star/uno/Any.hxx>
#include <com/sun/star/uno/Sequence.hxx>
#include "pickercallbacks.hxx"

class SvTabListBox;
class SvtFileView;
class SvtFileDialogFilter_Impl;


enum class PickerFlags {
    NONE              = 0x000000,
    AutoExtension     = 0x000001,
    FilterOptions     = 0x000002,
    ShowVersions      = 0x000004,
    InsertAsLink      = 0x000008,
    ShowPreview       = 0x000010,
    Templates         = 0x000020,
    PlayButton        = 0x000040,
    Selection         = 0x000080,
    ImageTemplate     = 0x000100,
    PathDialog        = 0x000200,
    Open              = 0x000400,
    SaveAs            = 0x000800,
    Password          = 0x001000,
    ReadOnly          = 0x002000,
    MultiSelection    = 0x004000,
    ImageAnchor       = 0x008000,
};
namespace o3tl {
    template<> struct typed_flags<PickerFlags> : is_typed_flags<PickerFlags, 0x00ffff> {};
}

#define FILEDIALOG_FILTER_ALL   "*.*"

// SvtFileDialog_Base

class SvtFileDialog_Base : public weld::GenericDialogController, public ::svt::IFilePickerController
{
public:
    SvtFileDialog_Base(weld::Window* pParent, const OUString& rUIXMLDescription, const OString& rID)
        : weld::GenericDialogController(pParent, rUIXMLDescription, rID)
    {
    }

    virtual bool PrepareExecute() { return true ; }

    virtual SvtFileView* GetView() = 0;

    virtual void SetHasFilename( bool bHasFilename ) = 0;
    virtual void SetDenyList( const css::uno::Sequence< OUString >& rDenyList ) = 0;
    virtual const css::uno::Sequence< OUString >& GetDenyList() const = 0;
    virtual void SetStandardDir( const OUString& rStdDir ) = 0;
    virtual const OUString& GetStandardDir() const = 0;
    virtual void SetPath( const OUString& rNewURL ) = 0;
    virtual const OUString& GetPath() = 0;
    virtual std::vector<OUString> GetPathList() const = 0;
    virtual bool ContentIsFolder( const OUString& rURL ) = 0;

    virtual OUString getCurrentFileText() const = 0;
    virtual void setCurrentFileText( const OUString& rText, bool bSelectAll = false ) = 0;

    virtual void AddFilter( const OUString& rFilter, const OUString& rType ) = 0;
    virtual void AddFilterGroup( const OUString& _rFilter,
                                const css::uno::Sequence< css::beans::StringPair >& rFilters ) = 0;
    virtual OUString GetCurFilter() const = 0;
    virtual void SetCurFilter( const OUString& rFilter ) = 0;
    virtual void FilterSelect() = 0;

    virtual void SetFileCallback( ::svt::IFilePickerListener *pNotifier ) = 0;
    virtual void onAsyncOperationStarted() = 0;
    virtual void onAsyncOperationFinished() = 0;
    virtual void UpdateControls( const OUString& rURL ) = 0;

    virtual void EnableAutocompletion( bool _bEnable = true ) = 0;

    virtual sal_Int32 getAvailableWidth() = 0;
    virtual sal_Int32 getAvailableHeight() = 0;

    virtual void setImage( const css::uno::Any& rImage ) = 0;

    virtual bool getShowState() = 0;
};

#define FILE_SELECTION_CHANGED  1
#define DIRECTORY_CHANGED       2
#define CTRL_STATE_CHANGED      4
#define DIALOG_SIZE_CHANGED     5


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
