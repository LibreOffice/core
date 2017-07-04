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

#include <cppuhelper/implbase.hxx>
#include <osl/mutex.hxx>
#include <unotools/charclass.hxx>
#include <unotools/collatorwrapper.hxx>
#include <osl/thread.hxx>

#include <deque>
#include <vector>

enum class TransliterationFlags;

/**
 * struct FmSearchProgress - the owner of SearchEngine receives this structure for status updates
 * (at the end of the search)
 */
struct FmSearchProgress
{
    enum class State { Progress, ProgressCounting, Canceled, Successful, NothingFound, Error };
    // (move to new record; progress during counting of records; cancelled; record found; nothing found;
    // any non-processable error)
    State       aSearchState;

    // current record - always valid (e.g. of interest for continuing search in case of cancellation)
    sal_uInt32  nCurrentRecord;
    // Overflow - only valid in case of STATE_PROGRESS
    bool        bOverflow;

    // the position of the search cursor - valid in case of STATE_SUCCESSFULL, STATE_CANCELED and STATE_NOTHING_FOUND
    css::uno::Any  aBookmark;
    // the field, in which the text was found - valid in case of STATE_SUCCESSFULL
    sal_Int32   nFieldIndex;
};

/**
 * class FmRecordCountListener - utility class for FmSearchEngine, listens at a certain cursor and provides
 *                               the differences in RecordCount
 */
class SAL_WARN_UNUSED FmRecordCountListener : public cppu::WeakImplHelper< css::beans::XPropertyChangeListener >
{
// attribute
    Link<sal_Int32,void>     m_lnkWhoWantsToKnow;
    css::uno::Reference< css::beans::XPropertySet >   m_xListening;

// attribute access
public:
    void SetPropChangeHandler(const Link<sal_Int32,void>& lnk);

// methods
public:
    FmRecordCountListener(const css::uno::Reference< css::sdbc::XResultSet >& dbcCursor);
    // the set has to support the sdb::ResultSet service
    virtual ~FmRecordCountListener() override;

    //  DECLARE_UNO3_AGG_DEFAULTS(FmPropertyListener, UsrObject)
    //  virtual sal_Bool queryInterface(css::uno::Uik aUik, css::uno::Reference< css::uno::XInterface >& rOut);

    // css::lang::XEventListener
    virtual void SAL_CALL disposing(const css::lang::EventObject& Source) override;

    // css::beans::XPropertyChangeListener
    virtual void SAL_CALL propertyChange(const css::beans::PropertyChangeEvent& evt) override;

    void DisConnect();

private:
    void NotifyCurrentCount();

};

/**
 * class FmSearchEngine - Impl class for FmSearchDialog
 */
namespace svxform {
    // We have three possible control types we may search in, determined by the supported interfaces : css::awt::XTextComponent,
    // css::awt::XListBox, css::awt::XCheckBox.
    // While searching we don't want to do this distinction for every control in every round. So we need some helpers.
    class SAL_WARN_UNUSED ControlTextWrapper
    {
        // attributes
        css::uno::Reference< css::uno::XInterface >   m_xControl;
        // attribute access
    public:
        const css::uno::Reference< css::uno::XInterface >&  getControl() const{ return m_xControl; }
    public:
        ControlTextWrapper(const css::uno::Reference< css::uno::XInterface >& _xControl) { m_xControl = _xControl; }
        virtual ~ControlTextWrapper() { }

        virtual OUString getCurrentText() const = 0;
    };

    class SAL_WARN_UNUSED SimpleTextWrapper : public ControlTextWrapper
    {
        css::uno::Reference< css::awt::XTextComponent >  m_xText;
    public:
        SimpleTextWrapper(const css::uno::Reference< css::awt::XTextComponent >& _xText);
        virtual OUString getCurrentText() const override;
    };

    class SAL_WARN_UNUSED ListBoxWrapper : public ControlTextWrapper
    {
        css::uno::Reference< css::awt::XListBox >  m_xBox;
    public:
        ListBoxWrapper(const css::uno::Reference< css::awt::XListBox >& _xBox);
        virtual OUString getCurrentText() const override;
    };

    class SAL_WARN_UNUSED CheckBoxWrapper : public ControlTextWrapper
    {
        css::uno::Reference< css::awt::XCheckBox >  m_xBox;
    public:
        CheckBoxWrapper(const css::uno::Reference< css::awt::XCheckBox >& _xBox);
        virtual OUString getCurrentText() const override;
    };
}

typedef std::vector< css::uno::Reference< css::uno::XInterface> > InterfaceArray;

class SAL_WARN_UNUSED SVX_DLLPUBLIC FmSearchEngine final
{
    friend class FmSearchThread;

    enum class SearchResult { Found, NotFound, Error, Cancelled };
    enum class SearchFor { String, Null, NotNull };

    CursorWrapper                   m_xSearchCursor;
    std::deque<sal_Int32>           m_arrFieldMapping;
    // Since the iterator could have more columns, as managed here (in this field listbox),
    // a mapping of this css::form keys on the indices of the respective columns is kept in the iterator

