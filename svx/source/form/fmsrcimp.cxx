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


#include <rtl/strbuf.hxx>
#include <svx/fmtools.hxx>
#include <svx/fmsrccfg.hxx>
#include <tools/debug.hxx>
#include <tools/diagnose_ex.h>
#include <tools/wldcrd.hxx>
#include <vcl/svapp.hxx>
#include <unotools/textsearch.hxx>
#include <com/sun/star/util/SearchAlgorithms2.hpp>
#include <com/sun/star/util/SearchResult.hpp>
#include <com/sun/star/util/SearchFlags.hpp>
#include <com/sun/star/lang/Locale.hpp>
#include <com/sun/star/i18n/CollatorOptions.hpp>

#include <com/sun/star/sdbcx/XColumnsSupplier.hpp>
#include <com/sun/star/util/NumberFormatter.hpp>
#include <com/sun/star/util/NumberFormat.hpp>
#include <com/sun/star/util/XNumberFormatsSupplier.hpp>
#include <com/sun/star/util/XNumberFormats.hpp>

#include <fmprop.hxx>
#include <fmservs.hxx>
#include <svx/fmsrcimp.hxx>
#include <svx/fmsearch.hxx>

#include <comphelper/types.hxx>
#include <unotools/syslocale.hxx>
#include <i18nutil/searchopt.hxx>

#define EQUAL_BOOKMARKS(a, b) a == b

#define IFACECAST(c)          static_cast<const Reference< XInterface >&>(c)

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::util;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::i18n;
using namespace ::com::sun::star::beans;
using namespace ::svxform;


// = FmRecordCountListener

//  SMART_UNO_IMPLEMENTATION(FmRecordCountListener, UsrObject);


FmRecordCountListener::FmRecordCountListener(const Reference< css::sdbc::XResultSet > & dbcCursor)
{

    m_xListening.set(dbcCursor, UNO_QUERY);
    if (!m_xListening.is())
        return;

    if (::comphelper::getBOOL(m_xListening->getPropertyValue(FM_PROP_ROWCOUNTFINAL)))
    {
        m_xListening = nullptr;
        // there's nothing to do as the record count is already known
        return;
    }

    m_xListening->addPropertyChangeListener(FM_PROP_ROWCOUNT, static_cast<css::beans::XPropertyChangeListener*>(this));
}


void FmRecordCountListener::SetPropChangeHandler(const Link<sal_Int32,void>& lnk)
{
    m_lnkWhoWantsToKnow = lnk;

    if (m_xListening.is())
        NotifyCurrentCount();
}


FmRecordCountListener::~FmRecordCountListener()
{

}


void FmRecordCountListener::DisConnect()
{
    if(m_xListening.is())
        m_xListening->removePropertyChangeListener(FM_PROP_ROWCOUNT, static_cast<css::beans::XPropertyChangeListener*>(this));
    m_xListening = nullptr;
}


void SAL_CALL FmRecordCountListener::disposing(const css::lang::EventObject& /*Source*/)
{
    DBG_ASSERT(m_xListening.is(), "FmRecordCountListener::disposing should never have been called without a propset !");
    DisConnect();
}


void FmRecordCountListener::NotifyCurrentCount()
{
    if (m_lnkWhoWantsToKnow.IsSet())
    {
        DBG_ASSERT(m_xListening.is(), "FmRecordCountListener::NotifyCurrentCount : I have no propset ... !?");
        sal_Int32 theCount = ::comphelper::getINT32(m_xListening->getPropertyValue(FM_PROP_ROWCOUNT));
        m_lnkWhoWantsToKnow.Call(theCount);
    }
}


void FmRecordCountListener::propertyChange(const  css::beans::PropertyChangeEvent& /*evt*/)
{
    NotifyCurrentCount();
}


// FmSearchEngine - local classes

SimpleTextWrapper::SimpleTextWrapper(const Reference< css::awt::XTextComponent > & _xText)
    :ControlTextWrapper(_xText.get())
    ,m_xText(_xText)
{
    DBG_ASSERT(m_xText.is(), "FmSearchEngine::SimpleTextWrapper::SimpleTextWrapper : invalid argument !");
}


OUString SimpleTextWrapper::getCurrentText() const
{
    return m_xText->getText();
}


ListBoxWrapper::ListBoxWrapper(const Reference< css::awt::XListBox > & _xBox)
    :ControlTextWrapper(_xBox.get())
    ,m_xBox(_xBox)
{
    DBG_ASSERT(m_xBox.is(), "FmSearchEngine::ListBoxWrapper::ListBoxWrapper : invalid argument !");
}


OUString ListBoxWrapper::getCurrentText() const
{
    return m_xBox->getSelectedItem();
}


CheckBoxWrapper::CheckBoxWrapper(const Reference< css::awt::XCheckBox > & _xBox)
    :ControlTextWrapper(_xBox.get())
    ,m_xBox(_xBox)
{
    DBG_ASSERT(m_xBox.is(), "FmSearchEngine::CheckBoxWrapper::CheckBoxWrapper : invalid argument !");
}


