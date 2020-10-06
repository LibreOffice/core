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

#ifndef INCLUDED_SD_INC_OUTLINER_HXX
#define INCLUDED_SD_INC_OUTLINER_HXX

#include <svx/svdoutl.hxx>
#include "pres.hxx"
#include "OutlinerIterator.hxx"
#include <editeng/SpellPortions.hxx>
#include <memory>

class SdrObject;
class SdrTextObj;
class SdDrawDocument;

namespace weld
{
class Window;
}

namespace sd
{
class View;
class ViewShell;
class Window;

/// Describes a single search hit: a set of rectangles on a given page.
struct SearchSelection
{
    /// 0-based index of the page that has the selection.
    int m_nPage;

    /**
     * List of selection rectangles in twips -- multiple rectangles only in
     * case the selection spans over more layout lines.
     */
    OString m_aRectangles;

    SearchSelection(int nPage, const OString& rRectangles)
        : m_nPage(nPage)
        , m_aRectangles(rRectangles)
    {
    }

    bool operator==(const SearchSelection& rOther) const
    {
        return m_nPage == rOther.m_nPage && m_aRectangles == rOther.m_aRectangles;
    }
};

} // end of namespace sd

/** The main purpose of this class is searching and replacing as well as
    spelling of impress documents.  The main part of both tasks lies in
    iterating over the pages and view modes of a document and apply the
    respective function to all objects containing text on those pages.

    <p>Relevant objects: There are two sets of objects to search/spell
    check.  One is the set of all selected objects.  The other consists of
    all objects on all pages in draw-, notes-, and handout view as well as
    slide- and background view (draw pages and master pages).</p>

    <p>Iteration: Search/replace and spelling functions operate on shapes
    containing text.  To cover all relevant objects an order has to be
    defined on the objects.  For the set of all selected objects this order
    is simply the order in which they can be retrieved from the selection
    object.<br>
    When there is no selection the order is nested.  The three modes of the
    draw view are on the outer level: draw mode, notes mode, handout mode.
    The inner level switches between draw pages and master pages.  This
    leads to the following order:
    <ol>
    <li>draw pages of draw mode</li>
    <li>master pages of draw mode</li>
    <li>draw pages of notes mode</li>
    <li>master pages of notes mode</li>
    <li>draw pages of handout mode</li>
    <li>master pages of handout mode</li>
    </ol>
    Iteration starts at the top of the current page.  When reaching the end
    of the document, i.e. the last master page of the handout mode, it jumps
    to the first draw page of draw mode.  In backward searches this order is
    reversed.  When doing a <em>replace all</em> then the whole document is
    searched for matches starting at the first page of the draw/slide view
    (or last page of handout/background view even though search
    direction).</p>

    <p>The start position is restored after finishing spell checking or
    replacing all matches in a document.</p>

    <p>Some related pieces of information:
    The search dialog (<type>SvxSearchDialog</type>) can be controlled in
    more than one way:
    <ul><li>A set of option flags returned by the slot call
    SID_SEARCH_OPTIONS handled by the
    <member>SdDrawDocument::GetState()</member> method.</li>
    <li>The contents of the search item of type
    <type>SvxSearchItem</type>.</li>
    <li>The <member>HasSelection()</member> view shell method that returns
    whether or not a selection exists.  However, it is called from the
    search dialog with an argument so that only text selections are
    queried.  This is only sufficient for searching the outline view.
    </p>
*/
class SdOutliner : public SdrOutliner
{
public:
    friend class ::sd::outliner::OutlinerContainer;

    /** Create a new sd outliner object.
        @param pDoc
            The draw document from which to take the content.
        @param nMode
            The valid values <const>OutlinerMode::DontKnow</const>,
            <const>OutlinerMode::TextObject</const>,
            <const>OutlinerMode::TitleObject</const>,
            <const>OutlinerMode::OutlineObject</const>, and
            <const>OutlinerMode::OutlineView</const> are defined in
            editeng/outliner.hxx.
    */
    SdOutliner(SdDrawDocument* pDoc, OutlinerMode nMode);
    virtual ~SdOutliner() override;
    /// Forbid copy construction and copy assignment
    SdOutliner(const Outliner&) = delete;
    SdOutliner& operator=(const Outliner&) = delete;

