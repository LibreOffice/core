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
        OTableDesignHelpBar*    pHelp;
        vcl::Window*                 pLastFocusWindow;
        vcl::Window*                 m_pActFocusWindow;

        FixedText*              pDefaultText;
        FixedText*              pRequiredText;
        FixedText*              pAutoIncrementText;
        FixedText*              pTextLenText;
        FixedText*              pNumTypeText;
        FixedText*              pLengthText;
        FixedText*              pScaleText;
        FixedText*              pFormatText;
        FixedText*              pBoolDefaultText;
        FixedText*              m_pColumnNameText;
        FixedText*              m_pTypeText;
        FixedText*              m_pAutoIncrementValueText;

        OPropListBoxCtrl*       pRequired;
        OPropListBoxCtrl*       pNumType;
        OPropListBoxCtrl*       pAutoIncrement;
        OPropEditCtrl*          pDefault;
        OPropNumericEditCtrl*   pTextLen;
        OPropNumericEditCtrl*   pLength;
        OPropNumericEditCtrl*   pScale;
        OPropEditCtrl*          pFormatSample;
        OPropListBoxCtrl*       pBoolDefault;
        OPropColumnEditCtrl*    m_pColumnName;
        OPropListBoxCtrl*       m_pType;
        OPropEditCtrl*          m_pAutoIncrementValue;

        PushButton*             pFormat;

        ScrollBar*              m_pVertScroll;
        ScrollBar*              m_pHorzScroll;

        TOTypeInfoSP            m_pPreviousType;
        short                   m_nPos;
        OUString                aYes;
        OUString                aNo;

        long                    m_nOldVThumb;
        long                    m_nOldHThumb;
        sal_Int32               m_nWidth;

        bool                m_bAdded;
        bool                    m_bRightAligned;

        OFieldDescription*      pActFieldDescr;

        DECL_LINK( OnScroll, ScrollBar*);

        DECL_LINK( FormatClickHdl, Button * );
        DECL_LINK( ChangeHdl, ListBox * );

        // used by ActivatePropertyField
        DECL_LINK( OnControlFocusLost, Control* );
        DECL_LINK( OnControlFocusGot, Control* );

        void                UpdateFormatSample(OFieldDescription* pFieldDescr);
        void                ArrangeAggregates();

        void                SetPosSize( Control** ppControl, long nRow, sal_uInt16 nCol );

        void                ScrollAggregate(Control* pText, Control* pInput, Control* pButton, long nDeltaX, long nDeltaY);
        void                ScrollAllAggregates();

        bool                isTextFormat(const OFieldDescription* _pFieldDescr,sal_uInt32& _nFormatKey) const;
        void                Contruct();
        OPropNumericEditCtrl* CreateNumericControl(sal_uInt16 _nHelpStr,short _nProperty,const OString& _sHelpId);
        FixedText*          CreateText(sal_uInt16 _nTextRes);
        void                InitializeControl(Control* _pControl,const OString& _sHelpId,bool _bAddChangeHandler);

    protected:
        inline  void    setRightAligned()       { m_bRightAligned = true; }
        inline  bool    isRightAligned() const  { return m_bRightAligned; }

        inline  void                saveCurrentFieldDescData() { SaveData( pActFieldDescr ); }
        inline  OFieldDescription*  getCurrentFieldDescData() { return pActFieldDescr; }
        inline  void                setCurrentFieldDescData( OFieldDescription* _pDesc ) { pActFieldDescr = _pDesc; }

        sal_uInt16          CountActiveAggregates() const;
        sal_Int32           GetMaxControlHeight() const;

        virtual void        ActivateAggregate( EControlType eType );
        virtual void        DeactivateAggregate( EControlType eType );
        virtual bool        IsReadOnly() { return false; };

        virtual ::com::sun::star::uno::Reference< ::com::sun::star::util::XNumberFormatter > GetFormatter() const = 0;

        virtual ::com::sun::star::lang::Locale  GetLocale() const = 0;

        virtual void                            CellModified(long nRow, sal_uInt16 nColId ) = 0;
        virtual void                            SetModified(bool bModified);    // base implementation is empty

        virtual TOTypeInfoSP                    getTypeInfo(sal_Int32 _nPos)        = 0;
        virtual const OTypeInfoMap*             getTypeInfo() const  = 0;

        virtual bool                        isAutoIncrementValueEnabled() const = 0;
        virtual OUString                 getAutoIncrementValue() const = 0;

        OUString                                BoolStringPersistent(const OUString& rUIString) const;
        OUString                                BoolStringUI(const OUString& rPersistentString) const;

        const OPropColumnEditCtrl*              getColumnCtrl() const { return m_pColumnName; }

        void    implFocusLost(vcl::Window* _pWhich);

    public:
        OFieldDescControl( vcl::Window* pParent, OTableDesignHelpBar* pHelpBar);
        virtual ~OFieldDescControl();

        void                DisplayData(OFieldDescription* pFieldDescr );

        void                SaveData( OFieldDescription* pFieldDescr );

        void                SetControlText( sal_uInt16 nControlId, const OUString& rText );
        OUString            GetControlText( sal_uInt16 nControlId );
        void                SetReadOnly( bool bReadOnly );

        // when resize is called
        void                CheckScrollBars();
        bool                isCutAllowed();
        bool                isCopyAllowed();
        bool                isPasteAllowed();

        void                cut();
        void                copy();
        void                paste();

        virtual void        Init();
        virtual void        GetFocus() SAL_OVERRIDE;
        virtual void        LoseFocus() SAL_OVERRIDE;
        virtual void        Resize() SAL_OVERRIDE;

        virtual ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XDatabaseMetaData> getMetaData() = 0;
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection> getConnection() = 0;

        OUString            getControlDefault( const OFieldDescription* _pFieldDescr, bool _bCheck = true) const;

        inline void setEditWidth(sal_Int32 _nWidth) { m_nWidth = _nWidth; }
    };
}
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
