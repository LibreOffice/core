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
#ifndef _SFX_DINFDLG_HXX
#define _SFX_DINFDLG_HXX

#include "sal/config.h"
#include "sfx2/dllapi.h"

#include <com/sun/star/util/DateTime.hpp>
#include <vcl/edit.hxx>
#include <vcl/field.hxx>
#include <svtools/stdctrl.hxx>
#include <svl/stritem.hxx>
#include <svtools/svmedit.hxx>

#include <vcl/edit.hxx>
#include <vcl/lstbox.hxx>
#include <vcl/scrbar.hxx>
#include <svtools/headbar.hxx>
#include <unotools/syslocale.hxx>
#include <svl/zforlist.hxx>
#include <com/sun/star/util/Duration.hpp>
#include "tabdlg.hxx"

namespace com { namespace sun { namespace star {
    namespace document {
        class XDocumentProperties;
    }
} } }

struct CustomProperty;


// class SfxDocumentInfoItem ---------------------------------------------

class SFX2_DLLPUBLIC SfxDocumentInfoItem : public SfxStringItem
{
private:
    sal_Int32                           m_AutoloadDelay;
    ::rtl::OUString                     m_AutoloadURL;
    sal_Bool                            m_isAutoloadEnabled;
    ::rtl::OUString                     m_DefaultTarget;
    ::rtl::OUString                     m_TemplateName;
    ::rtl::OUString                     m_Author;
    ::com::sun::star::util::DateTime    m_CreationDate;
    ::rtl::OUString                     m_ModifiedBy;
    ::com::sun::star::util::DateTime    m_ModificationDate;
    ::rtl::OUString                     m_PrintedBy;
    ::com::sun::star::util::DateTime    m_PrintDate;
    sal_Int16                           m_EditingCycles;
    sal_Int32                           m_EditingDuration;
    ::rtl::OUString                     m_Description;
    ::rtl::OUString                     m_Keywords;
    ::rtl::OUString                     m_Subject;
    ::rtl::OUString                     m_Title;
    sal_Bool                            m_bHasTemplate;
    sal_Bool                            m_bDeleteUserData;
    sal_Bool                            m_bUseUserData;
    std::vector< CustomProperty* >      m_aCustomProperties;

public:
    TYPEINFO();
    SfxDocumentInfoItem();
    SfxDocumentInfoItem( const String &rFileName,
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::document::XDocumentProperties> & i_xDocProps,
        sal_Bool bUseUserData );
    SfxDocumentInfoItem( const SfxDocumentInfoItem& );
    virtual ~SfxDocumentInfoItem();

    /// update i_xDocProps with the data in this object
    void UpdateDocumentInfo(
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::document::XDocumentProperties> & i_xDocProps,
        bool i_bDoNotUpdateUserDefined = false)
        const;

    sal_Bool    isAutoloadEnabled() const { return m_isAutoloadEnabled; }
    void        setAutoloadEnabled(sal_Bool i_val) { m_isAutoloadEnabled = i_val; }
    sal_Int32   getAutoloadDelay() const { return m_AutoloadDelay; }
    void        setAutoloadDelay(sal_Int32 i_val) { m_AutoloadDelay = i_val; }
    ::rtl::OUString getAutoloadURL() const { return m_AutoloadURL; }
    void        setAutoloadURL(::rtl::OUString i_val) { m_AutoloadURL = i_val; }
    ::rtl::OUString getDefaultTarget() const { return m_DefaultTarget; }
    void        setDefaultTarget(::rtl::OUString i_val) { m_DefaultTarget = i_val; }
    ::rtl::OUString getTemplateName() const { return m_TemplateName; }
    void        setTemplateName(::rtl::OUString i_val) { m_TemplateName = i_val; }
    ::rtl::OUString getAuthor() const { return m_Author; }
    void        setAuthor(::rtl::OUString i_val) { m_Author = i_val; }

    ::com::sun::star::util::DateTime
                getCreationDate() const { return m_CreationDate; }
    void        setCreationDate(::com::sun::star::util::DateTime i_val) {
                    m_CreationDate = i_val;
                }
    ::rtl::OUString getModifiedBy() const { return m_ModifiedBy; }
    void        setModifiedBy(::rtl::OUString i_val) { m_ModifiedBy = i_val; }