OUString CheckBoxWrapper::getCurrentText() const
{
    switch (static_cast<TriState>(m_xBox->getState()))
    {
        case TRISTATE_FALSE: return OUString("0");
        case TRISTATE_TRUE: return OUString("1");
        default: break;
    }
    return OUString();
}


// = FmSearchEngine

bool FmSearchEngine::MoveCursor()
{
    bool bSuccess = true;
    try
    {
        if (m_bForward)
            if (m_xSearchCursor.isLast())
                m_xSearchCursor.first();
            else
                m_xSearchCursor.next();
        else
            if (m_xSearchCursor.isFirst())
            {
                rtl::Reference<FmRecordCountListener> prclListener = new FmRecordCountListener(m_xSearchCursor);
                prclListener->SetPropChangeHandler(LINK(this, FmSearchEngine, OnNewRecordCount));

                m_xSearchCursor.last();

                prclListener->DisConnect();
            }
            else
                m_xSearchCursor.previous();
    }
    catch(css::sdbc::SQLException const& e)
    {
        SAL_WARN( "svx", "FmSearchEngine::MoveCursor: caught " << e << " sqlstate=" << e.SQLState );
        bSuccess = false;
    }
    catch(Exception const& e)
    {
        SAL_WARN( "svx", "FmSearchEngine::MoveCursor: caught " << e);
        bSuccess = false;
    }
    catch(...)
    {
        OSL_FAIL("FmSearchEngine::MoveCursor : caught an unknown Exception !");
        bSuccess = false;
    }

    return bSuccess;
}


bool FmSearchEngine::MoveField(sal_Int32& nPos, FieldCollection::iterator& iter, const FieldCollection::iterator& iterBegin, const FieldCollection::iterator& iterEnd)
{
    bool bSuccess(true);
    if (m_bForward)
    {
        ++iter;
        ++nPos;
        if (iter == iterEnd)
        {
            bSuccess = MoveCursor();
            iter = iterBegin;
            nPos = 0;
        }
    } else
    {
        if (iter == iterBegin)
        {
            bSuccess = MoveCursor();
            iter = iterEnd;
            nPos = iter-iterBegin;
        }
        --iter;
        --nPos;
    }
    return bSuccess;
}


void FmSearchEngine::BuildAndInsertFieldInfo(const Reference< css::container::XIndexAccess > & xAllFields, sal_Int32 nField)
{
    DBG_ASSERT( xAllFields.is() && ( nField >= 0 ) && ( nField < xAllFields->getCount() ),
        "FmSearchEngine::BuildAndInsertFieldInfo: invalid field descriptor!" );

    // the field itself
    Reference< XInterface > xCurrentField;
    xAllFields->getByIndex(nField) >>= xCurrentField;

    // From this I now know that it supports the DatabaseRecord service (I hope).
    // For the FormatKey and the type I need the PropertySet.
    Reference< css::beans::XPropertySet >  xProperties(xCurrentField, UNO_QUERY);

    // build the FieldInfo for that
    FieldInfo fiCurrent;
    fiCurrent.xContents.set(xCurrentField, UNO_QUERY);

    // and memorize
    m_arrUsedFields.insert(m_arrUsedFields.end(), fiCurrent);

}

OUString FmSearchEngine::FormatField(sal_Int32 nWhich)
{
    DBG_ASSERT(static_cast<sal_uInt32>(nWhich) < m_aControlTexts.size(), "FmSearchEngine::FormatField(sal_Int32) : invalid position !");
    DBG_ASSERT(m_aControlTexts[nWhich], "FmSearchEngine::FormatField(sal_Int32) : invalid object in array !");
    DBG_ASSERT(m_aControlTexts[nWhich]->getControl().is(), "FmSearchEngine::FormatField : invalid control !");

    if (m_nCurrentFieldIndex != -1)
    {
        DBG_ASSERT((nWhich == 0) || (nWhich == m_nCurrentFieldIndex), "FmSearchEngine::FormatField : parameter nWhich is invalid");
        // analogous situation as below
        nWhich = m_nCurrentFieldIndex;
    }

    DBG_ASSERT((nWhich >= 0) && (static_cast<sal_uInt32>(nWhich) < m_aControlTexts.size()),
        "FmSearchEngine::FormatField : invalid argument nWhich !");
    return m_aControlTexts[m_nCurrentFieldIndex == -1 ? nWhich : m_nCurrentFieldIndex]->getCurrentText();
}


