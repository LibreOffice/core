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

#ifndef INCLUDED_SW_SOURCE_CORE_INC_FRAME_HXX
#define INCLUDED_SW_SOURCE_CORE_INC_FRAME_HXX

#include <drawinglayer/primitive2d/baseprimitive2d.hxx>
#include <editeng/borderline.hxx>
#include <swtypes.hxx>
#include <swrect.hxx>
#include <calbck.hxx>
#include <svl/SfxBroadcaster.hxx>
#include <o3tl/typed_flags_set.hxx>
#include <com/sun/star/style/TabStop.hpp>
#include <basegfx/matrix/b2dhommatrix.hxx>
#include <vcl/outdev.hxx>

#include <memory>

namespace drawinglayer::processor2d { class BaseProcessor2D; }

class SwLayoutFrame;
class SwRootFrame;
class SwPageFrame;
class SwBodyFrame;
class SwFlyFrame;
class SwSectionFrame;
class SwFootnoteFrame;
class SwFootnoteBossFrame;
class SwTabFrame;
class SwRowFrame;
class SwContentFrame;
class SwAttrSet;
class Color;
class SwBorderAttrs;
class SwCache;
class SvxBrushItem;
class SvxFormatBreakItem;
class SwFormatPageDesc;
class SwSelectionList;
struct SwPosition;
struct SwCursorMoveState;
class SwFormat;
class SwPrintData;
class SwSortedObjs;
class SwAnchoredObject;
enum class SvxFrameDirection;
class IDocumentDrawModelAccess;

// Each FrameType is represented here as a bit.
// The bits must be set in a way that it can be determined with masking of
// which kind of FrameType an instance is _and_ from what classes it was derived.
// Each frame has in its base class a member that must be set by the
// constructors accordingly.
enum class SwFrameType
{
    None        = 0x0000,
    Root        = 0x0001,
    Page        = 0x0002,
    Column      = 0x0004,
    Header      = 0x0008,
    Footer      = 0x0010,
    FtnCont     = 0x0020,
    Ftn         = 0x0040,
    Body        = 0x0080,
    Fly         = 0x0100,
    Section     = 0x0200,
//  UNUSED      0x0400
    Tab         = 0x0800,
    Row         = 0x1000,
    Cell        = 0x2000,
    Txt         = 0x4000,
    NoTxt       = 0x8000,
};

namespace o3tl
{
    template<> struct typed_flags<SwFrameType> : is_typed_flags<SwFrameType, 0xfbff> {};
};

// for internal use some common combinations
#define FRM_LAYOUT      SwFrameType(0x3bFF)
#define FRM_ALL         SwFrameType(0xfbff)
#define FRM_CNTNT       (SwFrameType::Txt | SwFrameType::NoTxt)
#define FRM_FTNBOSS     (SwFrameType::Page | SwFrameType::Column)
#define FRM_ACCESSIBLE  (SwFrameType::Root | SwFrameType::Page | SwFrameType::Header | SwFrameType::Footer | SwFrameType::Ftn | SwFrameType::Fly | SwFrameType::Tab | SwFrameType::Cell | SwFrameType::Txt)
#define FRM_NEIGHBOUR   (SwFrameType::Column | SwFrameType::Cell)
#define FRM_NOTE_VERT   (SwFrameType::FtnCont | SwFrameType::Ftn | SwFrameType::Section | SwFrameType::Tab | SwFrameType::Row | SwFrameType::Cell | SwFrameType::Txt)
#define FRM_HEADFOOT    (SwFrameType::Header | SwFrameType::Footer)
#define FRM_BODYFTNC    (SwFrameType::FtnCont | SwFrameType::Body)

// for GetNextLeaf/GetPrevLeaf.
enum MakePageType
{
    MAKEPAGE_NONE,      // do not create page/footnote
    MAKEPAGE_APPEND,    // only append page if needed
    MAKEPAGE_INSERT,    // add or append page if needed
    MAKEPAGE_FTN,       // add footnote if needed
    MAKEPAGE_NOSECTION  // Don't create section frames
};

namespace drawinglayer { namespace attribute {
    class SdrAllFillAttributesHelper;
    typedef std::shared_ptr< SdrAllFillAttributesHelper > SdrAllFillAttributesHelperPtr;
}}

/// Helper class to isolate geometry-defining members of SwFrame
/// and to control their accesses. Moved to own class to have no
/// hidden accesses to 'private' members anymore.
///
/// Added most important flags about the state of this geometric
/// information and if it is valid or not
class SW_DLLPUBLIC SwFrameAreaDefinition
{
private:
    // The absolute position and size of the SwFrame in the document.
    // This values are set by the layouter implementations
    SwRect  maFrameArea;

    // The 'inner' Frame Area defined by a SwRect relative to FrameArea:
    // When identical to FrameArea, Pos() will be (0, 0) and Size identical.
    SwRect  maFramePrintArea;

    // bitfield
    bool mbFrameAreaPositionValid   : 1;
    bool mbFrameAreaSizeValid       : 1;
    bool mbFramePrintAreaValid      : 1;

    // #i65250#
    // frame ID is now in general available - used for layout loop control
    static sal_uInt32 mnLastFrameId;
    const  sal_uInt32 mnFrameId;

protected:
    // write access to mb*Valid flags
    void setFrameAreaPositionValid(bool bNew);
    void setFrameAreaSizeValid(bool bNew);
    void setFramePrintAreaValid(bool bNew);

public:
    SwFrameAreaDefinition();
    virtual ~SwFrameAreaDefinition();

    // read access to mb*Valid flags
    bool isFrameAreaPositionValid() const { return mbFrameAreaPositionValid; }
    bool isFrameAreaSizeValid() const { return mbFrameAreaSizeValid; }
    bool isFramePrintAreaValid() const { return mbFramePrintAreaValid; }

    // syntactic sugar: test whole FrameAreaDefinition
    bool isFrameAreaDefinitionValid() const { return isFrameAreaPositionValid() && isFrameAreaSizeValid() && isFramePrintAreaValid(); }

    // #i65250#
    sal_uInt32 GetFrameId() const { return mnFrameId; }

    // read accesses to FrameArea definitions - only const access allowed.
    // Do *not* const_cast results, it is necessary to track changes. use
    // the below offered WriteAccess helper classes instead
    const SwRect& getFrameArea() const { return maFrameArea; }
    const SwRect& getFramePrintArea() const { return maFramePrintArea; }

    // helper class(es) for FrameArea manipulation. These
    // have to be used to apply changes to FrameAreas. They hold a copy of the
    // SwRect for manipulation. It gets written back at destruction. Thus, this
    // mechanism depends on scope usage, take care. It prevents errors using
    // different instances of SwFrame in get/set methods which is more safe
    class FrameAreaWriteAccess : public SwRect
    {
    private:
        SwFrameAreaDefinition&        mrTarget;

        FrameAreaWriteAccess(const FrameAreaWriteAccess&) = delete;
        FrameAreaWriteAccess& operator=(const FrameAreaWriteAccess&) = delete;

    public:
        FrameAreaWriteAccess(SwFrameAreaDefinition& rTarget) : SwRect(rTarget.getFrameArea()), mrTarget(rTarget) {}
        ~FrameAreaWriteAccess();
        void setSwRect(const SwRect& rNew) { *reinterpret_cast< SwRect* >(this) = rNew; }
    };

    // same helper for FramePrintArea
    class FramePrintAreaWriteAccess : public SwRect
    {
    private:
        SwFrameAreaDefinition&        mrTarget;

