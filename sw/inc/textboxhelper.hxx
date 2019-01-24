/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_SW_INC_TEXTBOXHELPER_HXX
#define INCLUDED_SW_INC_TEXTBOXHELPER_HXX

#include <map>
#include <set>
#include <vector>

#include <com/sun/star/uno/Any.h>
#include <com/sun/star/uno/Type.h>

#include "swdllapi.h"

class SdrPage;
class SdrObject;
class SfxItemSet;
class SwFrameFormat;
class SwFrameFormats;
class SwFormatContent;
class SwDoc;
namespace tools
{
class Rectangle;
}
class ZSortFly;
namespace com
{
namespace sun
{
namespace star
{
namespace drawing
{
class XShape;
}
}
}
}

/**
 * A TextBox is a TextFrame, that is tied to a drawinglayer shape.
 *
 * This class provides helper methods to create, query and maintain such
 * TextBoxes.
 */
class SW_DLLPUBLIC SwTextBoxHelper
{
public:
    /// Maps a draw format to a fly format.
    using SavedLink = std::map<const SwFrameFormat*, const SwFrameFormat*>;
    /// Maps a draw format to content.
    using SavedContent = std::map<const SwFrameFormat*, SwFormatContent>;
    /// Create a TextBox for a shape.
    static void create(SwFrameFormat* pShape);
    /// Destroy a TextBox for a shape.
    static void destroy(SwFrameFormat* pShape);
    /// Get interface of a shape's TextBox, if there is any.
    static css::uno::Any queryInterface(const SwFrameFormat* pShape, const css::uno::Type& rType);

    /// Sync property of TextBox with the one of the shape.
    static void syncProperty(SwFrameFormat* pShape, sal_uInt16 nWID, sal_uInt8 nMemberID,
                             const css::uno::Any& rValue);
    /// Does the same, but works on properties which lack an sw-specific WID / MemberID.
    static void syncProperty(SwFrameFormat* pShape, const OUString& rPropertyName,
                             const css::uno::Any& rValue);
    /// Get a property of the underlying TextFrame.
    static void getProperty(SwFrameFormat const* pShape, sal_uInt16 nWID, sal_uInt8 nMemberID,
                            css::uno::Any& rValue);

    /// Similar to syncProperty(), but used by the internal API (e.g. for UI purposes).
    static void syncFlyFrameAttr(SwFrameFormat& rShape, SfxItemSet const& rSet);

    /**
     * If we have an associated TextFrame, then return that.
     *
     * @param nType Expected frame format type.
     *              Valid types are RES_DRAWFRMFMT and RES_FLYFRMFMT.
     *
     * @see isTextBox
     */
    static SwFrameFormat* getOtherTextBoxFormat(const SwFrameFormat* pFormat, sal_uInt16 nType);
    /// If we have an associated TextFrame, then return that.
    static SwFrameFormat*
    getOtherTextBoxFormat(css::uno::Reference<css::drawing::XShape> const& xShape);
    /// Return the textbox rectangle of a draw shape (in twips).
    static tools::Rectangle getTextRectangle(SwFrameFormat* pShape, bool bAbsolute = true);

    /**
     * Is the frame format a text box?
     *
     * A text box consists of a coupled fly and draw format. Most times you
     * just want to check for a single type, otherwise you get duplicate results.
     *
     * @param nType Expected frame format input type.
     *              Valid types are RES_DRAWFRMFMT and RES_FLYFRMFMT.
     */
    static bool isTextBox(const SwFrameFormat* pFormat, sal_uInt16 nType);

    /// Count number of shapes in the document, excluding TextBoxes.
    static sal_Int32 getCount(const SwDoc* pDoc);
    /// Count number of shapes on the page, excluding TextBoxes.
    static sal_Int32 getCount(SdrPage const* pPage);
    /// Get a shape by index, excluding TextBoxes.
    ///
    /// @throws css::lang::IndexOutOfBoundsException
    static css::uno::Any getByIndex(SdrPage const* pPage, sal_Int32 nIndex);
    /// Get the order of the shape, excluding TextBoxes.
    static sal_Int32 getOrdNum(const SdrObject* pObject);
    /// If pTextBox is a textbox, then set rWrapThrough to the surround of its shape.
    static void getShapeWrapThrough(const SwFrameFormat* pTextBox, bool& rWrapThrough);

    /// Saves the current shape -> textbox links in a map, so they can be restored later.
    static void saveLinks(const SwFrameFormats& rFormats,
                          std::map<const SwFrameFormat*, const SwFrameFormat*>& rLinks);
    /// Reset the shape -> textbox link on the shape, and save it to the map, so it can be restored later.
    static void resetLink(SwFrameFormat* pShape,
                          std::map<const SwFrameFormat*, SwFormatContent>& rOldContent);
    /// Undo the effect of saveLinks() + individual resetLink() calls.
    static void restoreLinks(std::set<ZSortFly>& rOld, std::vector<SwFrameFormat*>& rNew,
                             SavedLink& rSavedLinks, SavedContent& rResetContent);
};

#endif // INCLUDED_SW_INC_TEXTBOXHELPER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
