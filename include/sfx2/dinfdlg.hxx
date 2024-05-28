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
#ifndef INCLUDED_SFX2_DINFDLG_HXX
#define INCLUDED_SFX2_DINFDLG_HXX

#include <config_options.h>
#include <sal/config.h>
#include <sfx2/dllapi.h>

#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/util/DateTime.hpp>
#include <com/sun/star/util/Duration.hpp>
#include <com/sun/star/document/CmisProperty.hpp>

#include <svl/numformat.hxx>
#include <svl/stritem.hxx>
#include <svtools/ctrlbox.hxx>

#include <vcl/idle.hxx>
#include <vcl/weldutils.hxx>

#include <sfx2/tabdlg.hxx>

#include <optional>
#include <memory>
#include <vcl/abstdlg.hxx>

namespace com::sun::star::beans { struct PropertyValue; }

namespace com::sun::star {
    namespace document {
        class XDocumentProperties;
    }
}

struct CustomProperty;
class SvtCalendarBox;

// class SfxDocumentInfoItem ---------------------------------------------

class UNLESS_MERGELIBS(SFX2_DLLPUBLIC) SfxDocumentInfoItem final : public SfxStringItem
{
private:
    sal_Int32                    m_AutoloadDelay;
    OUString                     m_AutoloadURL;
    bool                         m_isAutoloadEnabled;
    OUString                     m_DefaultTarget;
    OUString                     m_TemplateName;
    OUString                     m_Author;
    css::util::DateTime          m_CreationDate;
    OUString                     m_ModifiedBy;
    css::util::DateTime          m_ModificationDate;
    OUString                     m_PrintedBy;
    css::util::DateTime          m_PrintDate;
    sal_Int16                    m_EditingCycles;
    sal_Int32                    m_EditingDuration;
    OUString                     m_Description;
    OUString                     m_Keywords;
    OUString                     m_Contributor;
    OUString                     m_Coverage;
    OUString                     m_Identifier;
    OUString                     m_Publisher;
    OUString                     m_Relation;
    OUString                     m_Rights;
    OUString                     m_Source;
    OUString                     m_Type;
    OUString                     m_Subject;
    OUString                     m_Title;
    sal_Int64                    m_nFileSize;
    bool                         m_bHasTemplate;
    bool                         m_bDeleteUserData;
    bool                         m_bUseUserData;
    bool                         m_bUseThumbnailSave;
    std::vector< std::unique_ptr<CustomProperty> >    m_aCustomProperties;
    css::uno::Sequence< css::document::CmisProperty > m_aCmisProperties;

public:
    static SfxPoolItem* CreateDefault();
    SfxDocumentInfoItem();
    SfxDocumentInfoItem( const OUString &rFileName,
        const css::uno::Reference< css::document::XDocumentProperties> & i_xDocProps,
        const css::uno::Sequence< css::document::CmisProperty> & i_cmisProps,
        bool bUseUserData, bool bUseThumbnailSave, sal_Int64 nFileSize );
    SfxDocumentInfoItem( const SfxDocumentInfoItem& );
    virtual ~SfxDocumentInfoItem() override;

    /// update i_xDocProps with the data in this object
    void UpdateDocumentInfo(
        const css::uno::Reference< css::document::XDocumentProperties> & i_xDocProps,
        bool i_bDoNotUpdateUserDefined = false)
        const;
    bool        isCmisDocument() const { return m_aCmisProperties.hasElements();}

    bool        isAutoloadEnabled() const { return m_isAutoloadEnabled; }
    sal_Int32   getAutoloadDelay() const { return m_AutoloadDelay; }
    const OUString& getAutoloadURL() const { return m_AutoloadURL; }
    const OUString& getDefaultTarget() const { return m_DefaultTarget; }
    const OUString& getTemplateName() const { return m_TemplateName; }
    const OUString& getAuthor() const { return m_Author; }

    const css::util::DateTime&
                getCreationDate() const { return m_CreationDate; }
    const OUString& getModifiedBy() const { return m_ModifiedBy; }