        FramePrintAreaWriteAccess(const FramePrintAreaWriteAccess&) = delete;
        FramePrintAreaWriteAccess& operator=(const FramePrintAreaWriteAccess&) = delete;

    public:
        FramePrintAreaWriteAccess(SwFrameAreaDefinition& rTarget) : SwRect(rTarget.getFramePrintArea()), mrTarget(rTarget) {}
        ~FramePrintAreaWriteAccess();
        void setSwRect(const SwRect& rNew) { *reinterpret_cast< SwRect* >(this) = rNew; }
    };

    // RotateFlyFrame3 - Support for Transformations
    // Hand out the Transformations for the current FrameAreaDefinition
    // for the FrameArea and FramePrintArea.
    // FramePrintArea is not relative to FrameArea in this
    // transformation representation (to make it easier to use and understand).
    // There is no 'set' method since SwFrame is a layout object. For
    // some cases rotation will be included (used for SwGrfNode in inner
    // SwFrame of a SwFlyFrame)
    virtual basegfx::B2DHomMatrix getFrameAreaTransformation() const;
    virtual basegfx::B2DHomMatrix getFramePrintAreaTransformation() const;

    // RotateFlyFrame3 - Support for Transformations
    // Modify current transformations by applying given translation
    virtual void transform_translate(const Point& rOffset);
};

/// RotateFlyFrame3: Helper class when you want to make your SwFrame derivate
/// transformable. It provides some tooling to do so. To use, add as member
/// (see e.g. SwFlyFreeFrame which uses 'std::unique_ptr< TransformableSwFrame >')
class SW_DLLPUBLIC TransformableSwFrame
{
private:
    // The SwFrameAreaDefinition to work on
    SwFrameAreaDefinition&  mrSwFrameAreaDefinition;

    // FrameAreaTransformation and FramePrintAreaTransformation
    // !identity when needed (translate/scale is used (e.g. rotation))
    basegfx::B2DHomMatrix   maFrameAreaTransformation;
    basegfx::B2DHomMatrix   maFramePrintAreaTransformation;

public:
    TransformableSwFrame(SwFrameAreaDefinition& rSwFrameAreaDefinition)
    :   mrSwFrameAreaDefinition(rSwFrameAreaDefinition),
        maFrameAreaTransformation(),
        maFramePrintAreaTransformation()
    {
    }

    // get SwFrameArea in transformation form
    const basegfx::B2DHomMatrix& getLocalFrameAreaTransformation() const
    {
        return maFrameAreaTransformation;
    }

    // get SwFramePrintArea in transformation form
    const basegfx::B2DHomMatrix& getLocalFramePrintAreaTransformation() const
    {
        return maFramePrintAreaTransformation;
    }

    // Helpers to re-create the untransformed SwRect(s) originally
    // in the SwFrameAreaDefinition, based on the current Transformations.
    SwRect getUntransformedFrameArea() const;
    SwRect getUntransformedFramePrintArea() const;

    // Helper method to re-create FrameAreaTransformations based on the
    // current FrameAreaDefinition transformed by given rotation and Center
    void createFrameAreaTransformations(
        double fRotation,
        const basegfx::B2DPoint& rCenter);

    // Tooling method to reset the SwRect(s) in the current
    // SwFrameAreaDefinition which are already adapted to
    // Transformation back to the untransformed state, using
    // the getUntransformedFrame*Area calls above when needed.
    // Only the SwRect(s) are changed back, not the transformations.
    void restoreFrameAreas();

    // Re-Creates the SwRect(s) as BoundAreas based on the current
    // set Transformations.
    void adaptFrameAreasToTransformations();

    // Modify current definitions by applying the given transformation
    void transform(const basegfx::B2DHomMatrix& aTransform);
};

/**
 * Base class of the Writer layout elements.
 *
 * This includes not only fly frames, but everything down to the paragraph
 * level: pages, headers, footers, etc. (Inside a paragraph SwLinePortion
 * instances are used.)
 */
class SW_DLLPUBLIC SwFrame : public SwFrameAreaDefinition, public SwClient, public SfxBroadcaster
{
    // the hidden Frame
    friend class SwFlowFrame;
    friend class SwLayoutFrame;
    friend class SwLooping;
    friend class SwDeletionChecker; // for GetDep()

    // voids lower during creation of a column
    friend SwFrame *SaveContent( SwLayoutFrame *, SwFrame* pStart );
    friend void   RestoreContent( SwFrame *, SwLayoutFrame *, SwFrame *pSibling );

    // for validating a mistakenly invalidated one in SwContentFrame::MakeAll
    friend void ValidateSz( SwFrame *pFrame );
    // implemented in text/txtftn.cxx, prevents Footnote oscillation
    friend void ValidateText( SwFrame *pFrame );

    friend void MakeNxt( SwFrame *pFrame, SwFrame *pNxt );

    // cache for (border) attributes
    static SwCache *mpCache;

    SwRootFrame   *mpRoot;
    SwLayoutFrame *mpUpper;
    SwFrame       *mpNext;
    SwFrame       *mpPrev;

    // sw_redlinehide: hide these dangerous SwClient functions
    using SwClient::GetRegisteredInNonConst;
    using SwClient::GetRegisteredIn;

    SwFrame *FindNext_();
    SwFrame *FindPrev_();

    /** method to determine next content frame in the same environment
        for a flow frame (content frame, table frame, section frame)

        #i27138# - adding documentation:
        Travelling downwards through the layout to determine the next content
        frame in the same environment. There are several environments in a
        document, which form a closed context regarding this function. These
        environments are:
        - Each page header
        - Each page footer
        - Each unlinked fly frame
        - Each group of linked fly frames
        - All footnotes
        - All document body frames
        #i27138# - adding parameter <_bInSameFootnote>
        Its default value is <false>. If its value is <true>, the environment
        'All footnotes' is no longer treated. Instead each footnote is treated
        as an own environment.

        @param _bInSameFootnote
        input parameter - boolean indicating, that the found next content
        frame has to be in the same footnote frame. This parameter is only
        relevant for flow frames in footnotes.

        @return SwContentFrame*
        pointer to the found next content frame. It's NULL, if none exists.
    */
    SwContentFrame* FindNextCnt_( const bool _bInSameFootnote );

    /** method to determine previous content frame in the same environment
        for a flow frame (content frame, table frame, section frame)

        #i27138#
        Travelling upwards through the layout to determine the previous content
        frame in the same environment. There are several environments in a
        document, which form a closed context regarding this function. These
        environments are:
        - Each page header
        - Each page footer
        - Each unlinked fly frame
        - Each group of linked fly frames
        - All footnotes
        - All document body frames
        #i27138# - adding parameter <_bInSameFootnote>
        Its default value is <false>. If its value is <true>, the environment
        'All footnotes' is no longer treated. Instead each footnote is treated
        as an own environment.

        The found previous content frame has to be in the same footnote frame. This is only
        relevant for flow frames in footnotes.

        @return SwContentFrame*
        pointer to the found previous content frame. It's NULL, if none exists.
    */
    SwContentFrame* FindPrevCnt_();

    void UpdateAttrFrame( const SfxPoolItem*, const SfxPoolItem*, sal_uInt8 & );
    SwFrame* GetIndNext_();
    void SetDirFlags( bool bVert );

    const SwLayoutFrame* ImplGetNextLayoutLeaf( bool bFwd ) const;