    CharClass               m_aCharacterClassficator;
    CollatorWrapper         m_aStringCompare;

    // the collection of all interesting fields (or their css::data::XDatabaseVariant interfaces and FormatKeys)
    struct FieldInfo
    {
        css::uno::Reference< css::sdb::XColumn >          xContents;
    };

    typedef std::vector<FieldInfo> FieldCollection;
    FieldCollection             m_arrUsedFields;
    sal_Int32                   m_nCurrentFieldIndex;   // the last parameter of RebuildUsedFields, it allows checks in FormatField

    typedef std::vector<svxform::ControlTextWrapper*> ControlTextSuppliers;
    ControlTextSuppliers    m_aControlTexts;

    CursorWrapper           m_xOriginalIterator;
    CursorWrapper           m_xClonedIterator;

    // Data for the decision in which field a "Found" is accepted
    css::uno::Any  m_aPreviousLocBookmark;     // position of the last finding
    FieldCollection::iterator   m_iterPreviousLocField;     // field of the last finding

    // Communication with the thread that does the actual searching
    OUString            m_strSearchExpression;              // forward direction
    SearchFor      m_eSearchForType;                   // ditto
    SearchResult       m_srResult;                         // backward direction

    // The link we broadcast the progress and the result to
    Link<const FmSearchProgress*,void>  m_aProgressHandler;
    bool                m_bSearchingCurrently : 1;      // is an (asynchronous) search running?
    bool                m_bCancelAsynchRequest : 1;     // should be cancelled?
    ::osl::Mutex        m_aCancelAsynchAccess;          // access to_bCancelAsynchRequest (technically only
                                                        // relevant for m_eMode == SM_USETHREAD)

    // parameters for the search
    bool        m_bFormatter : 1;       // use field formatting
    bool        m_bForward : 1;         // direction
    bool        m_bWildcard : 1;        // wildcard search
    bool        m_bRegular : 1;         // regular expression
    bool        m_bLevenshtein : 1;     // Levenshtein search
    bool        m_bTransliteration : 1; // Levenshtein search

    bool        m_bLevRelaxed : 1;      // parameters for Levenshtein search
    sal_uInt16  m_nLevOther;
    sal_uInt16  m_nLevShorter;
    sal_uInt16  m_nLevLonger;

    sal_uInt16  m_nPosition;            // if not regular or levenshtein, then one of the MATCHING_... values

    TransliterationFlags m_nTransliterationFlags;


// member access
private:
    SVX_DLLPRIVATE bool CancelRequested();      // provides a through m_aCancelAsynchAccess backed interpretation of m_bCancelAsynchRequest

public:
    void        SetCaseSensitive(bool bSet);
    bool        GetCaseSensitive() const;

    void        SetFormatterUsing(bool bSet);   // this is somewhat more extensive, so no inline ... here
    bool        GetFormatterUsing() const           { return m_bFormatter; }

    void        SetDirection(bool bForward)     { m_bForward = bForward; }
    bool        GetDirection() const                { return m_bForward; }

    void        SetWildcard(bool bSet)          { m_bWildcard = bSet; }
    bool        GetWildcard() const                 { return m_bWildcard; }

    void        SetRegular(bool bSet)           { m_bRegular = bSet; }
    bool        GetRegular() const                  { return m_bRegular; }

    void        SetLevenshtein(bool bSet)       { m_bLevenshtein = bSet; }
    bool        GetLevenshtein() const              { return m_bLevenshtein; }

    void        SetIgnoreWidthCJK(bool bSet);
    bool        GetIgnoreWidthCJK() const;

    void        SetTransliteration(bool bSet)   { m_bTransliteration = bSet; }
    bool        GetTransliteration() const          { return m_bTransliteration; }

    void        SetLevRelaxed(bool bSet)        { m_bLevRelaxed = bSet; }
    bool        GetLevRelaxed() const               { return m_bLevRelaxed; }
    void        SetLevOther(sal_uInt16 nHowMuch)    { m_nLevOther = nHowMuch; }
    sal_uInt16  GetLevOther() const                 { return m_nLevOther; }
    void        SetLevShorter(sal_uInt16 nHowMuch)  { m_nLevShorter = nHowMuch; }
    sal_uInt16  GetLevShorter() const               { return m_nLevShorter; }
    void        SetLevLonger(sal_uInt16 nHowMuch)   { m_nLevLonger = nHowMuch; }
    sal_uInt16  GetLevLonger() const                { return m_nLevLonger; }
    // all Lev. values will only be considered in case of m_bLevenshtein==sal_True

    void        SetTransliterationFlags(TransliterationFlags _nFlags)  { m_nTransliterationFlags = _nFlags; }
    TransliterationFlags
                GetTransliterationFlags() const             { return m_nTransliterationFlags; }

