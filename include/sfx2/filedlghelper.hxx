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
#ifndef INCLUDED_SFX2_FILEDLGHELPER_HXX
#define INCLUDED_SFX2_FILEDLGHELPER_HXX

#include <sal/config.h>
#include <sfx2/dllapi.h>
#include <sal/types.h>
#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/uno/Sequence.hxx>
#include <rtl/ustring.hxx>
#include <tools/errcode.hxx>
#include <vcl/dialog.hxx>
#include <vcl/edit.hxx>
#include <vcl/button.hxx>
#include <vcl/graph.hxx>
#include <sfx2/sfxuno.hxx>
#include <sfx2/docfilt.hxx>

#include <memory>

namespace com
{
    namespace sun
    {
        namespace star
        {
            namespace ui
            {
                namespace dialogs
                {
                    class XFilePicker2;
                    class XFilePickerListener;
                    struct FilePickerEvent;
                    struct DialogClosedEvent;
                }
            }
        }
    }
}

class SfxItemSet;
namespace vcl { class Window; }

// the SFXWB constants are for the nFlags parameter of the constructor
#define SFXWB_INSERT            0x04000000L     // turn Open into Insert dialog
#define SFXWB_EXPORT            0x40000000L     // turn Save into Export dialog
#define SFXWB_SAVEACOPY         0x00400000L     // turn Save into Save a Copy dialog
#define SFXWB_MULTISELECTION    0x20000000L
#define SFXWB_GRAPHIC           0x00800000L     // register graphic formats

#define FILEDIALOG_FILTER_ALL   "*.*"

namespace sfx2 {

class FileDialogHelper_Impl;

class SFX2_DLLPUBLIC FileDialogHelper
{
public:
    enum Context                        // context where the FileDialogHelper is used
    {
        UNKNOWN_CONTEXT,                // unknown context
        SW_INSERT_GRAPHIC,              // insert graphic in writer
        SW_INSERT_SOUND,                // insert sound in writer
        SW_INSERT_VIDEO,                // insert video in writer
        SC_INSERT_GRAPHIC,              // insert graphic in calc
        SC_INSERT_SOUND,                // insert sound in calc
        SC_INSERT_VIDEO,                // insert video in calc
        SD_INSERT_GRAPHIC,              // insert graphic in draw
        SD_INSERT_SOUND,                // insert sound in draw
        SD_INSERT_VIDEO,                // insert video in draw
        SD_EXPORT,                      // export in draw
        SI_EXPORT,                      // export in impress
        SW_EXPORT                       // export in writer
    };

private:
    Link<FileDialogHelper*,void>  m_aDialogClosedLink;
    ErrCode m_nError;

    css::uno::Reference < css::ui::dialogs::XFilePickerListener > mxImp;
    FileDialogHelper_Impl   *mpImp;


public:
                            FileDialogHelper( sal_Int16 nDialogType,
                                              sal_Int64 nFlags,
                                              vcl::Window* _pPreferredParent = nullptr );

                            FileDialogHelper( sal_Int16 nDialogType,
                                              sal_Int64 nFlags,
                                              const OUString& rFactory,
                                              SfxFilterFlags nMust = SfxFilterFlags::NONE,
                                              SfxFilterFlags nDont = SfxFilterFlags::NONE );

                            FileDialogHelper( sal_Int16 nDialogType,
                                              sal_Int64 nFlags,
                                              const OUString& rFactory,
                                              sal_Int16 nDialog,
                                              SfxFilterFlags nMust,
                                              SfxFilterFlags nDont,
                                              const OUString& rStandardDir,
                                              const css::uno::Sequence< OUString >& rBlackList);

                            FileDialogHelper( sal_Int16 nDialogType,
                                              sal_Int64 nFlags,
                                              const OUString& aFilterUIName,
                                              const OUString& aExtName,
                                              const OUString& rStandardDir,
                                              const css::uno::Sequence< OUString >& rBlackList,
                                              vcl::Window* _pPreferredParent = nullptr );


    virtual                 ~FileDialogHelper();

    ErrCode                 Execute();
    void                    StartExecuteModal( const Link<FileDialogHelper*,void>& rEndDialogHdl );
    inline ErrCode          GetError() const { return m_nError; }
    sal_Int16               GetDialogType() const;
    bool                    IsPasswordEnabled() const;
    OUString                GetRealFilter() const;

    void                    SetTitle( const OUString&  rNewTitle );
    OUString                GetPath() const;

    /** @deprecated: Don't use this method to retrieve the selected files
        There are file picker which can provide multiple selected file which belong
        to different folders. As this method always provides the root folder for all selected
        files this cannot work.
    */
    css::uno::Sequence< OUString > GetMPath() const;