    SwPageFrame* ImplFindPageFrame();

protected:
    std::unique_ptr<SwSortedObjs> m_pDrawObjs; // draw objects, can be null
    SwFrameType mnFrameType;  //Who am I?

    bool mbInDtor      : 1;
    bool mbInvalidR2L  : 1;
    bool mbDerivedR2L  : 1;
    bool mbRightToLeft : 1;
    bool mbInvalidVert : 1;
    bool mbDerivedVert : 1;
    bool mbVertical    : 1;

    bool mbVertLR      : 1;
    bool mbVertLRBT    : 1;

    bool mbValidLineNum  : 1;
    bool mbFixSize       : 1;

    // if true, frame will be painted completely even content was changed
    // only partially. For ContentFrames a border (from Action) will exclusively
    // painted if <mbCompletePaint> is true.
    bool mbCompletePaint : 1;

    bool mbRetouche      : 1; // frame is responsible for retouching

    bool mbInfInvalid    : 1;  // InfoFlags are invalid
    bool mbInfBody       : 1;  // Frame is in document body
    bool mbInfTab        : 1;  // Frame is in a table
    bool mbInfFly        : 1;  // Frame is in a Fly
    bool mbInfFootnote        : 1;  // Frame is in a footnote
    bool mbInfSct        : 1;  // Frame is in a section
    bool mbColLocked     : 1;  // lock Grow/Shrink for column-wise section
                                  // or fly frames, will be set in Format
    bool m_isInDestroy : 1;
    bool mbForbidDelete : 1;

    void ColLock()      { mbColLocked = true; }
    void ColUnlock()    { mbColLocked = false; }

    virtual void DestroyImpl();
    virtual ~SwFrame() override;

    // Only used by SwRootFrame Ctor to get 'this' into mpRoot...
    void setRootFrame( SwRootFrame* pRoot ) { mpRoot = pRoot; }

    SwPageFrame *InsertPage( SwPageFrame *pSibling, bool bFootnote );
    void PrepareMake(vcl::RenderContext* pRenderContext);
    void OptPrepareMake();
    virtual void MakePos();
    // Format next frame of table frame to assure keeping attributes.
    // In case of nested tables method <SwFrame::MakeAll()> is called to
    // avoid formatting of superior table frame.
    friend SwFrame* sw_FormatNextContentForKeep( SwTabFrame* pTabFrame );

    virtual void MakeAll(vcl::RenderContext* pRenderContext) = 0;
    // adjust frames of a page
    SwTwips AdjustNeighbourhood( SwTwips nDiff, bool bTst = false );

    // change only frame size not the size of PrtArea
    virtual SwTwips ShrinkFrame( SwTwips, bool bTst = false, bool bInfo = false ) = 0;
    virtual SwTwips GrowFrame  ( SwTwips, bool bTst = false, bool bInfo = false ) = 0;

    /// use these so we can grep for SwFrame's GetRegisteredIn accesses
    /// beware that SwTextFrame may return sw::WriterMultiListener
    SwModify        *GetDep()       { return GetRegisteredInNonConst(); }
    const SwModify  *GetDep() const { return GetRegisteredIn(); }

    SwFrame( SwModify*, SwFrame* );

    void CheckDir( SvxFrameDirection nDir, bool bVert, bool bOnlyBiDi, bool bBrowse );

    /** enumeration for the different invalidations
        #i28701#
    */
    enum InvalidationType
    {
        INVALID_SIZE, INVALID_PRTAREA, INVALID_POS, INVALID_LINENUM, INVALID_ALL
    };

    /** method to determine, if an invalidation is allowed.
        #i28701
    */
    virtual bool InvalidationAllowed( const InvalidationType _nInvalid ) const;

    /** method to perform additional actions on an invalidation

        #i28701#
        Method has *only* to contain actions, which has to be performed on
        *every* assignment of the corresponding flag to <false>.
    */
    virtual void ActionOnInvalidation( const InvalidationType _nInvalid );

    // draw shadow and borders
    void PaintShadow( const SwRect&, SwRect&, const SwBorderAttrs& ) const;
    virtual void Modify( const SfxPoolItem*, const SfxPoolItem* ) override;

    virtual const IDocumentDrawModelAccess& getIDocumentDrawModelAccess( );

public:
    virtual css::uno::Sequence< css::style::TabStop > GetTabStopInfo( SwTwips )
    {
        return css::uno::Sequence< css::style::TabStop >();
    }


    SwFrameType GetType() const { return mnFrameType; }

    static SwCache &GetCache()                { return *mpCache; }
    static SwCache *GetCachePtr()             { return mpCache;  }
    static void     SetCache( SwCache *pNew ) { mpCache = pNew;  }

    // change PrtArea size and FrameSize
    SwTwips Shrink( SwTwips, bool bTst = false, bool bInfo = false );
    SwTwips Grow  ( SwTwips, bool bTst = false, bool bInfo = false );

    // different methods for inserting in layout tree (for performance reasons)

    // insert before pBehind or at the end of the chain below mpUpper
    void InsertBefore( SwLayoutFrame* pParent, SwFrame* pBehind );
    // insert after pBefore or at the beginning of the chain below mpUpper
    void InsertBehind( SwLayoutFrame *pParent, SwFrame *pBefore );
    // insert before pBehind or at the end of the chain while considering
    // the siblings of pSct
    bool InsertGroupBefore( SwFrame* pParent, SwFrame* pWhere, SwFrame* pSct );
    void RemoveFromLayout();

    // For internal use only - who ignores this will be put in a sack and has
    // to stay there for two days
    // Does special treatment for Get_[Next|Prev]Leaf() (for tables).
    SwLayoutFrame *GetLeaf( MakePageType eMakePage, bool bFwd );
    SwLayoutFrame *GetNextLeaf   ( MakePageType eMakePage );
    SwLayoutFrame *GetNextFootnoteLeaf( MakePageType eMakePage );
    SwLayoutFrame *GetNextSctLeaf( MakePageType eMakePage );
    SwLayoutFrame *GetNextCellLeaf();
    SwLayoutFrame *GetPrevLeaf   ();
    SwLayoutFrame *GetPrevFootnoteLeaf( MakePageType eMakeFootnote );
    SwLayoutFrame *GetPrevSctLeaf();
    SwLayoutFrame *GetPrevCellLeaf();
    const SwLayoutFrame *GetLeaf ( MakePageType eMakePage, bool bFwd,
                                 const SwFrame *pAnch ) const;

    bool WrongPageDesc( SwPageFrame* pNew );

    //#i28701# - new methods to append/remove drawing objects
    void AppendDrawObj( SwAnchoredObject& _rNewObj );
    void RemoveDrawObj( SwAnchoredObject& _rToRemoveObj );

    // work with chain of FlyFrames
    void  AppendFly( SwFlyFrame *pNew );
    void  RemoveFly( SwFlyFrame *pToRemove );
    const SwSortedObjs *GetDrawObjs() const { return m_pDrawObjs.get(); }
          SwSortedObjs *GetDrawObjs()       { return m_pDrawObjs.get(); }
    // #i28701# - change purpose of method and adjust its name
    void InvalidateObjs( const bool _bNoInvaOfAsCharAnchoredObjs = true );