FmSearchEngine::SearchResult FmSearchEngine::SearchSpecial(bool _bSearchForNull, sal_Int32& nFieldPos,
    FieldCollection::iterator& iterFieldLoop, const FieldCollection::iterator& iterBegin, const FieldCollection::iterator& iterEnd)
{
    // memorize the start position
    Any aStartMark;
    try { aStartMark = m_xSearchCursor.getBookmark(); }
    catch ( const Exception& ) { DBG_UNHANDLED_EXCEPTION("svx"); return SearchResult::Error; }
    FieldCollection::const_iterator iterInitialField = iterFieldLoop;


    bool bFound(false);
    bool bMovedAround(false);
    do
    {
        Application::Reschedule( true );

        // the content to be compared currently
        iterFieldLoop->xContents->getString();  // needed for wasNull
        bFound = _bSearchForNull == bool(iterFieldLoop->xContents->wasNull());
        if (bFound)
            break;

        // next field (implicitly next record, if necessary)
        if (!MoveField(nFieldPos, iterFieldLoop, iterBegin, iterEnd))
        {   // When moving to the next field, something went wrong...
            // Continuing is not possible, since the next time exactly the same
            // will definitely go wrong again, thus abort.
            // Before, however, so that the search continues at the current position:
            try { m_aPreviousLocBookmark = m_xSearchCursor.getBookmark(); }
            catch ( const Exception& ) { DBG_UNHANDLED_EXCEPTION("svx"); }
            m_iterPreviousLocField = iterFieldLoop;
            // and leave
            return SearchResult::Error;
        }

        Any aCurrentBookmark;
        try { aCurrentBookmark = m_xSearchCursor.getBookmark(); }
        catch ( const Exception& ) { DBG_UNHANDLED_EXCEPTION("svx"); return SearchResult::Error; }

        bMovedAround = EQUAL_BOOKMARKS(aStartMark, aCurrentBookmark) && (iterFieldLoop == iterInitialField);

        if (nFieldPos == 0)
            // that is, I've moved to a new record
            PropagateProgress(bMovedAround);
                // if we moved to the starting position we don't have to propagate an 'overflow' message
                // FS - 07.12.99 - 68530

        // cancel requested?
        if (CancelRequested())
            return SearchResult::Cancelled;

    } while (!bMovedAround);

    return bFound ? SearchResult::Found : SearchResult::NotFound;
}


FmSearchEngine::SearchResult FmSearchEngine::SearchWildcard(const OUString& strExpression, sal_Int32& nFieldPos,
    FieldCollection::iterator& iterFieldLoop, const FieldCollection::iterator& iterBegin, const FieldCollection::iterator& iterEnd)
{
    // memorize the start position
    Any aStartMark;
    try { aStartMark = m_xSearchCursor.getBookmark(); }
    catch ( const Exception& ) { DBG_UNHANDLED_EXCEPTION("svx"); return SearchResult::Error; }
    FieldCollection::const_iterator iterInitialField = iterFieldLoop;

    WildCard aSearchExpression(strExpression);


    bool bFound(false);
    bool bMovedAround(false);
    do
    {
        Application::Reschedule( true );

        // the content to be compared currently
        OUString sCurrentCheck;
        if (m_bFormatter)
            sCurrentCheck = FormatField(nFieldPos);
        else
            sCurrentCheck = iterFieldLoop->xContents->getString();

        if (!GetCaseSensitive())
            // norm the string
            sCurrentCheck = m_aCharacterClassficator.lowercase(sCurrentCheck);

        // now the test is easy...
        bFound = aSearchExpression.Matches(sCurrentCheck);

        if (bFound)
            break;

        // next field (implicitly next record, if necessary)
        if (!MoveField(nFieldPos, iterFieldLoop, iterBegin, iterEnd))
        {   // When moving to the next field, something went wrong...
            // Continuing is not possible, since the next time exactly the same
            // will definitely go wrong again, thus abort.
            // Before, however, so that the search continues at the current position:
            try { m_aPreviousLocBookmark = m_xSearchCursor.getBookmark(); }
            catch ( const Exception& ) { DBG_UNHANDLED_EXCEPTION("svx"); }
            m_iterPreviousLocField = iterFieldLoop;
            // and leave
            return SearchResult::Error;
        }

        Any aCurrentBookmark;
        try { aCurrentBookmark = m_xSearchCursor.getBookmark(); }
        catch ( const Exception& ) { DBG_UNHANDLED_EXCEPTION("svx"); return SearchResult::Error; }

        bMovedAround = EQUAL_BOOKMARKS(aStartMark, aCurrentBookmark) && (iterFieldLoop == iterInitialField);

        if (nFieldPos == 0)
            // that is, I've moved to a new record
            PropagateProgress(bMovedAround);
                // if we moved to the starting position we don't have to propagate an 'overflow' message
                // FS - 07.12.99 - 68530

        //  cancel requested?
        if (CancelRequested())
            return SearchResult::Cancelled;

    } while (!bMovedAround);

    return bFound ? SearchResult::Found : SearchResult::NotFound;
}


