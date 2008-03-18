/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: filedlghelper.hxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: vg $ $Date: 2008-03-18 17:34:16 $
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
#ifndef _FILEDLGHELPER_HXX
#define _FILEDLGHELPER_HXX

#ifndef _SAL_CONFIG_H_
#include "sal/config.h"
#endif

#ifndef INCLUDED_SFX2_DLLAPI_H
#include "sfx2/dllapi.h"
#endif

#ifndef _SAL_TYPES_H_
#include "sal/types.h"
#endif
#ifndef  _COM_SUN_STAR_UNO_REFERENCE_HXX_
#include <com/sun/star/uno/Reference.hxx>
#endif
#ifndef  _COM_SUN_STAR_UNO_SEQUENCE_HXX_
#include <com/sun/star/uno/Sequence.hxx>
#endif
#ifndef  _RTL_USTRING_HXX_
#include <rtl/ustring.hxx>
#endif

#ifndef _SOLAR_H
#include <tools/solar.h>
#endif
#ifndef _STRING_HXX
#include <tools/string.hxx>
#endif
#ifndef _ERRCODE_HXX
#include <tools/errcode.hxx>
#endif

#ifndef _SV_DIALOG_HXX
#include <vcl/dialog.hxx>
#endif
#ifndef _SV_EDIT_HXX
#include <vcl/edit.hxx>
#endif
#ifndef _SV_BUTTON_HXX
#include <vcl/button.hxx>
#endif
#ifndef _SV_GRAPH_HXX
#include <vcl/graph.hxx>
#endif

#ifndef  _SFXDEFS_HXX
#include <sfx2/sfxdefs.hxx>
#endif
#ifndef  _SFX_SFXUNO_HXX
#include <sfx2/sfxuno.hxx>
#endif

//-----------------------------------------------------------------------------

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
                    class XFilePicker;
                    class XFilePickerListener;
                    struct FilePickerEvent;
                    struct DialogClosedEvent;
                }
            }
        }
    }
}

class SfxItemSet;
class SvStringsDtor;
class Window;

//-----------------------------------------------------------------------------
/*
#define WB_PATH                 0x00100000L
#define WB_OPEN                 0x00200000L
#define WB_SAVEAS               0x00400000L
#define WB_PASSWORD             0x01000000L
#define WB_READONLY             0x02000000L
*/

#define SFXWB_INSERT            ( 0x04000000L | WB_OPEN )   // ((WinBits)0x00200000)
#define SFXWB_PASSWORD          WB_PASSWORD                 // ((WinBits)0x01000000)
#define SFXWB_READONLY          WB_READONLY                 // ((WinBits)0x02000000)
#define SFXWB_PATHDIALOG        WB_PATH                     // ((WinBits)0x00100000)
#define SFXWB_CLASSPATH         ( 0x08000000L | SFXWB_PATHDIALOG )
#define SFXWB_SHOWALLFOLDER     0x10000000L     // alle Ordner auch Mail/News/...
#define SFXWB_MULTISELECTION    0x20000000L     // Multiselection an
#define SFXWB_NOREMOTE          0x40000000L
#define SFXWB_SHOWVERSIONS      0x80000000L     // Versionsauswahl anzeigen

#define SFXWB_GRAPHIC           0x00800000L     // FileOpen with link and preview box
#define SFXWB_SHOWSTYLES        0x01000000L     // FileOpen with link and preview box and styles

#define SFXWB_EXPORT            ( 0x040000000L | WB_SAVEAS )    // Export dialog

#define FILEDIALOG_FILTER_ALL   "*.*"

#define FILE_OPEN_SERVICE_NAME      "com.sun.star.ui.dialogs.FilePicker"
#define FOLDER_PICKER_SERVICE_NAME  "com.sun.star.ui.dialogs.FolderPicker"
#define FILE_OPEN_SERVICE_NAME_SYSTEM "com.sun.star.ui.dialogs.SystemFilePicker"
#define FILE_OPEN_SERVICE_NAME_OOO   "com.sun.star.ui.dialogs.OfficeFilePicker"