    /** Provides the selected files with full path information */
    css::uno::Sequence< OUString > GetSelectedFiles() const;

    void                     AddFilter( const OUString& rFilterName, const OUString& rExtension );
    void                     SetCurrentFilter( const OUString& rFilter );

    /** sets an initial display directory/file name

        @deprecated
            don't use this method. It contains a lot of magic in determining whether the
            last segment of the given path/URL denotes a file name or a folder, and by
            definition, it cannot succeed with this magic *all* the time - there will
            always be scenarios where it fails.

            Use SetDisplayFolder and SetFileName.
    */
    void                     SetDisplayDirectory( const OUString& rPath );

    /** sets a new folder whose content is to be displayed in the file picker

        @param _rURL
            specifies the URL of the folder whose content is to be displayed.<br/>
            If the URL doesn't denote a valid (existent and accessible) folder, the
            request is silently dropped.
        @throws css::uno::RuntimeException
            if the invocation of any of the file picker or UCB methods throws a RuntimeException.
    */
    void                     SetDisplayFolder( const OUString& _rURL );

    /** sets an initial file name to display

        This method is usually used in "save-as" contexts, where the application should
        suggest an initial name for the file to save.

        Calling this method is nearly equivalent to calling <code>GetFilePicker().setDefaultName( _rFileName )</code>,
        with the following differences:
        <ul><li>The FileDialogHelper remembers the given file name, and upon execution,
                strips its extension if the dialog is set up for "automatic file name extension".</li>
            <li>Exceptions thrown from the <code>XFilePicker2</code> are caught and silenced.</li>
        </ul>
    */
    void                     SetFileName( const OUString& _rFileName );

    OUString                 GetCurrentFilter() const;
    OUString                 GetDisplayDirectory() const;
    ErrCode                  GetGraphic( Graphic& rGraphic ) const;

    const css::uno::Reference < css::ui::dialogs::XFilePicker2 >& GetFilePicker() const;

    // XFilePickerListener methods
    void SAL_CALL   FileSelectionChanged( const css::ui::dialogs::FilePickerEvent& aEvent );
    void SAL_CALL   DirectoryChanged( const css::ui::dialogs::FilePickerEvent& aEvent );
    virtual void SAL_CALL   ControlStateChanged( const css::ui::dialogs::FilePickerEvent& aEvent );
    void SAL_CALL   DialogSizeChanged();
    static OUString SAL_CALL    HelpRequested( const css::ui::dialogs::FilePickerEvent& aEvent );

    // XDialogClosedListener methods
    void SAL_CALL   DialogClosed( const css::ui::dialogs::DialogClosedEvent& _rEvent );

    /** sets help ids for the controls in the dialog
        @param _pControlId
            Pointer to a 0-terminated array of control ids. They must be recruited from the
            CommonFilePickerElementIds and ExtendedFilePickerElementIds values.
        @param _pHelpId
            Pointer to an array of help ids. For each element in _pControlId, there must be
            a corresponding element herein.
    */
    void                    SetControlHelpIds( const sal_Int16* _pControlId, const char** _pHelpId );
    void                    CreateMatcher( const OUString& rName );

    /** sets the context of the dialog and trigger necessary actions e.g. loading config, setting help id
        @param _eNewContext
            New context for the dialog.
    */
    void                    SetContext( Context _eNewContext );

   DECL_LINK_TYPED( ExecuteSystemFilePicker, void*, void );

   ErrCode                  Execute( std::vector<OUString>& rpURLList,
                                     SfxItemSet *&   rpSet,
                                     OUString&         rFilter,
                                     const OUString&   rDirPath );
   ErrCode                  Execute( SfxItemSet *&   rpSet,
                                     OUString&         rFilter );
};

#define SFX2_IMPL_DIALOG_CONFIG 0
#define SFX2_IMPL_DIALOG_SYSTEM 1
#define SFX2_IMPL_DIALOG_OOO 2
#define SFX2_IMPL_DIALOG_REMOTE 3

ErrCode FileOpenDialog_Impl( sal_Int16 nDialogType,
                             sal_Int64 nFlags,
                             const OUString& rFact,
                             std::vector<OUString>& rpURLList,
                             OUString& rFilter,
                             SfxItemSet *& rpSet,
                             const OUString* pPath = nullptr,
                             sal_Int16 nDialog = SFX2_IMPL_DIALOG_CONFIG,
                             const OUString& rStandardDir = OUString(),
                             const css::uno::Sequence< OUString >& rBlackList = css::uno::Sequence< OUString >());


ErrCode RequestPassword(const std::shared_ptr<const SfxFilter>& pCurrentFilter, OUString& aURL, SfxItemSet* pSet);
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
