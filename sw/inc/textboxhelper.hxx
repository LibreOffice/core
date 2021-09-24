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
#include <unordered_map>
#include <optional>
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
    /// Helper function for syncing all texboxes of the group.
    static void doGroupTextBoxSync(SwFrameFormat* pShapeFormat, SdrObject* pMasterObj);
    /// Create a TextBox for a shape. If the third parameter is true,
    /// the original text in the shape will be copied to the frame
    /// The textbox is created for the shape given by the pObject parameter.
    static void create(SwFrameFormat* pShape, SdrObject* pObject, bool bCopyText = false);
    /// Destroy a TextBox for a shape. If the format has more textboxes
    /// like group shapes, it will destroy only that textbox what belongs
    /// to the given pObject shape.
    static void destroy(const SwFrameFormat* pShape, const SdrObject* pObject);
    /// Sets the given extising textframe for the shape to become a texbox.
    /// Used by the TEXT_BOX_CONTENT property setting.
    static void setTextBox(SwFrameFormat* pShape, SdrObject* pObject,
                           css::uno::Reference<css::text::XTextFrame> xNew);
    /// Get interface of a shape's TextBox, if there is any.
    static css::uno::Any queryInterface(const SwFrameFormat* pShape, const css::uno::Type& rType,
                                        SdrObject* pObj = nullptr);

    /// Sync property of TextBox with the one of the shape.
    static void syncProperty(SwFrameFormat* pShape, sal_uInt16 nWID, sal_uInt8 nMemberID,
                             const css::uno::Any& rValue, SdrObject* pObj = nullptr);
    /// Does the same, but works on properties which lack an sw-specific WID / MemberID.
    static void syncProperty(SwFrameFormat* pShape, std::u16string_view rPropertyName,
                             const css::uno::Any& rValue, SdrObject* pObj = nullptr);
    /// Get a property of the underlying TextFrame.
    static void getProperty(SwFrameFormat const* pShape, sal_uInt16 nWID, sal_uInt8 nMemberID,
                            css::uno::Any& rValue);
    /// Get a property of the underlying TextFrame.
    static css::uno::Any getProperty(SwFrameFormat const* pShape, const OUString& rPropName);

    /// There are two types of enum of anchor type, so this function maps this.
    static css::text::TextContentAnchorType mapAnchorType(const RndStdIds& rAnchorID);

    /// Similar to syncProperty(), but used by the internal API (e.g. for UI purposes).
    static void syncFlyFrameAttr(SwFrameFormat& rShape, SfxItemSet const& rSet, SdrObject* pObj);

    /// Copy shape attributes to the text frame
    static void updateTextBoxMargin(SdrObject* pObj);

    /// Sets the surround to through for the textframe of the given shape,
    /// not to interfere with the layout. Returns true on success.
    static bool setWrapThrough(SwFrameFormat* pShape, const SdrObject* pObj);

    /// Sets the anchor of the associated textframe of the given shape, and
    /// returns true on success.
    static bool changeAnchor(SwFrameFormat* pShape, SdrObject* pObj);

    /// Does the positioning for the associated textframe of the shape, and
    /// returns true on success.
    static bool doTextBoxPositioning(SwFrameFormat* pShape, SdrObject* pObj);

    /// Returns true if the anchor different for the  given shape, and the
    /// associated textframe of the given shape.
    /// Note: In case of AS_CHAR anchor the anchor type must be different,
    /// because if not, layout breaks, but this situation also handled by
    /// this function, and returns true in that case too.
    static std::optional<bool> isAnchorTypeDifferent(const SwFrameFormat* pShape,
                                                     const SdrObject* pObj);

    /// Returns true if the given shape has a valid textframe.
    static bool isTextBoxShapeHasValidTextFrame(const SwFrameFormat* pShape, const SdrObject* pObj);

    // Returns true on success. Synchronize z-order of the text frame of the given textbox
    // by setting it one level higher than the z-order of the shape of the textbox.
    static bool DoTextBoxZOrderCorrection(SwFrameFormat* pShape, const SdrObject* pObj);

    /**
     * If we have an associated TextFrame, then return that.
     *
     * If we have more textboxes for this format (group shape), that one will be
     * returned, what belongs to the pObject.
     *
     * @param nType Expected frame format type.
     *              Valid types are RES_DRAWFRMFMT and RES_FLYFRMFMT.
     *
     * @see isTextBox
     */
    static SwFrameFormat* getOtherTextBoxFormat(const SwFrameFormat* pFormat, sal_uInt16 nType,
                                                const SdrObject* pObject = nullptr);
    /// If we have an associated TextFrame, then return that.
    static SwFrameFormat*
    getOtherTextBoxFormat(css::uno::Reference<css::drawing::XShape> const& xShape);
    /// If we have an associated TextFrame, then return its XTextFrame.
    static css::uno::Reference<css::text::XTextFrame>
    getUnoTextFrame(css::uno::Reference<css::drawing::XShape> const& xShape);
    /// Return the textbox rectangle of a draw shape (in twips).
    static tools::Rectangle getTextRectangle(SdrObject* pShape, bool bAbsolute = true);

    /**
     * Is the frame format a text box?
     *
     * A text box consists of a coupled fly and draw format. Most times you
     * just want to check for a single type, otherwise you get duplicate results.
     *
     * @param pFormat: Is this format have a textbox?
     *
     * @param nType: Expected frame format input type.
     *              Valid types are RES_DRAWFRMFMT and RES_FLYFRMFMT.
     *
     * @param pObject: If the pFormat has more textboxes than one, like
     *                 groupshapes, the textbox what belongs to the given
     *                 pObject will be inspected. If this parameter nullptr,
     *                 the textbox what belongs to the pObject will only be inspected.
     */
    static bool isTextBox(const SwFrameFormat* pFormat, sal_uInt16 nType,
                          const SdrObject* pObject = nullptr);

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

