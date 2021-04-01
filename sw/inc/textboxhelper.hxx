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
#include <com/sun/star/text/TextContentAnchorType.hpp>
#include <svx/swframetypes.hxx>

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
namespace com::sun::star::drawing
{
class XShape;
}
namespace com::sun::star::text
{
class XTextFrame;
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
    /// Create a TextBox for a shape. If the second parameter is true,
    /// the original text in the shape will be copied to the frame
    static void create(SwFrameFormat* pShape, bool bCopyText = false);
    /// Destroy a TextBox for a shape.
    static void destroy(SwFrameFormat* pShape);
    /// Get interface of a shape's TextBox, if there is any.
    static css::uno::Any queryInterface(const SwFrameFormat* pShape, const css::uno::Type& rType);

    /// Sync property of TextBox with the one of the shape.
    static void syncProperty(SwFrameFormat* pShape, sal_uInt16 nWID, sal_uInt8 nMemberID,
                             const css::uno::Any& rValue);
    /// Does the same, but works on properties which lack an sw-specific WID / MemberID.
    static void syncProperty(SwFrameFormat* pShape, std::u16string_view rPropertyName,
                             const css::uno::Any& rValue);
    /// Get a property of the underlying TextFrame.
    static void getProperty(SwFrameFormat const* pShape, sal_uInt16 nWID, sal_uInt8 nMemberID,
                            css::uno::Any& rValue);
    /// Get a property of the underlying TextFrame.
    static css::uno::Any getProperty(SwFrameFormat const* pShape, OUString sPropName);

    /// There are two types of enum of anchor type, so this function maps this.
    static css::text::TextContentAnchorType mapAnchorType(const RndStdIds& rAnchorID);

    /// Similar to syncProperty(), but used by the internal API (e.g. for UI purposes).
    static void syncFlyFrameAttr(SwFrameFormat& rShape, SfxItemSet const& rSet);

    /// Returns the SwFrameFormat* of the given UNO-XShape if exists otherwise nullptr.
    static SwFrameFormat* getShapeFormat(css::uno::Reference<css::drawing::XShape> xShape);

    /// Copy shape attributes to the text frame
    static void updateTextBoxMargin(SdrObject* pObj);

    /// Sets the surround to through for the textframe of the given shape,
    /// not to interfere with the layout. Returns true on success.
    static bool setWrapThrough(SwFrameFormat* pShape);

    /// Sets the surround to through for the textframe of the given shape,
    /// not to interfere with the layout. Returns true on success.
    static bool setWrapThrough(css::uno::Reference<css::drawing::XShape> xShape);

    /// Sets the anchor of the associated textframe of the given shape, and
    /// returns true on success.
    static bool changeAnchor(SwFrameFormat* pShape);

    /// Sets the anchor of the associated textframe of the given shape, and
    /// returns true on success.
    static bool changeAnchor(css::uno::Reference<css::drawing::XShape> xShape);

    /// Does the positioning for the associated textframe of the shape, and
    /// returns true on success.
    static bool doTextBoxPositioning(SwFrameFormat* pShape);

    /// Does the positioning for the associated textframe of the shape, and
    /// returns true on success.
    static bool doTextBoxPositioning(css::uno::Reference<css::drawing::XShape> xShape);

    /// Returns true if the anchor different for the  given shape, and the
    /// associated textframe of the given shape.
    /// Note: In case of AS_CHAR anchor the anchor type must be different,
    /// because if not, layout breaks, but this situation also handled by
    /// this function, and returns true in that case too.
    static std::optional<bool> isAnchorTypeDifferent(SwFrameFormat* pShape);

    /// Returns true if the given shape has a valid textframe.
    static bool isTextBoxShapeHasValidTextFrame(SwFrameFormat* pShape);
    /// Returns true if the given shape has a valid textframe.
    static bool isTextBoxShapeHasValidTextFrame(css::uno::Reference<css::drawing::XShape> xShape);

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
    /// If we have an associated TextFrame, then return its XTextFrame.
    static css::uno::Reference<css::text::XTextFrame>
    getUnoTextFrame(css::uno::Reference<css::drawing::XShape> const& xShape);
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

    /// Returns true if the SdrObject has a SwTextFrame otherwise false
    static bool hasTextFrame(const SdrObject* pObj);

    /// Count number of shapes in the document, excluding TextBoxes.
    static sal_Int32 getCount(const SwDoc& rDoc);
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
    /// Undo the effect of saveLinks() + individual resetLink() calls.
    static void restoreLinks(std::set<ZSortFly>& rOld, std::vector<SwFrameFormat*>& rNew,
                             SavedLink& rSavedLinks);
};

#endif // INCLUDED_SW_INC_TEXTBOXHELPER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
