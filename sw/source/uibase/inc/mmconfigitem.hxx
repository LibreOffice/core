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
#ifndef INCLUDED_SW_SOURCE_UIBASE_INC_MMCONFIGITEM_HXX
#define INCLUDED_SW_SOURCE_UIBASE_INC_MMCONFIGITEM_HXX

#include <com/sun/star/uno/Sequence.hxx>
#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/view/XSelectionChangeListener.hpp>
#include <memory>
#include <set>
#include <vector>
#include <swdbdata.hxx>
#include <swdllapi.h>
#include "sharedconnection.hxx"

namespace com{namespace sun{namespace star{
    namespace sdbc{
        class XDataSource;
        class XResultSet;
    }
    namespace sdbcx{
        class XColumnsSupplier;
    }
}}}

class SwMailMergeConfigItem_Impl;
class SwView;
namespace sw { namespace mark { class IMark; }}

struct SwDocMergeInfo
{
    sw::mark::IMark* startPageInTarget;
    long    nDBRow;
};

class SW_DLLPUBLIC SwMailMergeConfigItem
{
    std::unique_ptr<SwMailMergeConfigItem_Impl> m_pImpl;
    //session information - not stored in configuration
    bool m_bAddressInserted;
    bool m_bGreetingInserted;
    sal_Int32 m_nGreetingMoves;
    std::set<sal_Int32> m_aExcludedRecords;
    css::uno::Reference<css::view::XSelectionChangeListener> m_xDBChangedListener;

    OUString m_sSelectedPrinter;

    SwView* m_pSourceView;
    SwView* m_pTargetView;
public:
    SwMailMergeConfigItem();
    ~SwMailMergeConfigItem();

    enum Gender
    {
        FEMALE,
        MALE,
        NEUTRAL
    };

    void Commit();

    const std::vector<std::pair<OUString, int>>& GetDefaultAddressHeaders() const;

    void SetCurrentConnection(
        css::uno::Reference< css::sdbc::XDataSource> const & xSource,
        const SharedConnection& rConnection,
        css::uno::Reference< css::sdbcx::XColumnsSupplier> const & xColumnsSupplier,
        const SwDBData& rDBData);

    css::uno::Reference< css::sdbc::XDataSource> const & GetSource();

    SharedConnection const & GetConnection();

    css::uno::Reference< css::sdbcx::XColumnsSupplier> const & GetColumnsSupplier();

    css::uno::Reference< css::sdbc::XResultSet> const & GetResultSet() const;

    void DisposeResultSet();

    OUString& GetFilter() const;
    void SetFilter(OUString const &);

    void SetCurrentDBData( const SwDBData& rDBData);
    const SwDBData& GetCurrentDBData() const;

    // move absolute, nTarget == -1 -> goto last record
    sal_Int32 MoveResultSet(sal_Int32 nTarget);
    sal_Int32 GetResultSetPosition()const;
    bool IsResultSetFirstLast(bool& bIsFirst, bool& bIsLast);

    bool IsRecordExcluded(sal_Int32 nRecord) const;
    void ExcludeRecord(sal_Int32 nRecord, bool bExclude);
    css::uno::Sequence< css::uno::Any> GetSelection() const;

    const css::uno::Sequence<OUString>& GetSavedDocuments() const;

    bool IsOutputToLetter()const;
    void SetOutputToLetter(bool bSet);

    bool IsAddressBlock()const;
    void SetAddressBlock(bool bSet);

    bool IsHideEmptyParagraphs() const;
    void SetHideEmptyParagraphs(bool bSet);

    const css::uno::Sequence<OUString> GetAddressBlocks() const;
    void SetAddressBlocks(const css::uno::Sequence< OUString>& rBlocks);

    void SetCurrentAddressBlockIndex( sal_Int32 nSet );
    sal_Int32 GetCurrentAddressBlockIndex() const;

    bool IsIncludeCountry() const;
    OUString& GetExcludeCountry() const;
    void SetCountrySettings(bool bSet, const OUString& sCountry);

    bool IsIndividualGreeting(bool bInEMail) const;
    void SetIndividualGreeting(bool bSet, bool bInEMail);

