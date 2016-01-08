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
#include "svx/fmresids.hrc"
#include "svx/fmtools.hxx"
#include "svx/fmsrccfg.hxx"
#include <tools/debug.hxx>
#include <tools/diagnose_ex.h>
#include <tools/wldcrd.hxx>
#include <vcl/msgbox.hxx>
#include <svx/dialmgr.hxx>
#include <vcl/svapp.hxx>
#include <unotools/textsearch.hxx>
#include <com/sun/star/util/SearchOptions.hpp>
#include <com/sun/star/util/SearchAlgorithms.hpp>
#include <com/sun/star/util/SearchResult.hpp>
#include <com/sun/star/util/SearchFlags.hpp>
#include <com/sun/star/lang/Locale.hpp>
#include <com/sun/star/i18n/TransliterationModules.hpp>
#include <com/sun/star/i18n/CollatorOptions.hpp>

#include <com/sun/star/sdbcx/XColumnsSupplier.hpp>
#include <com/sun/star/util/NumberFormatter.hpp>
#include <com/sun/star/util/NumberFormat.hpp>
#include <com/sun/star/util/XNumberFormatsSupplier.hpp>
#include <com/sun/star/util/XNumberFormats.hpp>
#include <comphelper/processfactory.hxx>

#include "fmprop.hrc"
#include "fmservs.hxx"
#include "svx/fmsrcimp.hxx"
#include <svx/fmsearch.hxx>

#include <comphelper/numbers.hxx>
#include <unotools/syslocale.hxx>

#define EQUAL_BOOKMARKS(a, b) a == b

#define IFACECAST(c)          ((const Reference< XInterface >&)c)

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::util;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::i18n;
using namespace ::com::sun::star::beans;
using namespace ::svxform;



// = FmSearchThread

void FmSearchThread::run()
{
    osl_setThreadName("FmSearchThread");

    m_pEngine->SearchNextImpl();
};


void FmSearchThread::onTerminated()
{
    m_aTerminationHdl.Call(this);
    delete this;
}


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


Link<sal_Int32,void> FmRecordCountListener::SetPropChangeHandler(const Link<sal_Int32,void>& lnk)
{
    Link<sal_Int32,void> lnkReturn = m_lnkWhoWantsToKnow;
    m_lnkWhoWantsToKnow = lnk;

    if (m_xListening.is())
        NotifyCurrentCount();

    return lnkReturn;
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


void SAL_CALL FmRecordCountListener::disposing(const css::lang::EventObject& /*Source*/) throw( RuntimeException, std::exception )
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


void FmRecordCountListener::propertyChange(const  css::beans::PropertyChangeEvent& /*evt*/) throw(css::uno::RuntimeException, std::exception)
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
    switch ((TriState)m_xBox->getState())
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
                FmRecordCountListener* prclListener = new FmRecordCountListener(m_xSearchCursor);
                prclListener->acquire();
                prclListener->SetPropChangeHandler(LINK(this, FmSearchEngine, OnNewRecordCount));

                m_xSearchCursor.last();

                prclListener->DisConnect();
                prclListener->release();
            }
            else
                m_xSearchCursor.previous();
    }
    catch(css::sdbc::SQLException const& e)
    {
#if OSL_DEBUG_LEVEL > 0
        OStringBuffer sDebugMessage("FmSearchEngine::MoveCursor : catched a DatabaseException (");
        sDebugMessage.append(OUStringToOString(e.SQLState, RTL_TEXTENCODING_ASCII_US));
        sDebugMessage.append(") !");
        OSL_FAIL(sDebugMessage.getStr());
#else
        (void)e;
#endif
        bSuccess = false;
    }
    catch(Exception const& e)
    {
#if OSL_DEBUG_LEVEL > 0
        OStringBuffer sDebugMessage("FmSearchEngine::MoveCursor : catched an Exception (");
        sDebugMessage.append(OUStringToOString(e.Message, RTL_TEXTENCODING_ASCII_US));
        sDebugMessage.append(") !");
        OSL_FAIL(sDebugMessage.getStr());
#else
        (void)e;
#endif
        bSuccess = false;
    }
    catch(...)
    {
        OSL_FAIL("FmSearchEngine::MoveCursor : catched an unknown Exception !");
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

    // das Feld selber
    Reference< XInterface > xCurrentField;
    xAllFields->getByIndex(nField) >>= xCurrentField;

    // von dem weiss ich jetzt, dass es den DatabaseRecord-Service unterstuetzt (hoffe ich)
    // fuer den FormatKey und den Typ brauche ich das PropertySet
    Reference< css::beans::XPropertySet >  xProperties(xCurrentField, UNO_QUERY);

    // die FieldInfo dazu aufbauen
    FieldInfo fiCurrent;
    fiCurrent.xContents.set(xCurrentField, UNO_QUERY);
    fiCurrent.nFormatKey = ::comphelper::getINT32(xProperties->getPropertyValue(FM_PROP_FORMATKEY));
    fiCurrent.bDoubleHandling = false;
    if (m_xFormatSupplier.is())
    {
        Reference< css::util::XNumberFormats >  xNumberFormats(m_xFormatSupplier->getNumberFormats());

        sal_Int16 nFormatType = ::comphelper::getNumberFormatType(xNumberFormats, fiCurrent.nFormatKey) & ~((sal_Int16)css::util::NumberFormat::DEFINED);
        fiCurrent.bDoubleHandling = (nFormatType != css::util::NumberFormat::TEXT);
    }

    // und merken
    m_arrUsedFields.insert(m_arrUsedFields.end(), fiCurrent);

}

