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
#ifndef INCLUDED_DBACCESS_SOURCE_UI_INC_QUERYDESIGNVIEW_HXX
#define INCLUDED_DBACCESS_SOURCE_UI_INC_QUERYDESIGNVIEW_HXX

#include "queryview.hxx"
#include <vcl/split.hxx>
#include "QEnumTypes.hxx"
#include <com/sun/star/beans/XPropertySet.hpp>
#include "querycontroller.hxx"
#include "ConnectionLineData.hxx"

namespace connectivity
{
    class OSQLParseNode;
}

class ComboBox;
namespace dbaui
{
    enum SqlParseError
    {
        eIllegalJoin,
        eStatementTooLong,
        eNoConnection,
        eNoSelectStatement,
        eStatementTooComplex,
        eColumnInLikeNotFound,
        eNoColumnInLike,
        eColumnNotFound,
        eNativeMode,
        eTooManyTables,
        eTooManyConditions,
        eTooManyColumns,
        eIllegalJoinCondition,
        eOk
    };

    class OSelectionBrowseBox;
    class OQueryContainerWindow;

    class OQueryDesignView : public OQueryView
    {
        enum ChildFocusState
        {
            SELECTION,
            TABLEVIEW,
            NONE
        };

        VclPtr<Splitter>                    m_aSplitter;

        css::lang::Locale                   m_aLocale;
        OUString                            m_sDecimalSep;

        VclPtr<OSelectionBrowseBox>         m_pSelectionBox;    // presents the lower window
        ChildFocusState                     m_eChildFocus;
        bool                                m_bInSplitHandler;

    public:
        OQueryDesignView(OQueryContainerWindow* pParent, OQueryController& _rController,const css::uno::Reference< css::uno::XComponentContext >& );
        virtual ~OQueryDesignView();
        virtual void dispose() SAL_OVERRIDE;

        virtual bool isCutAllowed() SAL_OVERRIDE;
        virtual bool isPasteAllowed() SAL_OVERRIDE;
        virtual bool isCopyAllowed() SAL_OVERRIDE;
        virtual void copy() SAL_OVERRIDE;
        virtual void cut() SAL_OVERRIDE;
        virtual void paste() SAL_OVERRIDE;
        // clears the whole query
        virtual void clear() SAL_OVERRIDE;
        // set the view readonly or not
        virtual void setReadOnly(bool _bReadOnly) SAL_OVERRIDE;
        // check if the statement is correct when not returning false
        bool checkStatement();
        // set the statement for representation
        virtual void setStatement(const OUString& _rsStatement) SAL_OVERRIDE;
        // returns the current sql statement
        virtual OUString getStatement() SAL_OVERRIDE;
        /// late construction
        virtual void Construct() SAL_OVERRIDE;
        virtual void initialize() SAL_OVERRIDE;
        // Window overrides
        virtual bool PreNotify( NotifyEvent& rNEvt ) SAL_OVERRIDE;
        virtual void GetFocus() SAL_OVERRIDE;

        bool isSlotEnabled(sal_Int32 _nSlotId);
        void setSlotEnabled(sal_Int32 _nSlotId, bool _bEnable);
        void setNoneVisbleRow(sal_Int32 _nRows);

        css::lang::Locale      getLocale() const           { return m_aLocale;}
        OUString                     getDecimalSeparator() const { return m_sDecimalSep;}

        SqlParseError   InsertField( const OTableFieldDescRef& rInfo, bool bVis = true, bool bActivate = true);
        bool            HasFieldByAliasName(const OUString& rFieldName, OTableFieldDescRef& rInfo) const;
        // save the position of the table window and the pos of the splitters
        // called when fields are deleted
        void DeleteFields( const OUString& rAliasName );
        // called when a table from tabview was deleted
        void TableDeleted(const OUString& rAliasName);

        sal_Int32 getColWidth( sal_uInt16 _nColPos) const;
        void fillValidFields(const OUString& strTableName, ComboBox* pFieldList);

        void SaveUIConfig();
        void stopTimer();
        void startTimer();
        void reset();

        /** initializes the view from the current parser / parse iterator of the controller

            @param _pErrorInfo
                When not <NULL/>, the instance pointed to by this parameter takes the error
                which happened during the initialization.
                If it is not <NULL/>, then any such error will be displayed, using the controller's
                showError method.

            @return <TRUE/> if and only if the initialization was successful
        */
        bool    initByParseIterator( ::dbtools::SQLExceptionInfo* _pErrorInfo );

        void    initByFieldDescriptions(
                    const css::uno::Sequence< css::beans::PropertyValue >& i_rFieldDescriptions
                );

        ::connectivity::OSQLParseNode* getPredicateTreeFromEntry(   OTableFieldDescRef pEntry,
                                                                    const OUString& _sCriteria,
                                                                    OUString& _rsErrorMessage,
                                                                    css::uno::Reference< css::beans::XPropertySet>& _rxColumn) const;

        void fillFunctionInfo(   const ::connectivity::OSQLParseNode* pNode
                                ,const OUString& sFunctionTerm
                                ,OTableFieldDescRef& aInfo);
    protected:
        // return the Rectangle where I can paint myself
        virtual void resizeDocumentView(Rectangle& rRect) SAL_OVERRIDE;
        DECL_LINK_TYPED( SplitHdl, Splitter*, void );

    private:
        using OQueryView::SaveTabWinUIConfig;
    };
}
#endif // INCLUDED_DBACCESS_SOURCE_UI_INC_QUERYDESIGNVIEW_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