    const css::util::DateTime&
                getModificationDate() const { return m_ModificationDate; }
    const OUString& getPrintedBy() const { return m_PrintedBy; }
    const css::util::DateTime&
                getPrintDate() const { return m_PrintDate; }
    sal_Int16   getEditingCycles() const { return m_EditingCycles; }
    sal_Int32   getEditingDuration() const { return m_EditingDuration; }
    const OUString& getDescription() const { return m_Description; }
    void        setDescription(const OUString& i_val) { m_Description = i_val; }
    const OUString& getKeywords() const { return m_Keywords; }
    void        setKeywords(const OUString& i_val) { m_Keywords = i_val; }
    const OUString& getContributor() const { return m_Contributor; }
    void        setContributor(const OUString& i_val) { m_Contributor = i_val; }
    const OUString& getCoverage() const { return m_Coverage; }
    void        setCoverage(const OUString& i_val) { m_Coverage = i_val; }
    const OUString& getIdentifier() const { return m_Identifier; }
    void        setIdentifier(const OUString& i_val) { m_Identifier = i_val; }
    const OUString& getPublisher() const { return m_Publisher; }
    void        setPublisher(const OUString& i_val) { m_Publisher = i_val; }
    const OUString& getRelation() const { return m_Relation; }
    void        setRelation(const OUString& i_val) { m_Relation = i_val; }
    const OUString& getRights() const { return m_Rights; }
    void        setRights(const OUString& i_val) { m_Rights = i_val; }
    const OUString& getSource() const { return m_Source; }
    void        setSource(const OUString& i_val) { m_Source = i_val; }
    const OUString& getType() const { return m_Type; }
    void        setType(const OUString& i_val) { m_Type = i_val; }
    const OUString& getSubject() const { return m_Subject; }
    void        setSubject(const OUString& i_val) { m_Subject = i_val; }
    const OUString& getTitle() const { return m_Title; }
    void        setTitle(const OUString& i_val) { m_Title = i_val; }
    sal_Int64   getFileSize() const { return m_nFileSize; }
    void setFileSize(sal_Int64 i_val) { m_nFileSize = i_val; }

    /// reset user-specific data (author, modified-by, ...)
    void        resetUserData(const OUString & i_rAuthor);

    void        SetTemplate( bool b ) { m_bHasTemplate = b; }
    bool        HasTemplate() const { return m_bHasTemplate; }
    void        SetDeleteUserData( bool bSet );
    void        SetUseUserData( bool bSet );
    void        SetUseThumbnailSave( bool bSet );
    bool        IsUseUserData() const { return m_bUseUserData;}
    bool        IsUseThumbnailSave() const { return m_bUseThumbnailSave;}


    std::vector< std::unique_ptr<CustomProperty> > GetCustomProperties() const;
    void        ClearCustomProperties();
    void        AddCustomProperty(  const OUString& sName,
                                    const css::uno::Any& rValue );

    const css::uno::Sequence< css::document::CmisProperty >&
                        GetCmisProperties() const { return m_aCmisProperties;}

    void        SetCmisProperties(const css::uno::Sequence< css::document::CmisProperty >& cmisProps );
    virtual SfxDocumentInfoItem* Clone( SfxItemPool* pPool = nullptr ) const override;
    virtual bool            operator==( const SfxPoolItem& ) const override;
    virtual bool        QueryValue( css::uno::Any& rVal, sal_uInt8 nMemberId = 0 ) const override;
    virtual bool        PutValue( const css::uno::Any& rVal, sal_uInt8 nMemberId ) override;
};

// class SfxDocumentPage -------------------------------------------------

class SfxDocumentPage final : public SfxTabPage
{
private:
    OUString                    m_aUnknownSize;
    OUString                    m_aMultiSignedStr;

    bool                        bEnableUseUserData  : 1,
                                bHandleDelete       : 1;

