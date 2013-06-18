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
#ifndef SFX_DINFDLG_HXX
#define SFX_DINFDLG_HXX

#include <boost/optional/optional.hpp>

#include "sal/config.h"
#include "sfx2/dllapi.h"

#include <com/sun/star/util/DateTime.hpp>
#include <com/sun/star/util/Duration.hpp>
#include <com/sun/star/document/CmisProperty.hpp>

#include <svl/stritem.hxx>
#include <svl/zforlist.hxx>

#include <svtools/headbar.hxx>
#include <svtools/stdctrl.hxx>
#include <svtools/svmedit.hxx>

#include <unotools/syslocale.hxx>

#include <vcl/edit.hxx>
#include <vcl/field.hxx>
#include <vcl/layout.hxx>
#include <vcl/lstbox.hxx>
#include <vcl/scrbar.hxx>

#include "tabdlg.hxx"

namespace com { namespace sun { namespace star {
    namespace document {
        class XDocumentProperties;
    }
} } }

struct CustomProperty;
struct CmisProperty;


// class SfxDocumentInfoItem ---------------------------------------------

class SFX2_DLLPUBLIC SfxDocumentInfoItem : public SfxStringItem
{
private:
    sal_Int32                           m_AutoloadDelay;
    OUString                     m_AutoloadURL;
    sal_Bool                            m_isAutoloadEnabled;
    OUString                     m_DefaultTarget;
    OUString                     m_TemplateName;
    OUString                     m_Author;
    ::com::sun::star::util::DateTime    m_CreationDate;
    OUString                     m_ModifiedBy;
    ::com::sun::star::util::DateTime    m_ModificationDate;
    OUString                     m_PrintedBy;
    ::com::sun::star::util::DateTime    m_PrintDate;
    sal_Int16                           m_EditingCycles;
    sal_Int32                           m_EditingDuration;
    OUString                     m_Description;
    OUString                     m_Keywords;
    OUString                     m_Subject;
    OUString                     m_Title;
    sal_Bool                            m_bHasTemplate;
    sal_Bool                            m_bDeleteUserData;
    sal_Bool                            m_bUseUserData;
    std::vector< CustomProperty* >    m_aCustomProperties;
    std::vector< CmisProperty* >      m_aCmisProperties;

public:
    TYPEINFO();
    SfxDocumentInfoItem();
    SfxDocumentInfoItem( const OUString &rFileName,
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::document::XDocumentProperties> & i_xDocProps,
        const ::com::sun::star::uno::Sequence<
            ::com::sun::star::document::CmisProperty> & i_cmisProps,
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
    OUString getAutoloadURL() const { return m_AutoloadURL; }
    void        setAutoloadURL(OUString i_val) { m_AutoloadURL = i_val; }
    OUString getDefaultTarget() const { return m_DefaultTarget; }
    void        setDefaultTarget(OUString i_val) { m_DefaultTarget = i_val; }
    OUString getTemplateName() const { return m_TemplateName; }
    void        setTemplateName(OUString i_val) { m_TemplateName = i_val; }
    OUString getAuthor() const { return m_Author; }
    void        setAuthor(OUString i_val) { m_Author = i_val; }

    ::com::sun::star::util::DateTime
                getCreationDate() const { return m_CreationDate; }
    void        setCreationDate(::com::sun::star::util::DateTime i_val) {
                    m_CreationDate = i_val;
                }
    OUString getModifiedBy() const { return m_ModifiedBy; }
    void        setModifiedBy(OUString i_val) { m_ModifiedBy = i_val; }

    ::com::sun::star::util::DateTime
                getModificationDate() const { return m_ModificationDate; }
    void        setModificationDate(::com::sun::star::util::DateTime i_val) {
                    m_ModificationDate = i_val;
                }
    OUString getPrintedBy() const { return m_PrintedBy; }
    void        setPrintedBy(OUString i_val) { m_PrintedBy = i_val; }
    ::com::sun::star::util::DateTime
                getPrintDate() const { return m_PrintDate; }
    void        setPrintDate(::com::sun::star::util::DateTime i_val) {
                    m_PrintDate = i_val;
                }
    sal_Int16   getEditingCycles() const { return m_EditingCycles; }
    void        setEditingCycles(sal_Int16 i_val) { m_EditingCycles = i_val; }
    sal_Int32   getEditingDuration() const { return m_EditingDuration; }
    void        setEditingDuration(sal_Int32 i_val) { m_EditingDuration = i_val; }
    OUString getDescription() const { return m_Description; }
    void        setDescription(OUString i_val) { m_Description = i_val; }
    OUString getKeywords() const { return m_Keywords; }
    void        setKeywords(OUString i_val) { m_Keywords = i_val; }
    OUString getSubject() const { return m_Subject; }
    void        setSubject(OUString i_val) { m_Subject = i_val; }
    OUString getTitle() const { return m_Title; }
    void        setTitle(OUString i_val) { m_Title = i_val; }

    /// reset user-specific data (author, modified-by, ...)
    void        resetUserData(const OUString & i_rAuthor);

    void        SetTemplate( sal_Bool b ) { m_bHasTemplate = b; }
    sal_Bool    HasTemplate() const { return m_bHasTemplate; }
    void        SetDeleteUserData( sal_Bool bSet );
    void        SetUseUserData( sal_Bool bSet );
    sal_Bool    IsDeleteUserData() const;
    sal_Bool    IsUseUserData() const;

    std::vector< CustomProperty* >  GetCustomProperties() const;
    void        ClearCustomProperties();
    void        AddCustomProperty(  const OUString& sName,
                                    const com::sun::star::uno::Any& rValue );

    std::vector< CmisProperty* >  GetCmisProperties() const;
    void        ClearCmisProperties();
    void        AddCmisProperty(  const OUString& sId, const OUString& sName,
                                  const bool bUpdatable, const bool bRequired,
                                  const bool bMultiValued, const bool bOpenChoice,
                                  const com::sun::star::uno::Any& Choices,
                                  const com::sun::star::uno::Any& rValue );

    virtual SfxPoolItem*    Clone( SfxItemPool* pPool = NULL ) const;
    virtual int             operator==( const SfxPoolItem& ) const;
    virtual bool        QueryValue( com::sun::star::uno::Any& rVal, sal_uInt8 nMemberId = 0 ) const;
    virtual bool        PutValue( const com::sun::star::uno::Any& rVal, sal_uInt8 nMemberId = 0 );
};

// class SfxDocumentPage -------------------------------------------------

class SfxDocumentPage : public SfxTabPage
{
private:
    FixedImage*                 m_pBmp;
    Edit*                       m_pNameED;
    PushButton*                 m_pChangePassBtn;

