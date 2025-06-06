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

#ifndef INCLUDED_SW_SOURCE_FILTER_WW8_WRITERHELPER_HXX
#define INCLUDED_SW_SOURCE_FILTER_WW8_WRITERHELPER_HXX

#include <vector>
#include <map>
#include <com/sun/star/embed/XEmbeddedObject.hpp>

#include <sfx2/objsh.hxx>
#include <svl/itempool.hxx>
#include <svl/itemset.hxx>
#include <svx/svdtypes.hxx>
#include <node.hxx>
#include <pam.hxx>
#include <tools/poly.hxx>
#include <doc.hxx>
#include <vcl/graph.hxx>

class SwTextFormatColl;
class SwCharFormat;
class SdrObject;
class SdrOle2Obj;
class SwNumFormat;
class SwTextNode;
class SwNoTextNode;
class SwFormatCharFormat;
class SwDoc;
class SwNumRule;

namespace sw
{
    namespace util
    {
        class ItemSort
        {
        public:
            bool operator()(sal_uInt16 nA, sal_uInt16 nB) const;
        };
    }
}

namespace ww8
{
    /// STL container of Paragraph Styles (SwTextFormatColl)
    typedef std::vector<SwTextFormatColl *> ParaStyles;
    /// STL container of SfxPoolItems (Attributes)
    typedef std::map<sal_uInt16, const SfxPoolItem *, sw::util::ItemSort> PoolItems;

    /** Make exporting a Writer Frame easy

        In word all frames are effectively anchored to character or as
        character. This is nice and simple, writer is massively complex in this
        area, so this ww8::Frame simplifies matters by providing a single unified
        view of the multitude of elements in writer and their differing quirks.

        A ww8::Frame wraps a writer frame and is guaranteed to have a suitable
        anchor position available from it. It hides much of the needless
        complexity of the multitude of floating/inline elements in writer, it...

        Guarantees an anchor position for a frame.
        Provides a readable way to see if we are anchored inline. (as character)
        Provides a simple way to flag what type of entity this frame describes.
        Provides the size of the element as drawn by writer.
    */
    class Frame
    {
    public:
        enum WriterSource {eTextBox, eGraphic, eOle, eDrawing, eFormControl,eBulletGrf};
    private:
        const SwFrameFormat* mpFlyFrame;
        SwPosition maPos;
        Size maSize;
        // #i43447# - Size of the frame in the layout.
        // Especially needed for graphics, whose layout size can differ from its
        // size, because it is scaled into its environment.
        Size maLayoutSize;

        WriterSource meWriterType;
        const SwNode *mpStartFrameContent;
        bool mbIsInline;
        bool mbForBullet:1;
        Graphic maGrf;
    public:
        Frame(const SwFrameFormat &rFlyFrame, SwPosition aPos);
        Frame(const Graphic&, SwPosition );

        /** Get the writer SwFrameFormat that this object describes

            @return
            The wrapped SwFrameFormat
        */
        const SwFrameFormat &GetFrameFormat() const { return *mpFlyFrame; }

        /** Get the position this frame is anchored at

            @return
            The anchor position of this frame
        */
        const SwPosition &GetPosition() const { return maPos; }
        void SetPosition(SwPosition const& rPos) { maPos = rPos; }

        /** Get the node this frame is anchored into

            @return
            The SwTextNode this frame is anchored inside
        */
        const SwContentNode *GetContentNode() const
            { return maPos.GetNode().GetContentNode(); }

        /** Get the type of frame that this wraps

            @return
            a WriterSource which describes the source type of this wrapper
        */
        WriterSource GetWriterType() const { return meWriterType; }

        /** Is this frame inline (as character)

            @return
            whether this is inline or not
        */
        bool IsInline() const { return mbIsInline; }

        /** Even if the frame isn't an inline frame, force it to behave as one

            There are a variety of circumstances where word cannot have
            anything except inline elements, e.g. inside frames. So its easier
            to force this ww8::Frame into behaving as one, instead of special
            casing export code all over the place.

        */
        void ForceTreatAsInline();

        /** Get the first node of content in the frame

         @return
         the first node of content in the frame, might not be any at all.
        */
        const SwNode *GetContent() const { return mpStartFrameContent; }
        const Graphic &GetGraphic() const { return maGrf; }
        bool HasGraphic() const { return mbForBullet; }

        /** Does this ww8::Frame refer to the same writer content as another

         @return
         if the two ww8::Frames are handling the same writer frame
        */
        bool RefersToSameFrameAs(const Frame &rOther) const
        {
            if (mbForBullet && rOther.mbForBullet)
                return (maGrf == rOther.maGrf);
            else if ((!mbForBullet) && (!rOther.mbForBullet))
                return (mpFlyFrame == rOther.mpFlyFrame);

            return false;
        }