    std::unique_ptr<weld::Image> m_xBmp;
    std::unique_ptr<weld::Label> m_xNameED;
    std::unique_ptr<weld::Button> m_xChangePassBtn;

    std::unique_ptr<weld::Label> m_xShowTypeFT;
    std::unique_ptr<weld::LinkButton> m_xFileValEd;
    std::unique_ptr<weld::Label> m_xShowSizeFT;

    std::unique_ptr<weld::Label> m_xCreateValFt;
    std::unique_ptr<weld::Label> m_xChangeValFt;
    std::unique_ptr<weld::Label> m_xSignedValFt;
    std::unique_ptr<weld::Button> m_xSignatureBtn;
    std::unique_ptr<weld::Label> m_xPrintValFt;
    std::unique_ptr<weld::Label> m_xTimeLogValFt;
    std::unique_ptr<weld::Label> m_xDocNoValFt;

    std::unique_ptr<weld::CheckButton> m_xUseUserDataCB;
    std::unique_ptr<weld::Button> m_xDeleteBtn;
    std::unique_ptr<weld::CheckButton> m_xUseThumbnailSaveCB;

    std::unique_ptr<weld::Label> m_xTemplFt;
    std::unique_ptr<weld::Label> m_xTemplValFt;
    std::unique_ptr<weld::CheckButton> m_xImagePreferredDpiCheckButton;
    std::unique_ptr<weld::ComboBox> m_xImagePreferredDpiComboBox;

    DECL_LINK(DeleteHdl, weld::Button&, void);
    DECL_LINK(SignatureHdl, weld::Button&, void);
    DECL_LINK(ChangePassHdl, weld::Button&, void);
    DECL_LINK(ImagePreferredDPICheckBoxClicked, weld::Toggleable&, void);

    void                ImplUpdateSignatures();
    void                ImplCheckPasswordState();

    virtual bool        FillItemSet( SfxItemSet* ) override;
    virtual void        Reset( const SfxItemSet* ) override;
    VclPtr<AbstractPasswordToOpenModifyDialog> m_xPasswordDialog;

public:
    SfxDocumentPage(weld::Container* pPage, weld::DialogController* pController, const SfxItemSet&);
    static std::unique_ptr<SfxTabPage> Create( weld::Container* pPage, weld::DialogController* pController, const SfxItemSet* );
    virtual ~SfxDocumentPage() override;

    void                EnableUseUserData();
};

// class SfxDocumentDescPage ---------------------------------------------

class SfxDocumentDescPage final : public SfxTabPage
{
private:
    SfxDocumentInfoItem* m_pInfoItem;
    std::unique_ptr<weld::Entry> m_xTitleEd;
    std::unique_ptr<weld::Entry> m_xThemaEd;
    std::unique_ptr<weld::Entry> m_xKeywordsEd;
    std::unique_ptr<weld::ComboBox> m_xMoreTypes;
    std::unique_ptr<weld::Entry> m_xMoreValue;
    std::unique_ptr<weld::TextView> m_xCommentEd;

    DECL_LINK(MoreTypesHdl, weld::ComboBox&, void);
    DECL_LINK(MoreValueHdl, weld::Entry&, void);

    OUString m_sMoreValue_Contributor;
    OUString m_sMoreValue_Coverage;
    OUString m_sMoreValue_Identifier;
    OUString m_sMoreValue_Publisher;
    OUString m_sMoreValue_Relation;
    OUString m_sMoreValue_Rights;
    OUString m_sMoreValue_Source;
    OUString m_sMoreValue_Type;

    OUString m_sMoreValue_Contributor_Stored;
    OUString m_sMoreValue_Coverage_Stored;
    OUString m_sMoreValue_Identifier_Stored;
    OUString m_sMoreValue_Publisher_Stored;
    OUString m_sMoreValue_Relation_Stored;
    OUString m_sMoreValue_Rights_Stored;
    OUString m_sMoreValue_Source_Stored;
    OUString m_sMoreValue_Type_Stored;

