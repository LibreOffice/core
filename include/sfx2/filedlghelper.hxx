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
#include <com/sun/star/uno/Sequence.hxx>

#include <rtl/ref.hxx>
#include <rtl/ustring.hxx>
#include <comphelper/documentconstants.hxx>
#include <tools/link.hxx>
#include <comphelper/errcode.hxx>
#include <o3tl/typed_flags_set.hxx>

#include <memory>
#include <optional>
#include <vector>

namespace com::sun::star::ui::dialogs
{
    class XFilePicker3;
    class XFolderPicker2;
    struct FilePickerEvent;
    struct DialogClosedEvent;
}
namespace com::sun::star::awt { class XWindow; }
namespace com::sun::star::uno { template <typename > class Reference; }
namespace com::sun::star::uno { class XComponentContext; }
namespace weld { class Window; }

class Graphic;
class SfxFilter;
class SfxItemSet;
class SfxAllItemSet;

enum class FileDialogFlags {
    NONE              = 0x00,
    Insert            = 0x01,    // turn Open into Insert dialog
    Export            = 0x02,    // turn Save into Export dialog
    SaveACopy         = 0x04,    // turn Save into Save a Copy dialog
    MultiSelection    = 0x08,
    Graphic           = 0x10,    // register graphic formats
    /// Sign existing PDF.
    SignPDF           = 0x20,
    InsertCompare     = 0x40,    /// Special insertion ("Compare" caption)
    InsertMerge       = 0x80,    /// Special insertion ("Merge" caption)
};
namespace o3tl {
    template<> struct typed_flags<FileDialogFlags> : is_typed_flags<FileDialogFlags, 0xFF> {};
}

inline constexpr OUString FILEDIALOG_FILTER_ALL = u"*.*"_ustr;

namespace sfx2 {

class FileDialogHelper_Impl;

class SFX2_DLLPUBLIC FileDialogHelper
{
public:
    // context where the FileDialogHelper is used
    enum Context
    {
        UnknownContext,
        AcceleratorConfig,
        AutoRedact,
        BaseDataSource,
        BaseSaveAs,
        BasicExportPackage,
        BasicExportDialog,
        BasicExportSource,
        BasicImportDialog,
        BasicImportSource,
        BasicInsertLib,
        BulletsAddImage,
        ExtensionManager,
        CalcDataProvider,
        CalcDataStream,
        CalcExport,
        CalcSaveAs,
        CalcXMLSource,
        DrawExport,
        DrawImpressInsertFile,
        DrawImpressOpenSound,
        DrawSaveAs,
        ExportImage,
        FormsAddInstance,
        FormsInsertImage,
        IconImport,
        ImpressClickAction,
        ImpressExport,
        ImpressPhotoDialog,
        ImpressSaveAs,
        LinkClientOLE,
        LinkClientFile,
        ImageMap,
        InsertDoc,
        InsertImage,
        InsertMedia,
        InsertOLE,
        JavaClassPath,
        ReportInsertImage,
        ScreenshotAnnotation,
        SignatureLine,
        TemplateImport,
        WriterCreateAddressList,
        WriterInsertImage,
        WriterInsertScript,
        WriterExport,
        WriterImportAutotext,
        WriterLoadTemplate,
        WriterMailMerge,
        WriterMailMergeSaveAs,
        WriterNewHTMLGlobalDoc,
        WriterRegisterDataSource,
        WriterSaveAs,
        WriterSaveHTML,
        XMLFilterSettings
    };
    static OUString contextToString(Context context);

private:
    Link<FileDialogHelper*,void>  m_aDialogClosedLink;
    ErrCode m_nError;

    rtl::Reference< FileDialogHelper_Impl > mpImpl;


public:
                            FileDialogHelper(sal_Int16 nDialogType,
                                             FileDialogFlags nFlags,
                                             weld::Window* pPreferredParent);

                            FileDialogHelper(sal_Int16 nDialogType,
                                             FileDialogFlags nFlags,
                                             const OUString& rFactory,
                                             SfxFilterFlags nMust,
                                             SfxFilterFlags nDont,
                                             weld::Window* pPreferredParent);

