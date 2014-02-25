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

#ifndef INCLUDED_SVX_FMSRCIMP_HXX
#define INCLUDED_SVX_FMSRCIMP_HXX

#include <svx/fmtools.hxx>
#include <svx/svxdllapi.h>

#include <com/sun/star/awt/XCheckBox.hpp>
#include <com/sun/star/awt/XListBox.hpp>
#include <com/sun/star/awt/XTextComponent.hpp>
#include <com/sun/star/util/XNumberFormatsSupplier.hpp>
#include <com/sun/star/util/XNumberFormatter.hpp>

#include <cppuhelper/implbase1.hxx>
#include <osl/mutex.hxx>
#include <unotools/charclass.hxx>
#include <unotools/collatorwrapper.hxx>
#include <osl/thread.hxx>

#include <deque>
#include <vector>

/**
 * class FmSearchThread
 */
class FmSearchEngine;
class FmSearchThread : public ::osl::Thread
{
    FmSearchEngine*     m_pEngine;
    Link                m_aTerminationHdl;

    virtual void SAL_CALL run();
    virtual void SAL_CALL onTerminated();

public:
    FmSearchThread(FmSearchEngine* pEngine) : m_pEngine(pEngine) { }
    void setTerminationHandler(Link aHdl) { m_aTerminationHdl = aHdl; }
};

/**
 * struct FmSearchProgress - the owner of SearchEngine receives this structure for status updates
 * (at the end of the search)
 */
struct FmSearchProgress
{
    enum STATE { STATE_PROGRESS, STATE_PROGRESS_COUNTING, STATE_CANCELED, STATE_SUCCESSFULL, STATE_NOTHINGFOUND, STATE_ERROR };
    // (move to new record; progress during counting of records; cancelled; record found; nothing found;
    // any non-processable error)
    STATE   aSearchState;

    // current record - always valid (e.g. of interest for continuing search in case of cancellation)
    sal_uInt32  nCurrentRecord;
    // Overflow - only valid in case of STATE_PROGRESS
    sal_Bool    bOverflow;

    // the position of the search cursor - valid in case of STATE_SUCCESSFULL, STATE_CANCELED and STATE_NOTHING_FOUND
    ::com::sun::star::uno::Any  aBookmark;
    // the field, in which the text was found - valid in case of STATE_SUCCESSFULL
    sal_Int32   nFieldIndex;
};

/**
 * class FmRecordCountListener - utility class for FmSearchEngine, listens at a certain cursor and provides
 *                               the differences in RecordCount
 */
class FmRecordCountListener : public ::cppu::WeakImplHelper1< ::com::sun::star::beans::XPropertyChangeListener>
{
// attribute
    Link            m_lnkWhoWantsToKnow;
    ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >   m_xListening;

// attribute access
public:
    Link SetPropChangeHandler(const Link& lnk);

// methods
public:
    FmRecordCountListener(const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XResultSet >& dbcCursor);
    // the set has to support the sdb::ResultSet service
    virtual ~FmRecordCountListener();

    //  DECLARE_UNO3_AGG_DEFAULTS(FmPropertyListener, UsrObject);
    //  virtual sal_Bool queryInterface(::com::sun::star::uno::Uik aUik, ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >& rOut);

    // ::com::sun::star::lang::XEventListener
    virtual void SAL_CALL disposing(const ::com::sun::star::lang::EventObject& Source) throw(::com::sun::star::uno::RuntimeException, std::exception);

    // ::com::sun::star::beans::XPropertyChangeListener
    virtual void SAL_CALL propertyChange(const ::com::sun::star::beans::PropertyChangeEvent& evt) throw(::com::sun::star::uno::RuntimeException, std::exception);

    void DisConnect();

private:
    void NotifyCurrentCount();

};

/**
 * class FmSearchEngine - Impl class for FmSearchDialog
 */
namespace svxform {
    // We have three possible control types we may search in, determined by the supported interfaces : ::com::sun::star::awt::XTextComponent,
    // ::com::sun::star::awt::XListBox, ::com::sun::star::awt::XCheckBox.
    // While searching we don't want to do this distinction for every control in every round. So we need some helpers.
    class ControlTextWrapper
    {
        // attributes
        ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >   m_xControl;
        // attribute access
    public:
        ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >  getControl() const{ return m_xControl; }
    public:
        ControlTextWrapper(const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >& _xControl) { m_xControl = _xControl; }
        virtual ~ControlTextWrapper() { }