    virtual bool            FillItemSet( SfxItemSet* ) override;
    virtual void            Reset( const SfxItemSet* ) override;

public:
    SfxDocumentDescPage(weld::Container* pPage, weld::DialogController* pController, const SfxItemSet&);
    virtual ~SfxDocumentDescPage() override;
    static std::unique_ptr<SfxTabPage> Create( weld::Container* pPage, weld::DialogController* pController, const SfxItemSet* );
};

// class SfxDocumentInfoDialog -------------------------------------------

class SFX2_DLLPUBLIC SfxDocumentInfoDialog final : public SfxTabDialogController
{
    virtual void PageCreated(const OUString& rId, SfxTabPage& rPage) override;

public:
    SfxDocumentInfoDialog(weld::Window* pParent, const SfxItemSet&);
    void AddFontTabPage();
};

// class CustomPropertiesRemoveButton ------------------------------------
struct CustomPropertyLine;

class CustomPropertiesDateField
{
private:
    std::unique_ptr<SvtCalendarBox> m_xDateField;
public:
    ::std::optional<sal_Int16> m_TZ;

    CustomPropertiesDateField(SvtCalendarBox* pDateField);
    void set_visible(bool bVisible);
    Date get_date() const;
    void set_date(const Date& rDate);
    ~CustomPropertiesDateField();
};

namespace weld
{
    class TimeFormatter;
}

class CustomPropertiesTimeField
{
public:
    std::unique_ptr<weld::FormattedSpinButton> m_xTimeField;
    std::unique_ptr<weld::TimeFormatter> m_xFormatter;
    bool m_isUTC;

    CustomPropertiesTimeField(std::unique_ptr<weld::FormattedSpinButton> xTimeField);
    void set_visible(bool bVisible) { m_xTimeField->set_visible(bVisible); }
    tools::Time get_value() const;
    void set_value(const tools::Time& rTime);

    ~CustomPropertiesTimeField();
};

class DurationDialog_Impl final : public weld::GenericDialogController
{
    std::unique_ptr<weld::CheckButton> m_xNegativeCB;
    std::unique_ptr<weld::SpinButton> m_xYearNF;
    std::unique_ptr<weld::SpinButton> m_xMonthNF;
    std::unique_ptr<weld::SpinButton> m_xDayNF;
    std::unique_ptr<weld::SpinButton> m_xHourNF;
    std::unique_ptr<weld::SpinButton> m_xMinuteNF;
    std::unique_ptr<weld::SpinButton> m_xSecondNF;
    std::unique_ptr<weld::SpinButton> m_xMSecondNF;

public:
    DurationDialog_Impl(weld::Widget* pParent, const css::util::Duration& rDuration);
    css::util::Duration  GetDuration() const;
};

class CustomPropertiesDurationField
{
    css::util::Duration             m_aDuration;
    std::unique_ptr<weld::Entry>    m_xEntry;
    std::unique_ptr<weld::Button>   m_xEditButton;
    std::shared_ptr<DurationDialog_Impl> m_xDurationDialog;

    DECL_LINK(ClickHdl, weld::Button&, void);
public:
    CustomPropertiesDurationField(std::unique_ptr<weld::Entry> xEntry,
                                  std::unique_ptr<weld::Button> xEditButton);

    void SetDuration( const css::util::Duration& rDuration );
    const css::util::Duration& GetDuration() const { return m_aDuration; }

    void set_visible(bool bVisible);

    ~CustomPropertiesDurationField();
};

class CustomPropertiesYesNoButton
{
private:
    std::unique_ptr<weld::Widget> m_xTopLevel;
    std::unique_ptr<weld::RadioButton> m_xYesButton;
    std::unique_ptr<weld::RadioButton> m_xNoButton;

public:
    CustomPropertiesYesNoButton(std::unique_ptr<weld::Widget>,
                                std::unique_ptr<weld::RadioButton> xYesButton,
                                std::unique_ptr<weld::RadioButton> xNoButton);
    ~CustomPropertiesYesNoButton();

