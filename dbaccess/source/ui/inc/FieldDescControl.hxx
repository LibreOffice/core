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
#ifndef INCLUDED_DBACCESS_SOURCE_UI_INC_FIELDDESCCONTROL_HXX
#define INCLUDED_DBACCESS_SOURCE_UI_INC_FIELDDESCCONTROL_HXX

#include <vcl/tabpage.hxx>
#include <vcl/weld.hxx>
#include "QEnumTypes.hxx"
#include <com/sun/star/sdbc/XDatabaseMetaData.hpp>
#include <com/sun/star/util/XNumberFormatter.hpp>
#include "TypeInfo.hxx"

// field description columns of a table
#define FIELD_NAME                      1
#define FIELD_TYPE                      2
#define HELP_TEXT                       3
#define COLUMN_DESCRIPTION              4

#define FIELD_FIRST_VIRTUAL_COLUMN      5

#define FIELD_PROPERTY_REQUIRED         5
#define FIELD_PROPERTY_NUMTYPE          6
#define FIELD_PROPERTY_AUTOINC          7
#define FIELD_PROPERTY_DEFAULT          8
#define FIELD_PROPERTY_TEXTLEN          9
#define FIELD_PROPERTY_LENGTH           10
#define FIELD_PROPERTY_SCALE            11
#define FIELD_PROPERTY_BOOL_DEFAULT     12
#define FIELD_PROPERTY_FORMAT           13
#define FIELD_PROPERTY_COLUMNNAME       14
#define FIELD_PROPERTY_TYPE             15
#define FIELD_PROPERTY_AUTOINCREMENT    16

class FixedText;
class PushButton;
class ScrollBar;
class Button;
class ListBox;
class Control;

namespace dbaui
{
    class OTableDesignHelpBar;
    class OPropListBoxCtrl;
    class OPropEditCtrl;
    class OPropNumericEditCtrl;
    class OFieldDescription;
    class OPropColumnEditCtrl;

    class OFieldDescControl : public TabPage
    {
    private:
        std::unique_ptr<weld::Builder> m_xBuilder;
        std::unique_ptr<weld::Container> m_xContainer;

        VclPtr<OTableDesignHelpBar>    pHelp;
        weld::Widget* m_pLastFocusWindow;
        weld::Widget* m_pActFocusWindow;

        std::unique_ptr<weld::Label>   m_xDefaultText;
        std::unique_ptr<weld::Label>   m_xRequiredText;
        std::unique_ptr<weld::Label>   m_xAutoIncrementText;
        std::unique_ptr<weld::Label>   m_xTextLenText;
        std::unique_ptr<weld::Label>   m_xNumTypeText;
        std::unique_ptr<weld::Label>   m_xLengthText;
        std::unique_ptr<weld::Label>   m_xScaleText;
        std::unique_ptr<weld::Label>   m_xFormatText;
        std::unique_ptr<weld::Label>   m_xBoolDefaultText;
        std::unique_ptr<weld::Label>   m_xColumnNameText;
        std::unique_ptr<weld::Label>   m_xTypeText;
        std::unique_ptr<weld::Label>   m_xAutoIncrementValueText;

        std::unique_ptr<OPropListBoxCtrl> m_xRequired;
        std::unique_ptr<OPropListBoxCtrl> m_xNumType;
        std::unique_ptr<OPropListBoxCtrl> m_xAutoIncrement;
        std::unique_ptr<OPropEditCtrl> m_xDefault;
        std::unique_ptr<OPropNumericEditCtrl> m_xTextLen;
        std::unique_ptr<OPropNumericEditCtrl> m_xLength;
        std::unique_ptr<OPropNumericEditCtrl> m_xScale;
        std::unique_ptr<OPropEditCtrl> m_xFormatSample;
        std::unique_ptr<OPropListBoxCtrl> m_xBoolDefault;
        std::unique_ptr<OPropColumnEditCtrl> m_xColumnName;
        std::unique_ptr<OPropListBoxCtrl> m_xType;
        std::unique_ptr<OPropEditCtrl> m_xAutoIncrementValue;