//*****************************************************************************
// SfxUrlDialog
//*****************************************************************************

//#if 0 // _SOLAR__PRIVATE
#ifndef DONT_USE_FILE_DIALOG_SERVICE

class SfxUrlDialog : public ModalDialog
{
private:
    Edit            aEdit;
    OKButton        aOk;
    CancelButton    aCancel;

public:
                    SfxUrlDialog( Window* pParent );

    String          GetUrl() const { return aEdit.GetText(); }
    void            SetUrl( const String& rUrl ) { aEdit.SetText( rUrl ); }
};

#endif
//#endif

//-----------------------------------------------------------------------------

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
        SI_EXPORT                       // export in impress
    };

private:
    Link    m_aDialogClosedLink;
    ErrCode m_nError;

    ::com::sun::star::uno::Reference < ::com::sun::star::ui::dialogs::XFilePickerListener > mxImp;
    FileDialogHelper_Impl   *mpImp;

//#if 0 // _SOLAR__PRIVATE

    SAL_DLLPRIVATE sal_Int16 getDialogType( sal_Int64 nFlags ) const;

//#endif

public:
                            FileDialogHelper( sal_Int64 nFlags,
                                              const String& rFactory,
                                              sal_Int16 nDialog,
                                              SfxFilterFlags nMust,
                                              SfxFilterFlags nDont );

                            FileDialogHelper( sal_Int64 nFlags,
                                              const String& rFact,
                                              sal_Int16 nDialog,
                                              SfxFilterFlags nMust,
                                              SfxFilterFlags nDont,
                                              const String& rStandardDir);

                            FileDialogHelper( sal_Int64 nFlags,
                                              const String& rFactory,
                                              SfxFilterFlags nMust = 0,
                                              SfxFilterFlags nDont = 0 );

                            FileDialogHelper( sal_Int16 nDialogType,
                                              sal_Int64 nFlags,
                                              const String& rFactory,
                                              SfxFilterFlags nMust = 0,
                                              SfxFilterFlags nDont = 0 );

                            FileDialogHelper( sal_Int16 nDialogType,
                                              sal_Int64 nFlags,
                                              const String& rFactory,
                                              sal_Int16 nDialog,
                                              SfxFilterFlags nMust,
                                              SfxFilterFlags nDont,
                                              const String& rStandardDir );

                            FileDialogHelper( sal_Int64 nFlags );

                            FileDialogHelper( sal_Int16 nDialogType,
                                              sal_Int64 nFlags,
                                              Window* _pPreferredParent = NULL );

                            FileDialogHelper( sal_Int16 nDialogType,
                                              sal_Int64 nFlags,
                                              const ::rtl::OUString& aFilterUIName,
                                              const ::rtl::OUString& aExtName,
                                              const ::rtl::OUString& rStandardDir,
                                              Window* _pPreferredParent = NULL );


    virtual                 ~FileDialogHelper();

    ErrCode                 Execute();
    void                    StartExecuteModal( const Link& rEndDialogHdl );
    inline ErrCode          GetError() const { return m_nError; }
    sal_Int16               GetDialogType() const;
    sal_Bool                IsPasswordEnabled() const;
    String                  GetRealFilter() const;

    void                    SetTitle( const String&  rNewTitle );
    String                  GetPath() const;

    ::com::sun::star::uno::Sequence< ::rtl::OUString > GetMPath() const;

    void                     AddFilter( const String& rFilterName, const String& rExtension );
    void                     SetCurrentFilter( const String& rFilter );

    /** sets an initial display directory/file name

        @deprecated
            don't use this method. It contains a lot of magic in determining whether the
            last segment of the given path/URL denotes a file name or a folder, and by
            definition, it cannot succeed with this magic *all* the time - there will
            always be scenarios where it fails.

            Use SetDisplayFolder and SetFileName.
    */
    void                     SetDisplayDirectory( const String& rPath );

    /** sets a new folder whose content is to be displayed in the file picker

        @param _rURL
            specifies the URL of the folder whose content is to be displayed.<br/>
            If the URL doesn't denote a valid (existent and accessible) folder, the
            request is silently dropped.
        @throws ::com::sun::star::uno::RuntimeException
            if the invocation of any of the file picker or UCB methods throws a RuntimeException.
    */
    void                     SetDisplayFolder( const String& _rURL );

    /** sets an initial file name to display

        This method is usually used in "save-as" contexts, where the application should
        suggest an initial name for the file to save.

        Calling this method is nearly equivalent to calling <code>GetFilePicker().setDefaultName( _rFileName )</code>,
        with the following differences:
        <ul><li>The FileDialogHelper remembers the given file name, and upon execution,
                strips its extension if the dialog is set up for "automatic file name extension".</li>
            <li>Exceptions thrown from the <code>XFilePicker</code> are caught and silenced.</li>
        </ul>
    */
    void                     SetFileName( const String& _rFileName );

    String                   GetCurrentFilter() const;
    String                   GetDisplayDirectory() const;
    ErrCode                  GetGraphic( Graphic& rGraphic ) const;

    ::com::sun::star::uno::Reference < ::com::sun::star::ui::dialogs::XFilePicker > GetFilePicker() const;

    // XFilePickerListener methods
    virtual void SAL_CALL   FileSelectionChanged( const ::com::sun::star::ui::dialogs::FilePickerEvent& aEvent );
    virtual void SAL_CALL   DirectoryChanged( const ::com::sun::star::ui::dialogs::FilePickerEvent& aEvent );
    virtual void SAL_CALL   ControlStateChanged( const ::com::sun::star::ui::dialogs::FilePickerEvent& aEvent );
    virtual void SAL_CALL   DialogSizeChanged();
    virtual ::rtl::OUString SAL_CALL    HelpRequested( const ::com::sun::star::ui::dialogs::FilePickerEvent& aEvent );

    // XDialogClosedListener methods
    virtual void SAL_CALL   DialogClosed( const ::com::sun::star::ui::dialogs::DialogClosedEvent& _rEvent );

    // retrieves the top-most file picker - i.e. the instance which is currently beeing executed
    static ::com::sun::star::uno::Reference< ::com::sun::star::ui::dialogs::XFilePicker >
                            GetTopMostFilePicker( );

    /** sets help ids for the controls in the dialog
        @param _pControlId
            Pointer to a 0-terminated array of control ids. They must be recruited from the
            CommonFilePickerElementIds and ExtendedFilePickerElementIds values.
        @param _pHelpId
            Pointer to an array of help ids. For each element in _pControlId, there must be
            a corresponding element herein.
    */
    void                    SetControlHelpIds( const sal_Int16* _pControlId, const sal_Int32* _pHelpId );
    void                    SetDialogHelpId( const sal_Int32 _nHelpId );
    void                    CreateMatcher( const String& rName );

    /** sets the context of the dialog and trigger necessary actions e.g. loading config, setting help id
        @param _eNewContext
            New context for the dialog.
    */
    void                    SetContext( Context _eNewContext );

   DECL_LINK( ExecuteSystemFilePicker, void* );

//#if 0 // _SOLAR__PRIVATE
    ErrCode                  Execute( SvStringsDtor*& rpURLList,
                                      SfxItemSet *&   rpSet,
                                      String&         rFilter,
                                      const String&   rDirPath );
    ErrCode                  Execute( SfxItemSet *&   rpSet,
                                      String&         rFilter );
//#endif
};

#define SFX2_IMPL_DIALOG_CONFIG 0
#define SFX2_IMPL_DIALOG_SYSTEM 1
#define SFX2_IMPL_DIALOG_OOO 2

//#if 0 // _SOLAR__PRIVATE
ErrCode FileOpenDialog_Impl( sal_Int64 nFlags,
                             const String& rFact,
                             SvStringsDtor *& rpURLList,
                             String& rFilter,
                             SfxItemSet *& rpSet,
                             const String* pPath = NULL,
                             sal_Int16 nDialog = SFX2_IMPL_DIALOG_CONFIG,
                             const String& rStandardDir = String::CreateFromAscii( "" ));
//#endif
}

//-----------------------------------------------------------------------------

#endif