    void     CheckYes() { m_xYesButton->set_active(true); }
    void     CheckNo() { m_xNoButton->set_active(true); }
    bool     IsYesChecked() const { return m_xYesButton->get_active(); }
    void     set_visible(bool bVisible) { m_xTopLevel->set_visible(bVisible); }
};

class CustomPropertiesWindow;

// struct CustomPropertyLine ---------------------------------------------
struct CustomPropertyLine
{
    CustomPropertiesWindow* m_pParent;

    std::unique_ptr<weld::Builder> m_xBuilder;
    std::unique_ptr<weld::Container> m_xLine;
    std::unique_ptr<weld::ComboBox> m_xNameBox;
    std::unique_ptr<weld::ComboBox> m_xTypeBox;
    std::unique_ptr<weld::Entry> m_xValueEdit;
    std::unique_ptr<weld::Widget> m_xDateTimeBox;
    std::unique_ptr<CustomPropertiesDateField> m_xDateField;
    std::unique_ptr<CustomPropertiesTimeField> m_xTimeField;
    std::unique_ptr<weld::Widget> m_xDurationBox;
    std::unique_ptr<CustomPropertiesDurationField> m_xDurationField;
    std::unique_ptr<CustomPropertiesYesNoButton> m_xYesNoButton;
    std::unique_ptr<weld::Button> m_xRemoveButton;

    bool m_bTypeLostFocus;

    CustomPropertyLine(CustomPropertiesWindow* pParent, weld::Widget* pContainer);
    DECL_LINK(TypeHdl, weld::ComboBox&, void);
    DECL_LINK(RemoveHdl, weld::Button&, void);
    DECL_LINK(EditLoseFocusHdl, weld::Widget&, void);
    DECL_LINK(BoxLoseFocusHdl, weld::Widget&, void);

    void DoTypeHdl(const weld::ComboBox& rBox);

    void Clear();
    void Hide();
};

// class CustomPropertiesWindow ------------------------------------------

class CustomPropertiesWindow
{
private:
    sal_Int32                           m_nHeight;
    sal_Int32                           m_nLineHeight;
    sal_Int32                           m_nScrollPos;
    std::vector<std::unique_ptr<CustomProperty>> m_aCustomProperties;
    std::vector<std::unique_ptr<CustomPropertyLine>> m_aCustomPropertiesLines;
    CustomPropertyLine*                 m_pCurrentLine;
    SvNumberFormatter                   m_aNumberFormatter;
    Idle                                m_aEditLoseFocusIdle;
    Idle                                m_aBoxLoseFocusIdle;
    Link<void*,void>                    m_aRemovedHdl;

    weld::Container& m_rBody;
    weld::Label& m_rHeaderAccName;
    weld::Label& m_rHeaderAccType;
    weld::Label& m_rHeaderAccValue;

    DECL_LINK(EditTimeoutHdl, Timer *, void);
    DECL_LINK(BoxTimeoutHdl, Timer *, void);

    bool        IsLineValid( CustomPropertyLine* pLine ) const;
    void        ValidateLine( CustomPropertyLine* pLine, bool bIsFromTypeBox );
    void        CreateNewLine();
    void        StoreCustomProperties();
    sal_uInt32  GetCurrentDataModelPosition() const { return -1 * m_nScrollPos / m_nLineHeight; }

public:
    CustomPropertiesWindow(weld::Container& rParent, weld::Label& rHeaderAccName,
                           weld::Label& rHeaderAccType, weld::Label& rHeaderAccValue);
    ~CustomPropertiesWindow();

