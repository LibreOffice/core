/*************************************************************************
 *
 *  $RCSfile: cuigaldlg.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: obo $ $Date: 2004-08-12 09:02:11 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef _CUI_GALDLG_HXX_
#define _CUI_GALDLG_HXX_

#include <vos/thread.hxx>
#include <bootstrp/sstring.hxx>
#include <vcl/dialog.hxx>
#include <vcl/graph.hxx>
#include <vcl/fixed.hxx>
#include <vcl/button.hxx>
#include <vcl/lstbox.hxx>
#include <vcl/menu.hxx>
#include <vcl/edit.hxx>
#include <vcl/combobox.hxx>
#include <svtools/slstitm.hxx>
#include <svtools/transfer.hxx>
#include <goodies/grfmgr.hxx>
#include <sfx2/tabdlg.hxx>
#include "galctrl.hxx"
#include "galmisc.hxx"
#include "galdlg.hxx" //CHINA001

#ifndef _COM_SUN_STAR_MEDIA_XPLAYER_HPP_
#include <com/sun/star/media/XPlayer.hpp>
#endif

// ------------
// - Forwards -
// ------------

class Gallery;
class GalleryTheme;
class SearchProgress;
class TakeProgress;
class TPGalleryThemeProperties;

// ---------------
// - FilterEntry -
// ---------------

struct FilterEntry
{
    String  aFilterName;
};

//CHINA001 // ----------------
//CHINA001 // - ExchangeData -
//CHINA001 // ----------------
//CHINA001
//CHINA001 struct ExchangeData
//CHINA001 {
//CHINA001 GalleryTheme*    pTheme;
//CHINA001 String           aEditedTitle;
//CHINA001 Date         aThemeChangeDate;
//CHINA001 Time         aThemeChangeTime;
//CHINA001 };

// ----------------
// - SearchThread -
// ----------------

class SearchThread : public ::vos::OThread
{
private:

    SearchProgress*             mpProgress;
    TPGalleryThemeProperties*   mpBrowser;
    INetURLObject               maStartURL;

    void                        ImplSearch( const INetURLObject& rStartURL,
                                            const ::std::vector< String >& rFormats,
                                            BOOL bRecursive );

    virtual void SAL_CALL       run();
    virtual void SAL_CALL       onTerminated();

public:

                                SearchThread( SearchProgress* pProgess,
                                              TPGalleryThemeProperties* pBrowser,
                                              const INetURLObject& rStartURL );
    virtual                     ~SearchThread();
};

// ------------------
// - SearchProgress -
// ------------------

class SearchProgress : public ModalDialog
{
private:

    FixedText           aFtSearchDir;
    FixedLine           aFLSearchDir;
    FixedText           aFtSearchType;
    FixedLine           aFLSearchType;
    CancelButton        aBtnCancel;
    SearchThread        maSearchThread;

                        DECL_LINK( ClickCancelBtn, void* );

public:
                        SearchProgress( Window* pParent, const INetURLObject& rStartURL );
                        ~SearchProgress() {};

                        DECL_LINK( CleanUpHdl, void* );

    virtual short       Execute();
    void                SetFileType( const String& rType ) { aFtSearchType.SetText( rType ); }
    void                SetDirectory( const INetURLObject& rURL ) { aFtSearchDir.SetText( GetReducedString( rURL, 30 ) ); }
};

// --------------
// - TakeThread -
// --------------

class TakeThread : public ::vos::OThread
{
private:

    TakeProgress*               mpProgress;
    TPGalleryThemeProperties*   mpBrowser;
    List&                       mrTakenList;

    virtual void SAL_CALL       run();
    virtual void SAL_CALL       onTerminated();

public:

                                TakeThread( TakeProgress* pProgess, TPGalleryThemeProperties* pBrowser, List& rTakenList );
    virtual                     ~TakeThread();
};

// ----------------
// - TakeProgress -
// ----------------

class TakeProgress : public ModalDialog
{
private:

    FixedText           aFtTakeFile;
    FixedLine           aFLTakeProgress;
    CancelButton        aBtnCancel;
    TakeThread          maTakeThread;
    List                maTakenList;

                        DECL_LINK( ClickCancelBtn, void* );

public:

                        TakeProgress( Window* pWindow );
                        ~TakeProgress() {};

                        DECL_LINK( CleanUpHdl, void* );

    void                SetFile( const INetURLObject& rURL ) { aFtTakeFile.SetText( GetReducedString( rURL, 30 ) ); }
    virtual short       Execute();
};

// ---------------------
// - ActualizeProgress -
// ---------------------

class ActualizeProgress : public ModalDialog
{
private:

    FixedText           aFtActualizeFile;
    FixedLine           aFLActualizeProgress;
    CancelButton        aBtnCancel;
    Timer*              pTimer;
    GalleryTheme*       pTheme;
    GalleryProgress     aStatusProgress;

                        DECL_LINK( ClickCancelBtn, void* );
                        DECL_LINK( TimeoutHdl, Timer* );
                        DECL_LINK( ActualizeHdl, INetURLObject* pURL );

public:
                        ActualizeProgress( Window* pWindow, GalleryTheme* pThm );
                        ~ActualizeProgress() {};

    virtual short       Execute();
};

// ---------------
// - TitleDialog -
// ---------------

class TitleDialog : public ModalDialog
{
private:

    OKButton            maOk;
    CancelButton        maCancel;
    HelpButton          maHelp;
    FixedLine           maFL;
    Edit                maEdit;

public:

                        TitleDialog( Window* pParent, const String& rOldText );
    String              GetTitle() const { return maEdit.GetText(); }
};

// -------------------
// - GalleryIdDialog -
// -------------------

class GalleryIdDialog : public ModalDialog
{
private:

    OKButton        aBtnOk;
    CancelButton    aBtnCancel;
    FixedLine       aFLId;
    ListBox         aLbResName;
    GalleryTheme*   pThm;

                    DECL_LINK( ClickOkHdl, void* );
                    DECL_LINK( ClickResNameHdl, void* );

public:

                    GalleryIdDialog( Window* pParent, GalleryTheme* pThm );
                    ~GalleryIdDialog() {}

    ULONG           GetId() const { return aLbResName.GetSelectEntryPos(); }
};

// --------------------------
// - GalleryThemeProperties -
// --------------------------

class GalleryThemeProperties : public SfxTabDialog
{
    ExchangeData*   pData;

    virtual void    PageCreated( USHORT nId, SfxTabPage &rPage );

public:

                    GalleryThemeProperties( Window* pParent, ExchangeData* pData, SfxItemSet* pItemSet  );
                    ~GalleryThemeProperties() {}
};

// -------------------------
// - TPGalleryThemeGeneral -
// -------------------------

class TPGalleryThemeGeneral : public SfxTabPage
{
private:

    FixedImage          aFiMSImage;
    Edit                aEdtMSName;
    FixedLine           aFlMSGeneralFirst;
    FixedText           aFtMSType;
    FixedText           aFtMSShowType;
    FixedText           aFtMSPath;
    FixedText           aFtMSShowPath;
    FixedText           aFtMSContent;
    FixedText           aFtMSShowContent;
    FixedLine           aFlMSGeneralSecond;
    FixedText           aFtMSChangeDate;
    FixedText           aFtMSShowChangeDate;
    ExchangeData*       pData;

    virtual void        Reset( const SfxItemSet& ) {}
    virtual BOOL        FillItemSet( SfxItemSet& rSet );


public:

                        TPGalleryThemeGeneral( Window* pParent, const SfxItemSet& rSet );
                        ~TPGalleryThemeGeneral() {}

    void                SetXChgData( ExchangeData* pData );
    const ExchangeData* GetXChgData() const { return pData; }

    static SfxTabPage*  Create( Window* pParent, const SfxItemSet& rSet );
};

// ----------------------------
// - TPGalleryThemeProperties -
// ----------------------------

class TPGalleryThemeProperties : public SfxTabPage
{
    friend class SearchThread;
    friend class TakeProgress;
    friend class TakeThread;

    PushButton          aBtnSearch;
    PushButton          aBtnTake;
    PushButton          aBtnTakeAll;
    CheckBox            aCbxPreview;
    ComboBox            aCbbFileType;
    MultiListBox        aLbxFound;
    FixedText           aFtFileType;
    GalleryPreview      aWndPreview;

    ExchangeData*       pData;
    StringList          aFoundList;
    List                aFilterEntryList;
    Timer               aPreviewTimer;
    String              aLastFilterName;
    String              aPreviewString;
    INetURLObject       aURL;
    USHORT              nCurFilterPos;
    USHORT              nFirstExtFilterPos;
    BOOL                bEntriesFound;
    BOOL                bInputAllowed;
    BOOL                bTakeAll;
    BOOL                bSearchRecursive;

    ::com::sun::star::uno::Reference< ::com::sun::star::media::XPlayer > xMediaPlayer;

    virtual void        Reset( const SfxItemSet& rSet ) {}
    virtual BOOL        FillItemSet( SfxItemSet& rSet ) { return TRUE; }
    ::rtl::OUString     addExtension( const ::rtl::OUString&, const ::rtl::OUString& );
    void                FillFilterList();

    void                SearchFiles();
    void                TakeFiles();
    void                DoPreview();

                        DECL_LINK( ClickPreviewHdl, void* );
                        DECL_LINK( ClickCloseBrowserHdl, void* );
                        DECL_LINK( ClickSearchHdl, void* );
                        DECL_LINK( ClickTakeHdl, void* );
                        DECL_LINK( ClickTakeAllHdl, void* );
                        DECL_LINK( SelectFoundHdl, void* );
                        DECL_LINK( SelectThemeHdl, void* );
                        DECL_LINK( SelectFileTypeHdl, void* );
                        DECL_LINK( DClickFoundHdl, void* );
                        DECL_LINK( PreviewTimerHdl, void* );

public:
                        TPGalleryThemeProperties( Window* pWindow, const SfxItemSet& rSet );
                        ~TPGalleryThemeProperties();

    void                SetXChgData( ExchangeData* pData );
    const ExchangeData* GetXChgData() const { return pData; }

    static SfxTabPage*  Create( Window* pParent, const SfxItemSet& rSet );
};

#endif // _CUI_GALDLG_HXX_