    virtual void PaintSwFrameShadowAndBorder(
        const SwRect&,
        const SwPageFrame* pPage,
        const SwBorderAttrs&) const;
    void PaintBaBo( const SwRect&, const SwPageFrame *pPage,
                    const bool bOnlyTextBackground = false) const;
    void PaintSwFrameBackground( const SwRect&, const SwPageFrame *pPage,
                          const SwBorderAttrs &,
                          const bool bLowerMode = false,
                          const bool bLowerBorder = false,
                          const bool bOnlyTextBackground = false ) const;
    void PaintBorderLine( const SwRect&, const SwRect&, const SwPageFrame*,
                          const Color *pColor,
                          const SvxBorderLineStyle = SvxBorderLineStyle::SOLID ) const;

    std::unique_ptr<drawinglayer::processor2d::BaseProcessor2D> CreateProcessor2D( ) const;
    void ProcessPrimitives( const drawinglayer::primitive2d::Primitive2DContainer& rSequence ) const;

    // retouch, not in the area of the given Rect!
    void Retouch( const SwPageFrame *pPage, const SwRect &rRect ) const;

    bool GetBackgroundBrush(
        drawinglayer::attribute::SdrAllFillAttributesHelperPtr& rFillAttributes,
        const SvxBrushItem*& rpBrush,
        const Color*& rpColor,
        SwRect &rOrigRect,
        bool bLowerMode,
        bool bConsiderTextBox ) const;

    inline void SetCompletePaint() const;
    inline void ResetCompletePaint() const;
    bool IsCompletePaint() const { return mbCompletePaint; }

    inline void SetRetouche() const;
    inline void ResetRetouche() const;
    bool IsRetouche() const { return mbRetouche; }

    void SetInfFlags();
    void InvalidateInfFlags() { mbInfInvalid = true; }
    inline bool IsInDocBody() const;    // use InfoFlags, determine flags
    inline bool IsInFootnote() const;        // if necessary
    inline bool IsInTab() const;
    inline bool IsInFly() const;
    inline bool IsInSct() const;

    // If frame is inside a split table row, this function returns
    // the corresponding row frame in the follow table.
    const SwRowFrame* IsInSplitTableRow() const;

    // If frame is inside a follow flow row, this function returns
    // the corresponding row frame master table
    const SwRowFrame* IsInFollowFlowRow() const;

    bool IsInBalancedSection() const;

    inline bool IsVertical() const;
    inline bool IsVertLR() const;
    inline bool IsVertLRBT() const;

    void SetDerivedVert( bool bNew ){ mbDerivedVert = bNew; }
    void SetInvalidVert( bool bNew) { mbInvalidVert = bNew; }
    inline bool IsRightToLeft() const;
    void SetDerivedR2L( bool bNew ) { mbDerivedR2L  = bNew; }

    void CheckDirChange();
    // returns upper left frame position for LTR and
    // upper right frame position for Asian / RTL frames
    Point   GetFrameAnchorPos( bool bIgnoreFlysAnchoredAtThisFrame ) const;

    /** determine, if frame is moveable in given environment

        method replaced 'old' method <bool IsMoveable() const>.
        Determines, if frame is moveable in given environment. if no environment
        is given (parameter _pLayoutFrame == 0), the movability in the actual
        environment (<GetUpper()) is checked.

        @param _pLayoutFrame
        input parameter - given environment (layout frame), in which the movability
        will be checked. If not set ( == 0 ), actual environment is taken.

        @return boolean, indicating, if frame is moveable in given environment
    */
    bool IsMoveable( const SwLayoutFrame* _pLayoutFrame = nullptr ) const;

    // Is it permitted for the (Text)Frame to add a footnote in the current
    // environment (not e.g. for repeating table headlines)
    bool IsFootnoteAllowed() const;

    virtual void  Format( vcl::RenderContext* pRenderContext, const SwBorderAttrs *pAttrs = nullptr );

    virtual void CheckDirection( bool bVert );

    void ReinitializeFrameSizeAttrFlags();

    /// WARNING: this may not return correct RES_PAGEDESC/RES_BREAK items for
    /// SwTextFrame, use GetBreakItem()/GetPageDescItem() instead
    const SwAttrSet *GetAttrSet() const;
    virtual const SvxFormatBreakItem& GetBreakItem() const;
    virtual const SwFormatPageDesc& GetPageDescItem() const;

    bool HasFixSize() const { return mbFixSize; }

    // check all pages (starting from the given) and correct them if needed
    static void CheckPageDescs( SwPageFrame *pStart, bool bNotifyFields = true, SwPageFrame** ppPrev = nullptr);

    // might return 0, with and without const
    SwFrame               *GetNext()  { return mpNext; }
    SwFrame               *GetPrev()  { return mpPrev; }
    SwLayoutFrame         *GetUpper() { return mpUpper; }
    SwRootFrame           *getRootFrame(){ return mpRoot; }
    SwPageFrame           *FindPageFrame() { return IsPageFrame() ? reinterpret_cast<SwPageFrame*>(this) : ImplFindPageFrame(); }
    SwFrame               *FindColFrame();
    SwRowFrame            *FindRowFrame();
    SwFootnoteBossFrame   *FindFootnoteBossFrame( bool bFootnotes = false );
    SwTabFrame            *ImplFindTabFrame();
    SwFootnoteFrame       *ImplFindFootnoteFrame();
    SwFlyFrame            *ImplFindFlyFrame();
    SwSectionFrame        *ImplFindSctFrame();
    const SwBodyFrame     *ImplFindBodyFrame() const;
    SwFrame               *FindFooterOrHeader();
    SwFrame               *GetLower();
    const SwFrame         *GetNext()  const { return mpNext; }
    const SwFrame         *GetPrev()  const { return mpPrev; }
    const SwLayoutFrame   *GetUpper() const { return mpUpper; }
    const SwRootFrame     *getRootFrame()   const { return mpRoot; }
    inline SwTabFrame     *FindTabFrame();
    inline SwFootnoteFrame     *FindFootnoteFrame();
    inline SwFlyFrame     *FindFlyFrame();
    inline SwSectionFrame *FindSctFrame();
    inline SwFrame        *FindNext();
    // #i27138# - add parameter <_bInSameFootnote>
    SwContentFrame* FindNextCnt( const bool _bInSameFootnote = false );
    inline SwFrame        *FindPrev();
    inline const SwPageFrame *FindPageFrame() const;
    inline const SwFootnoteBossFrame *FindFootnoteBossFrame( bool bFootnote = false ) const;
    inline const SwFrame     *FindColFrame() const;
    inline const SwFrame     *FindFooterOrHeader() const;
    inline const SwTabFrame  *FindTabFrame() const;
    inline const SwFootnoteFrame  *FindFootnoteFrame() const;
    inline const SwFlyFrame  *FindFlyFrame() const;
    inline const SwSectionFrame *FindSctFrame() const;
    inline const SwBodyFrame    *FindBodyFrame() const;
    inline const SwFrame     *FindNext() const;
    // #i27138# - add parameter <_bInSameFootnote>
    const SwContentFrame* FindNextCnt( const bool _bInSameFootnote = false ) const;
    inline const SwFrame     *FindPrev() const;
           const SwFrame     *GetLower()  const;

    SwContentFrame* FindPrevCnt();

    const SwContentFrame* FindPrevCnt() const;

    // #i79774#
    SwFrame* GetIndPrev_() const;
    SwFrame* GetIndPrev() const
        { return ( mpPrev || !IsInSct() ) ? mpPrev : GetIndPrev_(); }

    SwFrame* GetIndNext()
        { return ( mpNext || !IsInSct() ) ? mpNext : GetIndNext_(); }
    const SwFrame* GetIndNext() const { return const_cast<SwFrame*>(this)->GetIndNext(); }