    ::com::sun::star::util::DateTime
                getModificationDate() const { return m_ModificationDate; }
    void        setModificationDate(::com::sun::star::util::DateTime i_val) {
                    m_ModificationDate = i_val;
                }
    ::rtl::OUString getPrintedBy() const { return m_PrintedBy; }
    void        setPrintedBy(::rtl::OUString i_val) { m_PrintedBy = i_val; }
    ::com::sun::star::util::DateTime
                getPrintDate() const { return m_PrintDate; }
    void        setPrintDate(::com::sun::star::util::DateTime i_val) {
                    m_PrintDate = i_val;
                }
    sal_Int16   getEditingCycles() const { return m_EditingCycles; }
    void        setEditingCycles(sal_Int16 i_val) { m_EditingCycles = i_val; }
    sal_Int32   getEditingDuration() const { return m_EditingDuration; }
    void        setEditingDuration(sal_Int32 i_val) { m_EditingDuration = i_val; }
    ::rtl::OUString getDescription() const { return m_Description; }
    void        setDescription(::rtl::OUString i_val) { m_Description = i_val; }
    ::rtl::OUString getKeywords() const { return m_Keywords; }
    void        setKeywords(::rtl::OUString i_val) { m_Keywords = i_val; }
    ::rtl::OUString getSubject() const { return m_Subject; }
    void        setSubject(::rtl::OUString i_val) { m_Subject = i_val; }
    ::rtl::OUString getTitle() const { return m_Title; }
    void        setTitle(::rtl::OUString i_val) { m_Title = i_val; }

    /// reset user-specific data (author, modified-by, ...)
    void        resetUserData(const ::rtl::OUString & i_rAuthor);

    void        SetTemplate( sal_Bool b ) { m_bHasTemplate = b; }
    sal_Bool    HasTemplate() const { return m_bHasTemplate; }
    void        SetDeleteUserData( sal_Bool bSet );
    void        SetUseUserData( sal_Bool bSet );
    sal_Bool    IsDeleteUserData() const;
    sal_Bool    IsUseUserData() const;

    std::vector< CustomProperty* >  GetCustomProperties() const;
    void        ClearCustomProperties();
    void        AddCustomProperty(  const ::rtl::OUString& sName,
                                    const com::sun::star::uno::Any& rValue );

    virtual SfxPoolItem*    Clone( SfxItemPool* pPool = NULL ) const;
    virtual int             operator==( const SfxPoolItem& ) const;
    virtual sal_Bool        QueryValue( com::sun::star::uno::Any& rVal, sal_uInt8 nMemberId = 0 ) const;
    virtual sal_Bool        PutValue( const com::sun::star::uno::Any& rVal, sal_uInt8 nMemberId = 0 );
};

// class SfxDocumentPage -------------------------------------------------

class SfxDocumentPage : public SfxTabPage
{
private:
    FixedImage                  aBmp1;
    Edit                        aNameED;

    FixedLine                   aLine1FL;
    FixedText                   aTypeFT;
    svt::SelectableFixedText    aShowTypeFT;
    CheckBox                    aReadOnlyCB;
    FixedText                   aFileFt;
    svt::SelectableFixedText    aFileValFt;
    FixedText                   aSizeFT;
    svt::SelectableFixedText    aShowSizeFT;

    FixedLine                   aLine2FL;
    FixedText                   aCreateFt;
    svt::SelectableFixedText    aCreateValFt;
    FixedText                   aChangeFt;
    svt::SelectableFixedText    aChangeValFt;
    FixedText                   aSignedFt;
    svt::SelectableFixedText    aSignedValFt;
    PushButton                  aSignatureBtn;
    FixedText                   aPrintFt;
    svt::SelectableFixedText    aPrintValFt;
    FixedText                   aTimeLogFt;
    svt::SelectableFixedText    aTimeLogValFt;
    FixedText                   aDocNoFt;
    svt::SelectableFixedText    aDocNoValFt;
    CheckBox                    aUseUserDataCB;
    PushButton                  aDeleteBtn;

    FixedLine                   aLine3FL;
    FixedText                   aTemplFt;
    svt::SelectableFixedText    aTemplValFt;

    String                      aUnknownSize;
    String                      aMultiSignedStr;

    sal_Bool                        bEnableUseUserData  : 1,
                                bHandleDelete       : 1;