FmSearchEngine::SearchResult FmSearchEngine::SearchRegularApprox(const OUString& strExpression, sal_Int32& nFieldPos,
    FieldCollection::iterator& iterFieldLoop, const FieldCollection::iterator& iterBegin, const FieldCollection::iterator& iterEnd)
{
    DBG_ASSERT(m_bLevenshtein || m_bRegular,
        "FmSearchEngine::SearchRegularApprox : invalid search mode!");
    DBG_ASSERT(!m_bLevenshtein || !m_bRegular,
        "FmSearchEngine::SearchRegularApprox : cannot search for regular expressions and similarities at the same time!");

    // memorize start position
    Any aStartMark;
    try { aStartMark = m_xSearchCursor.getBookmark(); }
    catch ( const Exception& ) { DBG_UNHANDLED_EXCEPTION("svx"); return SearchResult::Error; }
    FieldCollection::const_iterator iterInitialField = iterFieldLoop;

    // collect parameters
    i18nutil::SearchOptions2 aParam;
    aParam.AlgorithmType2 = m_bRegular ? SearchAlgorithms2::REGEXP : SearchAlgorithms2::APPROXIMATE;
    aParam.searchFlag = 0;
    aParam.transliterateFlags = GetTransliterationFlags();
    if ( !GetTransliteration() )
    {   // if transliteration is not enabled, the only flags which matter are IGNORE_CASE and IGNORE_WIDTH
        aParam.transliterateFlags &= TransliterationFlags::IGNORE_CASE | TransliterationFlags::IGNORE_WIDTH;
    }
    if (m_bLevenshtein)
    {
        if (m_bLevRelaxed)
            aParam.searchFlag |= SearchFlags::LEV_RELAXED;
        aParam.changedChars = m_nLevOther;
        aParam.deletedChars = m_nLevShorter;
        aParam.insertedChars = m_nLevLonger;
    }
    aParam.searchString = strExpression;
    aParam.Locale = SvtSysLocale().GetLanguageTag().getLocale();
    ::utl::TextSearch aLocalEngine( aParam);


    bool bFound = false;
    bool bMovedAround(false);
    do
    {
        Application::Reschedule( true );

        // the content to be compared currently
        OUString sCurrentCheck;
        if (m_bFormatter)
            sCurrentCheck = FormatField(nFieldPos);
        else
            sCurrentCheck = iterFieldLoop->xContents->getString();

        // (don't care about case here, this is done by the TextSearch object, 'cause we passed our case parameter to it)

        sal_Int32 nStart = 0, nEnd = sCurrentCheck.getLength();
        bFound = aLocalEngine.SearchForward(sCurrentCheck, &nStart, &nEnd);
            // it says 'forward' here, but that only refers to the search within
            // sCurrentCheck, so it has nothing to do with the direction of my
            // record migration (MoveField takes care of that)

        // check if the position is correct
        if (bFound)
        {
            switch (m_nPosition)
            {
                case MATCHING_WHOLETEXT :
                    if (nEnd != sCurrentCheck.getLength())
                    {
                        bFound = false;
                        break;
                    }
                    SAL_FALLTHROUGH;
                case MATCHING_BEGINNING :
                    if (nStart != 0)
                        bFound = false;
                    break;
                case MATCHING_END :
                    if (nEnd != sCurrentCheck.getLength())
                        bFound = false;
                    break;
            }
        }

        if (bFound) // still?
            break;

        // next field (implicitly next record, if necessary)
        if (!MoveField(nFieldPos, iterFieldLoop, iterBegin, iterEnd))
        {   // When moving to the next field, something went wrong...
            // Continuing is not possible, since the next time exactly the same
            // will definitely go wrong again, thus abort (without error
            // notification, I expect it to be displayed in the Move).
            // Before, however, so that the search continues at the current position:
            try { m_aPreviousLocBookmark = m_xSearchCursor.getBookmark(); }
            catch ( const Exception& ) { DBG_UNHANDLED_EXCEPTION("svx"); }
            m_iterPreviousLocField = iterFieldLoop;
            // and leave
            return SearchResult::Error;
        }

        Any aCurrentBookmark;
        try { aCurrentBookmark = m_xSearchCursor.getBookmark(); }
        catch ( const Exception& ) { DBG_UNHANDLED_EXCEPTION("svx"); return SearchResult::Error; }
        bMovedAround = EQUAL_BOOKMARKS(aStartMark, aCurrentBookmark) && (iterFieldLoop == iterInitialField);

        if (nFieldPos == 0)
            // that is, I've moved to a new record
            PropagateProgress(bMovedAround);
                // if we moved to the starting position we don't have to propagate an 'overflow' message
                // FS - 07.12.99 - 68530

        // cancel requested?
        if (CancelRequested())
            return SearchResult::Cancelled;

    } while (!bMovedAround);

    return bFound ? SearchResult::Found : SearchResult::NotFound;
}


