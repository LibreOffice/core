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
#ifndef _MMCONFIGITEM_HXX
#define _MMCONFIGITEM_HXX

#include <com/sun/star/uno/Sequence.hxx>
#include <com/sun/star/uno/Reference.hxx>
#include <tools/resary.hxx>
#include <swdbdata.hxx>
#include "swdllapi.h"
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

struct SwDocMergeInfo
{
    long    nStartPageInTarget;
    long    nEndPageInTarget;
    long    nDBRow;
};

class SW_DLLPUBLIC SwMailMergeConfigItem
{
//    com::sun::star::uno::Sequence< ::rtl::OUString>     m_aSavedDocuments;
    SwMailMergeConfigItem_Impl*                                 m_pImpl;
    //session information - not stored in configuration
    bool                                                        m_bAddressInserted;
    bool                                                        m_bMergeDone;
    bool                                                        m_bGreetingInserted;
    sal_Int32                                                   m_nGreetingMoves;
    ::rtl::OUString                                             m_rAddressBlockFrame;
    ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any> m_aSelection;

    sal_uInt16                                                      m_nStartPrint;
    sal_uInt16                                                      m_nEndPrint;

    ::rtl::OUString                                             m_sSelectedPrinter;

    SwView*                                                     m_pSourceView;
    SwView*                                                     m_pTargetView;
public:
    SwMailMergeConfigItem();
    ~SwMailMergeConfigItem();

    enum Gender
    {
        FEMALE,
        MALE,
        NEUTRAL
    };

    void                Commit();

    const ResStringArray&   GetDefaultAddressHeaders() const;

    void                SetCurrentConnection(
                            ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XDataSource>          xSource,
                            SharedConnection                                                                xConnection,
                            ::com::sun::star::uno::Reference< ::com::sun::star::sdbcx::XColumnsSupplier>    xColumnsSupplier,
                            const SwDBData& rDBData);

    ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XDataSource>
                        GetSource();

    SharedConnection    GetConnection();

    ::com::sun::star::uno::Reference< ::com::sun::star::sdbcx::XColumnsSupplier>
                        GetColumnsSupplier();

    ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XResultSet>
                        GetResultSet() const;

    void                DisposeResultSet();

    ::rtl::OUString&    GetFilter() const;
    void                SetFilter(::rtl::OUString&);

    ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any>
                        GetSelection()const;
    void                SetSelection(::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any > rSelection);

    void                SetCurrentDBData( const SwDBData& rDBData);
    const SwDBData&     GetCurrentDBData() const;

    // move absolute, nTarget == -1 -> goto last record
    sal_Int32           MoveResultSet(sal_Int32 nTarget);
    sal_Int32           GetResultSetPosition()const;
    bool                IsResultSetFirstLast(bool& bIsFirst, bool& bIsLast);

    bool                IsRecordExcluded(sal_Int32 nRecord);
    void                ExcludeRecord(sal_Int32 nRecord, bool bExclude);

    const com::sun::star::uno::Sequence< ::rtl::OUString>&
                        GetSavedDocuments() const;
    void                AddSavedDocument(::rtl::OUString rName);

    sal_Bool            IsOutputToLetter()const;
    void                SetOutputToLetter(sal_Bool bSet);

    sal_Bool            IsAddressBlock()const;
    void                SetAddressBlock(sal_Bool bSet);

    sal_Bool            IsHideEmptyParagraphs() const;
    void                SetHideEmptyParagraphs(sal_Bool bSet);

    const com::sun::star::uno::Sequence< ::rtl::OUString>
                        GetAddressBlocks() const;
    void                SetAddressBlocks(const com::sun::star::uno::Sequence< ::rtl::OUString>& rBlocks);

    void                SetCurrentAddressBlockIndex( sal_Int32 nSet );
    sal_Int32           GetCurrentAddressBlockIndex() const;

    sal_Bool            IsIncludeCountry() const;
    rtl::OUString&      GetExcludeCountry() const;
    void                SetCountrySettings(sal_Bool bSet, const rtl::OUString& sCountry);

    sal_Bool            IsIndividualGreeting(sal_Bool bInEMail) const;
    void                SetIndividualGreeting(sal_Bool bSet, sal_Bool bInEMail);

    sal_Bool            IsGreetingLine(sal_Bool bInEMail) const;
    void                SetGreetingLine(sal_Bool bSet, sal_Bool bInEMail);

    const com::sun::star::uno::Sequence< ::rtl::OUString>
                        GetGreetings(Gender eType) const;
    void                SetGreetings(Gender eType, const com::sun::star::uno::Sequence< ::rtl::OUString>& rBlocks);

    sal_Int32           GetCurrentGreeting(Gender eType) const;
    void                SetCurrentGreeting(Gender eType, sal_Int32 nIndex);

