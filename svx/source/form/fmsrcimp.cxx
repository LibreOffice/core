/*************************************************************************
 *
 *  $RCSfile: fmsrcimp.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: fs $ $Date: 2000-10-20 14:18:56 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef _SVX_FMRESIDS_HRC
#include "fmresids.hrc"
#endif

#ifndef _MULTIPRO_HXX
#include "multipro.hxx"
#endif

#ifndef _SVX_FMTOOLS_HXX
#include "fmtools.hxx"
#endif

#ifndef _FMSRCCF_HXX_
#include "fmsrccfg.hxx"
#endif

#ifndef _TOOLS_DEBUG_HXX //autogen
#include <tools/debug.hxx>
#endif

#ifndef _WLDCRD_HXX //autogen
#include <tools/wldcrd.hxx>
#endif

#ifndef _SV_MSGBOX_HXX //autogen
#include <vcl/msgbox.hxx>
#endif

#ifndef _SHL_HXX
#include <tools/shl.hxx>
#endif

#ifndef _SVX_DIALMGR_HXX //autogen
#include <dialmgr.hxx>
#endif

#ifndef _CPPUHELPER_SERVICEFACTORY_HXX_
#include <cppuhelper/servicefactory.hxx>
#endif

#ifndef _SV_SVAPP_HXX //autogen
#include <vcl/svapp.hxx>
#endif

#ifndef _TXTCMP_HXX //autogen
#include <svtools/txtcmp.hxx>
#endif

#ifndef _ISOLANG_HXX
#include <tools/isolang.hxx>
#endif

#ifndef _COM_SUN_STAR_SDDB_XCOLUMNSSUPPLIER_HPP_
#include <com/sun/star/sdbcx/XColumnsSupplier.hpp>
#endif

#ifndef _COM_SUN_STAR_UTIL_XNUMBERFORMATTER_HPP_
#include <com/sun/star/util/XNumberFormatter.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_NUMBERFORMAT_HPP_
#include <com/sun/star/util/NumberFormat.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_XNUMBERFORMATSSUPPLIER_HPP_
#include <com/sun/star/util/XNumberFormatsSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_XNUMBERFORMATS_HPP_
#include <com/sun/star/util/XNumberFormats.hpp>
#endif
#ifndef _COMPHELPER_PROCESSFACTORY_HXX_
#include <comphelper/processfactory.hxx>
#endif

#ifndef _SVX_FMPROP_HRC
#include "fmprop.hrc"
#endif
#ifndef _SVX_FMSERVS_HXX
#include "fmservs.hxx"
#endif

#ifndef _FMSRCIMP_HXX
#include "fmsrcimp.hxx"
#endif

#ifndef _FMSEARCH_HXX
#include "fmsearch.hxx"
#endif

#ifndef _FMSEARCH_HRC
#include "fmsearch.hrc"
#endif

#ifndef _COMPHELPER_NUMBERS_HXX_
#include <comphelper/numbers.hxx>
#endif


//#define COMPARE_BOOKMARKS(a, b) compareUsrAny(a, b)
#define COMPARE_BOOKMARKS(a, b) ::comphelper::compare(a, b)

// damit ich waehrend des Debuggings keine inline-Methoden habe ...
#if DEBUG || DBG_UTIL
#define INLINE_METHOD
#else
#define INLINE_METHOD inline
#endif // DEBUG || DBG_UTIL

// ***************************************************************************************************

SV_IMPL_OBJARR(SvInt32Array, sal_Int32);
DECLARE_STL_VECTOR( ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > ,InterfaceArray);

//========================================================================
// = FmSearchThread
//------------------------------------------------------------------------
void FmSearchThread::run()
{
    m_pEngine->SearchNextImpl();
};

//------------------------------------------------------------------------
void FmSearchThread::onTerminated()
{
    if (m_aTerminationHdl.IsSet())
        m_aTerminationHdl.Call(this);
    delete this;
}

//========================================================================
// = FmRecordCountListener

//  SMART_UNO_IMPLEMENTATION(FmRecordCountListener, UsrObject);

DBG_NAME(FmRecordCountListener);
//------------------------------------------------------------------------
FmRecordCountListener::FmRecordCountListener(const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XResultSet > & dbcCursor)
{
    DBG_CTOR(FmRecordCountListener,NULL);

    m_xListening = ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet > (dbcCursor, ::com::sun::star::uno::UNO_QUERY);
    if (!m_xListening.is())
        return;

    if (::comphelper::getBOOL(m_xListening->getPropertyValue(FM_PROP_ROWCOUNTFINAL)))
    {
        m_xListening = NULL;
        // there's nothing to do as the record count is already known
        return;
    }

    m_xListening->addPropertyChangeListener(FM_PROP_ROWCOUNT, (::com::sun::star::beans::XPropertyChangeListener*)this);
}

//------------------------------------------------------------------------
Link FmRecordCountListener::SetPropChangeHandler(const Link& lnk)
{
    Link lnkReturn = m_lnkWhoWantsToKnow;
    m_lnkWhoWantsToKnow = lnk;

    if (m_xListening.is())
        NotifyCurrentCount();

    return lnkReturn;
}

//------------------------------------------------------------------------
FmRecordCountListener::~FmRecordCountListener()
{

    DBG_DTOR(FmRecordCountListener,NULL);
}

//------------------------------------------------------------------------
void FmRecordCountListener::DisConnect()
{
    if(m_xListening.is())
        m_xListening->removePropertyChangeListener(FM_PROP_ROWCOUNT, (::com::sun::star::beans::XPropertyChangeListener*)this);
    m_xListening = NULL;
}

//------------------------------------------------------------------------
void SAL_CALL FmRecordCountListener::disposing(const ::com::sun::star::lang::EventObject& Source) throw( ::com::sun::star::uno::RuntimeException )
{
    DBG_ASSERT(m_xListening.is(), "FmRecordCountListener::disposing should never have been called without a propset !");
    DisConnect();
}

//------------------------------------------------------------------------
void FmRecordCountListener::NotifyCurrentCount()
{
    if (m_lnkWhoWantsToKnow.IsSet())
    {
        DBG_ASSERT(m_xListening.is(), "FmRecordCountListener::NotifyCurrentCount : I have no propset ... !?");
        void* pTheCount = (void*)::comphelper::getINT32(m_xListening->getPropertyValue(FM_PROP_ROWCOUNT));
        m_lnkWhoWantsToKnow.Call(pTheCount);
    }
}

//------------------------------------------------------------------------
void FmRecordCountListener::propertyChange(const  ::com::sun::star::beans::PropertyChangeEvent& evt)
{
    NotifyCurrentCount();
}

//========================================================================
// FmSearchEngine - local classes
//------------------------------------------------------------------------
FmSearchEngine::SimpleTextWrapper::SimpleTextWrapper(const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XTextComponent > & _xText)
    :ControlTextWrapper(::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > (_xText, ::com::sun::star::uno::UNO_QUERY))
    ,m_xText(_xText)
{
    DBG_ASSERT(m_xText.is(), "FmSearchEngine::SimpleTextWrapper::SimpleTextWrapper : invalid argument !");
}

//------------------------------------------------------------------------
::rtl::OUString FmSearchEngine::SimpleTextWrapper::getCurrentText() const
{
    return m_xText->getText();
}

//------------------------------------------------------------------------
FmSearchEngine::ListBoxWrapper::ListBoxWrapper(const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XListBox > & _xBox)
    :ControlTextWrapper(::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > (_xBox, ::com::sun::star::uno::UNO_QUERY))
    ,m_xBox(_xBox)
{
    DBG_ASSERT(m_xBox.is(), "FmSearchEngine::ListBoxWrapper::ListBoxWrapper : invalid argument !");
}

//------------------------------------------------------------------------
::rtl::OUString FmSearchEngine::ListBoxWrapper::getCurrentText() const
{
    return m_xBox->getSelectedItem();
}

//------------------------------------------------------------------------
FmSearchEngine::CheckBoxWrapper::CheckBoxWrapper(const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XCheckBox > & _xBox)
    :ControlTextWrapper(::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > (_xBox, ::com::sun::star::uno::UNO_QUERY))
    ,m_xBox(_xBox)
{
    DBG_ASSERT(m_xBox.is(), "FmSearchEngine::CheckBoxWrapper::CheckBoxWrapper : invalid argument !");
}

//------------------------------------------------------------------------
::rtl::OUString FmSearchEngine::CheckBoxWrapper::getCurrentText() const
{
    switch ((TriState)m_xBox->getState())
    {
        case STATE_NOCHECK: return rtl::OUString::createFromAscii("0");
        case STATE_CHECK: return rtl::OUString::createFromAscii("1");
    }
    return rtl::OUString();
}

//========================================================================
// = FmSearchEngine
//------------------------------------------------------------------------
sal_Bool FmSearchEngine::MoveCursor()
{
    sal_Bool bSuccess = sal_True;
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
#if _DEBUG || DBG_UTIL
    catch(::com::sun::star::sdbc::SQLException  e)
    {
        String sDebugMessage;
        sDebugMessage.AssignAscii("FmSearchEngine::MoveCursor : catched a DatabaseException (");
        sDebugMessage += (const sal_Unicode*)e.SQLState;
        sDebugMessage.AppendAscii(") !");
        DBG_ERROR(ByteString(sDebugMessage, RTL_TEXTENCODING_ASCII_US).GetBuffer());

        bSuccess = sal_False;
    }
    catch(::com::sun::star::uno::Exception  e)
    {
        UniString sDebugMessage;
        sDebugMessage.AssignAscii("FmSearchEngine::MoveCursor : catched an ::com::sun::star::uno::Exception (");
        sDebugMessage += (const sal_Unicode*)e.Message;
        sDebugMessage.AppendAscii(") !");
        DBG_ERROR(ByteString(sDebugMessage, RTL_TEXTENCODING_ASCII_US).GetBuffer());

        bSuccess = sal_False;
    }
#endif // _DEBUG || DBG_UTIL
    catch(...)
    {
        DBG_ERROR("FmSearchEngine::MoveCursor : catched an unknown ::com::sun::star::uno::Exception !");
        bSuccess = sal_False;
    }


    return bSuccess;
}

//------------------------------------------------------------------------
INLINE_METHOD sal_Bool FmSearchEngine::MoveField(sal_Int32& nPos, FieldCollectionIterator& iter, const FieldCollectionIterator& iterBegin, const FieldCollectionIterator& iterEnd)
{
    sal_Bool bSuccess(sal_True);
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

//------------------------------------------------------------------------
void FmSearchEngine::BuildAndInsertFieldInfo(const ::com::sun::star::uno::Reference< ::com::sun::star::container::XIndexAccess > & xAllFields, sal_Int32 nField)
{
    // das Feld selber
    ::com::sun::star::uno::Any anyCurrentField = xAllFields->getByIndex(nField);
    ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > & xCurrentField = *(::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > *)anyCurrentField.getValue();

    // von dem weiss ich jetzt, dass es den DatabaseRecord-Service unterstuetzt (hoffe ich)
    // fuer den FormatKey und den Typ brauche ich das PropertySet
    ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >  xProperties(xCurrentField, ::com::sun::star::uno::UNO_QUERY);

    // die FieldInfo dazu aufbauen
    FieldInfo fiCurrent;
    fiCurrent.xContents = ::com::sun::star::uno::Reference< ::com::sun::star::sdb::XColumn > (xCurrentField, ::com::sun::star::uno::UNO_QUERY);
    fiCurrent.nFormatKey = ::comphelper::getINT32(xProperties->getPropertyValue(FM_PROP_FORMATKEY));
    fiCurrent.bDoubleHandling = sal_False;
    if (m_xFormatSupplier.is())
    {
        ::com::sun::star::uno::Reference< ::com::sun::star::util::XNumberFormats >  xNumberFormats(m_xFormatSupplier->getNumberFormats());

        sal_Int16 nFormatType = ::comphelper::getNumberFormatType(xNumberFormats, fiCurrent.nFormatKey) & ~((sal_Int16)::com::sun::star::util::NumberFormat::DEFINED);
        fiCurrent.bDoubleHandling = (nFormatType != ::com::sun::star::util::NumberFormat::TEXT);
    }

    // und merken
    m_arrUsedFields.insert(m_arrUsedFields.end(), fiCurrent);

}
//------------------------------------------------------------------------
INLINE_METHOD ::rtl::OUString FmSearchEngine::FormatField(const FieldInfo& rField)
{
    DBG_ASSERT(!m_bUsingTextComponents, "FmSearchEngine::FormatField : im UsingTextComponents-Mode bitte FormatField(sal_Int32) benutzen !");

    if (!m_xFormatter.is())
        return ::rtl::OUString();
    // sonst werden Datumsflder zum Beispiel zu irgendeinem Default-Wert formatiert

    ::rtl::OUString sReturn;
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
            ::rtl::OUString sValue = rField.xContents->getString();
            if (!rField.xContents->wasNull())
                sReturn = m_xFormatter->formatString(rField.nFormatKey, sValue);
        }
    }
    catch(...)
    {
    }


    return sReturn;
}

//------------------------------------------------------------------------
::rtl::OUString FmSearchEngine::FormatField(sal_Int32 nWhich)
{
    if (m_bUsingTextComponents)
    {
        DBG_ASSERT(nWhich < m_aControlTexts.size(), "FmSearchEngine::FormatField(sal_Int32) : invalid position !");
        DBG_ASSERT(m_aControlTexts[nWhich] != NULL, "FmSearchEngine::FormatField(sal_Int32) : invalid object in array !");
        DBG_ASSERT(m_aControlTexts[nWhich]->getControl().is(), "FmSearchEngine::FormatField : invalid control !");

        if (m_nCurrentFieldIndex != -1)
        {
            DBG_ASSERT((nWhich == 0) || (nWhich == m_nCurrentFieldIndex), "FmSearchEngine::FormatField : Parameter nWhich ist ungueltig");
            // analoge Situation wie unten
            nWhich = m_nCurrentFieldIndex;
        }

        DBG_ASSERT((nWhich>=0) && (nWhich < m_aControlTexts.size()),
            "FmSearchEngine::FormatField : invalid argument nWhich !");
        return m_aControlTexts[m_nCurrentFieldIndex == -1 ? nWhich : m_nCurrentFieldIndex]->getCurrentText();
    }
    else
    {
        if (m_nCurrentFieldIndex != -1)
        {
            DBG_ASSERT((nWhich == 0) || (nWhich == m_nCurrentFieldIndex), "FmSearchEngine::FormatField : Parameter nWhich ist ungueltig");
            // ich bin im single-field-modus, da ist auch die richtige Feld-Nummer erlaubt, obwohl dann der richtige ::com::sun::star::sdbcx::Index
            // fuer meinen Array-Zugriff natuerlich 0 ist
            nWhich = 0;
        }

        DBG_ASSERT((nWhich>=0) && (nWhich < (m_arrUsedFields.end() - m_arrUsedFields.begin())),
            "FmSearchEngine::FormatField : Parameter nWhich ist ungueltig");
        return FormatField(m_arrUsedFields[nWhich]);
    }
}

//------------------------------------------------------------------------
INLINE_METHOD FmSearchEngine::SEARCH_RESULT FmSearchEngine::SearchSpecial(sal_Bool _bSearchForNull, sal_Int32& nFieldPos,
    FieldCollectionIterator& iterFieldLoop, const FieldCollectionIterator& iterBegin, const FieldCollectionIterator& iterEnd)
{
    // die Startposition merken
    ::com::sun::star::uno::Any aStartMark = m_xSearchCursor.getBookmark();
    FieldCollectionIterator iterInitialField = iterFieldLoop;

    // --------------------------------------------------------------
    sal_Bool bFound(sal_False);
    sal_Bool bMovedAround(sal_False);
    do
    {
        if (m_eMode == FmSearchDialog::SM_ALLOWSCHEDULE)
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
        bFound = _bSearchForNull == iterFieldLoop->xContents->wasNull();
        if (bFound)
            break;

        // naechstes Feld (implizit naechster Datensatz, wenn noetig)
        if (!MoveField(nFieldPos, iterFieldLoop, iterBegin, iterEnd))
        {   // beim Bewegen auf das naechste Feld ging was schief ... weitermachen ist nicht drin, da das naechste Mal genau
            // das selbe bestimmt wieder schief geht, also Abbruch
            // vorher aber noch, damit das Weitersuchen an der aktuellen Position weitermacht :
            m_aPreviousLocBookmark = m_xSearchCursor.getBookmark();
            m_iterPreviousLocField = iterFieldLoop;
            // und wech
            return SR_ERROR;
        }

        bMovedAround = COMPARE_BOOKMARKS(aStartMark, m_xSearchCursor.getBookmark()) && (iterFieldLoop == iterInitialField);

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

//------------------------------------------------------------------------
INLINE_METHOD FmSearchEngine::SEARCH_RESULT FmSearchEngine::SearchWildcard(const ::rtl::OUString& strExpression, sal_Int32& nFieldPos,
    FieldCollectionIterator& iterFieldLoop, const FieldCollectionIterator& iterBegin, const FieldCollectionIterator& iterEnd)
{
    // die Startposition merken
    ::com::sun::star::uno::Any aStartMark = m_xSearchCursor.getBookmark();
    FieldCollectionIterator iterInitialField = iterFieldLoop;

    WildCard aSearchExpression(strExpression);

    // --------------------------------------------------------------
    sal_Bool bFound(sal_False);
    sal_Bool bMovedAround(sal_False);
    do
    {
        if (m_eMode == FmSearchDialog::SM_ALLOWSCHEDULE)
        {
            Application::Reschedule();
            Application::Reschedule();
            // do 2 reschedules because of #70226# : some things done within this loop's body may cause an user event
            // to be posted (deep within vcl), and these user events will be handled before any keyinput or paintings
            // or anything like that. So within each loop we create one user event and hanel one user event (and no
            // paintings and these), so the office seems to be frozen while searching.
            // FS - 70226 - 02.12.99
        }

        // der aktuell zu vergleichende Inhalt
        ::rtl::OUString sCurrentCheck;
        if (m_bFormatter)
            sCurrentCheck = FormatField(nFieldPos);
        else
            sCurrentCheck = iterFieldLoop->xContents->getString();

        if (!m_bCase)
        {// normieren, wenn kein Gross/Klein
            String sLanguage, sCountry;
            ConvertLanguageToIsoNames(Application::GetAppInternational().GetLanguage(), sLanguage, sCountry);
            sCurrentCheck.toLowerCase(::rtl::OLocale::registerLocale(sLanguage, sCountry));
        }

        // jetzt ist der Test einfach ...
        bFound = aSearchExpression.Matches(sCurrentCheck);

        if (bFound)
            break;

        // naechstes Feld (implizit naechster Datensatz, wenn noetig)
        if (!MoveField(nFieldPos, iterFieldLoop, iterBegin, iterEnd))
        {   // beim Bewegen auf das naechste Feld ging was schief ... weitermachen ist nicht drin, da das naechste Mal genau
            // das selbe bestimmt wieder schief geht, also Abbruch
            // vorher aber noch, damit das Weitersuchen an der aktuellen Position weitermacht :
            m_aPreviousLocBookmark = m_xSearchCursor.getBookmark();
            m_iterPreviousLocField = iterFieldLoop;
            // und wech
            return SR_ERROR;
        }

        bMovedAround = COMPARE_BOOKMARKS(aStartMark, m_xSearchCursor.getBookmark()) && (iterFieldLoop == iterInitialField);

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

//------------------------------------------------------------------------
INLINE_METHOD FmSearchEngine::SEARCH_RESULT FmSearchEngine::SearchRegularApprox(const ::rtl::OUString& strExpression, sal_Int32& nFieldPos,
    FieldCollectionIterator& iterFieldLoop, const FieldCollectionIterator& iterBegin, const FieldCollectionIterator& iterEnd)
{
    DBG_ASSERT(m_bLevenshtein || m_bRegular,
        "FmSearchEngine::SearchRegularApprox : ungueltiger Suchmodus !");
    DBG_ASSERT(!m_bLevenshtein || !m_bRegular,
        "FmSearchEngine::SearchRegularApprox : kann nicht nach regulaeren Ausdruecken und nach Aehnlichkeiten gleichzeitig suchen !");

    // Startposition merken
    ::com::sun::star::uno::Any aStartMark = m_xSearchCursor.getBookmark();
    FieldCollectionIterator iterInitialField = iterFieldLoop;

    // Parameter sammeln
    SearchParam aParam(strExpression, m_bRegular ? SearchParam::SRCH_REGEXP : SearchParam::SRCH_LEVDIST, m_bCase, sal_False, sal_False);
    if (m_bLevenshtein)
    {
        aParam.SetSrchRelaxed(m_bLevRelaxed);
        aParam.SetLEVOther(m_nLevOther);
        aParam.SetLEVShorter(m_nLevShorter);
        aParam.SetLEVLonger(m_nLevLonger);
    }

    SearchText aLocalEngine(aParam, GetpApp()->GetAppInternational());

    // --------------------------------------------------------------
    sal_Bool bFound(sal_False);
    sal_Bool bMovedAround(sal_False);
    do
    {
        if (m_eMode == FmSearchDialog::SM_ALLOWSCHEDULE)
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
        ::rtl::OUString sCurrentCheck;
        if (m_bFormatter)
            sCurrentCheck = FormatField(nFieldPos);
        else
            sCurrentCheck = iterFieldLoop->xContents->getString();

        // (um Case brauche ich mir hier nicht zu kuemmern, das macht der SearchText, da ich ihm meinen Case-Parameter mitgegeben habe)

        xub_StrLen nStart = 0, nEnd = sCurrentCheck.getLength();
        bFound = aLocalEngine.SearchFrwrd(sCurrentCheck, &nStart, &nEnd);
            // das heisst hier 'forward' aber das bezieht sich nur auf die Suche innerhalb von sCurrentCheck, hat also mit
            // der Richtung meines Datensatz-Durchwanderns nix zu tun (darum kuemmert sich MoveField)

        // checken, ob die Position stimmt
        if (bFound)
        {
            switch (m_nPosition)
            {
                case MATCHING_WHOLETEXT :
                    if (nEnd != sCurrentCheck.getLength() - 1)
                    {
                        bFound = sal_False;
                        break;
                    }
                    // laeuft in den naechsten Case rein !
                case MATCHING_BEGINNING :
                    if (nStart != 0)
                        bFound = sal_False;
                    break;
                case MATCHING_END :
                    if (nEnd != sCurrentCheck.getLength() - 1)
                        bFound = sal_False;
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
            m_aPreviousLocBookmark = m_xSearchCursor.getBookmark();
            m_iterPreviousLocField = iterFieldLoop;
            // und wech
            return SR_ERROR;
        }

        bMovedAround = COMPARE_BOOKMARKS(aStartMark, m_xSearchCursor.getBookmark()) && (iterFieldLoop == iterInitialField);

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


DBG_NAME(FmSearchEngine);
//------------------------------------------------------------------------
FmSearchEngine::FmSearchEngine(const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XResultSet > & xCursor, const ::rtl::OUString& sVisibleFields,
    const ::com::sun::star::uno::Reference< ::com::sun::star::util::XNumberFormatsSupplier > & xFormatSupplier, FmSearchDialog::SEARCH_MODE eMode)
    :m_xSearchCursor(xCursor)
    ,m_xFormatSupplier(xFormatSupplier)
    ,m_bUsingTextComponents(sal_False)
    ,m_bCase(sal_False)
    ,m_bFormatter(sal_False)
    ,m_bForward(sal_False)
    ,m_nPosition(MATCHING_ANYWHERE)
    ,m_nCurrentFieldIndex(-2)   // -1 hat schon eine Bedeutung, also nehme ich -2 fuer 'ungueltig'
    ,m_eMode(eMode)
    ,m_bCancelAsynchRequest(sal_False)
    ,m_bSearchingCurrently(sal_False)
{
    DBG_CTOR(FmSearchEngine,NULL);

    m_xFormatter = ::com::sun::star::uno::Reference< ::com::sun::star::util::XNumberFormatter > (::comphelper::getProcessServiceFactory()
                    ->createInstance(FM_NUMBER_FORMATTER), ::com::sun::star::uno::UNO_QUERY);
    if (m_xFormatter.is())
        m_xFormatter->attachNumberFormatsSupplier(m_xFormatSupplier);

    Init(sVisibleFields);
}

//------------------------------------------------------------------------
FmSearchEngine::FmSearchEngine(const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XResultSet > & xCursor, const ::rtl::OUString& sVisibleFields,
    const InterfaceArray& arrFields, FmSearchDialog::SEARCH_MODE eMode)
    :m_xSearchCursor(xCursor)
    ,m_xOriginalIterator(xCursor)
    ,m_xClonedIterator(m_xOriginalIterator, sal_True)
    ,m_bUsingTextComponents(sal_True)
    ,m_bCase(sal_False)
    ,m_bFormatter(sal_True)     // das muss konsistent sein mit m_xSearchCursor, der i.A. == m_xOriginalIterator ist
    ,m_bForward(sal_False)
    ,m_nPosition(MATCHING_ANYWHERE)
    ,m_nCurrentFieldIndex(-2)
    ,m_eMode(eMode)
    ,m_bCancelAsynchRequest(sal_False)
    ,m_bSearchingCurrently(sal_False)
{
    DBG_CTOR(FmSearchEngine,NULL);

    fillControlTexts(arrFields);
    Init(sVisibleFields);
}

//------------------------------------------------------------------------
FmSearchEngine::~FmSearchEngine()
{
    clearControlTexts();

    DBG_DTOR(FmSearchEngine,NULL);
}

//------------------------------------------------------------------------
void FmSearchEngine::clearControlTexts()
{
    for (   ControlTextSuppliersIterator aIter = m_aControlTexts.begin();
            aIter < m_aControlTexts.end();
            ++aIter
        )
    {
        delete *aIter;
    }
    m_aControlTexts.clear();
}

//------------------------------------------------------------------------
void FmSearchEngine::fillControlTexts(const InterfaceArray& arrFields)
{
    clearControlTexts();
    ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >  xCurrent;
    for (int i=0; i<arrFields.size(); ++i)
    {
        xCurrent = arrFields.at(i);
        DBG_ASSERT(xCurrent.is(), "FmSearchEngine::fillControlTexts : invalid field interface !");
        // check which type of control this is
        ::com::sun::star::uno::Reference< ::com::sun::star::awt::XTextComponent >  xAsText(xCurrent, ::com::sun::star::uno::UNO_QUERY);
        if (xAsText.is())
        {
            m_aControlTexts.insert(m_aControlTexts.end(), new SimpleTextWrapper(xAsText));
            continue;
        }

        ::com::sun::star::uno::Reference< ::com::sun::star::awt::XListBox >  xAsListBox(xCurrent, ::com::sun::star::uno::UNO_QUERY);
        if (xAsListBox.is())
        {
            m_aControlTexts.insert(m_aControlTexts.end(), new ListBoxWrapper(xAsListBox));
            continue;
        }

        ::com::sun::star::uno::Reference< ::com::sun::star::awt::XCheckBox >  xAsCheckBox(xCurrent, ::com::sun::star::uno::UNO_QUERY);
        DBG_ASSERT(xAsCheckBox.is(), "FmSearchEngine::fillControlTexts : invalid field interface (no supported type) !");
            // we don't have any more options ...
        m_aControlTexts.insert(m_aControlTexts.end(), new CheckBoxWrapper(xAsCheckBox));
    }
}

//------------------------------------------------------------------------
void FmSearchEngine::Init(const ::rtl::OUString& sVisibleFields)
{
    // die Felder der Tabelle auseinanderdroeseln
    // ausserdem gleich das Mapping aufbauen: da die Liste der gueltigen Spalten durchaus kuerzer sein kann als die Liste der
    // Spalten, die der Iterator verwaltet, brauche ich ein solches Mapping : die gueltige Spalte Nummer x entspricht der vom
    // Iterator gelieferten Spalte y
    m_arrFieldMapping.Remove(0, m_arrFieldMapping.Count());

    // der Cursor kann mir einen Record (als PropertySet) liefern, dieser unterstuetzt den DatabaseRecord-Service
    ::com::sun::star::uno::Reference< ::com::sun::star::sdbcx::XColumnsSupplier >   xSupplyCols(m_xSearchCursor, ::com::sun::star::uno::UNO_QUERY);
    DBG_ASSERT(xSupplyCols.is(), "FmSearchEngine::Init : invalid cursor (no columns supplier) !");
    ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess >        xAllFieldNames = xSupplyCols->getColumns();
    ::com::sun::star::uno::Sequence< ::rtl::OUString >  seqFieldNames = xAllFieldNames->getElementNames();
    ::rtl::OUString*            pFieldNames = seqFieldNames.getArray();

    ::rtl::OUString sCurrentField;
    UniString sVis(sVisibleFields.getStr());
    for (xub_StrLen i=0; i<sVis.GetTokenCount(); ++i)
    {
        sCurrentField = sVis.GetToken(i);

        // in der Feld-Sammlung suchen
        sal_Int32 nFoundIndex = -1;
        for (sal_Int32 j=0; j<seqFieldNames.getLength(); ++j, ++pFieldNames)
        {
            if (pFieldNames->equals(sCurrentField))
            {
                nFoundIndex = j;
                break;
            }
        }
        DBG_ASSERT(nFoundIndex != -1, "FmSearchEngine::Init : Es wurden ungueltige Feldnamen angegeben !");
        m_arrFieldMapping.Insert(nFoundIndex, m_arrFieldMapping.Count());
    }
}

//------------------------------------------------------------------------
void FmSearchEngine::SetFormatterUsing(sal_Bool bSet)
{
    if (m_bFormatter == bSet)
        return;
    m_bFormatter = bSet;

    if (m_bUsingTextComponents)
    {
        // ich benutzte keinen Formatter, sondern TextComponents -> der SearchIterator muss angepasst werden
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

        // ich muss die Fields neu binden, da der Textaustausch eventuell ueber diese Fields erfolgt und sich der unterliegende Cursor
        // geaendert hat
        RebuildUsedFields(m_nCurrentFieldIndex, sal_True);
    }
    else
        InvalidatePreviousLoc();
}

//------------------------------------------------------------------------
void FmSearchEngine::PropagateProgress(sal_Bool _bDontPropagateOverflow)
{
    if (m_aProgressHandler.IsSet())
    {
        FmSearchProgress aProgress;
        aProgress.aSearchState = FmSearchProgress::STATE_PROGRESS;
        aProgress.nCurrentRecord = m_xSearchCursor.getRow() - 1;
        if (m_bForward)
            aProgress.bOverflow = !_bDontPropagateOverflow && m_xSearchCursor.isFirst();
        else
            aProgress.bOverflow = !_bDontPropagateOverflow && m_xSearchCursor.isLast();
        m_aProgressHandler.Call(&aProgress);
    }
}

//------------------------------------------------------------------------
void FmSearchEngine::SearchNextImpl()
{
    DBG_ASSERT(!(m_bWildcard && m_bRegular) && !(m_bRegular && m_bLevenshtein) && !(m_bLevenshtein && m_bWildcard),
        "FmSearchEngine::SearchNextImpl : Suchparameter schliessen sich gegenseitig aus !");

    DBG_ASSERT(m_xSearchCursor.is(), "FmSearchEngine::SearchNextImpl : habe ungueltigen Iterator !");

    // die Parameter der Suche
    ::rtl::OUString strSearchExpression(m_strSearchExpression); // brauche ich non-const
    if (!m_bCase)
    {// normieren, wenn kein Gross/Klein
        XubString sLanguage, sCountry;
        ConvertLanguageToIsoNames(Application::GetAppInternational().GetLanguage(), sLanguage, sCountry);
        strSearchExpression.toLowerCase(::rtl::OLocale::registerLocale(sLanguage, sCountry));
    }

    if (!m_bRegular && !m_bLevenshtein)
    {   // 'normale' Suche fuehre ich auf jeden Fall ueber WildCards durch, muss aber vorher je nach Modus den ::rtl::OUString anpassen

        if (!m_bWildcard)
        {   // da natuerlich in allen anderen Faellen auch * und ? im Suchstring erlaubt sind, aber nicht als WildCards zaehlen
            // sollen, muss ich normieren
            UniString aTmp(strSearchExpression.getStr());
            static const UniString s_sStar = UniString::CreateFromAscii("\\*");
            static const UniString s_sQuotation = UniString::CreateFromAscii("\\?");
            aTmp.SearchAndReplaceAll('*', s_sStar);
            aTmp.SearchAndReplaceAll('?', s_sQuotation);
            strSearchExpression = aTmp;

            switch (m_nPosition)
            {
                case MATCHING_ANYWHERE :
                    strSearchExpression = ::rtl::OUString::createFromAscii("*") + strSearchExpression
                    + ::rtl::OUString::createFromAscii("*");
                    break;
                case MATCHING_BEGINNING :
                    strSearchExpression = strSearchExpression + ::rtl::OUString::createFromAscii("*");
                    break;
                case MATCHING_END :
                    strSearchExpression = ::rtl::OUString::createFromAscii("*") + strSearchExpression;
                    break;
                case MATCHING_WHOLETEXT :
                    break;
                default :
                    DBG_ERROR("FmSearchEngine::SearchNextImpl() : die Methoden-Listbox duerfte nur 4 Eintraege enthalten ...");
            }
        }
    }

    // fuer Arbeit auf Feldliste
    FieldCollectionIterator iterBegin = m_arrUsedFields.begin();
    FieldCollectionIterator iterEnd = m_arrUsedFields.end();
    FieldCollectionIterator iterFieldCheck;

    sal_Int32 nFieldPos;

    if (HasPreviousLoc())
    {
        DBG_ASSERT(COMPARE_BOOKMARKS(m_aPreviousLocBookmark, m_xSearchCursor.getBookmark()),
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

    // ---------------------
    // die eigentliche Suche
#ifndef NOOLDSV
    if (m_eMode == FmSearchDialog::SM_BRUTE)
        Application::EnterWait();
#endif

    PropagateProgress(sal_True);
    SEARCH_RESULT srResult;
    if (m_eSearchForType != SEARCHFOR_STRING)
        srResult = SearchSpecial(m_eSearchForType == SEARCHFOR_NULL, nFieldPos, iterFieldCheck, iterBegin, iterEnd);
    else if (!m_bRegular && !m_bLevenshtein)
        srResult = SearchWildcard(strSearchExpression, nFieldPos, iterFieldCheck, iterBegin, iterEnd);
    else
        srResult = SearchRegularApprox(strSearchExpression, nFieldPos, iterFieldCheck, iterBegin, iterEnd);

#ifndef NOOLDSV
    if (m_eMode == FmSearchDialog::SM_BRUTE)
        Application::LeaveWait();
#endif

    // ---------------------
    m_srResult = srResult;

    if (SR_ERROR == m_srResult)
        return;

    // gefunden ?
    if (SR_FOUND == m_srResult)
    {
        // die Pos merken
        m_aPreviousLocBookmark = m_xSearchCursor.getBookmark();
        m_iterPreviousLocField = iterFieldCheck;
    }
    else
        // die "letzte Fundstelle" invalidieren
        InvalidatePreviousLoc();
}

//------------------------------------------------------------------------
IMPL_LINK(FmSearchEngine, OnSearchTerminated, FmSearchThread*, pThread)
{
    if (!m_aProgressHandler.IsSet())
        return 0L;

    FmSearchProgress aProgress;
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

    // per definitionem muss der Link Thread-sicher sein (das verlange ich einfach), so dass ich mich um so etwas hier nicht kuemmern muss
    m_aProgressHandler.Call(&aProgress);

    m_bSearchingCurrently = sal_False;
    return 0L;
}

//------------------------------------------------------------------------
IMPL_LINK(FmSearchEngine, OnNewRecordCount, void*, pCounterAsVoid)
{
    if (!m_aProgressHandler.IsSet())
        return 0L;

    FmSearchProgress aProgress;
    aProgress.nCurrentRecord = (sal_uInt32)pCounterAsVoid;
    aProgress.aSearchState = FmSearchProgress::STATE_PROGRESS_COUNTING;
    m_aProgressHandler.Call(&aProgress);

    return 0L;
}

//------------------------------------------------------------------------
sal_Bool FmSearchEngine::CancelRequested()
{
    m_aCancelAsynchAccess.acquire();
    sal_Bool bReturn = m_bCancelAsynchRequest;
    m_aCancelAsynchAccess.release();
    return bReturn;
}

//------------------------------------------------------------------------
void FmSearchEngine::CancelSearch()
{
    m_aCancelAsynchAccess.acquire();
    m_bCancelAsynchRequest = sal_True;
    m_aCancelAsynchAccess.release();
}

//------------------------------------------------------------------------
sal_Bool FmSearchEngine::SwitchToContext(const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XResultSet > & xCursor, const ::rtl::OUString& sVisibleFields, const InterfaceArray& arrFields,
    sal_Int32 nFieldIndex)
{
    DBG_ASSERT(!m_bSearchingCurrently, "FmSearchEngine::SwitchToContext : please do not call while I'm searching !");
    if (m_bSearchingCurrently)
        return sal_False;

    m_xSearchCursor = xCursor;
    m_xOriginalIterator = xCursor;
    m_xClonedIterator = CursorWrapper(m_xOriginalIterator, sal_True);
    m_bUsingTextComponents = sal_True;

    fillControlTexts(arrFields);

    Init(sVisibleFields);
    RebuildUsedFields(nFieldIndex, sal_True);

    return sal_True;
}

//------------------------------------------------------------------------
void FmSearchEngine::ImplStartNextSearch()
{
    m_bCancelAsynchRequest = sal_False;
    m_bSearchingCurrently = sal_True;

    if (m_eMode == FmSearchDialog::SM_USETHREAD)
    {
        FmSearchThread* pSearcher = new FmSearchThread(this);
            // der loescht sich nach Beendigung selber ...
        pSearcher->setTerminationHandler(LINK(this, FmSearchEngine, OnSearchTerminated));

        pSearcher->createSuspended();
        pSearcher->setPriority(::vos::OThread::TPriority_Lowest);
        pSearcher->resume();
    }
    else
    {
        SearchNextImpl();
        LINK(this, FmSearchEngine, OnSearchTerminated).Call(NULL);
    }
}

//------------------------------------------------------------------------
void FmSearchEngine::SearchNext(const ::rtl::OUString& strExpression)
{
    m_strSearchExpression = strExpression;
    m_eSearchForType = SEARCHFOR_STRING;
    ImplStartNextSearch();
}

//------------------------------------------------------------------------
void FmSearchEngine::SearchNextSpecial(sal_Bool _bSearchForNull)
{
    m_eSearchForType = _bSearchForNull ? SEARCHFOR_NULL : SEARCHFOR_NOTNULL;
    ImplStartNextSearch();
}

//------------------------------------------------------------------------
void FmSearchEngine::StartOver(const ::rtl::OUString& strExpression)
{
    if (m_bForward)
        m_xSearchCursor.first();
    else
        m_xSearchCursor.last();

    InvalidatePreviousLoc();
    SearchNext(strExpression);
}

//------------------------------------------------------------------------
void FmSearchEngine::StartOverSpecial(sal_Bool _bSearchForNull)
{
    if (m_bForward)
        m_xSearchCursor.first();
    else
        m_xSearchCursor.last();

    InvalidatePreviousLoc();
    SearchNextSpecial(_bSearchForNull);
}

//------------------------------------------------------------------------
void FmSearchEngine::InvalidatePreviousLoc()
{
    m_aPreviousLocBookmark.setValue(0,getVoidCppuType());
    m_iterPreviousLocField = m_arrUsedFields.end();
}

//------------------------------------------------------------------------
void FmSearchEngine::RebuildUsedFields(sal_Int32 nFieldIndex, sal_Bool bForce)
{
    if (!bForce && (nFieldIndex == m_nCurrentFieldIndex))
        return;
    // (da ich keinen Wechsel des Iterators von aussen zulasse, heisst selber ::com::sun::star::sdbcx::Index auch immer selbe Spalte, also habe ich nix zu tun)

    DBG_ASSERT((nFieldIndex >= -1) && (nFieldIndex<m_arrFieldMapping.Count()), "FmSearchEngine::RebuildUsedFields : nFieldIndex ist ungueltig !");
    // alle Felder, die ich durchsuchen muss, einsammeln
    m_arrUsedFields.clear();
    if (nFieldIndex == -1)
    {
        ::com::sun::star::uno::Reference< ::com::sun::star::container::XIndexAccess >  xFields;
        for (sal_uInt16 i=0; i<m_arrFieldMapping.Count(); ++i)
        {
            ::com::sun::star::uno::Reference< ::com::sun::star::sdbcx::XColumnsSupplier >  xSupplyCols(m_xSearchCursor, ::com::sun::star::uno::UNO_QUERY);
            DBG_ASSERT(xSupplyCols.is(), "FmSearchEngine::RebuildUsedFields : invalid cursor (no columns supplier) !");
            xFields = ::com::sun::star::uno::Reference< ::com::sun::star::container::XIndexAccess > (xSupplyCols->getColumns(), ::com::sun::star::uno::UNO_QUERY);
            BuildAndInsertFieldInfo(xFields, m_arrFieldMapping.GetObject(i));
        }
    }
    else
    {
        ::com::sun::star::uno::Reference< ::com::sun::star::container::XIndexAccess >  xFields;
        ::com::sun::star::uno::Reference< ::com::sun::star::sdbcx::XColumnsSupplier >  xSupplyCols(m_xSearchCursor, ::com::sun::star::uno::UNO_QUERY);
        DBG_ASSERT(xSupplyCols.is(), "FmSearchEngine::RebuildUsedFields : invalid cursor (no columns supplier) !");
        xFields = ::com::sun::star::uno::Reference< ::com::sun::star::container::XIndexAccess > (xSupplyCols->getColumns(), ::com::sun::star::uno::UNO_QUERY);
        BuildAndInsertFieldInfo(xFields, m_arrFieldMapping.GetObject(nFieldIndex));
    }

    m_nCurrentFieldIndex = nFieldIndex;
    // und natuerlich beginne ich die naechste Suche wieder jungfraeulich
    InvalidatePreviousLoc();
}