FmSearchEngine::FmSearchEngine(const Reference< XComponentContext >& _rxContext,
        const Reference< XResultSet > & xCursor, const OUString& sVisibleFields,
        const InterfaceArray& arrFields)
    :m_xSearchCursor(xCursor)
    ,m_aCharacterClassficator( _rxContext, SvtSysLocale().GetLanguageTag() )
    ,m_aStringCompare( _rxContext )
    ,m_nCurrentFieldIndex(-2)   // -1 already has a meaning, so I take -2 for 'invalid'
    ,m_xOriginalIterator(xCursor)
    ,m_xClonedIterator(m_xOriginalIterator, true)
    ,m_eSearchForType(SearchFor::String)
    ,m_srResult(SearchResult::Found)
    ,m_bSearchingCurrently(false)
    ,m_bCancelAsynchRequest(false)
    ,m_bFormatter(true)     // this must be consistent with m_xSearchCursor, which is generally == m_xOriginalIterator
    ,m_bForward(false)
    ,m_bWildcard(false)
    ,m_bRegular(false)
    ,m_bLevenshtein(false)
    ,m_bTransliteration(false)
    ,m_bLevRelaxed(false)
    ,m_nLevOther(0)
    ,m_nLevShorter(0)
    ,m_nLevLonger(0)
    ,m_nPosition(MATCHING_ANYWHERE)
    ,m_nTransliterationFlags(TransliterationFlags::NONE)
{

    fillControlTexts(arrFields);
    Init(sVisibleFields);
}


void FmSearchEngine::SetIgnoreWidthCJK(bool bSet)
{
    if (bSet)
        m_nTransliterationFlags |= TransliterationFlags::IGNORE_WIDTH;
    else
        m_nTransliterationFlags &= ~TransliterationFlags::IGNORE_WIDTH;
}


bool FmSearchEngine::GetIgnoreWidthCJK() const
{
    return bool(m_nTransliterationFlags & TransliterationFlags::IGNORE_WIDTH);
}


void FmSearchEngine::SetCaseSensitive(bool bSet)
{
    if (bSet)
        m_nTransliterationFlags &= ~TransliterationFlags::IGNORE_CASE;
    else
        m_nTransliterationFlags |= TransliterationFlags::IGNORE_CASE;
}


bool FmSearchEngine::GetCaseSensitive() const
{
    return !(m_nTransliterationFlags & TransliterationFlags::IGNORE_CASE);
}


void FmSearchEngine::clearControlTexts()
{
    m_aControlTexts.clear();
}


void FmSearchEngine::fillControlTexts(const InterfaceArray& arrFields)
{
    clearControlTexts();
    Reference< XInterface >  xCurrent;
    for (const auto & rField : arrFields)
    {
        xCurrent = rField;
        DBG_ASSERT(xCurrent.is(), "FmSearchEngine::fillControlTexts : invalid field interface !");
        // check which type of control this is
        Reference< css::awt::XTextComponent >  xAsText(xCurrent, UNO_QUERY);
        if (xAsText.is())
        {
            m_aControlTexts.emplace_back(new SimpleTextWrapper(xAsText));
            continue;
        }

        Reference< css::awt::XListBox >  xAsListBox(xCurrent, UNO_QUERY);
        if (xAsListBox.is())
        {
            m_aControlTexts.emplace_back(new ListBoxWrapper(xAsListBox));
            continue;
        }

        Reference< css::awt::XCheckBox >  xAsCheckBox(xCurrent, UNO_QUERY);
        DBG_ASSERT(xAsCheckBox.is(), "FmSearchEngine::fillControlTexts : invalid field interface (no supported type) !");
            // we don't have any more options ...
        m_aControlTexts.emplace_back(new CheckBoxWrapper(xAsCheckBox));
    }
}