        /** The Size of the contained element

         @return
         the best size to use to export to word
        */
        const Size& GetSize() const { return maSize; }

        /** The layout size of the contained element

            #i43447# - Needed for graphics, which are scaled into its environment

            @return layout size
        */
        const Size& GetLayoutSize() const
        {
            return maLayoutSize;
        }
    };

    /// STL container of Frames
    typedef std::vector<Frame> Frames;
    /// STL iterator for Frames
    typedef std::vector<Frame>::iterator FrameIter;
}

namespace sw
{
    namespace util
    {
        /** Provide a dynamic_cast style cast for SfxPoolItems

            A SfxPoolItem generally need to be cast back to its original type
            to be useful, which is both tedious and error prone. So item_cast is
            a helper template to aid the process and test if the cast is
            correct.

            @param rItem
            The SfxPoolItem which is to be casted

            @tplparam T
            A SfxPoolItem derived class to cast rItem to

            @return A rItem upcasted back to a T

            @exception std::bad_cast Thrown if the rItem was not a T
        */
        template<class T> const T & item_cast(const SfxPoolItem &rItem)
        {
            assert(dynamic_cast<const T *>(&rItem) && "bad type cast");
            return static_cast<const T &>(rItem);
        }

        /** Provide a dynamic_cast style cast for SfxPoolItems

            A SfxPoolItem generally need to be cast back to its original type
            to be useful, which is both tedious and error prone. So item_cast is
            a helper template to aid the process and test if the cast is
            correct.

            @param pItem
            The SfxPoolItem which is to be casted

            @tplparam T
            A SfxPoolItem derived class to cast pItem to

            @return A pItem upcasted back to a T or 0 if pItem was not a T
        */
        template<class T> const T * item_cast(const SfxPoolItem *pItem)
        {
            return dynamic_cast<const T *>(pItem);
        }

        /** Get the Paragraph Styles of a SwDoc

            Writer's styles are in one of those dreaded macro based pre-STL
            containers. Give me an STL container of the paragraph styles
            instead.

            @param rDoc
            The SwDoc document to get the styles from

            @return A ParaStyles containing the SwDoc's Paragraph Styles
        */
        ww8::ParaStyles GetParaStyles(const SwDoc &rDoc);

        /** Get a Paragraph Style which fits a given name

            Its surprisingly tricky to get a style when all you have is a name,
            but that's what this does

            @param rDoc
            The SwDoc document to search in

            @param rName
            The name of the style to search for

            @return A Paragraph Style if one exists which matches the name
        */
        SwTextFormatColl* GetParaStyle(SwDoc &rDoc, const UIName& rName);

        /** Get a Character Style which fits a given name

            Its surprisingly tricky to get a style when all you have is a name,
            but that's what this does

            @param rDoc
            The SwDoc document to search in

            @param rName
            The name of the style to search for

            @return A Character Style if one exists which matches the name
        */
        SwCharFormat* GetCharStyle(SwDoc &rDoc, const UIName& rName);

        /** Sort sequence of Paragraph Styles by assigned outline style list level

            Sort ParaStyles in ascending order of assigned outline style list level,
            e.g.  given Normal/Heading1/Heading2/.../Heading10 at their default
            assigned outline style list levels of body level/level 1/level 2/.../level 10

            #i98791#
            adjust the sorting algorithm due to introduced outline level attribute

            @param rStyles
            The ParaStyles to sort
        */
        void SortByAssignedOutlineStyleListLevel(ww8::ParaStyles &rStyles);

        /** Get the SfxPoolItems of a SfxItemSet

            Writer's SfxPoolItems (attributes) are in one of those dreaded
            macro based pre-STL containers. Give me an STL container of the
            items instead.

            @param rSet
            The SfxItemSet to get the items from

            @param rItems
            The sw::PoolItems to put the items into
        */
        void GetPoolItems(const SfxItemSet &rSet, ww8::PoolItems &rItems, bool bExportParentItemSet );

        const SfxPoolItem *SearchPoolItems(const ww8::PoolItems &rItems,
            sal_uInt16 eType);

        template<class T> const T* HasItem(const ww8::PoolItems &rItems,
            sal_uInt16 eType)
        {
            return item_cast<T>(SearchPoolItems(rItems, eType));
        }