    /** Despite the name this method is called prior to spell checking *and*
        searching and replacing.  The position of current view
        mode/page/object/caret position is remembered and, depending on the
        search mode, may be restored after finishing searching/spell
        checking.
    */
    void PrepareSpelling();

    /** Initialize a spell check but do not start it yet.  This method
        is a better candidate for the name PrepareSpelling.
    */
    void StartSpelling();

    /** Initiate a find and/or replace on the next relevant text object.
        @return
            Returns </sal_True> when the search/replace is finished (as
            indicated by user input to the search dialog).  A </sal_False> value
            indicates that another call to this method is required.
    */
    bool StartSearchAndReplace(const SvxSearchItem* pSearchItem);

    /** Iterate over the sentences in all text shapes and stop at the
        next sentence with spelling errors. While doing so the view
        mode may be changed and text shapes are set into edit mode.
    */
    svx::SpellPortions GetNextSpellSentence();

    /** Release all resources that have been created during the find&replace
        or spell check.
    */
    void EndSpelling();

    /** callback for textconversion */
    bool ConvertNextDocument() override;

    /** Starts the text conversion (hangul/hanja or Chinese simplified/traditional)
    for the current viewshell */
    void StartConversion(LanguageType nSourceLanguage, LanguageType nTargetLanguage,
                         const vcl::Font* pTargetFont, sal_Int32 nOptions, bool bIsInteractive);

    /** This is called internally when text conversion is started.
        The position of current view mode/page/object/caret position
        is remembered and will be restored after conversion.
    */
    void BeginConversion();

    /** Release all resources that have been created during the conversion */
    void EndConversion();

    int GetIgnoreCurrentPageChangesLevel() const { return mnIgnoreCurrentPageChangesLevel; };
    void IncreIgnoreCurrentPageChangesLevel() { mnIgnoreCurrentPageChangesLevel++; };
    void DecreIgnoreCurrentPageChangesLevel() { mnIgnoreCurrentPageChangesLevel--; };
    SdDrawDocument* GetDoc() const { return mpDrawDocument; }

private:
    class Implementation;
    ::std::unique_ptr<Implementation> mpImpl;

    /// Specifies whether to search and replace, to spell check or to do a
    /// text conversion.
    enum mode
    {
        SEARCH,
        SPELL,
        TEXT_CONVERSION
    } meMode;

    /// The view which displays the searched objects.
    ::sd::View* mpView;
    /** The view shell containing the view.  It is held as weak
        pointer to avoid keeping it alive when the view is changed
        during searching.
    */
    std::weak_ptr<::sd::ViewShell> mpWeakViewShell;
    /// This window contains the view.
    VclPtr<::sd::Window> mpWindow;
    /// The document on whose objects and pages this class operates.
    SdDrawDocument* mpDrawDocument;

    /** this is the language that is used for current text conversion.
        Only valid if meMode is TEXT_CONVERSION.
    */
    LanguageType mnConversionLanguage;

    /** While the value of this flag is greater than 0 changes of the current page
        do not lead to selecting the corresponding text in the outliner.
    */
    int mnIgnoreCurrentPageChangesLevel;

    /// Specifies whether the search string has been found so far.
    bool mbStringFound;

    /** This flag indicates whether there may exist a match of the search
        string before/after the current position in the document.  It can be
        set to </sal_False> only when starting from the beginning/end of the
        document.  When reaching the end/beginning with it still be set to
        </sal_False> then there exists no match and the search can be terminated.
    */
    bool mbMatchMayExist;

    /// The number of pages in the current view.
    sal_uInt16 mnPageCount;

    /** A <TRUE/> value indicates that the end of the find&replace or spell
        check has been reached.
    */
    bool mbEndOfSearch;

    /** Set to <TRUE/> when an object has been prepared successfully for
        searching/spell checking.  This flag directs the internal iteration
        which stops when set to </sal_True>.
    */
    bool mbFoundObject;

    /** This flag indicates whether to search forward or backwards.
    */
    bool mbDirectionIsForward;

    /** This flag indicates that only the selected objects are to be
        searched.
    */
    bool mbRestrictSearchToSelection;