OUString FmSearchEngine::FormatField(const FieldInfo& rField)
{
    DBG_ASSERT(!m_bUsingTextComponents, "FmSearchEngine::FormatField : im UsingTextComponents-Mode bitte FormatField(sal_Int32) benutzen !");

    if (!m_xFormatter.is())
        return OUString();
    // sonst werden Datumsflder zum Beispiel zu irgendeinem Default-Wert formatiert

    OUString sReturn;
    try
    {
        if (rField.bDoubleHandling)
        {
            double fValue = rField.xContents->getDouble();
            if (!rField.xContents->wasNull())
                sReturn = m_xFormatter->convertNumberToString(rField.nFormatKey, fValue);
        }
        else
        {
            OUString sValue = rField.xContents->getString();
            if (!rField.xContents->wasNull())
                sReturn = m_xFormatter->formatString(rField.nFormatKey, sValue);
        }
    }
    catch(...)
    {
    }


    return sReturn;
}


OUString FmSearchEngine::FormatField(sal_Int32 nWhich)
{
    if (m_bUsingTextComponents)
    {
        DBG_ASSERT((sal_uInt32)nWhich < m_aControlTexts.size(), "FmSearchEngine::FormatField(sal_Int32) : invalid position !");
        DBG_ASSERT(m_aControlTexts[nWhich] != nullptr, "FmSearchEngine::FormatField(sal_Int32) : invalid object in array !");
        DBG_ASSERT(m_aControlTexts[nWhich]->getControl().is(), "FmSearchEngine::FormatField : invalid control !");

        if (m_nCurrentFieldIndex != -1)
        {
            DBG_ASSERT((nWhich == 0) || (nWhich == m_nCurrentFieldIndex), "FmSearchEngine::FormatField : Parameter nWhich ist ungueltig");
            // analoge Situation wie unten
            nWhich = m_nCurrentFieldIndex;
        }

        DBG_ASSERT((nWhich >= 0) && ((sal_uInt32)nWhich < m_aControlTexts.size()),
            "FmSearchEngine::FormatField : invalid argument nWhich !");
        return m_aControlTexts[m_nCurrentFieldIndex == -1 ? nWhich : m_nCurrentFieldIndex]->getCurrentText();
    }
    else
    {
        if (m_nCurrentFieldIndex != -1)
        {
            DBG_ASSERT((nWhich == 0) || (nWhich == m_nCurrentFieldIndex), "FmSearchEngine::FormatField : Parameter nWhich ist ungueltig");
            // ich bin im single-field-modus, da ist auch die richtige Feld-Nummer erlaubt, obwohl dann der richtige css::sdbcx::Index
            // fuer meinen Array-Zugriff natuerlich 0 ist
            nWhich = 0;
        }

        DBG_ASSERT((nWhich>=0) && (nWhich < (m_arrUsedFields.end() - m_arrUsedFields.begin())),
            "FmSearchEngine::FormatField : Parameter nWhich ist ungueltig");
        return FormatField(m_arrUsedFields[nWhich]);
    }
}