    sal_uInt16 GetPhyPageNum() const;   // page number without offset
    sal_uInt16 GetVirtPageNum() const;  // page number with offset
    bool OnRightPage() const { return 0 != GetPhyPageNum() % 2; };
    bool WannaRightPage() const;
    bool OnFirstPage() const;

    inline const  SwLayoutFrame *GetPrevLayoutLeaf() const;
    inline const  SwLayoutFrame *GetNextLayoutLeaf() const;
    inline SwLayoutFrame *GetPrevLayoutLeaf();
    inline SwLayoutFrame *GetNextLayoutLeaf();

    virtual void Calc(vcl::RenderContext* pRenderContext) const; // here might be "formatted"
    inline void OptCalc() const;    // here we assume (for optimization) that
                                    // the predecessors are already formatted
    Point   GetRelPos() const;

    // PaintArea is the area where content might be displayed.
    // The margin of a page or the space between columns belongs to it.
    const SwRect GetPaintArea() const;

    // UnionFrame is the union of Frame- and PrtArea, normally identical
    // to the FrameArea except in case of negative Prt margins.
    const SwRect UnionFrame( bool bBorder = false ) const;

    virtual Size ChgSize( const Size& aNewSize );

    virtual void Cut() = 0;
    virtual void Paste( SwFrame* pParent, SwFrame* pSibling = nullptr ) = 0;

    void ValidateLineNum() { mbValidLineNum = true; }

    bool GetValidLineNumFlag()const { return mbValidLineNum; }

    // Only invalidate Frame
    // #i28701# - add call to method <ActionOnInvalidation(..)>
    //            for all invalidation methods.
    // #i28701# - use method <InvalidationAllowed(..)> to
    //            decide, if invalidation will to be performed or not.
    // #i26945# - no additional invalidation, if it's already
    //            invalidate.
    void InvalidateSize_()
    {
        if ( isFrameAreaSizeValid() && InvalidationAllowed( INVALID_SIZE ) )
        {
            setFrameAreaSizeValid(false);
            ActionOnInvalidation( INVALID_SIZE );
        }
    }
    void InvalidatePrt_()
    {
        if ( isFramePrintAreaValid() && InvalidationAllowed( INVALID_PRTAREA ) )
        {
            setFramePrintAreaValid(false);
            ActionOnInvalidation( INVALID_PRTAREA );
        }
    }
    void InvalidatePos_()
    {
        if ( isFrameAreaPositionValid() && InvalidationAllowed( INVALID_POS ) )
        {
            setFrameAreaPositionValid(false);
            ActionOnInvalidation( INVALID_POS );
        }
    }
    void InvalidateLineNum_()
    {
        if ( mbValidLineNum && InvalidationAllowed( INVALID_LINENUM ) )
        {
            mbValidLineNum = false;
            ActionOnInvalidation( INVALID_LINENUM );
        }
    }
    void InvalidateAll_()
    {
        if ( ( isFrameAreaSizeValid() || isFramePrintAreaValid() || isFrameAreaPositionValid() ) && InvalidationAllowed( INVALID_ALL ) )
        {
            setFrameAreaSizeValid(false);
            setFrameAreaPositionValid(false);
            setFramePrintAreaValid(false);
            ActionOnInvalidation( INVALID_ALL );
        }
    }
    // also notify page at the same time
    inline void InvalidateSize();
    inline void InvalidatePrt();
    inline void InvalidatePos();
    inline void InvalidateLineNum();
    inline void InvalidateAll();
    void ImplInvalidateSize();
    void ImplInvalidatePrt();
    void ImplInvalidatePos();
    void ImplInvalidateLineNum();

    inline void InvalidateNextPos( bool bNoFootnote = false );
    void ImplInvalidateNextPos( bool bNoFootnote );

    /** method to invalidate printing area of next frame
        #i11859#
    */
    void InvalidateNextPrtArea();

    void InvalidatePage( const SwPageFrame *pPage = nullptr ) const;

    virtual bool    FillSelection( SwSelectionList& rList, const SwRect& rRect ) const;

    virtual bool    GetCursorOfst( SwPosition *, Point&,
                                 SwCursorMoveState* = nullptr, bool bTestBackground = false ) const;
    virtual bool    GetCharRect( SwRect &, const SwPosition&,
                                 SwCursorMoveState* = nullptr, bool bAllowFarAway = true ) const;
    virtual void PaintSwFrame( vcl::RenderContext& rRenderContext, SwRect const&,
                        SwPrintData const*const pPrintData = nullptr ) const;

    // HACK: shortcut between frame and formatting
    // It's your own fault if you cast void* incorrectly! In any case check
    // the void* for 0.
    virtual bool Prepare( const PrepareHint ePrep = PREP_CLEAR,
                          const void *pVoid = nullptr, bool bNotify = true );

    // true if it is the correct class, false otherwise
    inline bool IsLayoutFrame() const;
    inline bool IsRootFrame() const;
    inline bool IsPageFrame() const;
    inline bool IsColumnFrame() const;
    inline bool IsFootnoteBossFrame() const;  // footnote bosses might be PageFrames or ColumnFrames
    inline bool IsHeaderFrame() const;
    inline bool IsFooterFrame() const;
    inline bool IsFootnoteContFrame() const;
    inline bool IsFootnoteFrame() const;
    inline bool IsBodyFrame() const;
    inline bool IsColBodyFrame() const;  // implemented in layfrm.hxx, BodyFrame above ColumnFrame
    inline bool IsPageBodyFrame() const; // implemented in layfrm.hxx, BodyFrame above PageFrame
    inline bool IsFlyFrame() const;
    inline bool IsSctFrame() const;
    inline bool IsTabFrame() const;
    inline bool IsRowFrame() const;
    inline bool IsCellFrame() const;
    inline bool IsContentFrame() const;
    inline bool IsTextFrame() const;
    inline bool IsNoTextFrame() const;
    // Frames where its PrtArea depends on their neighbors and that are
    // positioned in the content flow
    inline bool IsFlowFrame() const;
    // Frames that are capable of retouching or that might need to retouch behind
    // themselves
    inline bool IsRetoucheFrame() const;
    inline bool IsAccessibleFrame() const;

    void PrepareCursor();                 // CursorShell is allowed to call this

    // Is the Frame (or the section containing it) protected? Same for Fly in
    // Fly in ... and footnotes
    bool IsProtected() const;

    bool IsColLocked()  const { return mbColLocked; }
    virtual bool IsDeleteForbidden() const { return mbForbidDelete; }

    /// this is the only way to delete a SwFrame instance
    static void DestroyFrame(SwFrame *const pFrame);

    bool IsInDtor() const { return mbInDtor; }

    // No inline cause we need the function pointers
    long GetTopMargin() const;
    long GetBottomMargin() const;
    long GetLeftMargin() const;
    long GetRightMargin() const;
    void SetTopBottomMargins( long, long );
    void SetLeftRightMargins( long, long );
    void SetRightLeftMargins( long, long );
    long GetPrtLeft() const;
    long GetPrtBottom() const;
    long GetPrtRight() const;
    long GetPrtTop() const;
    bool SetMinLeft( long );
    bool SetMaxBottom( long );
    bool SetMaxRight( long );
    void MakeBelowPos( const SwFrame*, const SwFrame*, bool );
    void MakeLeftPos( const SwFrame*, const SwFrame*, bool );
    void MakeRightPos( const SwFrame*, const SwFrame*, bool );
    bool IsNeighbourFrame() const
        { return bool(GetType() & FRM_NEIGHBOUR); }