                            FileDialogHelper(sal_Int16 nDialogType,
                                             FileDialogFlags nFlags,
                                             const OUString& rFactory,
                                             sal_Int16 nDialog,
                                             SfxFilterFlags nMust,
                                             SfxFilterFlags nDont,
                                             const OUString& rStandardDir,
                                             const css::uno::Sequence< OUString >& rDenyList,
                                             weld::Window* pPreferredParent);

                            FileDialogHelper(sal_Int16 nDialogType,
                                             FileDialogFlags nFlags,
                                             const OUString& aFilterUIName,
                                             std::u16string_view aExtName,
                                             const OUString& rStandardDir,
                                             const css::uno::Sequence< OUString >& rDenyList,
                                             weld::Window* pPreferredParent);

    virtual                 ~FileDialogHelper();

    FileDialogHelper& operator=(const FileDialogHelper &) = delete;
    FileDialogHelper(const FileDialogHelper &) = delete;

    ErrCode                 Execute();
    void                    StartExecuteModal( const Link<FileDialogHelper*,void>& rEndDialogHdl );
    ErrCode const &         GetError() const { return m_nError; }
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
            <li>Exceptions thrown from the <code>XFilePicker3</code> are caught and silenced.</li>
        </ul>
    */
    void                     SetFileName( const OUString& _rFileName );

    OUString                 GetCurrentFilter() const;
    OUString                 GetDisplayDirectory() const;
    ErrCode                  GetGraphic( Graphic& rGraphic ) const;

    const css::uno::Reference < css::ui::dialogs::XFilePicker3 >& GetFilePicker() const;

    // XFilePickerListener methods
    void   FileSelectionChanged();
    void   DirectoryChanged();
    virtual void   ControlStateChanged( const css::ui::dialogs::FilePickerEvent& aEvent );
    void   DialogSizeChanged();
    static OUString    HelpRequested( const css::ui::dialogs::FilePickerEvent& aEvent );

    // XDialogClosedListener methods
    void   DialogClosed( const css::ui::dialogs::DialogClosedEvent& _rEvent );

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
        This will also store the last used directory for this context, so that the last directory
        gets preselected on next filepicker launch (with the same context)
        @param _eNewContext
            New context for the dialog.
    */
    void                    SetContext( Context _eNewContext );

   DECL_DLLPRIVATE_LINK( ExecuteSystemFilePicker, void*, void );

   ErrCode                  Execute( std::vector<OUString>& rpURLList,
                                     std::optional<SfxAllItemSet>& rpSet,
                                     OUString&         rFilter,
                                     const OUString&   rDirPath );
   ErrCode                  Execute( std::optional<SfxAllItemSet>& rpSet,
                                     OUString&         rFilter );
};

#define SFX2_IMPL_DIALOG_CONFIG 0
#define SFX2_IMPL_DIALOG_SYSTEM 1
#define SFX2_IMPL_DIALOG_OOO 2
#define SFX2_IMPL_DIALOG_REMOTE 3

ErrCode FileOpenDialog_Impl( weld::Window* pParent,
                             sal_Int16 nDialogType,
                             FileDialogFlags nFlags,
                             std::vector<OUString>& rpURLList,
                             OUString& rFilter,
                             std::optional<SfxAllItemSet>& rpSet,
                             const OUString* pPath,
                             sal_Int16 nDialog,
                             const OUString& rStandardDir,
                             const css::uno::Sequence< OUString >& rDenyList,
                             bool& rShowFilterDialog );

css::uno::Reference<css::ui::dialogs::XFolderPicker2> SFX2_DLLPUBLIC createFolderPicker(const css::uno::Reference<css::uno::XComponentContext>& rContext, weld::Window* pPreferredParent);

ErrCode RequestPassword(const std::shared_ptr<const SfxFilter>& pCurrentFilter, OUString const & aURL, SfxItemSet* pSet, const css::uno::Reference<css::awt::XWindow>& rParent);
ErrCode SetPassword(const std::shared_ptr<const SfxFilter>& pCurrentFilter, SfxItemSet* pSet,
                    const OUString& rPasswordToOpen, std::u16string_view rPasswordToModify,
                    bool bAllowPasswordReset = false);
bool IsOOXML(const std::shared_ptr<const SfxFilter>& pCurrentFilter);
bool IsMSType(const std::shared_ptr<const SfxFilter>& pCurrentFilter);
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