    bool IsGreetingLine(bool bInEMail) const;
    void SetGreetingLine(bool bSet, bool bInEMail);

    const css::uno::Sequence<OUString> GetGreetings(Gender eType) const;
    void SetGreetings(Gender eType, const css::uno::Sequence< OUString>& rBlocks);

    sal_Int32 GetCurrentGreeting(Gender eType) const;
    void SetCurrentGreeting(Gender eType, sal_Int32 nIndex);

    //the content of the gender column that marks it as female
    const OUString& GetFemaleGenderValue() const;
    void SetFemaleGenderValue(const OUString& rValue);

    //returns the assignment in the order of the default headers (GetDefaultAddressHeaders())
    css::uno::Sequence<OUString> GetColumnAssignment( const SwDBData& rDBData ) const;
    void SetColumnAssignment(
        const SwDBData& rDBData,
        const css::uno::Sequence< OUString>& );

    bool IsAddressFieldsAssigned() const;
    bool IsGreetingFieldsAssigned() const;

    //e-Mail settings:
    OUString const & GetMailDisplayName() const;
    void SetMailDisplayName(const OUString& rName);

    OUString const & GetMailAddress() const;
    void SetMailAddress(const OUString& rAddress);

    bool IsMailReplyTo() const;
    void SetMailReplyTo(bool bSet);

    OUString const & GetMailReplyTo() const;
    void SetMailReplyTo(const OUString& rReplyTo);

    OUString const & GetMailServer() const;
    void SetMailServer(const OUString& rAddress);

    sal_Int16 GetMailPort() const;
    void SetMailPort(sal_Int16 nSet);

    bool IsSecureConnection() const;
    void SetSecureConnection(bool bSet);

    bool IsAuthentication() const;
    void SetAuthentication(bool bSet);

    OUString const & GetMailUserName() const;
    void SetMailUserName(const OUString& rName);

    OUString const & GetMailPassword() const;
    void SetMailPassword(const OUString& rPassword);

    bool IsSMTPAfterPOP() const;
    void SetSMTPAfterPOP(bool bSet);

    OUString const & GetInServerName() const;
    void SetInServerName(const OUString& rServer);

    sal_Int16 GetInServerPort() const;
    void SetInServerPort(sal_Int16 nSet);

    bool IsInServerPOP() const;
    void SetInServerPOP(bool bSet);

    OUString const & GetInServerUserName() const;
    void SetInServerUserName(const OUString& rName);

    OUString const & GetInServerPassword() const;
    void SetInServerPassword(const OUString& rPassword);

    //session information
    bool IsAddressInserted() const { return m_bAddressInserted; }
    void SetAddressInserted()
    {
        m_bAddressInserted = true;
    }

    bool IsGreetingInserted() const
        { return m_bGreetingInserted; }
    void SetGreetingInserted()
        { m_bGreetingInserted = true; }

    // counts the moves in the layout page
    void MoveGreeting( sal_Int32 nMove) { m_nGreetingMoves += nMove;}
    sal_Int32 GetGreetingMoves() const { return m_nGreetingMoves;}

    // new source document - reset some flags
    void DocumentReloaded();

    bool IsMailAvailable() const;

    // notify a completed merge, provid the appropriate e-Mail address if available
    void AddMergedDocument(SwDocMergeInfo const & rInfo);
    //returns the page and database cursor information of each merged document
    SwDocMergeInfo& GetDocumentMergeInfo(sal_uInt32 nDocument);
    sal_uInt32 GetMergedDocumentCount();

    const OUString&  GetSelectedPrinter() const
        { return m_sSelectedPrinter; }
    void SetSelectedPrinter(const OUString& rSet)
        { m_sSelectedPrinter = rSet; }

    SwView* GetTargetView();
    void SetTargetView(SwView* pView);

    SwView* GetSourceView();
    void SetSourceView(SwView* pView);

    //helper methods
    OUString GetAssignedColumn(sal_uInt32 nColumn) const;
    void stopDBChangeListening();
    void updateCurrentDBDataFromDocument();
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