void FmSearchEngine::Init(const OUString& sVisibleFields)
{
    // analyze the fields
    // additionally, create the mapping: because the list of used columns can be shorter than the list
    // of columns of the cursor, we need a mapping: "used column number n" -> "cursor column m"
    m_arrFieldMapping.clear();

    // important: The case of the columns does not need to be exact - for instance:
    // - a user created a form which works on a table, for which the driver returns a column name "COLUMN"
    // - the driver itself works case-insensitive with column names
    // - a control in the form is bound to "column" - not the different case
    // In such a scenario, the form and the field would work okay, but we here need to case for the different case
    // explicitly
    // #i8755#

    // so first of all, check if the database handles identifiers case sensitive
    Reference< XConnection > xConn;
    Reference< XDatabaseMetaData > xMeta;
    Reference< XPropertySet > xCursorProps( IFACECAST( m_xSearchCursor ), UNO_QUERY );
    if ( xCursorProps.is() )
    {
        try
        {
            xCursorProps->getPropertyValue( FM_PROP_ACTIVE_CONNECTION ) >>= xConn;
        }
        catch( const Exception& ) { /* silent this - will be asserted below */ }
    }
    if ( xConn.is() )
        xMeta = xConn->getMetaData();
    OSL_ENSURE( xMeta.is(), "FmSearchEngine::Init: very strange cursor (could not derive connection meta data from it)!" );

    bool bCaseSensitiveIdentifiers = true;  // assume case sensitivity
    if ( xMeta.is() )
        bCaseSensitiveIdentifiers = xMeta->supportsMixedCaseQuotedIdentifiers();

    // now that we have this information, we need a collator which is able to case (in)sensitivity compare strings
    m_aStringCompare.loadDefaultCollator( SvtSysLocale().GetLanguageTag().getLocale(),
        bCaseSensitiveIdentifiers ? 0 : css::i18n::CollatorOptions::CollatorOptions_IGNORE_CASE );

    try
    {
        // the cursor can give me a record (as PropertySet), which supports the DatabaseRecord service
        Reference< css::sdbcx::XColumnsSupplier >  xSupplyCols(IFACECAST(m_xSearchCursor), UNO_QUERY);
        DBG_ASSERT(xSupplyCols.is(), "FmSearchEngine::Init : invalid cursor (no columns supplier) !");
        Reference< css::container::XNameAccess >       xAllFieldNames = xSupplyCols->getColumns();
        Sequence< OUString > seqFieldNames = xAllFieldNames->getElementNames();
        OUString*            pFieldNames = seqFieldNames.getArray();


        OUString sCurrentField;
        sal_Int32 nIndex = 0;
        do
        {
            sCurrentField = sVisibleFields.getToken(0, ';' , nIndex);

            // search in the field collection
            sal_Int32 nFoundIndex = -1;
            for (sal_Int32 j=0; j<seqFieldNames.getLength(); ++j, ++pFieldNames)
            {
                if ( 0 == m_aStringCompare.compareString( *pFieldNames, sCurrentField ) )
                {
                    nFoundIndex = j;
                    break;
                }
            }
            // set the field selection back to the first
            pFieldNames = seqFieldNames.getArray();
            DBG_ASSERT(nFoundIndex != -1, "FmSearchEngine::Init : Invalid field name were given !");
            m_arrFieldMapping.push_back(nFoundIndex);
        }
        while ( nIndex >= 0 );
    }
    catch (const Exception&)
    {
        OSL_FAIL("Exception occurred!");
    }

}


void FmSearchEngine::SetFormatterUsing(bool bSet)
{
    if (m_bFormatter == bSet)
        return;
    m_bFormatter = bSet;

    // I did not use a formatter, but TextComponents -> the SearchIterator needs to be adjusted
    try
    {
        if (m_bFormatter)
        {
            DBG_ASSERT(m_xSearchCursor == m_xClonedIterator, "FmSearchEngine::SetFormatterUsing : inconsistent state !");
            m_xSearchCursor = m_xOriginalIterator;
            m_xSearchCursor.moveToBookmark(m_xClonedIterator.getBookmark());
                // so that I continue with the new iterator at the actual place where I previously stopped
        }
        else
        {
            DBG_ASSERT(m_xSearchCursor == m_xOriginalIterator, "FmSearchEngine::SetFormatterUsing : inconsistent state !");
            m_xSearchCursor = m_xClonedIterator;
            m_xSearchCursor.moveToBookmark(m_xOriginalIterator.getBookmark());
        }
    }
    catch( const Exception& )
    {
        DBG_UNHANDLED_EXCEPTION("svx");
    }

    // I have to re-bind the fields, because the text exchange might take
    // place over these fields and the underlying cursor has changed
    RebuildUsedFields(m_nCurrentFieldIndex, true);
}


void FmSearchEngine::PropagateProgress(bool _bDontPropagateOverflow)
{
    if (m_aProgressHandler.IsSet())
    {
        FmSearchProgress aProgress;
        try
        {
            aProgress.aSearchState = FmSearchProgress::State::Progress;
            aProgress.nCurrentRecord = m_xSearchCursor.getRow() - 1;
            if (m_bForward)
                aProgress.bOverflow = !_bDontPropagateOverflow && m_xSearchCursor.isFirst();
            else
                aProgress.bOverflow = !_bDontPropagateOverflow && m_xSearchCursor.isLast();
        }
        catch( const Exception& )
        {
            DBG_UNHANDLED_EXCEPTION("svx");
        }

        m_aProgressHandler.Call(&aProgress);
    }
}