FmSearchEngine::SEARCH_RESULT FmSearchEngine::SearchSpecial(bool _bSearchForNull, sal_Int32& nFieldPos,
    FieldCollection::iterator& iterFieldLoop, const FieldCollection::iterator& iterBegin, const FieldCollection::iterator& iterEnd)
{
    // die Startposition merken
    Any aStartMark;
    try { aStartMark = m_xSearchCursor.getBookmark(); }
    catch ( const Exception& ) { DBG_UNHANDLED_EXCEPTION(); return SR_ERROR; }
    FieldCollection::iterator iterInitialField = iterFieldLoop;


    bool bFound(false);
    bool bMovedAround(false);
    do
    {
        if (m_eMode == SM_ALLOWSCHEDULE)
        {
            Application::Reschedule();
            Application::Reschedule();
            // do 2 reschedules because of #70226# : some things done within this loop's body may cause an user event
            // to be posted (deep within vcl), and these user events will be handled before any keyinput or paintings
            // or anything like that. So within each loop we create one user event and handle one user event (and no
            // paintings and these), so the office seems to be frozen while searching.
            // FS - 70226 - 02.12.99
        }

        // der aktuell zu vergleichende Inhalt
        iterFieldLoop->xContents->getString();  // needed for wasNull
        bFound = _bSearchForNull == bool(iterFieldLoop->xContents->wasNull());
        if (bFound)
            break;

        // naechstes Feld (implizit naechster Datensatz, wenn noetig)
        if (!MoveField(nFieldPos, iterFieldLoop, iterBegin, iterEnd))
        {   // beim Bewegen auf das naechste Feld ging was schief ... weitermachen ist nicht drin, da das naechste Mal genau
            // das selbe bestimmt wieder schief geht, also Abbruch
            // vorher aber noch, damit das Weitersuchen an der aktuellen Position weitermacht :
            try { m_aPreviousLocBookmark = m_xSearchCursor.getBookmark(); }
            catch ( const Exception& ) { DBG_UNHANDLED_EXCEPTION(); }
            m_iterPreviousLocField = iterFieldLoop;
            // und wech
            return SR_ERROR;
        }

        Any aCurrentBookmark;
        try { aCurrentBookmark = m_xSearchCursor.getBookmark(); }
        catch ( const Exception& ) { DBG_UNHANDLED_EXCEPTION(); return SR_ERROR; }

        bMovedAround = EQUAL_BOOKMARKS(aStartMark, aCurrentBookmark) && (iterFieldLoop == iterInitialField);

        if (nFieldPos == 0)
            // das heisst, ich habe mich auf einen neuen Datensatz bewegt
            PropagateProgress(bMovedAround);
                // if we moved to the starting position we don't have to propagate an 'overflow' message
                // FS - 07.12.99 - 68530

        // abbrechen gefordert ?
        if (CancelRequested())
            return SR_CANCELED;

    } while (!bMovedAround);

    return bFound ? SR_FOUND : SR_NOTFOUND;
}


FmSearchEngine::SEARCH_RESULT FmSearchEngine::SearchWildcard(const OUString& strExpression, sal_Int32& nFieldPos,
    FieldCollection::iterator& iterFieldLoop, const FieldCollection::iterator& iterBegin, const FieldCollection::iterator& iterEnd)
{
    // die Startposition merken
    Any aStartMark;
    try { aStartMark = m_xSearchCursor.getBookmark(); }
    catch ( const Exception& ) { DBG_UNHANDLED_EXCEPTION(); return SR_ERROR; }
    FieldCollection::iterator iterInitialField = iterFieldLoop;

    WildCard aSearchExpression(strExpression);


    bool bFound(false);
    bool bMovedAround(false);
    do
    {
        if (m_eMode == SM_ALLOWSCHEDULE)
        {
            Application::Reschedule();
            Application::Reschedule();
            // do 2 reschedules because of #70226# : some things done within this loop's body may cause an user event
            // to be posted (deep within vcl), and these user events will be handled before any keyinput or paintings
            // or anything like that. So within each loop we create one user event and handle one user event (and no
            // paintings and these), so the office seems to be frozen while searching.
            // FS - 70226 - 02.12.99
        }

        // der aktuell zu vergleichende Inhalt
        OUString sCurrentCheck;
        if (m_bFormatter)
            sCurrentCheck = FormatField(nFieldPos);
        else
            sCurrentCheck = iterFieldLoop->xContents->getString();

        if (!GetCaseSensitive())
            // norm the string
            sCurrentCheck = m_aCharacterClassficator.lowercase(sCurrentCheck);

        // jetzt ist der Test einfach ...
        bFound = aSearchExpression.Matches(sCurrentCheck);

        if (bFound)
            break;

        // naechstes Feld (implizit naechster Datensatz, wenn noetig)
        if (!MoveField(nFieldPos, iterFieldLoop, iterBegin, iterEnd))
        {   // beim Bewegen auf das naechste Feld ging was schief ... weitermachen ist nicht drin, da das naechste Mal genau
            // das selbe bestimmt wieder schief geht, also Abbruch
            // vorher aber noch, damit das Weitersuchen an der aktuellen Position weitermacht :
            try { m_aPreviousLocBookmark = m_xSearchCursor.getBookmark(); }
            catch ( const Exception& ) { DBG_UNHANDLED_EXCEPTION(); }
            m_iterPreviousLocField = iterFieldLoop;
            // und wech
            return SR_ERROR;
        }

        Any aCurrentBookmark;
        try { aCurrentBookmark = m_xSearchCursor.getBookmark(); }
        catch ( const Exception& ) { DBG_UNHANDLED_EXCEPTION(); return SR_ERROR; }

        bMovedAround = EQUAL_BOOKMARKS(aStartMark, aCurrentBookmark) && (iterFieldLoop == iterInitialField);

        if (nFieldPos == 0)
            // das heisst, ich habe mich auf einen neuen Datensatz bewegt
            PropagateProgress(bMovedAround);
                // if we moved to the starting position we don't have to propagate an 'overflow' message
                // FS - 07.12.99 - 68530

        // abbrechen gefordert ?
        if (CancelRequested())
            return SR_CANCELED;

    } while (!bMovedAround);

    return bFound ? SR_FOUND : SR_NOTFOUND;
}