    sal_uInt16          GetExistingLineCount() const { return m_aCustomPropertiesLines.size(); }
    sal_uInt16          GetTotalLineCount() const { return m_aCustomProperties.size(); }
    void                SetVisibleLineCount(sal_uInt32 nCount);
    sal_Int32           GetHeight() const { return m_nHeight; }
    void                SetHeight(int nHeight) { m_nHeight = nHeight; }
    sal_Int32           GetLineHeight() const { return m_nLineHeight; }
    void                SetLineHeight(sal_Int32 nLineHeight) { m_nLineHeight = nLineHeight; }
    void                AddLine( const OUString& sName, css::uno::Any const & rAny );
    bool                AreAllLinesValid() const;
    void                ClearAllLines();
    void                DoScroll( sal_Int32 nNewPos );
    void                ReloadLinesContent();

    css::uno::Sequence< css::beans::PropertyValue >
                        GetCustomProperties();
    void                SetCustomProperties(std::vector< std::unique_ptr<CustomProperty> >&& rProperties);
    void                SetRemovedHdl( const Link<void*,void>& rLink ) { m_aRemovedHdl = rLink; }

    void                BoxLoseFocus(CustomPropertyLine* pLine);
    void                EditLoseFocus(CustomPropertyLine* pLine);
    void                Remove(const CustomPropertyLine* pLine);
};

// class CustomPropertiesControl -----------------------------------------

class CustomPropertiesControl
{
private:
    sal_Int32               m_nThumbPos;

    std::unique_ptr<weld::Widget> m_xBox;
    std::unique_ptr<weld::Container> m_xBody;
    std::unique_ptr<weld::ScrolledWindow> m_xVertScroll;
    std::unique_ptr<CustomPropertiesWindow> m_xPropertiesWin;
    std::unique_ptr<weld::Label> m_xName;
    std::unique_ptr<weld::Label> m_xType;
    std::unique_ptr<weld::Label> m_xValue;

    DECL_LINK( ResizeHdl, const Size&, void );
    DECL_LINK( ScrollHdl, weld::ScrolledWindow&, void );
    DECL_LINK( RemovedHdl, void*, void );

public:
    CustomPropertiesControl();
    ~CustomPropertiesControl();

    void         AddLine(css::uno::Any const & rAny);

    bool         AreAllLinesValid() const { return m_xPropertiesWin->AreAllLinesValid(); }
    void         ClearAllLines() { m_xPropertiesWin->ClearAllLines(); }

    css::uno::Sequence<css::beans::PropertyValue>
                 GetCustomProperties() const
                        { return m_xPropertiesWin->GetCustomProperties(); }
    void         SetCustomProperties(std::vector< std::unique_ptr<CustomProperty> >&& rProperties);

    void         Init(weld::Builder& rParent);
};

// class SfxCustomPropertiesPage -----------------------------------------

class SfxCustomPropertiesPage final : public SfxTabPage
{
private:
    DECL_LINK(AddHdl, weld::Button&, void);

    std::unique_ptr<CustomPropertiesControl> m_xPropertiesCtrl;
    std::unique_ptr<weld::Button> m_xAdd;

    virtual bool        FillItemSet( SfxItemSet* ) override;
    virtual void        Reset( const SfxItemSet* ) override;
    virtual DeactivateRC DeactivatePage( SfxItemSet* pSet ) override;

public:
    SfxCustomPropertiesPage(weld::Container* pPage, weld::DialogController* pController, const SfxItemSet&);
    static std::unique_ptr<SfxTabPage> Create( weld::Container* pPage, weld::DialogController* pController, const SfxItemSet* );
    virtual ~SfxCustomPropertiesPage() override;
};

struct CmisValue
{
    std::unique_ptr<weld::Builder> m_xBuilder;
    std::unique_ptr<weld::Frame>  m_xFrame;
    std::unique_ptr<weld::Entry> m_xValueEdit;

    CmisValue(weld::Widget* pParent, const OUString& rStr);
};

struct CmisDateTime
{
    std::unique_ptr<weld::Builder> m_xBuilder;
    std::unique_ptr<weld::Frame>  m_xFrame;
    std::unique_ptr<SvtCalendarBox>  m_xDateField;
    std::unique_ptr<weld::FormattedSpinButton> m_xTimeField;
    std::unique_ptr<weld::TimeFormatter> m_xFormatter;