    // NEW TABLES
    // Some functions for covered/covering table cells. This way unnecessary
    // includes can be avoided
    bool IsLeaveUpperAllowed() const;
    bool IsCoveredCell() const;
    bool IsInCoveredCell() const;

    // #i81146# new loop control
    bool KnowsFormat( const SwFormat& rFormat ) const;
    void RegisterToFormat( SwFormat& rFormat );
    void ValidateThisAndAllLowers( const sal_uInt16 nStage );

    void ForbidDelete()      { mbForbidDelete = true; }
    void AllowDelete()    { mbForbidDelete = false; }

    drawinglayer::attribute::SdrAllFillAttributesHelperPtr getSdrAllFillAttributesHelper() const;
    bool supportsFullDrawingLayerFillAttributeSet() const;

public:
    // if writer is NULL, dumps the layout structure as XML in layout.xml
    virtual void dumpAsXml(xmlTextWriterPtr writer = nullptr) const;
    void dumpTopMostAsXml(xmlTextWriterPtr writer = nullptr) const;
    void dumpInfosAsXml(xmlTextWriterPtr writer) const;
    virtual void dumpAsXmlAttributes(xmlTextWriterPtr writer) const;
    void dumpChildrenAsXml(xmlTextWriterPtr writer) const;
    bool IsCollapse() const;
};

inline bool SwFrame::IsInDocBody() const
{
    if ( mbInfInvalid )
        const_cast<SwFrame*>(this)->SetInfFlags();
    return mbInfBody;
}
inline bool SwFrame::IsInFootnote() const
{
    if ( mbInfInvalid )
        const_cast<SwFrame*>(this)->SetInfFlags();
    return mbInfFootnote;
}
inline bool SwFrame::IsInTab() const
{
    if ( mbInfInvalid )
        const_cast<SwFrame*>(this)->SetInfFlags();
    return mbInfTab;
}
inline bool SwFrame::IsInFly() const
{
    if ( mbInfInvalid )
        const_cast<SwFrame*>(this)->SetInfFlags();
    return mbInfFly;
}
inline bool SwFrame::IsInSct() const
{
    if ( mbInfInvalid )
        const_cast<SwFrame*>(this)->SetInfFlags();
    return mbInfSct;
}
bool SwFrame::IsVertical() const
{
    if( mbInvalidVert )
        const_cast<SwFrame*>(this)->SetDirFlags( true );
    return mbVertical;
}
inline bool SwFrame::IsVertLR() const
{
    return mbVertLR;
}
inline bool SwFrame::IsVertLRBT() const
{
    return mbVertLRBT;
}
inline bool SwFrame::IsRightToLeft() const
{
    if( mbInvalidR2L )
        const_cast<SwFrame*>(this)->SetDirFlags( false );
    return mbRightToLeft;
}

inline void SwFrame::SetCompletePaint() const
{
    const_cast<SwFrame*>(this)->mbCompletePaint = true;
}
inline void SwFrame::ResetCompletePaint() const
{
    const_cast<SwFrame*>(this)->mbCompletePaint = false;
}

inline void SwFrame::SetRetouche() const
{
    const_cast<SwFrame*>(this)->mbRetouche = true;
}
inline void SwFrame::ResetRetouche() const
{
    const_cast<SwFrame*>(this)->mbRetouche = false;
}

inline SwLayoutFrame *SwFrame::GetNextLayoutLeaf()
{
    return const_cast<SwLayoutFrame*>(static_cast<const SwFrame*>(this)->GetNextLayoutLeaf());
}
inline SwLayoutFrame *SwFrame::GetPrevLayoutLeaf()
{
    return const_cast<SwLayoutFrame*>(static_cast<const SwFrame*>(this)->GetPrevLayoutLeaf());
}
inline const SwLayoutFrame *SwFrame::GetNextLayoutLeaf() const
{
    return ImplGetNextLayoutLeaf( true );
}
inline const SwLayoutFrame *SwFrame::GetPrevLayoutLeaf() const
{
    return ImplGetNextLayoutLeaf( false );
}

inline void SwFrame::InvalidateSize()
{
    if ( isFrameAreaSizeValid() )
    {
        ImplInvalidateSize();
    }
}
inline void SwFrame::InvalidatePrt()
{
    if ( isFramePrintAreaValid() )
    {
        ImplInvalidatePrt();
    }
}
inline void SwFrame::InvalidatePos()
{
    if ( isFrameAreaPositionValid() )
    {
        ImplInvalidatePos();
    }
}
inline void SwFrame::InvalidateLineNum()
{
    if ( mbValidLineNum )
        ImplInvalidateLineNum();
}
inline void SwFrame::InvalidateAll()
{
    if ( InvalidationAllowed( INVALID_ALL ) )
    {
        if ( isFrameAreaDefinitionValid()  )
        {
            ImplInvalidatePos();
        }

        setFrameAreaSizeValid(false);
        setFrameAreaPositionValid(false);
        setFramePrintAreaValid(false);

        // #i28701#
        ActionOnInvalidation( INVALID_ALL );
    }
}
inline void SwFrame::InvalidateNextPos( bool bNoFootnote )
{
    if ( mpNext && !mpNext->IsSctFrame() )
        mpNext->InvalidatePos();
    else
        ImplInvalidateNextPos( bNoFootnote );
}

inline void SwFrame::OptCalc() const
{
    if ( !isFrameAreaPositionValid() || !isFramePrintAreaValid() || !isFrameAreaSizeValid() )
    {
        const_cast<SwFrame*>(this)->OptPrepareMake();
    }
}
inline const SwPageFrame *SwFrame::FindPageFrame() const
{
    return const_cast<SwFrame*>(this)->FindPageFrame();
}
inline const SwFrame *SwFrame::FindColFrame() const
{
    return const_cast<SwFrame*>(this)->FindColFrame();
}
inline const SwFrame *SwFrame::FindFooterOrHeader() const
{
    return const_cast<SwFrame*>(this)->FindFooterOrHeader();
}
inline SwTabFrame *SwFrame::FindTabFrame()
{
    return IsInTab() ? ImplFindTabFrame() : nullptr;
}
inline const SwFootnoteBossFrame *SwFrame::FindFootnoteBossFrame( bool bFootnote ) const
{
    return const_cast<SwFrame*>(this)->FindFootnoteBossFrame( bFootnote );
}
inline SwFootnoteFrame *SwFrame::FindFootnoteFrame()
{
    return IsInFootnote() ? ImplFindFootnoteFrame() : nullptr;
}
inline SwFlyFrame *SwFrame::FindFlyFrame()
{
    return IsInFly() ? ImplFindFlyFrame() : nullptr;
}
inline SwSectionFrame *SwFrame::FindSctFrame()
{
    return IsInSct() ? ImplFindSctFrame() : nullptr;
}

inline const SwBodyFrame *SwFrame::FindBodyFrame() const
{
    return IsInDocBody() ? ImplFindBodyFrame() : nullptr;
}