    SelectableFixedText*        m_pShowTypeFT;
    CheckBox*                   m_pReadOnlyCB;
    SelectableFixedText*        m_pFileValFt;
    SelectableFixedText*        m_pShowSizeFT;

    SelectableFixedText*        m_pCreateValFt;
    SelectableFixedText*        m_pChangeValFt;
    SelectableFixedText*        m_pSignedValFt;
    PushButton*                 m_pSignatureBtn;
    SelectableFixedText*        m_pPrintValFt;
    SelectableFixedText*        m_pTimeLogValFt;
    SelectableFixedText*        m_pDocNoValFt;

    CheckBox*                   m_pUseUserDataCB;
    PushButton*                 m_pDeleteBtn;

    FixedText*                  m_pTemplFt;
    SelectableFixedText*        m_pTemplValFt;

    OUString                    m_aUnknownSize;
    OUString                    m_aMultiSignedStr;

    sal_Bool                        bEnableUseUserData  : 1,
                                bHandleDelete       : 1;

    DECL_LINK(DeleteHdl, void *);
    DECL_LINK(SignatureHdl, void *);
    DECL_LINK(ChangePassHdl, void *);
    void                ImplUpdateSignatures();
    void                ImplCheckPasswordState();

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
    Edit*                   m_pTitleEd;
    Edit*                   m_pThemaEd;
    Edit*                   m_pKeywordsEd;
    VclMultiLineEdit*       m_pCommentEd;
    SfxDocumentInfoItem*    m_pInfoItem;

protected:
    SfxDocumentDescPage( Window* pParent, const SfxItemSet& );

    virtual sal_Bool            FillItemSet( SfxItemSet& );
    virtual void            Reset( const SfxItemSet& );

public:
    static SfxTabPage*      Create( Window* pParent, const SfxItemSet& );
};

// class SfxDocumentInfoDialog -------------------------------------------

class SFX2_DLLPUBLIC SfxDocumentInfoDialog : public SfxTabDialog
{
private:
    sal_uInt16 m_nDocInfoId;
protected:
    virtual void    PageCreated( sal_uInt16 nId, SfxTabPage& rPage );

public:
    SfxDocumentInfoDialog( Window* pParent, const SfxItemSet& );
    void AddFontTabPage();
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
    ::boost::optional<sal_Int16> m_TZ;

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
    bool m_isUTC;

    inline CustomPropertiesTimeField(
        Window* pParent, const ResId& rResId, CustomPropertyLine* pLine ) :
            TimeField( pParent, rResId ), m_pLine( pLine ), m_isUTC(false) {}

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

    DECL_LINK(ClickHdl, void *);
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
    const OUString                  m_sDurationFormat;
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

    DECL_LINK(EditTimeoutHdl, void *);
    DECL_LINK(BoxTimeoutHdl, void *);