        /** Remove properties from an SfxItemSet which a SwFormatCharFormat overrides

            Given an SfxItemSet and a SwFormatCharFormat remove from the rSet all the
            properties which the SwFormatCharFormat would override. An SfxItemSet
            contains attributes, and a SwFormatCharFormat is a "Character Style",
            so if the SfxItemSet contains bold and so does the character style
            then delete bold from the SfxItemSet

            @param
            rFormat the SwFormatCharFormat which describes the Character Style

            @param
            rSet the SfxItemSet from which we want to remove any properties
            which the rFormat would override

            @see #i24291# for examples
        */
        void ClearOverridesFromSet(const SwFormatCharFormat &rFormat, SfxItemSet &rSet);

        /** Get the Floating elements in a SwDoc

            Writer's FrameFormats may or may not be anchored to some text content,
            e.g. Page Anchored elements will not be. For the winword export we
            need them to have something to be anchored to. So this method
            returns all the floating elements in a document as a STL container
            of ww8::Frames which are guaranteed to have an appropriate anchor.

            @param rDoc
            The SwDoc document to get the styles from

            @param pPaM
            The SwPam to describe the selection in the document to get the
            elements from. 0 means the entire document.

            @return A Frames containing the selections Floating elements
        */
        ww8::Frames GetFrames(const SwDoc &rDoc, SwPaM const *pPaM);

        /** fix up frame positions, must be called after SetRedlineFlags */
        void UpdateFramePositions(ww8::Frames & rFrames);

        /** Get the Frames anchored to a given node

            Given a container of frames, find the ones anchored to a given node

            @param rFrames
            The container of frames to search in

            @param rNode
            The SwNode to check for anchors to

            @return the Frames in rFrames anchored to rNode
        */
        ww8::Frames GetFramesInNode(const ww8::Frames &rFrames, const SwNode &rNode);

        /** Get the Numbering Format used on a paragraph

            There are two differing types of numbering formats that may be on a
            paragraph, normal and outline. The outline is that numbering you
            see in tools->outline numbering. There's no difference in the
            numbering itself, just how you get it from the SwTextNode. Needless
            to say the filter generally couldn't care less what type of
            numbering is in use.

            @param rTextNode
            The SwTextNode that is the paragraph

            @return A SwNumFormat pointer that describes the numbering level
            on this paragraph, or 0 if there is none.
        */
        const SwNumFormat* GetNumFormatFromTextNode(const SwTextNode &rTextNode);

        /** Get the Numbering Format for a given level from a numbering rule

            @param rRule
            The numbering rule

            @param nLevel
            The numbering level

            @return A SwNumFormat pointer that describes the numbering level
            or 0 if the nLevel is out of range
        */
        const SwNumFormat* GetNumFormatFromSwNumRuleLevel(const SwNumRule &rRule,
            int nLevel);

        const SwNumRule* GetNumRuleFromTextNode(const SwTextNode &rTextNd);
        const SwNumRule* GetNormalNumRuleFromTextNode(const SwTextNode &rTextNd);

        /** Get the SwNoTextNode associated with a SwFrameFormat if here is one

            There are two differing types of numbering formats that may be on a
            paragraph, normal and outline. The outline is that numbering you
            see in tools->outline numbering. There's no difference in the
            numbering itself, just how you get it from the SwTextNode. Needless
            to say the filter generally couldn't care less what type of
            numbering is in use.

            @param rFormat
            The SwFrameFormat that may describe a graphic

            @return A SwNoTextNode pointer that describes the graphic of this
            frame if there is one, or 0 if there is none.
        */
        SwNoTextNode *GetNoTextNodeFromSwFrameFormat(const SwFrameFormat &rFormat);

        /** Does a node have a "page break before" applied

            Both text nodes and tables in writer can have "page break before"
            This function gives a unified view to both entities

            @param rNode
            The SwNode to query the page break of

            @return true if there is a page break, false otherwise
        */
        bool HasPageBreak(const SwNode &rNode);

        /** Make a best fit Polygon from a PolyPolygon

            For custom contours in writer we use a PolyPolygon, while word uses
            a simple polygon, so we need to try and make the best polygon from
            a PolyPolygon

            @param rPolyPoly
            The tools::PolyPolygon to try and turn into a Polygon

            @return best fit Polygon from rPolyPoly
        */
        tools::Polygon PolygonFromPolyPolygon(const tools::PolyPolygon &rPolyPoly);

        /// Undo all scaling / move tricks of the wrap polygon done during import.
        tools::Polygon CorrectWordWrapPolygonForExport(const tools::PolyPolygon& rPolyPoly, const SwNoTextNode* pNd, bool bCorrectCrop);

