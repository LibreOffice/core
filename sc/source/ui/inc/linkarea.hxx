/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



#ifndef SC_LINKAREA_HXX
#define SC_LINKAREA_HXX

#include <com/sun/star/embed/XEmbeddedObject.hpp>
#include <vcl/dialog.hxx>

#ifndef _SV_BUTTON_HXX
#include <vcl/button.hxx>
#endif
#include <vcl/field.hxx>
#include <vcl/fixed.hxx>
#include <vcl/lstbox.hxx>
#include <svtools/stdctrl.hxx>
#include <svtools/inettbc.hxx>

//REMOVE    #ifndef SO2_DECL_SVEMBEDDEDOBJECT_DEFINED
//REMOVE    #define SO2_DECL_SVEMBEDDEDOBJECT_DEFINED
//REMOVE    SO2_DECL_REF(SvEmbeddedObject)
//REMOVE    #endif

namespace sfx2 { class DocumentInserter; }
namespace sfx2 { class FileDialogHelper; }

class ScDocShell;

//------------------------------------------------------------------------

class ScLinkedAreaDlg : public ModalDialog
{
private:
    FixedLine           aFlLocation;
    SvtURLBox           aCbUrl;
    PushButton          aBtnBrowse;
    FixedInfo           aTxtHint;
    FixedText           aFtRanges;
    MultiListBox        aLbRanges;
    CheckBox            aBtnReload;
    NumericField        aNfDelay;
    FixedText           aFtSeconds;
    OKButton            aBtnOk;
    CancelButton        aBtnCancel;
    HelpButton          aBtnHelp;

    ScDocShell*             pSourceShell;
    sfx2::DocumentInserter* pDocInserter;

    SfxObjectShellRef   aSourceRef;
    ::com::sun::star::uno::Reference< ::com::sun::star::embed::XEmbeddedObject > xSourceObject;

    DECL_LINK( FileHdl, ComboBox* );
    DECL_LINK( BrowseHdl, PushButton* );
    DECL_LINK( RangeHdl, MultiListBox* );
    DECL_LINK( ReloadHdl, CheckBox* );
    DECL_LINK( DialogClosedHdl, sfx2::FileDialogHelper* );

    void                UpdateSourceRanges();
    void                UpdateEnable();
    void                LoadDocument( const String& rFile, const String& rFilter,
                                        const String& rOptions );

public:
            ScLinkedAreaDlg( Window* pParent );
            ~ScLinkedAreaDlg();

    void            InitFromOldLink( const String& rFile, const String& rFilter,
                                        const String& rOptions, const String& rSource,
                                        sal_uLong nRefresh );

    virtual short   Execute();          // overwritten to set dialog parent

    String          GetURL();
    String          GetFilter();        // may be empty
    String          GetOptions();       // filter options
    String          GetSource();        // separated by ";"
    sal_uLong           GetRefresh();       // 0 if disabled
};

#endif