        virtual OUString getCurrentText() const = 0;
    };
    class SimpleTextWrapper : public ControlTextWrapper
    {
        ::com::sun::star::uno::Reference< ::com::sun::star::awt::XTextComponent >  m_xText;
    public:
        SimpleTextWrapper(const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XTextComponent >& _xText);
        virtual OUString getCurrentText() const;
    };
    class ListBoxWrapper : public ControlTextWrapper
    {
        ::com::sun::star::uno::Reference< ::com::sun::star::awt::XListBox >  m_xBox;
    public:
        ListBoxWrapper(const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XListBox >& _xBox);
        virtual OUString getCurrentText() const;
    };
    class CheckBoxWrapper : public ControlTextWrapper
    {
        ::com::sun::star::uno::Reference< ::com::sun::star::awt::XCheckBox >  m_xBox;
    public:
        CheckBoxWrapper(const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XCheckBox >& _xBox);
        virtual OUString getCurrentText() const;
    };
}

enum FMSEARCH_MODE { SM_BRUTE, SM_ALLOWSCHEDULE, SM_USETHREAD };

typedef std::vector< ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface> > InterfaceArray;

class SVX_DLLPUBLIC FmSearchEngine
{
    friend class FmSearchThread;

    enum SEARCH_RESULT { SR_FOUND, SR_NOTFOUND, SR_ERROR, SR_CANCELED };
    enum SEARCHFOR_TYPE { SEARCHFOR_STRING, SEARCHFOR_NULL, SEARCHFOR_NOTNULL };

    CursorWrapper                   m_xSearchCursor;
    std::deque<sal_Int32>           m_arrFieldMapping;
    // Since the iterator could have more columns, as managed here (in this field listbox),
    // a mapping of this ::com::sun::star::form keys on the indices of the respective columns is kept in the iterator

    // the formatter
    ::com::sun::star::uno::Reference< ::com::sun::star::util::XNumberFormatsSupplier >  m_xFormatSupplier;
    ::com::sun::star::uno::Reference< ::com::sun::star::util::XNumberFormatter >        m_xFormatter;

    CharClass               m_aCharacterClassficator;
    CollatorWrapper         m_aStringCompare;

    // the collection of all interesting fields (or their ::com::sun::star::data::XDatabaseVariant interfaces and FormatKeys)
    struct FieldInfo
    {
        ::com::sun::star::uno::Reference< ::com::sun::star::sdb::XColumn >          xContents;
        sal_uInt32              nFormatKey;
        sal_Bool                bDoubleHandling;
    };

    typedef std::vector<FieldInfo> FieldCollection;
    FieldCollection             m_arrUsedFields;
    sal_Int32                   m_nCurrentFieldIndex;   // the last parameter of RebuildUsedFields, it allows checks in FormatField

    typedef std::vector<svxform::ControlTextWrapper*> ControlTextSuppliers;
    ControlTextSuppliers    m_aControlTexts;

    sal_Bool                m_bUsingTextComponents;
    CursorWrapper           m_xOriginalIterator;
    CursorWrapper           m_xClonedIterator;

    // Data for the decision in which field a "Found" is accepted
    ::com::sun::star::uno::Any  m_aPreviousLocBookmark;     // position of the last finding
    FieldCollection::iterator     m_iterPreviousLocField;     // field of the last finding

    // Communication with the thread that does the actual searching
    OUString            m_strSearchExpression;              // forward direction
    SEARCHFOR_TYPE      m_eSearchForType;                   // ditto
    SEARCH_RESULT       m_srResult;                         // backward direction

    // The link we broadcast the progress and the result to
    Link                m_aProgressHandler;
    sal_Bool            m_bSearchingCurrently : 1;      // is an (asynchronous) search running?
    sal_Bool            m_bCancelAsynchRequest : 1;     // should be cancelled?
    ::osl::Mutex        m_aCancelAsynchAccess;          // access to_bCancelAsynchRequest (technically only
                                                        // relevant for m_eMode == SM_USETHREAD)
    FMSEARCH_MODE   m_eMode;                            // current mode

    // parameters for the search
    sal_Bool    m_bFormatter : 1;       // use field formatting
    sal_Bool    m_bForward : 1;         // direction
    sal_Bool    m_bWildcard : 1;        // wildcard search
    sal_Bool    m_bRegular : 1;         // regular expression
    sal_Bool    m_bLevenshtein : 1;     // Levenshtein search
    sal_Bool    m_bTransliteration : 1; // Levenshtein search