    DECL_LINK(          DeleteHdl, PushButton * );
    DECL_LINK(          SignatureHdl, PushButton * );
    void                ImplUpdateSignatures();

protected:
    SfxDocumentPage( Window* pParent, const SfxItemSet& );

    virtual sal_Bool        FillItemSet( SfxItemSet& );
    virtual void        Reset( const SfxItemSet& );

public:
    static SfxTabPage*  Create( Window* pParent, const SfxItemSet& );

    void                EnableUseUserData();
};

// class SfxDocumentDescPage ---------------------------------------------

class SfxDocumentDescPage : public SfxTabPage
{
private:
    FixedText               aTitleFt;
    Edit                    aTitleEd;
    FixedText               aThemaFt;
    Edit                    aThemaEd;
    FixedText               aKeywordsFt;
    Edit                    aKeywordsEd;
    FixedText               aCommentFt;
    MultiLineEdit           aCommentEd;
    SfxDocumentInfoItem*    pInfoItem;

protected:
    SfxDocumentDescPage( Window* pParent, const SfxItemSet& );

    virtual sal_Bool            FillItemSet( SfxItemSet& );
    virtual void            Reset( const SfxItemSet& );

public:
    static SfxTabPage*      Create( Window* pParent, const SfxItemSet& );
};

// class SfxInternetPage -------------------------------------------------

class TargetList;
namespace sfx2
{
    class FileDialogHelper;
}

class SfxInternetPage : public SfxTabPage
{
private:
    RadioButton             aRBNoAutoUpdate;
    RadioButton             aRBReloadUpdate;
    RadioButton             aRBForwardUpdate;

    FixedText               aFTEvery;
    NumericField            aNFReload;
    FixedText               aFTReloadSeconds;

    FixedText               aFTAfter;
    NumericField            aNFAfter;
    FixedText               aFTAfterSeconds;
    FixedText               aFTURL;
    Edit                    aEDForwardURL;
    PushButton              aPBBrowseURL;
    FixedText               aFTFrame;
    ComboBox                aCBFrame;

    String                  aForwardErrorMessg;
    String                  aBaseURL;
    SfxDocumentInfoItem*    pInfoItem;
    sfx2::FileDialogHelper* pFileDlg;

    enum STATE              { S_Init, S_NoUpdate, S_Reload, S_Forward };
                            // S_Init is only valid as initial value
    STATE                   eState;

    void                    ChangeState( STATE eNewState );     // S_Init is not a valid value here
                                                                // also checks corresponding radiobutton
    void                    EnableNoUpdate( sal_Bool bEnable );
    void                    EnableReload( sal_Bool bEnable );
    void                    EnableForward( sal_Bool bEnable );

    DECL_LINK( ClickHdlNoUpdate, Control* );
    DECL_LINK( ClickHdlReload, Control* );
    DECL_LINK( ClickHdlForward, Control* );
    DECL_LINK( ClickHdlBrowseURL, PushButton* );
    DECL_LINK( DialogClosedHdl, sfx2::FileDialogHelper* );

        using TabPage::DeactivatePage;

protected:
    SfxInternetPage( Window* pParent, const SfxItemSet& );
    ~SfxInternetPage();

    virtual sal_Bool            FillItemSet( SfxItemSet& );
    virtual void            Reset( const SfxItemSet& );
    virtual int                     DeactivatePage( SfxItemSet* pSet = 0 );

public:
    static SfxTabPage*  Create( Window* pParent, const SfxItemSet& );
};

// class SfxDocumentInfoDialog -------------------------------------------

class SFX2_DLLPUBLIC SfxDocumentInfoDialog : public SfxTabDialog
{
protected:
    virtual void    PageCreated( sal_uInt16 nId, SfxTabPage& rPage );

public:
    SfxDocumentInfoDialog(  Window* pParent, const SfxItemSet& );
};

// class CustomPropertiesRemoveButton ------------------------------------

struct CustomPropertyLine;

class CustomPropertiesEdit : public Edit
{
private:
    CustomPropertyLine*             m_pLine;

public:
    inline CustomPropertiesEdit(
        Window* pParent, const ResId& rResId, CustomPropertyLine* pLine ) :
            Edit( pParent, rResId ), m_pLine( pLine ) {}

    inline CustomPropertyLine*      GetLine() const { return m_pLine; }
};