void FmSearchEngine::SearchNextImpl()
{
    DBG_ASSERT(!(m_bWildcard && m_bRegular) && !(m_bRegular && m_bLevenshtein) && !(m_bLevenshtein && m_bWildcard),
        "FmSearchEngine::SearchNextImpl : search parameters are mutually exclusive!");

    DBG_ASSERT(m_xSearchCursor.is(), "FmSearchEngine::SearchNextImpl : have invalid iterator!");

    // the parameters of the search
    OUString strSearchExpression(m_strSearchExpression); // I need non-const
    if (!GetCaseSensitive())
        // norm the string
        strSearchExpression = m_aCharacterClassficator.lowercase(strSearchExpression);

    if (!m_bRegular && !m_bLevenshtein)
    {   // 'normal' search I run through WildCards in any case, but must before adjust the OUString depending on the mode

        if (!m_bWildcard)
        {   // since in all other cases * and ? in the search string are of course
            // also allowed, but should not count as WildCards, I need to normalize
            OUString aTmp(strSearchExpression);
            const OUString s_sStar("\\*");
            const OUString s_sQuotation("\\?");
            aTmp = aTmp.replaceAll("*", s_sStar);
            aTmp = aTmp.replaceAll("?", s_sQuotation);
            strSearchExpression = aTmp;

            switch (m_nPosition)
            {
                case MATCHING_ANYWHERE :
                    strSearchExpression = "*" + strSearchExpression + "*";
                    break;
                case MATCHING_BEGINNING :
                    strSearchExpression = strSearchExpression + "*";
                    break;
                case MATCHING_END :
                    strSearchExpression = "*" + strSearchExpression;
                    break;
                case MATCHING_WHOLETEXT :
                    break;
                default :
                    OSL_FAIL("FmSearchEngine::SearchNextImpl() : the methods listbox may contain only 4 entries ...");
            }
        }
    }

    // for work on field list
    FieldCollection::iterator iterBegin = m_arrUsedFields.begin();
    FieldCollection::iterator iterEnd = m_arrUsedFields.end();
    FieldCollection::iterator iterFieldCheck;

    sal_Int32 nFieldPos;

    if (m_aPreviousLocBookmark.hasValue())
    {
        DBG_ASSERT(EQUAL_BOOKMARKS(m_aPreviousLocBookmark, m_xSearchCursor.getBookmark()),
            "FmSearchEngine::SearchNextImpl : invalid position!");
        iterFieldCheck = m_iterPreviousLocField;
        // continue in the field after (or before) the last discovery
        nFieldPos = iterFieldCheck - iterBegin;
        MoveField(nFieldPos, iterFieldCheck, iterBegin, iterEnd);
    }
    else
    {
        if (m_bForward)
            iterFieldCheck = iterBegin;
        else
        {
            iterFieldCheck = iterEnd;
            --iterFieldCheck;
        }
        nFieldPos = iterFieldCheck - iterBegin;
    }

    PropagateProgress(true);
    SearchResult srResult;
    if (m_eSearchForType != SearchFor::String)
        srResult = SearchSpecial(m_eSearchForType == SearchFor::Null, nFieldPos, iterFieldCheck, iterBegin, iterEnd);
    else if (!m_bRegular && !m_bLevenshtein)
        srResult = SearchWildcard(strSearchExpression, nFieldPos, iterFieldCheck, iterBegin, iterEnd);
    else
        srResult = SearchRegularApprox(strSearchExpression, nFieldPos, iterFieldCheck, iterBegin, iterEnd);

    m_srResult = srResult;

    if (SearchResult::Error == m_srResult)
        return;

    // found?
    if (SearchResult::Found == m_srResult)
    {
        // memorize the position
        try { m_aPreviousLocBookmark = m_xSearchCursor.getBookmark(); }
        catch ( const Exception& ) { DBG_UNHANDLED_EXCEPTION("svx"); }
        m_iterPreviousLocField = iterFieldCheck;
    }
    else
        // invalidate the "last discovery"
        InvalidatePreviousLoc();
}


void FmSearchEngine::OnSearchTerminated()
{
    if (!m_aProgressHandler.IsSet())
        return;

    FmSearchProgress aProgress;
    try
    {
        switch (m_srResult)
        {
            case SearchResult::Error :
                aProgress.aSearchState = FmSearchProgress::State::Error;
                break;
            case SearchResult::Found :
                aProgress.aSearchState = FmSearchProgress::State::Successful;
                aProgress.aBookmark = m_aPreviousLocBookmark;
                aProgress.nFieldIndex = m_iterPreviousLocField - m_arrUsedFields.begin();
                break;
            case SearchResult::NotFound :
                aProgress.aSearchState = FmSearchProgress::State::NothingFound;
                aProgress.aBookmark = m_xSearchCursor.getBookmark();
                break;
            case SearchResult::Cancelled :
                aProgress.aSearchState = FmSearchProgress::State::Canceled;
                aProgress.aBookmark = m_xSearchCursor.getBookmark();
                break;
        }
        aProgress.nCurrentRecord = m_xSearchCursor.getRow() - 1;
    }
    catch( const Exception& )
    {
        DBG_UNHANDLED_EXCEPTION("svx");
    }

    // by definition, the link must be thread-safe (I just require that),
    // so that I do not have to worry about such things here
    m_aProgressHandler.Call(&aProgress);

    m_bSearchingCurrently = false;
}


IMPL_LINK(FmSearchEngine, OnNewRecordCount, sal_Int32, theCounter, void)
{
    if (!m_aProgressHandler.IsSet())
        return;

    FmSearchProgress aProgress;
    aProgress.nCurrentRecord = theCounter;
    aProgress.aSearchState = FmSearchProgress::State::ProgressCounting;
    m_aProgressHandler.Call(&aProgress);
}