        std::unique_ptr<weld::Button>  m_xFormat;

        TOTypeInfoSP            m_pPreviousType;
        short                   m_nPos;
        OUString                aYes;
        OUString                aNo;

        sal_Int32               m_nEditWidth;

        bool                m_bAdded;

        OFieldDescription*      pActFieldDescr;

        DECL_LINK(OnScroll, weld::ScrolledWindow&, void);

        DECL_LINK(FormatClickHdl, weld::Button&, void);
        DECL_LINK(ChangeHdl, weld::ComboBox&, void);

        // used by ActivatePropertyField
        DECL_LINK( OnControlFocusLost, weld::Widget&, void );
        DECL_LINK( OnControlFocusGot, weld::Widget&, void );

        void                UpdateFormatSample(OFieldDescription const * pFieldDescr);

        bool                isTextFormat(const OFieldDescription* _pFieldDescr,sal_uInt32& _nFormatKey) const;
        std::unique_ptr<OPropNumericEditCtrl> CreateNumericControl(const OString& rId, const char* pHelpId, short _nProperty, const OString& _sHelpId);
        void                InitializeControl(weld::Widget* _pControl,const OString& _sHelpId);
        void                InitializeControl(OPropListBoxCtrl* _pControl,const OString& _sHelpId,bool _bAddChangeHandler);

    protected:
        void                saveCurrentFieldDescData() { SaveData( pActFieldDescr ); }
        OFieldDescription*  getCurrentFieldDescData() { return pActFieldDescr; }
        void                setCurrentFieldDescData( OFieldDescription* _pDesc ) { pActFieldDescr = _pDesc; }

        virtual void        ActivateAggregate( EControlType eType );
        virtual void        DeactivateAggregate( EControlType eType );
        virtual bool        IsReadOnly() { return false; };

        virtual css::uno::Reference< css::util::XNumberFormatter > GetFormatter() const = 0;

        virtual css::lang::Locale               GetLocale() const = 0;

        virtual void                            CellModified(long nRow, sal_uInt16 nColId ) = 0;
        virtual void                            SetModified(bool bModified);    // base implementation is empty

        virtual TOTypeInfoSP                    getTypeInfo(sal_Int32 _nPos)        = 0;
        virtual const OTypeInfoMap*             getTypeInfo() const  = 0;

        virtual bool                            isAutoIncrementValueEnabled() const = 0;
        virtual OUString                        getAutoIncrementValue() const = 0;

        OUString                                BoolStringPersistent(const OUString& rUIString) const;
        OUString                                BoolStringUI(const OUString& rPersistentString) const;

        const OPropColumnEditCtrl*              getColumnCtrl() const { return m_xColumnName.get(); }

        void    implFocusLost(weld::Widget* _pWhich);

    public:
        OFieldDescControl(weld::Container* pPage, vcl::Window* pParent, OTableDesignHelpBar* pHelpBar);
        virtual ~OFieldDescControl() override;
        virtual void        dispose() override;

        void                DisplayData(OFieldDescription* pFieldDescr );

        void                SaveData( OFieldDescription* pFieldDescr );

        void                SetControlText( sal_uInt16 nControlId, const OUString& rText );
        void                SetReadOnly( bool bReadOnly );

        // when resize is called
        bool                isCutAllowed() const;
        bool                isCopyAllowed() const;
        bool                isPasteAllowed() const;

        void                cut();
        void                copy();
        void                paste();

        void                Init();
        virtual void        GetFocus() override;
        virtual void        LoseFocus() override;

        virtual css::uno::Reference< css::sdbc::XDatabaseMetaData> getMetaData() = 0;
        virtual css::uno::Reference< css::sdbc::XConnection> getConnection() = 0;

        OUString            getControlDefault( const OFieldDescription* _pFieldDescr, bool _bCheck = true) const;

        void setEditWidth(sal_Int32 _nWidth) { m_nEditWidth = _nWidth; }
    };
}
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