        /** Make setting a drawing object's layer in a Writer document easy

            Word has the simple concept of a drawing object either in the
            foreground and in the background. We have an additional complexity
            that form components live in a separate layer, which seems
            unnecessarily complicated. So in the winword filter we set the
            object's layer through this class with either SendObjectToHell for
            the bottom layer and SendObjectToHeaven for the top and we don't
            worry about the odd form layer design wrinkle.
        */
        class SetLayer
        {
        private:
            SdrLayerID mnHeavenLayer, mnHellLayer, mnFormLayer;
            enum Layer {eHeaven, eHell};
            void SetObjectLayer(SdrObject &rObject, Layer eLayer) const;
        public:

            /** Make Object live in the bottom drawing layer

                @param rObject
                The object to be set to the bottom layer
            */
            void SendObjectToHell(SdrObject &rObject) const;

            /** Make Object lives in the top layer

                @param rObject
                The object to be set to the top layer
            */
            void SendObjectToHeaven(SdrObject &rObject) const;

            /** Normal constructor

                @param rDoc
                The Writer document whose drawing layers we will be inserting
                objects into
            */
            explicit SetLayer(const SwDoc &rDoc);
        };

        const SwCharFormat* GetSwCharFormat(const SwFormatINetFormat& rINet, SwDoc& rDoc);
    }

    namespace hack
    {
            /** Map an ID valid in one SfxItemPool to its equivalent in another

            Given a WhichId (the id that identifies a property e.g. bold) which
            is correct in a given SfxItemPool, get the equivalent whichId in
            another SfxItemPool

            This arises because the drawing layer uses the same properties as
            writer e.g. SvxWeight, but for some reason uses different ids
            for the same properties as writer.

            @param rDestPool
            The SfxItemPool in whose terms the Id is returned

            @param rSrcPool
            The SfxItemPool in whose terms the Id is passed in

            @param nWhich
            The Id to transform from source to dest

            @return 0 on failure, the correct property Id on success
        */
        sal_uInt16 TransformWhichBetweenPools(const SfxItemPool &rDestPool,
            const SfxItemPool &rSrcPool, sal_uInt16 nWhich);

        /** Map a SwDoc WhichId to the equivalent Id for a given SfxItemSet

            Given a WhichId (the id that identifies a property e.g. bold) which
            is correct for a Writer document, get the equivalent whichId which
            for a given SfxItemSet.

            This arises because the drawing layer uses the same properties as
            writer e.g. SvxWeight, but for some reason uses different ids
            for the same properties as writer.

            This is effectively the same as TransformWhichBetweenPools except
            at a slightly different layer.

            @param rSet
            The SfxItemSet in whose terms the Id is returned

            @param rDoc
            The SwDoc in whose terms the Id is passed in

            @param nWhich
            The Id to transform from writer to the SfxItemSet's domain

            @return 0 on failure, the correct SfxItemSet Id on success
        */
        sal_uInt16 GetSetWhichFromSwDocWhich(const SfxItemSet &rSet,
            const SwDoc &rDoc, sal_uInt16 nWhich);

        /** Make inserting an OLE object into a Writer document easy

            The rest of Office uses SdrOle2Obj for their OLE objects, Writer
            doesn't, which makes things a bit difficult as this is the type of
            object that the escher import code shared by the MSOffice filters
            produces when it imports an OLE object.

            This utility class takes ownership of the OLE object away from a
            SdrOle2Obj and can massage it into the condition best suited to
            insertion into Writer.

            If the object was not transferred into Writer then it is deleted
            during destruction.
        */
        class DrawingOLEAdaptor
        {
        private:
            css::uno::Reference < css::embed::XEmbeddedObject > mxIPRef;
            SfxObjectShell& mrPers;
            const Graphic* mpGraphic;
        public:
            /** Take ownership of a SdrOle2Objs OLE object

                @param rObj
                The SdrOle2Obj whose OLE object we want to take control of

                @param rPers
                The SvPersist of a SwDoc (SwDoc::GetPersist()) into which we
                may want to move the object, or remove it from if unwanted.
            */
            DrawingOLEAdaptor(SdrOle2Obj &rObj, SfxObjectShell &rPers);

            /// Destructor will destroy the owned OLE object if not transferred
            ~DrawingOLEAdaptor();

            /** Transfer ownership of the OLE object to a document's SvPersist

                TransferToDoc moves the object into the persist under the name
                passed in. This name is then suitable to be used as an argument
                to SwDoc::InsertOLE.

                The object is no longer owned by the adaptor after this call,
                subsequent calls are an error and return false.

                @param rName
                The name to store the object under in the document.

                @return On success true is returned, otherwise false. On
                success rName is then suitable for user with SwDoc::InsertOLE
            */
            bool TransferToDoc(OUString &rName);
        private:
            DrawingOLEAdaptor& operator=(const DrawingOLEAdaptor&) = delete;
            DrawingOLEAdaptor(const DrawingOLEAdaptor &rDoc) = delete;
        };
    }
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