    CmisDateTime(weld::Widget* pParent, const css::util::DateTime& rDateTime);
};

struct CmisYesNo
{
    std::unique_ptr<weld::Builder> m_xBuilder;
    std::unique_ptr<weld::Frame>  m_xFrame;
    std::unique_ptr<weld::RadioButton> m_xYesButton;
    std::unique_ptr<weld::RadioButton> m_xNoButton;

    CmisYesNo(weld::Widget* pParent, bool bValue);
};

// struct CmisPropertyLine ---------------------------------------------

struct CmisPropertyLine
{
    std::unique_ptr<weld::Builder> m_xBuilder;
    OUString                      m_sId;
    OUString                      m_sType;
    bool                          m_bUpdatable;
    bool                          m_bRequired;
    bool                          m_bMultiValued;
    bool                          m_bOpenChoice;
    std::unique_ptr<weld::Frame>  m_xFrame;
    std::unique_ptr<weld::Label>  m_xName;
    std::unique_ptr<weld::Label>  m_xType;
    std::vector< std::unique_ptr<CmisValue> >     m_aValues;
    std::vector< std::unique_ptr<CmisDateTime> >  m_aDateTimes;
    std::vector< std::unique_ptr<CmisYesNo> >     m_aYesNos;
    CmisPropertyLine(weld::Widget* pParent);
    ~CmisPropertyLine();
};

// class CmisPropertiesWindow ------------------------------------------

class CmisPropertiesWindow
{
private:
    std::unique_ptr<weld::Container>    m_xBox;
    SvNumberFormatter                   m_aNumberFormatter;
    std::vector< std::unique_ptr<CmisPropertyLine> > m_aCmisPropertiesLines;
public:
    CmisPropertiesWindow(std::unique_ptr<weld::Container> xParent);
    ~CmisPropertiesWindow();

    void AddLine( const OUString& sId, const OUString& sName,
                  const OUString& sType, const bool bUpdatable,
                  const bool bRequired, const bool bMultiValued,
                  const bool bOpenChoice,
                  css::uno::Any& aChoices,
                  css::uno::Any const & rAny );
    void ClearAllLines();

    css::uno::Sequence< css::document::CmisProperty >
                        GetCmisProperties() const;
};

// class CmisPropertiesControl -----------------------------------------

class CmisPropertiesControl
{
private:
    CmisPropertiesWindow m_aPropertiesWin;
    std::unique_ptr<weld::ScrolledWindow> m_xScrolledWindow;

public:
    CmisPropertiesControl(weld::Builder& rBuilder);

    void AddLine( const OUString& sId, const OUString& sName,
                  const OUString& sType, const bool bUpdatable,
                  const bool bRequired, const bool bMultiValude,
                  const bool bOpenChoice,
                  css::uno::Any& aChoices,
                  css::uno::Any const & rAny );

    void ClearAllLines();
    css::uno::Sequence< css::document::CmisProperty >
                    GetCmisProperties() const
                        { return m_aPropertiesWin.GetCmisProperties(); }
};

// class SfxCmisPropertiesPage -------------------------------------------------

class SfxCmisPropertiesPage final : public SfxTabPage
{
private:
    std::unique_ptr<CmisPropertiesControl> m_xPropertiesCtrl;

    virtual bool        FillItemSet( SfxItemSet* ) override;
    virtual void        Reset( const SfxItemSet* ) override;
    virtual DeactivateRC DeactivatePage( SfxItemSet* pSet ) override;

public:
    SfxCmisPropertiesPage(weld::Container* pPage, weld::DialogController* pController, const SfxItemSet&);
    virtual ~SfxCmisPropertiesPage() override;
    static std::unique_ptr<SfxTabPage> Create(weld::Container* pPage, weld::DialogController* pController, const SfxItemSet*);
};

#endif // #ifndef _ INCLUDED_SFX2_DINFDLG_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