    sal_Bool    m_bLevRelaxed : 1;      // parameters for Levenshtein search
    sal_uInt16  m_nLevOther;
    sal_uInt16  m_nLevShorter;
    sal_uInt16  m_nLevLonger;

    sal_uInt16  m_nPosition;            // if not regular or levenshtein, then one of the MATCHING_... values

    sal_Int32   m_nTransliterationFlags;

// -------------
// member access
private:
    SVX_DLLPRIVATE sal_Bool CancelRequested();      // provides a through m_aCancelAsynchAccess backed interpretation of m_bCancelAsynchRequest

public:
    void        SetCaseSensitive(sal_Bool bSet);
    sal_Bool    GetCaseSensitive() const;

    void        SetFormatterUsing(sal_Bool bSet);   // this is somewhat more extensive, so no inline ... here
    sal_Bool    GetFormatterUsing() const           { return m_bFormatter; }

    void        SetDirection(sal_Bool bForward)     { m_bForward = bForward; }
    sal_Bool    GetDirection() const                { return m_bForward; }

    void        SetWildcard(sal_Bool bSet)          { m_bWildcard = bSet; }
    sal_Bool    GetWildcard() const                 { return m_bWildcard; }

    void        SetRegular(sal_Bool bSet)           { m_bRegular = bSet; }
    sal_Bool    GetRegular() const                  { return m_bRegular; }

    void        SetLevenshtein(sal_Bool bSet)       { m_bLevenshtein = bSet; }
    sal_Bool    GetLevenshtein() const              { return m_bLevenshtein; }

    void        SetIgnoreWidthCJK(sal_Bool bSet);
    sal_Bool    GetIgnoreWidthCJK() const;

    void        SetTransliteration(sal_Bool bSet)   { m_bTransliteration = bSet; }
    sal_Bool    GetTransliteration() const          { return m_bTransliteration; }

    void        SetLevRelaxed(sal_Bool bSet)        { m_bLevRelaxed = bSet; }
    sal_Bool    GetLevRelaxed() const               { return m_bLevRelaxed; }
    void        SetLevOther(sal_uInt16 nHowMuch)    { m_nLevOther = nHowMuch; }
    sal_uInt16  GetLevOther() const                 { return m_nLevOther; }
    void        SetLevShorter(sal_uInt16 nHowMuch)  { m_nLevShorter = nHowMuch; }
    sal_uInt16  GetLevShorter() const               { return m_nLevShorter; }
    void        SetLevLonger(sal_uInt16 nHowMuch)   { m_nLevLonger = nHowMuch; }
    sal_uInt16  GetLevLonger() const                { return m_nLevLonger; }
    // all Lev. values will only be considered in case of m_bLevenshtein==sal_True

    void        SetTransliterationFlags(sal_Int32 _nFlags)  { m_nTransliterationFlags = _nFlags; }
    sal_Int32   GetTransliterationFlags() const             { return m_nTransliterationFlags; }

    void    SetPosition(sal_uInt16 nValue)      { m_nPosition = nValue; }
    sal_uInt16  GetPosition() const             { return m_nPosition; }
    // position will be ignored in case of m_bWildCard==sal_True

    FMSEARCH_MODE GetSearchMode() const { return m_eMode; }

public:
    /** two constructs, both analogical to FmSearchDialog, therefore look this up for explanations ....
        xCursor has to implement ::com::sun::star::data::DatabaseCursor service  each time.
        If eMode == SM_USETHREAD, a ProgressHandler should be set, because in this case the result forwarding will be done
        by this handler.
        If eMode != SM_USETHREAD, SearchNext and StarOver won't return, until the search has finished (independently of its
        success), only then the result can be requested. If additionally the ProgressHandler is set, it will be called for
        every record as well as at the end of the search.
    */
    FmSearchEngine(
        const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >& _rxContext,
        const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XResultSet >& xCursor,
        const OUString& strVisibleFields,
        const ::com::sun::star::uno::Reference< ::com::sun::star::util::XNumberFormatsSupplier >& xFormat,
        FMSEARCH_MODE eMode);
    FmSearchEngine(
        const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >& _rxContext,
        const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XResultSet >& xCursor,
        const OUString& strVisibleFields,
        const InterfaceArray& arrFields,
        FMSEARCH_MODE eMode);

    virtual ~FmSearchEngine();

    /** the link will be called on every record and after the completion of the search, the parameter is a pointer to
        a FmSearchProgress structure
        the handler should be in any case thread-safe
    */
    void SetProgressHandler(Link aHdl) { m_aProgressHandler = aHdl; }