    /** When the search is restricted to the current selection then
        this list contains pointers to all the objects of the
        selection.  This copy is necessary because during the search
        process the mark list is modified.
    */
    ::std::vector<tools::WeakReference<SdrObject>> maMarkListCopy;

    /** Current object that may be a text object.  The object pointer to
        corresponds to <member>mnObjIndex</member>.  While iterating over the
        objects on a page <member>mpObj</member> will point to every object
        while <member>mpTextObj</member> will be set only to valid text
        objects.
    */
    SdrObject* mpObj;

    /** this stores the first object that is used for text conversion.
        Conversion automatically wraps around the document and stops when it
        finds this object again.
    */
    SdrObject* mpFirstObj;

    /// Candidate for being searched/spell checked.
    SdrTextObj* mpSearchSpellTextObj;

    /// Current text to be searched/spelled inside the current text object
    sal_Int32 mnText;

    /// Paragraph object of <member>mpTextObj</member>.
    OutlinerParaObject* mpParaObj;

    /// The view mode that was active when starting to search/spell check.
    PageKind meStartViewMode;

    /// The master page mode that was active when starting to search/spell check.
    EditMode meStartEditMode;

    /// The current page index on starting to search/spell check.
    sal_uInt16 mnStartPageIndex;

    /// The object in edit mode when searching /spell checking was started
    /// (if any).
    SdrObject* mpStartEditedObject;

    /// The position of the caret when searching /spell checking was started.
    ESelection maStartSelection;

    /** The search item contains various attributes that define the type of
        search.  It is set every time the
        <member>SearchAndReplaceAll</member> method is called.
    */
    const SvxSearchItem* mpSearchItem;

    /// The actual object iterator.
    ::sd::outliner::Iterator maObjectIterator;
    /// The current position of the object iterator.
    ::sd::outliner::IteratorPosition maCurrentPosition;
    /// The position when the search started.  Corresponds largely to the
    /// m?Start* members.
    ::sd::outliner::Iterator maSearchStartPosition;
    /** The last valid position describes where the last text object has been
        found.  This position is restored when some dialogs are shown.  The
        position is initially set to the where the search begins.
    */
    ::sd::outliner::IteratorPosition maLastValidPosition;

    /** When this flag is true then a PrepareSpelling() is executed when
        StartSearchAndReplace() is called the next time.
    */
    bool mbPrepareSpellingPending;

    /** Initialize the object iterator.  Call this method after being
        invoked from the search or spellcheck dialog.  It creates a new
        iterator pointing at the current object when this has not been done
        before.  It reverses the direction of iteration if the given flag
        differs from the current direction.
        @param bDirectionIsForward
            This flag specifies in which direction to iterator over the
            objects.  If it differs from the current direction the iterator
            is reversed.
    */
    void Initialize(bool bDirectionIsForward);

    /** Do search and replace for whole document.
    */
    bool SearchAndReplaceAll();

    /** Do search and replace for next match.
        @param pSelections
            When tiled rendering and not 0, then don't emit LOK events, instead
            assume the caller will do so.
        @return
            The return value specifies whether the search ended (</sal_True>) or
            another call to this method is required (</sal_False>).
    */
    bool SearchAndReplaceOnce(std::vector<::sd::SearchSelection>* pSelections = nullptr);

    void sendLOKSearchResultCallback(const std::shared_ptr<sd::ViewShell>& pViewShell,
                                     const OutlinerView* pOutlinerView,
                                     std::vector<sd::SearchSelection>* pSelections);

    /** Detect changes of the document or view and react accordingly.  Such
        changes may occur because different calls to
        <member>SearchAndReplace()</member> there usually is user
        interaction.  This is at least the press of the search or replace
        button but may include any other action some of which affect the
        search.
    */
    void DetectChange();

    /** Detect whether the selection has changed.
        @return
            Return <TRUE/> when the selection has been changed since the
            last call to this method.
    */
    bool DetectSelectionChange();

    /** Remember the current edited object/caret position/page/view mode
        when starting to search/spell check so that it can be restored on
        termination.
    */
    void RememberStartPosition();

    /** Restore the position stored in the last call of
        <member>RememberStartPositiony</member>.
    */
    void RestoreStartPosition();