FmSearchEngine::SEARCH_RESULT FmSearchEngine::SearchRegularApprox(const OUString& strExpression, sal_Int32& nFieldPos,
    FieldCollection::iterator& iterFieldLoop, const FieldCollection::iterator& iterBegin, const FieldCollection::iterator& iterEnd)
{
    DBG_ASSERT(m_bLevenshtein || m_bRegular,
        "FmSearchEngine::SearchRegularApprox : ungueltiger Suchmodus !");
    DBG_ASSERT(!m_bLevenshtein || !m_bRegular,
        "FmSearchEngine::SearchRegularApprox : kann nicht nach regulaeren Ausdruecken und nach Aehnlichkeiten gleichzeitig suchen !");

    // Startposition merken
    Any aStartMark;
    try { aStartMark = m_xSearchCursor.getBookmark(); }
    catch ( const Exception& ) { DBG_UNHANDLED_EXCEPTION(); return SR_ERROR; }
    FieldCollection::iterator iterInitialField = iterFieldLoop;

    // Parameter sammeln
    SearchOptions aParam;
    aParam.algorithmType = m_bRegular ? SearchAlgorithms_REGEXP : SearchAlgorithms_APPROXIMATE;
    aParam.searchFlag = 0;
    aParam.transliterateFlags = GetTransliterationFlags();
    if ( !GetTransliteration() )
    {   // if transliteration is not enabled, the only flags which matter are IGNORE_CASE and IGNORE_WIDTH
        aParam.transliterateFlags &= TransliterationModules_IGNORE_CASE | TransliterationModules_IGNORE_WIDTH;
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
    ::utl::TextSearch aLocalEngine(aParam);


    bool bFound = false;
    bool bMovedAround(false);
    do
    {
        if (m_eMode == SM_ALLOWSCHEDULE)
        {
            Application::Reschedule();
            Application::Reschedule();
            // do 2 reschedules because of #70226# : some things done within this loop's body may cause an user event
            // to be posted (deep within vcl), and these user events will be handled before any keyinput or paintings
            // or anything like that. So within each loop we create one user event and handle one user event (and no
            // paintings and these), so the office seems to be frozen while searching.
            // FS - 70226 - 02.12.99
        }

        // der aktuell zu vergleichende Inhalt
        OUString sCurrentCheck;
        if (m_bFormatter)
            sCurrentCheck = FormatField(nFieldPos);
        else
            sCurrentCheck = iterFieldLoop->xContents->getString();

        // (don't care about case here, this is done by the TextSearch object, 'cause we passed our case parameter to it)

        sal_Int32 nStart = 0, nEnd = sCurrentCheck.getLength();
        bFound = aLocalEngine.SearchForward(sCurrentCheck, &nStart, &nEnd);
            // das heisst hier 'forward' aber das bezieht sich nur auf die Suche innerhalb von sCurrentCheck, hat also mit
            // der Richtung meines Datensatz-Durchwanderns nix zu tun (darum kuemmert sich MoveField)

        // checken, ob die Position stimmt
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
                    // laeuft in den naechsten Case rein !
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

        if (bFound) // immer noch ?
            break;

        // naechstes Feld (implizit naechster Datensatz, wenn noetig)
        if (!MoveField(nFieldPos, iterFieldLoop, iterBegin, iterEnd))
        {   // beim Bewegen auf das naechste Feld ging was schief ... weitermachen ist nicht drin, da das naechste Mal genau
            // das selbe bestimmt wieder schief geht, also Abbruch (ohne Fehlermeldung, von der erwarte ich, dass sie im Move
            // angezeigt wurde)
            // vorher aber noch, damit das Weitersuchen an der aktuellen Position weitermacht :
            try { m_aPreviousLocBookmark = m_xSearchCursor.getBookmark(); }
            catch ( const Exception& ) { DBG_UNHANDLED_EXCEPTION(); }
            m_iterPreviousLocField = iterFieldLoop;
            // und wech
            return SR_ERROR;
        }

        Any aCurrentBookmark;
        try { aCurrentBookmark = m_xSearchCursor.getBookmark(); }
        catch ( const Exception& ) { DBG_UNHANDLED_EXCEPTION(); return SR_ERROR; }
        bMovedAround = EQUAL_BOOKMARKS(aStartMark, aCurrentBookmark) && (iterFieldLoop == iterInitialField);

        if (nFieldPos == 0)
            // das heisst, ich habe mich auf einen neuen Datensatz bewegt
            PropagateProgress(bMovedAround);
                // if we moved to the starting position we don't have to propagate an 'overflow' message
                // FS - 07.12.99 - 68530

        // abbrechen gefordert ?
        if (CancelRequested())
            return SR_CANCELED;

    } while (!bMovedAround);

    return bFound ? SR_FOUND : SR_NOTFOUND;
}



FmSearchEngine::FmSearchEngine(const Reference< XComponentContext >& _rxContext,
            const Reference< XResultSet > & xCursor, const OUString& sVisibleFields,
            const Reference< XNumberFormatsSupplier > & xFormatSupplier, FMSEARCH_MODE eMode)

    :m_xSearchCursor(xCursor)
    ,m_xFormatSupplier(xFormatSupplier)
    ,m_aCharacterClassficator( _rxContext, SvtSysLocale().GetLanguageTag() )
    ,m_aStringCompare( _rxContext )
    ,m_nCurrentFieldIndex(-2)   // -1 hat schon eine Bedeutung, also nehme ich -2 fuer 'ungueltig'
    ,m_bUsingTextComponents(false)
    ,m_eSearchForType(SEARCHFOR_STRING)
    ,m_srResult(SR_FOUND)
    ,m_bSearchingCurrently(false)
    ,m_bCancelAsynchRequest(false)
    ,m_eMode(eMode)
    ,m_bFormatter(false)
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
    ,m_nTransliterationFlags(0)
{

    m_xFormatter.set( css::util::NumberFormatter::create( ::comphelper::getProcessComponentContext() ),
                      UNO_QUERY_THROW);
    m_xFormatter->attachNumberFormatsSupplier(m_xFormatSupplier);

    Init(sVisibleFields);
}


FmSearchEngine::FmSearchEngine(const Reference< XComponentContext >& _rxContext,
        const Reference< XResultSet > & xCursor, const OUString& sVisibleFields,
        const InterfaceArray& arrFields, FMSEARCH_MODE eMode)
    :m_xSearchCursor(xCursor)
    ,m_aCharacterClassficator( _rxContext, SvtSysLocale().GetLanguageTag() )
    ,m_aStringCompare( _rxContext )
    ,m_nCurrentFieldIndex(-2)   // -1 hat schon eine Bedeutung, also nehme ich -2 fuer 'ungueltig'
    ,m_bUsingTextComponents(true)
    ,m_xOriginalIterator(xCursor)
    ,m_xClonedIterator(m_xOriginalIterator, true)
    ,m_eSearchForType(SEARCHFOR_STRING)
    ,m_srResult(SR_FOUND)
    ,m_bSearchingCurrently(false)
    ,m_bCancelAsynchRequest(false)
    ,m_eMode(eMode)
    ,m_bFormatter(true)     // das muss konsistent sein mit m_xSearchCursor, der i.A. == m_xOriginalIterator ist
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
    ,m_nTransliterationFlags(0)
{

    fillControlTexts(arrFields);
    Init(sVisibleFields);
}


FmSearchEngine::~FmSearchEngine()
{
    clearControlTexts();

}


void FmSearchEngine::SetIgnoreWidthCJK(bool bSet)
{
    if (bSet)
        m_nTransliterationFlags |= TransliterationModules_IGNORE_WIDTH;
    else
        m_nTransliterationFlags &= ~TransliterationModules_IGNORE_WIDTH;
}


bool FmSearchEngine::GetIgnoreWidthCJK() const
{
    return 0 != (m_nTransliterationFlags & TransliterationModules_IGNORE_WIDTH);
}


void FmSearchEngine::SetCaseSensitive(bool bSet)
{
    if (bSet)
        m_nTransliterationFlags &= ~TransliterationModules_IGNORE_CASE;
    else
        m_nTransliterationFlags |= TransliterationModules_IGNORE_CASE;
}


bool FmSearchEngine::GetCaseSensitive() const
{
    return 0 == (m_nTransliterationFlags & TransliterationModules_IGNORE_CASE);
}


void FmSearchEngine::clearControlTexts()
{
    for (   ControlTextSuppliers::iterator aIter = m_aControlTexts.begin();
            aIter < m_aControlTexts.end();
            ++aIter
        )
    {
        delete *aIter;
    }
    m_aControlTexts.clear();
}


void FmSearchEngine::fillControlTexts(const InterfaceArray& arrFields)
{
    clearControlTexts();
    Reference< XInterface >  xCurrent;
    for (size_t i=0; i<arrFields.size(); ++i)
    {
        xCurrent = arrFields.at(i);
        DBG_ASSERT(xCurrent.is(), "FmSearchEngine::fillControlTexts : invalid field interface !");
        // check which type of control this is
        Reference< css::awt::XTextComponent >  xAsText(xCurrent, UNO_QUERY);
        if (xAsText.is())
        {
            m_aControlTexts.insert(m_aControlTexts.end(), new SimpleTextWrapper(xAsText));
            continue;
        }

        Reference< css::awt::XListBox >  xAsListBox(xCurrent, UNO_QUERY);
        if (xAsListBox.is())
        {
            m_aControlTexts.insert(m_aControlTexts.end(), new ListBoxWrapper(xAsListBox));
            continue;
        }

        Reference< css::awt::XCheckBox >  xAsCheckBox(xCurrent, UNO_QUERY);
        DBG_ASSERT(xAsCheckBox.is(), "FmSearchEngine::fillControlTexts : invalid field interface (no supported type) !");
            // we don't have any more options ...
        m_aControlTexts.insert(m_aControlTexts.end(), new CheckBoxWrapper(xAsCheckBox));
    }
}


void FmSearchEngine::Init(const OUString& sVisibleFields)
{
    // analyze the fields
    // additionally, create the mapping: because the list of used columns can be shorter than the list
    // of columns of the cursor, we need a mapping: "used column numer n" -> "cursor column m"
    m_arrFieldMapping.clear();

    // important: The case of the columns does not need to be exact - for instance:
    // - a user created a form which works on a table, for which the driver returns a column name "COLUMN"
    // - the driver itself works case-insensitve with column names
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
        // der Cursor kann mir einen Record (als PropertySet) liefern, dieser unterstuetzt den DatabaseRecord-Service
        Reference< css::sdbcx::XColumnsSupplier >  xSupplyCols(IFACECAST(m_xSearchCursor), UNO_QUERY);
        DBG_ASSERT(xSupplyCols.is(), "FmSearchEngine::Init : invalid cursor (no columns supplier) !");
        Reference< css::container::XNameAccess >       xAllFieldNames = xSupplyCols->getColumns();
        Sequence< OUString > seqFieldNames = xAllFieldNames->getElementNames();
        OUString*            pFieldNames = seqFieldNames.getArray();


        OUString sCurrentField;
        OUString sVis(sVisibleFields.getStr());
        sal_Int32 nIndex = 0;
        do
        {
            sCurrentField = sVis.getToken(0, ';' , nIndex);

            // in der Feld-Sammlung suchen
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

    if (m_bUsingTextComponents)
    {
        // ich benutzte keinen Formatter, sondern TextComponents -> der SearchIterator muss angepasst werden
        try
        {
            if (m_bFormatter)
            {
                DBG_ASSERT(m_xSearchCursor == m_xClonedIterator, "FmSearchEngine::SetFormatterUsing : inkonsistenter Zustand !");
                m_xSearchCursor = m_xOriginalIterator;
                m_xSearchCursor.moveToBookmark(m_xClonedIterator.getBookmark());
                    // damit ich mit dem neuen Iterator wirklich dort weitermache, wo ich vorher aufgehoert habe
            }
            else
            {
                DBG_ASSERT(m_xSearchCursor == m_xOriginalIterator, "FmSearchEngine::SetFormatterUsing : inkonsistenter Zustand !");
                m_xSearchCursor = m_xClonedIterator;
                m_xSearchCursor.moveToBookmark(m_xOriginalIterator.getBookmark());
            }
        }
        catch( const Exception& )
        {
            DBG_UNHANDLED_EXCEPTION();
        }

        // ich muss die Fields neu binden, da der Textaustausch eventuell ueber diese Fields erfolgt und sich der unterliegende Cursor
        // geaendert hat
        RebuildUsedFields(m_nCurrentFieldIndex, true);
    }
    else
        InvalidatePreviousLoc();
}


void FmSearchEngine::PropagateProgress(bool _bDontPropagateOverflow)
{
    if (m_aProgressHandler.IsSet())
    {
        FmSearchProgress aProgress;
        try
        {
            aProgress.aSearchState = FmSearchProgress::STATE_PROGRESS;
            aProgress.nCurrentRecord = m_xSearchCursor.getRow() - 1;
            if (m_bForward)
                aProgress.bOverflow = !_bDontPropagateOverflow && m_xSearchCursor.isFirst();
            else
                aProgress.bOverflow = !_bDontPropagateOverflow && m_xSearchCursor.isLast();
        }
        catch( const Exception& )
        {
            DBG_UNHANDLED_EXCEPTION();
        }

        m_aProgressHandler.Call(&aProgress);
    }
}


void FmSearchEngine::SearchNextImpl()
{
    DBG_ASSERT(!(m_bWildcard && m_bRegular) && !(m_bRegular && m_bLevenshtein) && !(m_bLevenshtein && m_bWildcard),
        "FmSearchEngine::SearchNextImpl : Suchparameter schliessen sich gegenseitig aus !");

    DBG_ASSERT(m_xSearchCursor.is(), "FmSearchEngine::SearchNextImpl : habe ungueltigen Iterator !");

    // die Parameter der Suche
    OUString strSearchExpression(m_strSearchExpression); // brauche ich non-const
    if (!GetCaseSensitive())
        // norm the string
        strSearchExpression = m_aCharacterClassficator.lowercase(strSearchExpression);

    if (!m_bRegular && !m_bLevenshtein)
    {   // 'normale' Suche fuehre ich auf jeden Fall ueber WildCards durch, muss aber vorher je nach Modus den OUString anpassen

        if (!m_bWildcard)
        {   // da natuerlich in allen anderen Faellen auch * und ? im Suchstring erlaubt sind, aber nicht als WildCards zaehlen
            // sollen, muss ich normieren
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
                    OSL_FAIL("FmSearchEngine::SearchNextImpl() : die Methoden-Listbox duerfte nur 4 Eintraege enthalten ...");
            }
        }
    }

    // fuer Arbeit auf Feldliste
    FieldCollection::iterator iterBegin = m_arrUsedFields.begin();
    FieldCollection::iterator iterEnd = m_arrUsedFields.end();
    FieldCollection::iterator iterFieldCheck;

    sal_Int32 nFieldPos;

    if (HasPreviousLoc())
    {
        DBG_ASSERT(EQUAL_BOOKMARKS(m_aPreviousLocBookmark, m_xSearchCursor.getBookmark()),
            "FmSearchEngine::SearchNextImpl : ungueltige Position !");
        iterFieldCheck = m_iterPreviousLocField;
        // im Feld nach (oder vor) der letzten Fundstelle weitermachen
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
    SEARCH_RESULT srResult;
    if (m_eSearchForType != SEARCHFOR_STRING)
        srResult = SearchSpecial(m_eSearchForType == SEARCHFOR_NULL, nFieldPos, iterFieldCheck, iterBegin, iterEnd);
    else if (!m_bRegular && !m_bLevenshtein)
        srResult = SearchWildcard(strSearchExpression, nFieldPos, iterFieldCheck, iterBegin, iterEnd);
    else
        srResult = SearchRegularApprox(strSearchExpression, nFieldPos, iterFieldCheck, iterBegin, iterEnd);

    m_srResult = srResult;

    if (SR_ERROR == m_srResult)
        return;

    // gefunden ?
    if (SR_FOUND == m_srResult)
    {
        // die Pos merken
        try { m_aPreviousLocBookmark = m_xSearchCursor.getBookmark(); }
        catch ( const Exception& ) { DBG_UNHANDLED_EXCEPTION(); }
        m_iterPreviousLocField = iterFieldCheck;
    }
    else
        // die "letzte Fundstelle" invalidieren
        InvalidatePreviousLoc();
}


IMPL_LINK_NOARG_TYPED(FmSearchEngine, OnSearchTerminated, FmSearchThread*, void)
{
    if (!m_aProgressHandler.IsSet())
        return;

    FmSearchProgress aProgress;
    try
    {
        switch (m_srResult)
        {
            case SR_ERROR :
                aProgress.aSearchState = FmSearchProgress::STATE_ERROR;
                break;
            case SR_FOUND :
                aProgress.aSearchState = FmSearchProgress::STATE_SUCCESSFULL;
                aProgress.aBookmark = m_aPreviousLocBookmark;
                aProgress.nFieldIndex = m_iterPreviousLocField - m_arrUsedFields.begin();
                break;
            case SR_NOTFOUND :
                aProgress.aSearchState = FmSearchProgress::STATE_NOTHINGFOUND;
                aProgress.aBookmark = m_xSearchCursor.getBookmark();
                break;
            case SR_CANCELED :
                aProgress.aSearchState = FmSearchProgress::STATE_CANCELED;
                aProgress.aBookmark = m_xSearchCursor.getBookmark();
                break;
        }
        aProgress.nCurrentRecord = m_xSearchCursor.getRow() - 1;
    }
    catch( const Exception& )
    {
        DBG_UNHANDLED_EXCEPTION();
    }

    // per definitionem muss der Link Thread-sicher sein (das verlange ich einfach), so dass ich mich um so etwas hier nicht kuemmern muss
    m_aProgressHandler.Call(&aProgress);

    m_bSearchingCurrently = false;
}


IMPL_LINK_TYPED(FmSearchEngine, OnNewRecordCount, sal_Int32, theCounter, void)
{
    if (!m_aProgressHandler.IsSet())
        return;

    FmSearchProgress aProgress;
    aProgress.nCurrentRecord = theCounter;
    aProgress.aSearchState = FmSearchProgress::STATE_PROGRESS_COUNTING;
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


bool FmSearchEngine::SwitchToContext(const Reference< css::sdbc::XResultSet > & xCursor, const OUString& sVisibleFields, const InterfaceArray& arrFields,
    sal_Int32 nFieldIndex)
{
    DBG_ASSERT(!m_bSearchingCurrently, "FmSearchEngine::SwitchToContext : please do not call while I'm searching !");
    if (m_bSearchingCurrently)
        return false;

    m_xSearchCursor = xCursor;
    m_xOriginalIterator = xCursor;
    m_xClonedIterator = CursorWrapper(m_xOriginalIterator, true);
    m_bUsingTextComponents = true;

    fillControlTexts(arrFields);

    Init(sVisibleFields);
    RebuildUsedFields(nFieldIndex, true);

    return true;
}


void FmSearchEngine::ImplStartNextSearch()
{
    m_bCancelAsynchRequest = false;
    m_bSearchingCurrently = true;

    if (m_eMode == SM_USETHREAD)
    {
        FmSearchThread* pSearcher = new FmSearchThread(this);
            // der loescht sich nach Beendigung selber ...
        pSearcher->setTerminationHandler(LINK(this, FmSearchEngine, OnSearchTerminated));

        pSearcher->createSuspended();
        pSearcher->setPriority(osl_Thread_PriorityLowest);
        pSearcher->resume();
    }
    else
    {
        SearchNextImpl();
        LINK(this, FmSearchEngine, OnSearchTerminated).Call(nullptr);
    }
}


void FmSearchEngine::SearchNext(const OUString& strExpression)
{
    m_strSearchExpression = strExpression;
    m_eSearchForType = SEARCHFOR_STRING;
    ImplStartNextSearch();
}


void FmSearchEngine::SearchNextSpecial(bool _bSearchForNull)
{
    m_eSearchForType = _bSearchForNull ? SEARCHFOR_NULL : SEARCHFOR_NOTNULL;
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
        DBG_UNHANDLED_EXCEPTION();
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
        DBG_UNHANDLED_EXCEPTION();
        return;
    }

    InvalidatePreviousLoc();
    SearchNextSpecial(_bSearchForNull);
}


void FmSearchEngine::InvalidatePreviousLoc()
{
    m_aPreviousLocBookmark.setValue(nullptr,cppu::UnoType<void>::get());
    m_iterPreviousLocField = m_arrUsedFields.end();
}


void FmSearchEngine::RebuildUsedFields(sal_Int32 nFieldIndex, bool bForce)
{
    if (!bForce && (nFieldIndex == m_nCurrentFieldIndex))
        return;
    // (da ich keinen Wechsel des Iterators von aussen zulasse, heisst selber css::sdbcx::Index auch immer selbe Spalte, also habe ich nix zu tun)

    DBG_ASSERT((nFieldIndex == -1) ||
               ((nFieldIndex >= 0) &&
                (static_cast<size_t>(nFieldIndex) < m_arrFieldMapping.size())),
            "FmSearchEngine::RebuildUsedFields : nFieldIndex is invalid!");
    // alle Felder, die ich durchsuchen muss, einsammeln
    m_arrUsedFields.clear();
    if (nFieldIndex == -1)
    {
        Reference< css::container::XIndexAccess >  xFields;
        for (size_t i=0; i<m_arrFieldMapping.size(); ++i)
        {
            Reference< css::sdbcx::XColumnsSupplier >  xSupplyCols(IFACECAST(m_xSearchCursor), UNO_QUERY);
            DBG_ASSERT(xSupplyCols.is(), "FmSearchEngine::RebuildUsedFields : invalid cursor (no columns supplier) !");
            xFields.set(xSupplyCols->getColumns(), UNO_QUERY);
            BuildAndInsertFieldInfo(xFields, m_arrFieldMapping[i]);
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
    // und natuerlich beginne ich die naechste Suche wieder jungfraeulich
    InvalidatePreviousLoc();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
