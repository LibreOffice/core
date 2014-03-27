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
#ifndef INCLUDED_DBACCESS_SOURCE_UI_INC_WTYPESELECT_HXX
#define INCLUDED_DBACCESS_SOURCE_UI_INC_WTYPESELECT_HXX

#include "FieldDescControl.hxx"
#include "TypeInfo.hxx"
#include "WTabPage.hxx"

#include <vcl/button.hxx>
#include <vcl/field.hxx>
#include <vcl/fixed.hxx>
#include <vcl/lstbox.hxx>

class SvStream;
class SvParser;
namespace dbaui
{
    class OTableDesignHelpBar;
    // OWizTypeSelectControl
    class OWizTypeSelectControl : public OFieldDescControl
    {
    protected:
        virtual void        ActivateAggregate( EControlType eType ) SAL_OVERRIDE;
        virtual void        DeactivateAggregate( EControlType eType ) SAL_OVERRIDE;

        virtual void        CellModified(long nRow, sal_uInt16 nColId ) SAL_OVERRIDE;

        virtual ::com::sun::star::lang::Locale  GetLocale() const SAL_OVERRIDE;
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::util::XNumberFormatter > GetFormatter() const SAL_OVERRIDE;
        virtual TOTypeInfoSP        getTypeInfo(sal_Int32 _nPos) SAL_OVERRIDE;
        virtual const OTypeInfoMap* getTypeInfo() const SAL_OVERRIDE;
        virtual sal_Bool            isAutoIncrementValueEnabled() const SAL_OVERRIDE;
        virtual OUString     getAutoIncrementValue() const SAL_OVERRIDE;

    public:
        OWizTypeSelectControl(Window* pParent, const ResId& rResId,OTableDesignHelpBar* pHelpBar=NULL);
        virtual ~OWizTypeSelectControl();

        virtual ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XDatabaseMetaData> getMetaData() SAL_OVERRIDE;
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection> getConnection() SAL_OVERRIDE;
    };

    // Wizard Page: OWizTypeSelectList
    // just defines the ::com::sun::star::ucb::Command for the Contextmenu
    class OWizTypeSelectList : public MultiListBox
    {
        sal_Bool                m_bPKey;
        sal_Bool                IsPrimaryKeyAllowed() const;
        void                    setPrimaryKey(  OFieldDescription* _pFieldDescr,
                                                sal_uInt16 _nPos,
                                                sal_Bool _bSet = sal_False);
    protected:
        virtual bool            PreNotify( NotifyEvent& rNEvt ) SAL_OVERRIDE;
    public:
        OWizTypeSelectList( Window* pParent, WinBits nStyle = WB_BORDER )
            : MultiListBox(pParent,nStyle)
            , m_bPKey(sal_False)
            {}
        OWizTypeSelectList( Window* pParent, const ResId& rResId )
            : MultiListBox(pParent,rResId)
            , m_bPKey(sal_False)
            {}
        void                    SetPKey(sal_Bool bPKey) { m_bPKey = bPKey; }
    };

    // Wizard Page: OWizTypeSelect
    // Serves as base class for different copy properties.
    // Calls FillColumnList, when button AUTO is triggered
    class OWizTypeSelect : public OWizardPage
    {
        friend class OWizTypeSelectControl;
        friend class OWizTypeSelectList;

        DECL_LINK( ColumnSelectHdl, MultiListBox* );
        DECL_LINK( ButtonClickHdl, Button * );
    protected:
        OWizTypeSelectList      m_lbColumnNames;
        FixedLine               m_flColumns;
        OWizTypeSelectControl   m_aTypeControl;
        FixedLine               m_flAutoType;
        FixedText               m_ftAuto;
        NumericField            m_etAuto;
        PushButton              m_pbAuto;

        Image                   m_imgPKey;
        SvStream*               m_pParserStream; // stream to read the tokens from or NULL
        OUString         m_sAutoIncrementValue;
        sal_Int32               m_nDisplayRow;
        sal_Bool                m_bAutoIncrementEnabled;
        sal_Bool                m_bDuplicateName;

        void                    fillColumnList(sal_uInt32 nRows);
        virtual SvParser*       createReader(sal_Int32 _nRows) = 0;

        void                    EnableAuto(sal_Bool bEnable);
    public:
        virtual void            Reset ( ) SAL_OVERRIDE;
        virtual void            ActivatePage( ) SAL_OVERRIDE;
        virtual void            Resize() SAL_OVERRIDE;
        virtual sal_Bool        LeavePage() SAL_OVERRIDE;
        virtual OUString        GetTitle() const SAL_OVERRIDE;

        OWizTypeSelect(Window* pParent, SvStream* _pStream = NULL );
        virtual ~OWizTypeSelect();

        inline void setDisplayRow(sal_Int32 _nRow) { m_nDisplayRow = _nRow - 1; }
        inline void setDuplicateName(sal_Bool _bDuplicateName) { m_bDuplicateName = _bDuplicateName; }
    };

    typedef OWizTypeSelect* (*TypeSelectionPageFactory)( Window*, SvStream& );
}
#endif // INCLUDED_DBACCESS_SOURCE_UI_INC_WTYPESELECT_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