    //the content of the gender column that marks it as female
    const ::rtl::OUString& GetFemaleGenderValue() const;
    void                   SetFemaleGenderValue(const ::rtl::OUString rValue);

    //returns the assignment in the order of the default headers (GetDefaultAddressHeaders())
    com::sun::star::uno::Sequence< ::rtl::OUString >
                        GetColumnAssignment( const SwDBData& rDBData ) const;
    void                SetColumnAssignment(
                            const SwDBData& rDBData,
                            const com::sun::star::uno::Sequence< ::rtl::OUString>& );

    bool                IsAddressFieldsAssigned() const;
    bool                IsGreetingFieldsAssigned() const;

    //e-Mail settings:
    ::rtl::OUString     GetMailDisplayName() const;
    void                SetMailDisplayName(const ::rtl::OUString& rName);

    ::rtl::OUString     GetMailAddress() const;
    void                SetMailAddress(const ::rtl::OUString& rAddress);

    sal_Bool            IsMailReplyTo() const;
    void                SetMailReplyTo(sal_Bool bSet);

    ::rtl::OUString     GetMailReplyTo() const;
    void                SetMailReplyTo(const ::rtl::OUString& rReplyTo);

    ::rtl::OUString     GetMailServer() const;
    void                SetMailServer(const ::rtl::OUString& rAddress);

    sal_Int16           GetMailPort() const;
    void                SetMailPort(sal_Int16 nSet);

    sal_Bool            IsSecureConnection() const;
    void                SetSecureConnection(sal_Bool bSet);

    sal_Bool            IsAuthentication() const;
    void                SetAuthentication(sal_Bool bSet);

    ::rtl::OUString     GetMailUserName() const;
    void                SetMailUserName(const ::rtl::OUString& rName);

    ::rtl::OUString     GetMailPassword() const;
    void                SetMailPassword(const ::rtl::OUString& rPassword);

    sal_Bool            IsSMTPAfterPOP() const;
    void                SetSMTPAfterPOP(sal_Bool bSet);

    ::rtl::OUString     GetInServerName() const;
    void                SetInServerName(const ::rtl::OUString& rServer);

    sal_Int16           GetInServerPort() const;
    void                SetInServerPort(sal_Int16 nSet);

    sal_Bool            IsInServerPOP() const;
    void                SetInServerPOP(sal_Bool bSet);

    ::rtl::OUString     GetInServerUserName() const;
    void                SetInServerUserName(const ::rtl::OUString& rName);

    ::rtl::OUString     GetInServerPassword() const;
    void                SetInServerPassword(const ::rtl::OUString& rPassword);

    //session information
    bool                IsAddressInserted() const { return m_bAddressInserted; }
    void                SetAddressInserted(const ::rtl::OUString& rFrameName)
                            { m_bAddressInserted = true;
                              m_rAddressBlockFrame = rFrameName;
                            }

    bool                IsGreetingInserted() const { return m_bGreetingInserted; }
    void                SetGreetingInserted()
                            { m_bGreetingInserted = true; }

    // counts the moves in the layout page
    void                MoveGreeting( sal_Int32 nMove) { m_nGreetingMoves += nMove;}
    sal_Int32           GetGreetingMoves() const { return m_nGreetingMoves;}

    bool                IsMergeDone() const { return m_bMergeDone;}
    void                SetMergeDone(  ) { m_bMergeDone = true; }

    // new source document - reset some flags
    void                DocumentReloaded();

    bool                IsMailAvailable() const;

    // notify a completed merge, provid the appropriate e-Mail address if available
    void                AddMergedDocument(SwDocMergeInfo& rInfo);
    //returns the page and database cursor information of each merged document
    SwDocMergeInfo&     GetDocumentMergeInfo(sal_uInt32 nDocument);
    sal_uInt32          GetMergedDocumentCount() const;

    void                SetPrintRange( sal_uInt16 nStartDocument, sal_uInt16 nEndDocument)
                            {m_nStartPrint = nStartDocument; m_nEndPrint = nEndDocument;}
    sal_uInt16              GetPrintRangeStart() const  {return m_nStartPrint;}
    sal_uInt16              GetPrintRangeEnd() const {return m_nEndPrint;}

    const ::rtl::OUString&  GetSelectedPrinter() const {return m_sSelectedPrinter;}
    void                    SetSelectedPrinter(const ::rtl::OUString& rSet )
                                    {m_sSelectedPrinter = rSet;}


    SwView*             GetTargetView();
    void                SetTargetView(SwView* pView);

    SwView*             GetSourceView();
    void                SetSourceView(SwView* pView);

    //helper methods
    ::rtl::OUString     GetAssignedColumn(sal_uInt32 nColumn)const;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