    /** Provide next object to search or spell check as text object in edit
        mode on the current page.  This skips all objects that do not
        match or are no text object.
    */
    void ProvideNextTextObject();

    /** Handle the situation that the iterator has reached the last object.
        This may result in setting the <member>mbEndOfSearch</member> flag
        back to </sal_False>.  This method may show either the end-of-search
        dialog or the wrap-around dialog.
    */
    void EndOfSearch();

    /** Show a dialog that tells the user that the search has ended either
        because there are no more matches after finding at least one or that
        no match has been found at all.
    */
    void ShowEndOfSearchDialog();

    /** Show a dialog that asks the user whether to wrap around to the
        beginning/end of the document and continue with the search/spell
        check.
    */
    bool ShowWrapAroundDialog();

    /** Put text of current text object into outliner so that the text can
        be searched/spell checked.
    */
    void PutTextIntoOutliner();

    /** Prepare to do spell checking on the current text object.  This
        includes putting it into edit mode.  Under certain conditions this
        method sets <member>mbEndOfSearch</member> to <TRUE/>.
    */
    void PrepareSpellCheck();

    /** Prepare to search and replace on the current text object.  This
        includes putting it into edit mode.
    */
    void PrepareSearchAndReplace();

    /** Prepare to do a text conversion on the current text
        object. This includes putting it into edit mode.
    */
    void PrepareConversion();

    /** Switch to a new view mode.  Try to restore the original edit mode
        before doing so.
        @param ePageKind
            Specifies the new view mode.
    */
    void SetViewMode(PageKind ePageKind);

    /** Switch to the page or master page specified by the
        <member>mnPage</member> index.  Master page mode is specified by
        <member>meEditMode</member>.
        @param eEditMode
            The new edit mode.
        @param nPageIndex
            The new page index.
    */
    void SetPage(EditMode eEditMode, sal_uInt16 nPageIndex);

    /** Switch on edit mode for the currently selected text object.
    */
    void EnterEditMode(bool bGrabFocus);

    /** Return the position at which a new search is started with respect to
        the search direction as specified by the argument.
        @return
            The position mentioned above in form of a selection with start
            equals end.
    */
    ESelection GetSearchStartPosition() const;

    /** Detect whether there exists a previous match.  Note that only the
        absence of such a match can be detected reliably.  An existing match
        is assumed when the search started not at the beginning/end of the
        presentation.  This does not have to be true.  The user can have set
        the cursor at the middle of the text without a prior search.
        @return
            Returns </True> when there is no previous match and </False>
            when there may be one.
    */
    bool HasNoPreviousMatch();

    /** Handle a failed search (with or without replace) for the outline
        mode.  Show message boxes when the search failed completely,
        i.e. there is no match in the whole presentation, or when no further
        match exists.
        @return
            The returned value indicates whether another (wrapped around)
            search shall take place.  If that is so, then it is the caller's
            responsibility to set the cursor position accordingly.
    */
    bool HandleFailedSearch();

    /** Take a position as returned by an object iterator and switch to the
        view and page on which the object specified by this position is
        located.
        @param rPosition
            This position points to a <type>SdrObject</type> object and
            contains the view and page where it is located.
        @return
            Return a pointer to the <type>SdrObject</type>.
    */
    SdrObject* SetObject(const ::sd::outliner::IteratorPosition& rPosition);

    /** Use this method when the view shell in which to search has changed.
        It handles i.e. registering at the associated view as selection
        change listener.
    */
    void SetViewShell(const std::shared_ptr<::sd::ViewShell>& rpViewShell);

    /** Activate or deactivate the search in the current selection.  Call
        this method whenever the selection has changed.  This method creates
        a copy of the current selection and reassigns the object iterator to
        the current() iterator.
    */
    void HandleChangedSelection();

    /** Initiate the spell check of the next relevant text object.
        When the outline view is active then this method is called
        after a wrap around to continue at the beginning of the document.
        @return
            Returns <TRUE/> to indicate that another call to this method is
            required.  When all text objects have been processed then
            <FALSE/> is returned.
    */
    virtual bool SpellNextDocument() override;

    /** Find the right parent to use for a message. This function makes sure
        that the otherwise non-modal search or spell dialogs, if visible, are
        locked, too.
    */
    weld::Window* GetMessageBoxParent();
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
