/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_SVL_INDEXEDSTYLESHEETS_HXX
#define INCLUDED_SVL_INDEXEDSTYLESHEETS_HXX

#include <sal/types.h>

#include <rsc/rscsfx.hxx>
#include <rtl/ustring.hxx>
#include <rtl/ref.hxx>

#include <svl/svldllapi.h>

#include <unordered_map>
#include <vector>

class SfxStyleSheetBase;

namespace svl {

/** Function object to check whether a style sheet a fulfills specific criteria.
 * Derive from this class and override the Check() method.
 */
struct StyleSheetPredicate {
    virtual bool Check(const SfxStyleSheetBase& styleSheet) = 0;
    virtual ~StyleSheetPredicate() {;}
};

/** Function object for cleanup-Strategy for IndexedSfxStyleSheets::Clear().
 * Derive from it and do what is necessary to dispose of a style sheet in Dispose().
 */
struct StyleSheetDisposer {
    virtual void Dispose(rtl::Reference<SfxStyleSheetBase> styleSheet) = 0;
    virtual ~StyleSheetDisposer() {;}
};

/** Function object to apply a method on all style sheets.
 * Derive from it and do whatever you want to with the style sheet in the DoIt() method.
 */
struct StyleSheetCallback {
    virtual void DoIt(const SfxStyleSheetBase& styleSheet) = 0;
    virtual ~StyleSheetCallback() {;}
};

/** This class holds SfxStyleSheets and allows for access via an id and a name.
 *
 * @warning
 * The identification of style sheets happens by their name. If the name of a sheet changes,
 * it will not be found again! Please call Reindex() after changing a style sheet's name.
 *
 * @internal
 * This class was implemented to mitigate solve #fdo 30770.
 * The issue describes an Excel file which takes several hours to open.
 * An analysis revealed that the time is spent searching for style sheets with linear scans in an array.
 * This class implements access to the style sheets via their name in (usually) constant time.
 *
 * The return type for most methods is a vector of unsigned integers which denote the position
 * of the style sheets in the vector, and not of pointers to style sheets.
 * You will need a non-const StyleSheetPool to obtain the actual style sheets.
 *
 *
 * Index-based access is required in several code portions. Hence we have to store the style sheets
 * in a vector as well as in a map.
 */
class SVL_DLLPUBLIC IndexedStyleSheets final
{
public:
    IndexedStyleSheets();

    /** Destructor.
     *
     * @internal
     * Is explicit because it has to know how to dispose of SfxStyleSheetBase objects.
     */
    ~IndexedStyleSheets();

    /** Adds a style sheet.
     *
     * If the style sheet is already contained, this call has no effect.
     */
    void
    AddStyleSheet(rtl::Reference< SfxStyleSheetBase > style);

    /** Removes a style sheet. */
    bool
    RemoveStyleSheet(rtl::Reference< SfxStyleSheetBase > style);

    /** Check whether a specified style sheet is stored. */
    bool
    HasStyleSheet(rtl::Reference< SfxStyleSheetBase > style) const;

    /** Obtain the number of style sheets which are held */
    unsigned
    GetNumberOfStyleSheets() const;

    /** Obtain the number of style sheets for which a certain condition holds */
    unsigned
    GetNumberOfStyleSheetsWithPredicate(StyleSheetPredicate& predicate) const;

    /** Return the stylesheet by its position.
     * You can obtain the position by, e.g., FindStyleSheetPosition()
     * @internal
     * Method is not const because the returned style sheet is not const
     */
    rtl::Reference< SfxStyleSheetBase >
    GetStyleSheetByPosition(unsigned pos);

    /** Find the position of a provided style.
     *
     * @throws std::runtime_error if the style has not been found.
     */
    unsigned
    FindStyleSheetPosition(const SfxStyleSheetBase& style) const;

    /** Obtain the positions of all styles which have a given name
     */
    std::vector<unsigned>
    FindPositionsByName(const rtl::OUString& name) const;

    enum SearchBehavior { RETURN_ALL, RETURN_FIRST };
    /** Obtain the positions of all styles which have a certain name and fulfill a certain condition.
     *
     * This method is fast because it can use the name-based index
     */
    std::vector<unsigned>
    FindPositionsByNameAndPredicate(const rtl::OUString& name, StyleSheetPredicate& predicate,
            SearchBehavior behavior = RETURN_ALL) const;

    /** Obtain the positions of all styles which fulfill a certain condition.
     *
     * This method is slow because it cannot use the name-based index
     */
    std::vector<unsigned>
    FindPositionsByPredicate(StyleSheetPredicate& predicate) const;

    /** Execute a callback on all style sheets */
    void
    ApplyToAllStyleSheets(StyleSheetCallback& callback) const;

    /** Clear the contents of the index.
     * The StyleSheetDisposer::Dispose() method is called on each style sheet, e.g., if you want to broadcast
     * changes.
     */
    void
    Clear(StyleSheetDisposer& cleanup);

    void
    Reindex();

    /** Warning: counting for n starts at 0, i.e., the 0th style sheet is the first that is found. */
    rtl::Reference<SfxStyleSheetBase>
    GetNthStyleSheetThatMatchesPredicate(unsigned n, StyleSheetPredicate& predicate,
            unsigned startAt = 0);

    /** Get the positions of the style sheets which belong to a certain family.
     */
    const std::vector<unsigned>&
    GetStyleSheetPositionsByFamily(SfxStyleFamily) const;

private:
    /** Register the position of a styleName in the index */
    void
    Register(const SfxStyleSheetBase& style, unsigned pos);

    typedef std::vector<rtl::Reference<SfxStyleSheetBase> > VectorType;
    /** Vector with the stylesheets to allow for index-based access.
     */
    VectorType mStyleSheets;

    /** The map type that is used to store the mapping from strings to ids in mStyleSheets
     *
     * @internal
     * Must be an unordered map. A regular map is too slow for some files. */
    typedef std::unordered_multimap<rtl::OUString, unsigned, rtl::OUStringHash> MapType;

    /** A map which stores the positions of style sheets by their name */
    MapType mPositionsByName;

    std::vector<std::vector<unsigned> > mStyleSheetPositionsByFamily;
};

} /* namespace svl */

#endif // INCLUDED_SVL_INDEXEDSTYLESHEETS_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