    bool        IsLineValid( CustomPropertyLine* pLine ) const;
    void        ValidateLine( CustomPropertyLine* pLine, bool bIsFromTypeBox );

public:
    CustomPropertiesWindow(Window* pParent,
        const OUString &rHeaderAccName,
        const OUString &rHeaderAccType,
        const OUString &rHeaderAccValue);
    ~CustomPropertiesWindow();

    void                InitControls( HeaderBar* pHeaderBar, const ScrollBar* pScrollBar );
    sal_uInt16              GetVisibleLineCount() const;
    inline sal_Int32    GetLineHeight() const { return m_nLineHeight; }
    void                AddLine( const OUString& sName, com::sun::star::uno::Any& rAny );
    bool                AreAllLinesValid() const;
    void                ClearAllLines();
    void                DoScroll( sal_Int32 nNewPos );

    ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >
                        GetCustomProperties() const;
    void                SetRemovedHdl( const Link& rLink ) { m_aRemovedHdl = rLink; }

    void                InitRemoveButton(const ScrollBar &rScrollBar);
    void                updateLineWidth();
};

// class CustomPropertiesControl -----------------------------------------

class CustomPropertiesControl : public VclVBox
{
private:
    HeaderBar*              m_pHeaderBar;
    VclHBox*                m_pBody;
    CustomPropertiesWindow* m_pPropertiesWin;
    ScrollBar*              m_pVertScroll;

    sal_Int32               m_nThumbPos;

    DECL_LINK( ScrollHdl, ScrollBar* );
    DECL_LINK( RemovedHdl, void* );

public:
    CustomPropertiesControl(Window* pParent);
    ~CustomPropertiesControl();

    void            AddLine( const OUString& sName, com::sun::star::uno::Any& rAny, bool bInteractive );

    inline bool     AreAllLinesValid() const { return m_pPropertiesWin->AreAllLinesValid(); }
    inline void     ClearAllLines() { m_pPropertiesWin->ClearAllLines(); }
    inline ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >
                    GetCustomProperties() const
                        { return m_pPropertiesWin->GetCustomProperties(); }
    void    Init(VclBuilderContainer& rParent);
    virtual void setAllocation(const Size &rAllocation);
};

// class SfxCustomPropertiesPage -----------------------------------------

class SfxCustomPropertiesPage : public SfxTabPage
{
private:
    CustomPropertiesControl* m_pPropertiesCtrl;

    DECL_LINK(AddHdl, void *);

    using TabPage::DeactivatePage;

protected:
    SfxCustomPropertiesPage( Window* pParent, const SfxItemSet& );

    virtual sal_Bool        FillItemSet( SfxItemSet& );
    virtual void        Reset( const SfxItemSet& );
    virtual int         DeactivatePage( SfxItemSet* pSet = NULL );

public:
    static SfxTabPage*  Create( Window* pParent, const SfxItemSet& );
};

struct CmisPropertyLine;

class CmisPropertiesEdit : public Edit
{
private:
    CmisPropertyLine*             m_pLine;

public:
    inline CmisPropertiesEdit(
        Window* pParent, const ResId& rResId, CmisPropertyLine* pLine ) :
            Edit( pParent, rResId ), m_pLine( pLine ) {}

    inline CmisPropertyLine*      GetLine() const { return m_pLine; }
};

class CmisPropertiesDateField : public DateField
{
private:
    CmisPropertyLine*             m_pLine;

public:
    inline CmisPropertiesDateField(
        Window* pParent, const ResId& rResId, CmisPropertyLine* pLine ) :
            DateField( pParent, rResId ), m_pLine( pLine ) {}

    inline CmisPropertyLine*      GetLine() const { return m_pLine; }
};
class CmisPropertiesTimeField : public TimeField
{
private:
    CmisPropertyLine*             m_pLine;

public:
    inline CmisPropertiesTimeField(
        Window* pParent, const ResId& rResId, CmisPropertyLine* pLine ) :
            TimeField( pParent, rResId ), m_pLine( pLine ) {}

    inline CmisPropertyLine*      GetLine() const { return m_pLine; }
};

class CmisPropertiesEditButton : public PushButton
{
    CmisPropertyLine*             m_pLine;

public:
    CmisPropertiesEditButton( Window* pParent, const ResId& rResId, CmisPropertyLine* pLine );
    ~CmisPropertiesEditButton();

    DECL_LINK(ClickHdl, void *);
};

class CmisPropertiesYesNoButton : public Control
{
private:
    RadioButton                     m_aYesButton;
    RadioButton                     m_aNoButton;

public:
    CmisPropertiesYesNoButton( Window* pParent, const ResId& rResId );

    virtual void    Resize();