class CustomPropertiesTypeBox : public ListBox
{
private:
    CustomPropertyLine*             m_pLine;

public:
    inline CustomPropertiesTypeBox(
        Window* pParent, const ResId& rResId, CustomPropertyLine* pLine ) :
            ListBox( pParent, rResId ), m_pLine( pLine ) {}

    inline CustomPropertyLine*      GetLine() const { return m_pLine; }
};

class CustomPropertiesDateField : public DateField
{
private:
    CustomPropertyLine*             m_pLine;

public:
    inline CustomPropertiesDateField(
        Window* pParent, const ResId& rResId, CustomPropertyLine* pLine ) :
            DateField( pParent, rResId ), m_pLine( pLine ) {}

    inline CustomPropertyLine*      GetLine() const { return m_pLine; }
};
class CustomPropertiesTimeField : public TimeField
{
private:
    CustomPropertyLine*             m_pLine;

public:
    inline CustomPropertiesTimeField(
        Window* pParent, const ResId& rResId, CustomPropertyLine* pLine ) :
            TimeField( pParent, rResId ), m_pLine( pLine ) {}

    inline CustomPropertyLine*      GetLine() const { return m_pLine; }
};
class CustomPropertiesDurationField : public Edit
{
    CustomPropertyLine*             m_pLine;
    com::sun::star::util::Duration  m_aDuration;
protected:
    virtual void    RequestHelp(const HelpEvent& rEvt);
public:
    CustomPropertiesDurationField( Window* pParent, const ResId& rResId, CustomPropertyLine* pLine );
    ~CustomPropertiesDurationField();

    void SetDuration( const com::sun::star::util::Duration& rDuration );
    const com::sun::star::util::Duration& GetDuration() const { return m_aDuration; }
};

class CustomPropertiesEditButton : public PushButton
{
    CustomPropertyLine*             m_pLine;

public:
    CustomPropertiesEditButton( Window* pParent, const ResId& rResId, CustomPropertyLine* pLine );
    ~CustomPropertiesEditButton();

    DECL_LINK(ClickHdl, PushButton*);
};

class CustomPropertiesRemoveButton : public ImageButton
{
private:
    CustomPropertyLine*             m_pLine;

public:
    inline CustomPropertiesRemoveButton(
        Window* pParent, const ResId& rResId, CustomPropertyLine* pLine ) :
            ImageButton( pParent, rResId ), m_pLine( pLine ) {}

    inline CustomPropertyLine*      GetLine() const { return m_pLine; }
};

class CustomPropertiesYesNoButton : public Control
{
private:
    RadioButton                     m_aYesButton;
    RadioButton                     m_aNoButton;

public:
    CustomPropertiesYesNoButton( Window* pParent, const ResId& rResId );

    virtual void    Resize();

    inline void     CheckYes() { m_aYesButton.Check(); }
    inline void     CheckNo() { m_aNoButton.Check(); }
    inline bool     IsYesChecked() const { return m_aYesButton.IsChecked() != sal_False; }
};

// struct CustomPropertyLine ---------------------------------------------

struct CustomPropertyLine
{
    ComboBox                        m_aNameBox;
    CustomPropertiesTypeBox         m_aTypeBox;
    CustomPropertiesEdit            m_aValueEdit;
    CustomPropertiesDateField       m_aDateField;
    CustomPropertiesTimeField       m_aTimeField;
    const String                    m_sDurationFormat;
    CustomPropertiesDurationField   m_aDurationField;
    CustomPropertiesEditButton     m_aEditButton;
    CustomPropertiesYesNoButton     m_aYesNoButton;
    CustomPropertiesRemoveButton    m_aRemoveButton;

    Point                           m_aDatePos;
    Point                           m_aTimePos;
    Size                            m_aDateTimeSize;

    bool                            m_bIsRemoved;
    bool                            m_bTypeLostFocus;

    CustomPropertyLine( Window* pParent );

    void    SetRemoved();
};

// class CustomPropertiesWindow ------------------------------------------

class CustomPropertiesWindow : public Window
{
private:
    ComboBox                            m_aNameBox;
    ListBox                             m_aTypeBox;
    Edit                                m_aValueEdit;
    DateField                           m_aDateField;
    TimeField                           m_aTimeField;
    Edit                                m_aDurationField;
    PushButton                          m_aEditButton;
    CustomPropertiesYesNoButton         m_aYesNoButton;
    ImageButton                         m_aRemoveButton;