inline const SwTabFrame *SwFrame::FindTabFrame() const
{
    return IsInTab() ? const_cast<SwFrame*>(this)->ImplFindTabFrame() : nullptr;
}
inline const SwFootnoteFrame *SwFrame::FindFootnoteFrame() const
{
    return IsInFootnote() ? const_cast<SwFrame*>(this)->ImplFindFootnoteFrame() : nullptr;
}
inline const SwFlyFrame *SwFrame::FindFlyFrame() const
{
    return IsInFly() ? const_cast<SwFrame*>(this)->ImplFindFlyFrame() : nullptr;
}
inline const SwSectionFrame *SwFrame::FindSctFrame() const
{
    return IsInSct() ? const_cast<SwFrame*>(this)->ImplFindSctFrame() : nullptr;
}
inline SwFrame *SwFrame::FindNext()
{
    if ( mpNext )
        return mpNext;
    else
        return FindNext_();
}
inline const SwFrame *SwFrame::FindNext() const
{
    if ( mpNext )
        return mpNext;
    else
        return const_cast<SwFrame*>(this)->FindNext_();
}
inline SwFrame *SwFrame::FindPrev()
{
    if ( mpPrev && !mpPrev->IsSctFrame() )
        return mpPrev;
    else
        return FindPrev_();
}
inline const SwFrame *SwFrame::FindPrev() const
{
    if ( mpPrev && !mpPrev->IsSctFrame() )
        return mpPrev;
    else
        return const_cast<SwFrame*>(this)->FindPrev_();
}

inline bool SwFrame::IsLayoutFrame() const
{
    return bool(GetType() & FRM_LAYOUT);
}
inline bool SwFrame::IsRootFrame() const
{
    return mnFrameType == SwFrameType::Root;
}
inline bool SwFrame::IsPageFrame() const
{
    return mnFrameType == SwFrameType::Page;
}
inline bool SwFrame::IsColumnFrame() const
{
    return mnFrameType == SwFrameType::Column;
}
inline bool SwFrame::IsFootnoteBossFrame() const
{
    return bool(GetType() & FRM_FTNBOSS);
}
inline bool SwFrame::IsHeaderFrame() const
{
    return mnFrameType == SwFrameType::Header;
}
inline bool SwFrame::IsFooterFrame() const
{
    return mnFrameType == SwFrameType::Footer;
}
inline bool SwFrame::IsFootnoteContFrame() const
{
    return mnFrameType == SwFrameType::FtnCont;
}
inline bool SwFrame::IsFootnoteFrame() const
{
    return mnFrameType == SwFrameType::Ftn;
}
inline bool SwFrame::IsBodyFrame() const
{
    return mnFrameType == SwFrameType::Body;
}
inline bool SwFrame::IsFlyFrame() const
{
    return mnFrameType == SwFrameType::Fly;
}
inline bool SwFrame::IsSctFrame() const
{
    return mnFrameType == SwFrameType::Section;
}
inline bool SwFrame::IsTabFrame() const
{
    return mnFrameType == SwFrameType::Tab;
}
inline bool SwFrame::IsRowFrame() const
{
    return mnFrameType == SwFrameType::Row;
}
inline bool SwFrame::IsCellFrame() const
{
    return mnFrameType == SwFrameType::Cell;
}
inline bool SwFrame::IsContentFrame() const
{
    return bool(GetType() & FRM_CNTNT);
}
inline bool SwFrame::IsTextFrame() const
{
    return mnFrameType == SwFrameType::Txt;
}
inline bool SwFrame::IsNoTextFrame() const
{
    return mnFrameType == SwFrameType::NoTxt;
}
inline bool SwFrame::IsFlowFrame() const
{
    return bool(GetType() & (FRM_CNTNT|SwFrameType::Tab|SwFrameType::Section));
}
inline bool SwFrame::IsRetoucheFrame() const
{
    return bool(GetType() & (FRM_CNTNT|SwFrameType::Tab|SwFrameType::Section|SwFrameType::Ftn));
}
inline bool SwFrame::IsAccessibleFrame() const
{
    return bool(GetType() & FRM_ACCESSIBLE);
}

//use this to protect a SwFrame for a given scope from getting deleted
class SwFrameDeleteGuard
{
private:
    SwFrame *m_pForbidFrame;
public:
    //Flag pFrame for SwFrameDeleteGuard lifetime that we shouldn't delete
    //it in e.g. SwSectionFrame::MergeNext etc because we will need it
    //again after the SwFrameDeleteGuard dtor
    explicit SwFrameDeleteGuard(SwFrame* pFrame)
        : m_pForbidFrame((pFrame && !pFrame->IsDeleteForbidden()) ?
            pFrame : nullptr)
    {
        if (m_pForbidFrame)
            m_pForbidFrame->ForbidDelete();
    }

    ~SwFrameDeleteGuard()
    {
        if (m_pForbidFrame)
            m_pForbidFrame->AllowDelete();
    }
};

typedef long (SwFrame:: *SwFrameGet)() const;
typedef bool (SwFrame:: *SwFrameMax)( long );
typedef void (SwFrame:: *SwFrameMakePos)( const SwFrame*, const SwFrame*, bool );
typedef long (*SwOperator)( long, long );
typedef void (SwFrame:: *SwFrameSet)( long, long );

struct SwRectFnCollection
{
    SwRectGet     fnGetTop;
    SwRectGet     fnGetBottom;
    SwRectGet     fnGetLeft;
    SwRectGet     fnGetRight;
    SwRectGet     fnGetWidth;
    SwRectGet     fnGetHeight;
    SwRectPoint   fnGetPos;
    SwRectSize    fnGetSize;

    SwRectSet     fnSetTop;
    SwRectSet     fnSetBottom;
    SwRectSet     fnSetLeft;
    SwRectSet     fnSetRight;
    SwRectSet     fnSetWidth;
    SwRectSet     fnSetHeight;

    SwRectSet     fnSubTop;
    SwRectSet     fnAddBottom;
    SwRectSet     fnSubLeft;
    SwRectSet     fnAddRight;
    SwRectSet     fnAddWidth;
    SwRectSet     fnAddHeight;

    SwRectSet     fnSetPosX;
    SwRectSet     fnSetPosY;

    SwFrameGet      fnGetTopMargin;
    SwFrameGet      fnGetBottomMargin;
    SwFrameGet      fnGetLeftMargin;
    SwFrameGet      fnGetRightMargin;
    SwFrameSet      fnSetXMargins;
    SwFrameSet      fnSetYMargins;
    SwFrameGet      fnGetPrtTop;
    SwFrameGet      fnGetPrtBottom;
    SwFrameGet      fnGetPrtLeft;
    SwFrameGet      fnGetPrtRight;
    SwRectDist      fnTopDist;
    SwRectDist      fnBottomDist;
    SwRectDist      fnLeftDist;
    SwRectDist      fnRightDist;
    SwFrameMax      fnSetLimit;
    SwRectMax       fnOverStep;

    SwRectSetPos    fnSetPos;
    SwFrameMakePos  fnMakePos;
    SwOperator      fnXDiff;
    SwOperator      fnYDiff;
    SwOperator      fnXInc;
    SwOperator      fnYInc;

    SwRectSetTwice  fnSetLeftAndWidth;
    SwRectSetTwice  fnSetTopAndHeight;
};

typedef SwRectFnCollection* SwRectFn;

// This class allows to use proper methods regardless of orientation (LTR/RTL, horizontal or vertical)
extern SwRectFn fnRectHori, fnRectVert, fnRectVertL2R, fnRectVertL2RB2T;
class SwRectFnSet {
public:
    explicit SwRectFnSet(const SwFrame *pFrame)
        : m_bVert(pFrame->IsVertical())
        , m_bVertL2R(pFrame->IsVertLR())
        , m_bVertL2RB2T(pFrame->IsVertLRBT())
    {
        m_fnRect = m_bVert ? (m_bVertL2R ? (m_bVertL2RB2T ? fnRectVertL2RB2T : fnRectVertL2R) : fnRectVert) : fnRectHori;
    }