bool FmSearchEngine::CancelRequested()
{
    m_aCancelAsynchAccess.acquire();
    bool bReturn = m_bCancelAsynchRequest;
    m_aCancelAsynchAccess.release();
    return bReturn;
}


void FmSearchEngine::CancelSearch()
{
    m_aCancelAsynchAccess.acquire();
    m_bCancelAsynchRequest = true;
    m_aCancelAsynchAccess.release();
}


void FmSearchEngine::SwitchToContext(const Reference< css::sdbc::XResultSet > & xCursor, const OUString& sVisibleFields, const InterfaceArray& arrFields,
    sal_Int32 nFieldIndex)
{
    DBG_ASSERT(!m_bSearchingCurrently, "FmSearchEngine::SwitchToContext : please do not call while I'm searching !");
    if (m_bSearchingCurrently)
        return;

    m_xSearchCursor = xCursor;
    m_xOriginalIterator = xCursor;
    m_xClonedIterator = CursorWrapper(m_xOriginalIterator, true);

    fillControlTexts(arrFields);

    Init(sVisibleFields);
    RebuildUsedFields(nFieldIndex, true);
}


void FmSearchEngine::ImplStartNextSearch()
{
    m_bCancelAsynchRequest = false;
    m_bSearchingCurrently = true;

    SearchNextImpl();
    OnSearchTerminated();
}


void FmSearchEngine::SearchNext(const OUString& strExpression)
{
    m_strSearchExpression = strExpression;
    m_eSearchForType = SearchFor::String;
    ImplStartNextSearch();
}


void FmSearchEngine::SearchNextSpecial(bool _bSearchForNull)
{
    m_eSearchForType = _bSearchForNull ? SearchFor::Null : SearchFor::NotNull;
    ImplStartNextSearch();
}


void FmSearchEngine::StartOver(const OUString& strExpression)
{
    try
    {
        if (m_bForward)
            m_xSearchCursor.first();
        else
            m_xSearchCursor.last();
    }
    catch( const Exception& )
    {
        DBG_UNHANDLED_EXCEPTION("svx");
        return;
    }

    InvalidatePreviousLoc();
    SearchNext(strExpression);
}


void FmSearchEngine::StartOverSpecial(bool _bSearchForNull)
{
    try
    {
        if (m_bForward)
            m_xSearchCursor.first();
        else
            m_xSearchCursor.last();
    }
    catch( const Exception& )
    {
        DBG_UNHANDLED_EXCEPTION("svx");
        return;
    }

    InvalidatePreviousLoc();
    SearchNextSpecial(_bSearchForNull);
}


void FmSearchEngine::InvalidatePreviousLoc()
{
    m_aPreviousLocBookmark.clear();
    m_iterPreviousLocField = m_arrUsedFields.end();
}


void FmSearchEngine::RebuildUsedFields(sal_Int32 nFieldIndex, bool bForce)
{
    if (!bForce && (nFieldIndex == m_nCurrentFieldIndex))
        return;
    // (since I allow no change of the iterator from the outside, the same css::sdbcx::Index
    // also always means the same column, so I have nothing to do)

    DBG_ASSERT((nFieldIndex == -1) ||
               ((nFieldIndex >= 0) &&
                (static_cast<size_t>(nFieldIndex) < m_arrFieldMapping.size())),
            "FmSearchEngine::RebuildUsedFields : nFieldIndex is invalid!");
    // collect all fields I need to search through
    m_arrUsedFields.clear();
    if (nFieldIndex == -1)
    {
        Reference< css::container::XIndexAccess >  xFields;
        for (sal_Int32 i : m_arrFieldMapping)
        {
            Reference< css::sdbcx::XColumnsSupplier >  xSupplyCols(IFACECAST(m_xSearchCursor), UNO_QUERY);
            DBG_ASSERT(xSupplyCols.is(), "FmSearchEngine::RebuildUsedFields : invalid cursor (no columns supplier) !");
            xFields.set(xSupplyCols->getColumns(), UNO_QUERY);
            BuildAndInsertFieldInfo(xFields, i);
        }
    }
    else
    {
        Reference< css::container::XIndexAccess >  xFields;
        Reference< css::sdbcx::XColumnsSupplier >  xSupplyCols(IFACECAST(m_xSearchCursor), UNO_QUERY);
        DBG_ASSERT(xSupplyCols.is(), "FmSearchEngine::RebuildUsedFields : invalid cursor (no columns supplier) !");
        xFields.set (xSupplyCols->getColumns(), UNO_QUERY);
        BuildAndInsertFieldInfo(xFields, m_arrFieldMapping[static_cast< size_t >(nFieldIndex)]);
    }

    m_nCurrentFieldIndex = nFieldIndex;
    // and of course I start the next search in a virgin state again
    InvalidatePreviousLoc();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