    void    SetPosition(sal_uInt16 nValue)      { m_nPosition = nValue; }
    sal_uInt16  GetPosition() const             { return m_nPosition; }
    // position will be ignored in case of m_bWildCard==sal_True

public:
    /** two constructs, both analogical to FmSearchDialog, therefore look this up for explanations ....
        xCursor has to implement css::data::DatabaseCursor service  each time.
        If eMode == SM_USETHREAD, a ProgressHandler should be set, because in this case the result forwarding will be done
        by this handler.
        If eMode != SM_USETHREAD, SearchNext and StarOver won't return, until the search has finished (independently of its
        success), only then the result can be requested. If additionally the ProgressHandler is set, it will be called for
        every record as well as at the end of the search.
    */
    FmSearchEngine(
        const css::uno::Reference< css::uno::XComponentContext >& _rxContext,
        const css::uno::Reference< css::sdbc::XResultSet >& xCursor,
        const OUString& strVisibleFields,
        const InterfaceArray& arrFields);

    ~FmSearchEngine();

    /** the link will be called on every record and after the completion of the search, the parameter is a pointer to
        a FmSearchProgress structure
        the handler should be in any case thread-safe
    */
    void SetProgressHandler(Link<const FmSearchProgress*,void> aHdl) { m_aProgressHandler = aHdl; }

    /// search for the next appearance (for nDirection values check DIRECTION_*-defines)
    void SearchNext(const OUString& strExpression);
    /// analogous, search for "NULL" (_bSearchForNull==sal_True) or "not NULL"
    void SearchNextSpecial(bool _bSearchForNull);
    /// search for the next appearance, dependent on nDirection from the start or end
    void StartOver(const OUString& strExpression);
    /// analogous, search for "NULL" (_bSearchForNull==sal_True) or "not NULL"
    void StartOverSpecial(bool _bSearchForNull);
    /// invalidate previous search reference
    void InvalidatePreviousLoc();

    /** rebuilds m_arrUsedFields (nFieldIndex==-1 means all fields, otherwise it specifies the field index)
        if bForce is not set, nothing will happen in case of nFieldIndex == m_nCurrentFieldIndex
        (calls InvalidatePreviousLoc)
    */
    void RebuildUsedFields(sal_Int32 nFieldIndex, bool bForce = false);
    OUString FormatField(sal_Int32 nWhich);

    /// returns directly; once it was really aborted, ProgressHandler is called with STATE_CANCELED
    void CancelSearch();

    /** only valid, if not an (asynchronous) search is running, the next search will then be executed
        on top of the new iterator with the new parameter
    */
    void SwitchToContext(const css::uno::Reference< css::sdbc::XResultSet >& xCursor, const OUString& strVisibleFields, const InterfaceArray& arrFields,
        sal_Int32 nFieldIndex);

private:
    void Init(const OUString& strVisibleFields);

    void SearchNextImpl();
    // this Impl method is running in SearchThread

    // start a thread-search (or call SearchNextImpl directly, depending on the search mode)
    void ImplStartNextSearch();

    SVX_DLLPRIVATE void clearControlTexts();
    SVX_DLLPRIVATE void fillControlTexts(const InterfaceArray& arrFields);

    // three methods implementing a complete search loop (null/not null, wildcard, SearchText)
    // (they all have some code in common, but with this solution we have to do a distinction only once per search (before
    // starting the loop), not in every loop step
    SVX_DLLPRIVATE SearchResult SearchSpecial(bool _bSearchForNull, sal_Int32& nFieldPos, FieldCollection::iterator& iterFieldLoop,
        const FieldCollection::iterator& iterBegin, const FieldCollection::iterator& iterEnd);
    SVX_DLLPRIVATE SearchResult SearchWildcard(const OUString& strExpression, sal_Int32& nFieldPos, FieldCollection::iterator& iterFieldLoop,
        const FieldCollection::iterator& iterBegin, const FieldCollection::iterator& iterEnd);
    SVX_DLLPRIVATE SearchResult SearchRegularApprox(const OUString& strExpression, sal_Int32& nFieldPos, FieldCollection::iterator& iterFieldLoop,
        const FieldCollection::iterator& iterBegin, const FieldCollection::iterator& iterEnd);

    SVX_DLLPRIVATE void PropagateProgress(bool _bDontPropagateOverflow);
    // call the ProgressHandler with STATE_PROGRESS and the current position of the search iterator

    // helpers, that are needed several times
    SVX_DLLPRIVATE bool MoveCursor();
    // moves m_xSearchIterator with respect to direction/overflow cursor
    SVX_DLLPRIVATE bool MoveField(sal_Int32& nPos, FieldCollection::iterator& iter, const FieldCollection::iterator& iterBegin, const FieldCollection::iterator& iterEnd);
    // moves the iterator with respect to the direction/overflow iterator/overflow cursor
    SVX_DLLPRIVATE void BuildAndInsertFieldInfo(const css::uno::Reference< css::container::XIndexAccess >& xAllFields, sal_Int32 nField);

    void OnSearchTerminated();
    // is used by SearchThread, after the return from this handler the thread removes itself
    DECL_LINK(OnNewRecordCount, sal_Int32, void);
};

#endif // INCLUDED_SVX_FMSRCIMP_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