    sal_Int32                           m_nLineHeight;
    sal_Int32                           m_nScrollPos;
    SvtSysLocale                        m_aSysLocale;
    std::vector< CustomPropertyLine* >  m_aCustomPropertiesLines;
    CustomPropertyLine*                 m_pCurrentLine;
    SvNumberFormatter                   m_aNumberFormatter;
    Timer                               m_aEditLoseFocusTimer;
    Timer                               m_aBoxLoseFocusTimer;
    Link                                m_aRemovedHdl;

    DECL_LINK(  TypeHdl, CustomPropertiesTypeBox* );
    DECL_LINK(  RemoveHdl, CustomPropertiesRemoveButton* );
    DECL_LINK(  EditLoseFocusHdl, CustomPropertiesEdit* );
    DECL_LINK(  BoxLoseFocusHdl, CustomPropertiesTypeBox* );
    //add lose focus handlers of Date/TimeField?

    DECL_LINK(  EditTimeoutHdl, Timer* );
    DECL_LINK(  BoxTimeoutHdl, Timer* );

    bool        IsLineValid( CustomPropertyLine* pLine ) const;
    void        ValidateLine( CustomPropertyLine* pLine, bool bIsFromTypeBox );

public:
    CustomPropertiesWindow( Window* pParent, const ResId& rResId );
    ~CustomPropertiesWindow();

    void                InitControls( HeaderBar* pHeaderBar, const ScrollBar* pScrollBar );
    sal_uInt16              GetVisibleLineCount() const;
    inline sal_Int32    GetLineHeight() const { return m_nLineHeight; }
    void                AddLine( const ::rtl::OUString& sName, com::sun::star::uno::Any& rAny );
    bool                AreAllLinesValid() const;
    void                ClearAllLines();
    void                DoScroll( sal_Int32 nNewPos );

    bool                DoesCustomPropertyExist( const String& rName ) const;
    ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >
                        GetCustomProperties() const;
    void                SetRemovedHdl( const Link& rLink ) { m_aRemovedHdl = rLink; }
};

// class CustomPropertiesControl -----------------------------------------

class CustomPropertiesControl : public Control
{
private:
    HeaderBar               m_aHeaderBar;
    CustomPropertiesWindow  m_aPropertiesWin;
    ScrollBar               m_aVertScroll;

    bool                    m_bIsInitialized;
    sal_Int32               m_nThumbPos;

    void                    Initialize();

    DECL_LINK( ScrollHdl, ScrollBar* );
    DECL_LINK( RemovedHdl, void* );

public:
    CustomPropertiesControl( Window* pParent, const ResId& rResId );
    ~CustomPropertiesControl();

    void            AddLine( const ::rtl::OUString& sName, com::sun::star::uno::Any& rAny, bool bInteractive );

    inline bool     AreAllLinesValid() const { return m_aPropertiesWin.AreAllLinesValid(); }
    inline void     ClearAllLines() { m_aPropertiesWin.ClearAllLines(); }
    inline bool     DoesCustomPropertyExist( const String& rName ) const
                        { return m_aPropertiesWin.DoesCustomPropertyExist( rName ); }
    inline ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >
                    GetCustomProperties() const
                        { return m_aPropertiesWin.GetCustomProperties(); }
};

// class SfxCustomPropertiesPage -----------------------------------------

class SfxCustomPropertiesPage : public SfxTabPage
{
private:
    CustomPropertiesControl m_aPropertiesCtrl;
    PushButton              m_aAddBtn;
    FixedText               m_aPropertiesFT; // Sym2_5121----, Moved by Steve Yin

    DECL_LINK(  AddHdl, PushButton* );

    using TabPage::DeactivatePage;

protected:
    SfxCustomPropertiesPage( Window* pParent, const SfxItemSet& );

    virtual sal_Bool        FillItemSet( SfxItemSet& );
    virtual void        Reset( const SfxItemSet& );
    virtual int         DeactivatePage( SfxItemSet* pSet = NULL );

public:
    static SfxTabPage*  Create( Window* pParent, const SfxItemSet& );
};

#endif // #ifndef _SFX_DINFDLG_HXX