    void Refresh(const SwFrame *pFrame)
    {
        m_bVert = pFrame->IsVertical();
        m_bVertL2R = pFrame->IsVertLR();
        m_bVertL2RB2T = pFrame->IsVertLRBT();
        m_fnRect = m_bVert ? (m_bVertL2R ? (m_bVertL2RB2T ? fnRectVertL2RB2T : fnRectVertL2R) : fnRectVert) : fnRectHori;
    }

    bool IsVert() const    { return m_bVert; }
    bool IsVertL2R() const { return m_bVertL2R; }
    SwRectFn FnRect() const { return m_fnRect; }

    bool PosDiff(const SwRect &rRect1, const SwRect &rRect2) const
    {
        return ((rRect1.*m_fnRect->fnGetTop)() != (rRect2.*m_fnRect->fnGetTop)()
            || (rRect1.*m_fnRect->fnGetLeft)() != (rRect2.*m_fnRect->fnGetLeft)());
    }

    long  GetTop   (const SwRect& rRect) const { return (rRect.*m_fnRect->fnGetTop)   (); }
    long  GetBottom(const SwRect& rRect) const { return (rRect.*m_fnRect->fnGetBottom)(); }
    long  GetLeft  (const SwRect& rRect) const { return (rRect.*m_fnRect->fnGetLeft)  (); }
    long  GetRight (const SwRect& rRect) const { return (rRect.*m_fnRect->fnGetRight) (); }
    long  GetWidth (const SwRect& rRect) const { return (rRect.*m_fnRect->fnGetWidth) (); }
    long  GetHeight(const SwRect& rRect) const { return (rRect.*m_fnRect->fnGetHeight)(); }
    Point GetPos   (const SwRect& rRect) const { return (rRect.*m_fnRect->fnGetPos)   (); }
    Size  GetSize  (const SwRect& rRect) const { return (rRect.*m_fnRect->fnGetSize)  (); }

    void SetTop   (SwRect& rRect, long nNew) const { (rRect.*m_fnRect->fnSetTop)   (nNew); }
    void SetBottom(SwRect& rRect, long nNew) const { (rRect.*m_fnRect->fnSetBottom)(nNew); }
    void SetLeft  (SwRect& rRect, long nNew) const { (rRect.*m_fnRect->fnSetLeft)  (nNew); }
    void SetRight (SwRect& rRect, long nNew) const { (rRect.*m_fnRect->fnSetRight) (nNew); }
    void SetWidth (SwRect& rRect, long nNew) const { (rRect.*m_fnRect->fnSetWidth) (nNew); }
    void SetHeight(SwRect& rRect, long nNew) const { (rRect.*m_fnRect->fnSetHeight)(nNew); }

    void SubTop   (SwRect& rRect, long nNew) const { (rRect.*m_fnRect->fnSubTop)   (nNew); }
    void AddBottom(SwRect& rRect, long nNew) const { (rRect.*m_fnRect->fnAddBottom)(nNew); }
    void SubLeft  (SwRect& rRect, long nNew) const { (rRect.*m_fnRect->fnSubLeft)  (nNew); }
    void AddRight (SwRect& rRect, long nNew) const { (rRect.*m_fnRect->fnAddRight) (nNew); }
    void AddWidth (SwRect& rRect, long nNew) const { (rRect.*m_fnRect->fnAddWidth) (nNew); }
    void AddHeight(SwRect& rRect, long nNew) const { (rRect.*m_fnRect->fnAddHeight)(nNew); }

    void SetPosX(SwRect& rRect, long nNew) const { (rRect.*m_fnRect->fnSetPosX)(nNew); }
    void SetPosY(SwRect& rRect, long nNew) const { (rRect.*m_fnRect->fnSetPosY)(nNew); }

    long  GetTopMargin   (const SwFrame& rFrame) const { return (rFrame.*m_fnRect->fnGetTopMargin)   (); }
    long  GetBottomMargin(const SwFrame& rFrame) const { return (rFrame.*m_fnRect->fnGetBottomMargin)(); }
    long  GetLeftMargin  (const SwFrame& rFrame) const { return (rFrame.*m_fnRect->fnGetLeftMargin)  (); }
    long  GetRightMargin (const SwFrame& rFrame) const { return (rFrame.*m_fnRect->fnGetRightMargin) (); }
    void  SetXMargins(SwFrame& rFrame, long nLeft, long nRight) const { (rFrame.*m_fnRect->fnSetXMargins)(nLeft, nRight); }
    void  SetYMargins(SwFrame& rFrame, long nTop, long nBottom) const { (rFrame.*m_fnRect->fnSetYMargins)(nTop, nBottom); }
    long  GetPrtTop      (const SwFrame& rFrame) const { return (rFrame.*m_fnRect->fnGetPrtTop)      (); }
    long  GetPrtBottom   (const SwFrame& rFrame) const { return (rFrame.*m_fnRect->fnGetPrtBottom)   (); }
    long  GetPrtLeft     (const SwFrame& rFrame) const { return (rFrame.*m_fnRect->fnGetPrtLeft)     (); }
    long  GetPrtRight    (const SwFrame& rFrame) const { return (rFrame.*m_fnRect->fnGetPrtRight)    (); }
    long  TopDist   (const SwRect& rRect, long nPos) const { return (rRect.*m_fnRect->fnTopDist)    (nPos); }
    long  BottomDist(const SwRect& rRect, long nPos) const { return (rRect.*m_fnRect->fnBottomDist) (nPos); }
    long  LeftDist   (const SwRect& rRect, long nPos) const { return (rRect.*m_fnRect->fnLeftDist)    (nPos); }
    long  RightDist   (const SwRect& rRect, long nPos) const { return (rRect.*m_fnRect->fnRightDist)    (nPos); }
    void  SetLimit (SwFrame& rFrame, long nNew) const { (rFrame.*m_fnRect->fnSetLimit) (nNew); }
    bool  OverStep  (const SwRect& rRect, long nPos) const { return (rRect.*m_fnRect->fnOverStep)   (nPos); }

    void SetPos(SwRect& rRect, const Point& rNew) const { (rRect.*m_fnRect->fnSetPos)(rNew); }
    void MakePos(SwFrame& rFrame, const SwFrame* pUp, const SwFrame* pPrv, bool bNotify) const { (rFrame.*m_fnRect->fnMakePos)(pUp, pPrv, bNotify); }
    long XDiff(long n1, long n2) const { return (m_fnRect->fnXDiff) (n1, n2); }
    long YDiff(long n1, long n2) const { return (m_fnRect->fnYDiff) (n1, n2); }
    long XInc (long n1, long n2) const { return (m_fnRect->fnXInc)  (n1, n2); }
    long YInc (long n1, long n2) const { return (m_fnRect->fnYInc)  (n1, n2); }

    void SetLeftAndWidth(SwRect& rRect, long nLeft, long nWidth) const { (rRect.*m_fnRect->fnSetLeftAndWidth)(nLeft, nWidth); }
    void SetTopAndHeight(SwRect& rRect, long nTop, long nHeight) const { (rRect.*m_fnRect->fnSetTopAndHeight)(nTop, nHeight); }

private:
    bool m_bVert;
    bool m_bVertL2R;
    bool m_bVertL2RB2T;
    SwRectFn m_fnRect;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