    /// search for the next appearance (for nDirection values check DIRECTION_*-defines)
    void SearchNext(const OUString& strExpression);
    /// analogous, search for "NULL" (_bSearchForNull==sal_True) or "not NULL"
    void SearchNextSpecial(sal_Bool _bSearchForNull);
    /// search for the next appearance, dependent on nDirection from the start or end
    void StartOver(const OUString& strExpression);
    /// analogous, search for "NULL" (_bSearchForNull==sal_True) or "not NULL"
    void StartOverSpecial(sal_Bool _bSearchForNull);
    /// invalidate previous search reference
    void InvalidatePreviousLoc();

    /** rebuilds m_arrUsedFields (nFieldIndex==-1 means all fields, otherwise it specifies the field index)
        if bForce is not set, nothing will happen in case of nFieldIndex == m_nCurrentFieldIndex
        (calls InvalidatePreviousLoc)
    */
    void RebuildUsedFields(sal_Int32 nFieldIndex, sal_Bool bForce = sal_False);
    OUString FormatField(sal_Int32 nWhich);

    /// returns directly; once it was really aborted, ProgressHandler is called with STATE_CANCELED
    void CancelSearch();

    /** only valid, if not an (asynchronous) search is running, the next search will then be executed
        on top of the new iterator with the new parameter
    */
    sal_Bool SwitchToContext(const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XResultSet >& xCursor, const OUString& strVisibleFields, const InterfaceArray& arrFields,
        sal_Int32 nFieldIndex);

protected:
    void Init(const OUString& strVisibleFields);

    void SearchNextImpl();
    // this Impl method is running in SearchThread

    // start a thread-search (or call SearchNextImpl directly, depending on the search mode)
    void ImplStartNextSearch();

private:
    SVX_DLLPRIVATE void clearControlTexts();
    SVX_DLLPRIVATE void fillControlTexts(const InterfaceArray& arrFields);

    // three methods implementing a complete search loop (null/not null, wildcard, SearchText)
    // (they all have some code in common, but with this solution we have do do a distinction only once per search (before
    // starting the loop), not in every loop step
    SVX_DLLPRIVATE SEARCH_RESULT SearchSpecial(sal_Bool _bSearchForNull, sal_Int32& nFieldPos, FieldCollection::iterator& iterFieldLoop,
        const FieldCollection::iterator& iterBegin, const FieldCollection::iterator& iterEnd);
    SVX_DLLPRIVATE SEARCH_RESULT SearchWildcard(const OUString& strExpression, sal_Int32& nFieldPos, FieldCollection::iterator& iterFieldLoop,
        const FieldCollection::iterator& iterBegin, const FieldCollection::iterator& iterEnd);
    SVX_DLLPRIVATE SEARCH_RESULT SearchRegularApprox(const OUString& strExpression, sal_Int32& nFieldPos, FieldCollection::iterator& iterFieldLoop,
        const FieldCollection::iterator& iterBegin, const FieldCollection::iterator& iterEnd);

    SVX_DLLPRIVATE void PropagateProgress(sal_Bool _bDontPropagateOverflow);
    // call the ProgressHandler with STATE_PROGRESS and the current position of the search iterator

    // helpers, that are needed several times
    SVX_DLLPRIVATE sal_Bool MoveCursor();
    // moves m_xSearchIterator with respect to direction/overflow cursor
    SVX_DLLPRIVATE sal_Bool MoveField(sal_Int32& nPos, FieldCollection::iterator& iter, const FieldCollection::iterator& iterBegin, const FieldCollection::iterator& iterEnd);
    // moves the iterator with respect to the direction/overflow iterator/overflow cursor
    SVX_DLLPRIVATE void BuildAndInsertFieldInfo(const ::com::sun::star::uno::Reference< ::com::sun::star::container::XIndexAccess >& xAllFields, sal_Int32 nField);
    // builds a FieldInfo in relation to field number nField (in xAllFields) and adds it to m_arrUsedFields
    // xAllFields needs to support the DatabaseRecord service
    SVX_DLLPRIVATE OUString FormatField(const FieldInfo& rField);
    // formats the field with the NumberFormatter

    SVX_DLLPRIVATE sal_Bool HasPreviousLoc() { return m_aPreviousLocBookmark.hasValue(); }

    DECL_LINK(OnSearchTerminated, FmSearchThread*);
    // is used by SearchThread, after the return from this handler the thread removes itself
    DECL_LINK(OnNewRecordCount, void*);
};

#endif // INCLUDED_SVX_FMSRCIMP_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