/// Textboxes are basically textframe + shape pairs. This means one shape has one frame.
/// This is not enough for group shapes, because they have only one shape format and
/// can have many frame formats. This class provides if there is a group shape for example,
/// it can have multiple textboxes.
class SwTextBoxNode
{
    // This map stores the textboxes what belongs to this node
    std::unordered_map<const SdrObject*, SwFrameFormat*> m_pTextBoxTable;
    // This is the pointer to the shape format, which has this node
    // (and the textboxes)
    SwFrameFormat* m_pOwnerShapeFormat;

public:
    // Not needed.
    SwTextBoxNode() = delete;

    // ctor
    SwTextBoxNode(SwFrameFormat* pOwnerShapeFormat);
    // dtor
    ~SwTextBoxNode();

    // default copy ctor is enough
    SwTextBoxNode(SwTextBoxNode&) = default;

    // This method adds a textbox entry to the shape
    // Parameters:
    //     pDrawObject: The shape what the textbox be added to.
    //     pNewTextBox: The newly created textbox format what will be added to the shape.
    void AddTextBox(SdrObject* pDrawObject, SwFrameFormat* pNewTextBox);

    // This will remove the textbox entry.
    // Parameters:
    //     pDrawObject: The shape which have the textbox to be deleted.
    void DelTextBox(const SdrObject* pDrawObject);
    void DelTextBox(SwFrameFormat* pTextBox);

    // This will return with the frame format of the textbox what belongs
    // to the given shape (pDrawObject)
    SwFrameFormat* GetTextBox(const SdrObject* pDrawObject) const;

    // This returns with the shape what this class belongs to.
    SwFrameFormat* GetOwnerShape() { return m_pOwnerShapeFormat; };
    // This will give the current number of textboxes.
    size_t GetTextBoxCount() const { return m_pTextBoxTable.size(); };
};

#endif // INCLUDED_SW_INC_TEXTBOXHELPER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