    inline void     CheckYes() { m_aYesButton.Check(); }
    inline void     CheckNo() { m_aNoButton.Check(); }
    inline bool     IsYesChecked() const { return m_aYesButton.IsChecked() != sal_False; }
};

// struct CmisPropertyLine ---------------------------------------------

struct CmisPropertyLine
{
    FixedText                     m_aName;
    FixedText                     m_aType;
    CmisPropertiesEdit            m_aValueEdit;
    CmisPropertiesDateField       m_aDateField;
    CmisPropertiesTimeField       m_aTimeField;
    CmisPropertiesYesNoButton     m_aYesNoButton;
    Point                         m_aDatePos;
    Point                         m_aTimePos;
    Size                          m_aDateTimeSize;
    bool                          m_bTypeLostFocus;
    CmisPropertyLine( Window* pParent );
};

// class CmisPropertiesWindow ------------------------------------------

class CmisPropertiesWindow : public Window
{
private:
    FixedText                           m_aName;
    FixedText                           m_aType;
    Edit                                m_aValueEdit;
    DateField                           m_aDateField;
    TimeField                           m_aTimeField;
    CmisPropertiesYesNoButton           m_aYesNoButton;

    sal_Int32                           m_nLineHeight;
    sal_Int32                           m_nScrollPos;
    SvtSysLocale                        m_aSysLocale;
    std::vector< CmisPropertyLine* >    m_aCmisPropertiesLines;
    CmisPropertyLine*                   m_pCurrentLine;
    SvNumberFormatter                   m_aNumberFormatter;
    Timer                               m_aEditLoseFocusTimer;
    Timer                               m_aBoxLoseFocusTimer;

    DECL_LINK( EditLoseFocusHdl, CmisPropertiesEdit* );

    bool        IsLineValid( CmisPropertyLine* pLine ) const;
    void        ValidateLine( CmisPropertyLine* pLine, bool bIsFromTypeBox );

public:
    CmisPropertiesWindow(Window* pParent,
        const OUString &rHeaderAccName,
        const OUString &rHeaderAccType,
        const OUString &rHeaderAccValue);
    ~CmisPropertiesWindow();

    void                InitControls( HeaderBar* pHeaderBar, const ScrollBar* pScrollBar );
    sal_uInt16          GetLineCount() const;
    inline sal_Int32    GetLineHeight() const { return m_nLineHeight; }
    void                AddLine( const OUString& sId, const OUString& sName,
                                 const bool bUpdatable, const bool bRequired,
                                 const bool bMultiValued, const bool bOpenChoice,
                                 com::sun::star::uno::Any& aChoices,
                                 com::sun::star::uno::Any& rAny );
    bool                AreAllLinesValid() const;
    void                ClearAllLines();
    void                DoScroll( sal_Int32 nNewPos );

    ::com::sun::star::uno::Sequence< ::com::sun::star::document::CmisProperty >
                        GetCmisProperties() const;
    void                updateLineWidth();
};

// class CmisPropertiesControl -----------------------------------------

class CmisPropertiesControl : public VclVBox
{
private:
    HeaderBar*              m_pHeaderBar;
    VclHBox*                m_pBody;
    CmisPropertiesWindow*   m_pPropertiesWin;
    ScrollBar*              m_pVertScroll;
    sal_Int32               m_nThumbPos;

    DECL_LINK( ScrollHdl, ScrollBar* );

public:
    CmisPropertiesControl(Window* pParent);
    ~CmisPropertiesControl();

    void            AddLine( const OUString& sId, const OUString& sName,
                             const bool bUpdatable, const bool bRequired,
                             const bool bMultiValude, const bool bOpenChoice,
                             com::sun::star::uno::Any& aChoices,
                             com::sun::star::uno::Any& rAny, bool bInteractive );

    inline bool     AreAllLinesValid() const { return m_pPropertiesWin->AreAllLinesValid(); }
    inline void     ClearAllLines() { m_pPropertiesWin->ClearAllLines(); }
    inline ::com::sun::star::uno::Sequence< ::com::sun::star::document::CmisProperty >
                    GetCmisProperties() const
                        { return m_pPropertiesWin->GetCmisProperties(); }
    void    Init(VclBuilderContainer& rParent);
    virtual void setAllocation(const Size &rAllocation);
};

// class SfxCmisPropertiesPage -------------------------------------------------

class SfxCmisPropertiesPage : public SfxTabPage
{
private:
    CmisPropertiesControl* m_pPropertiesCtrl;
    using TabPage::DeactivatePage;

protected:
    SfxCmisPropertiesPage( Window* pParent, const SfxItemSet& );

    virtual sal_Bool    FillItemSet( SfxItemSet& );
    virtual void        Reset( const SfxItemSet& );
    virtual int         DeactivatePage( SfxItemSet* pSet = NULL );

public:
    static SfxTabPage*  Create( Window* pParent, const SfxItemSet& );
};

#endif // #ifndef _SFX_DINFDLG_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
