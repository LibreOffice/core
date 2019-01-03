/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



#ifndef SD_SLIDESORTER_INSERTION_INDICATOR_OVERLAY_HXX
#define SD_SLIDESORTER_INSERTION_INDICATOR_OVERLAY_HXX

#include "model/SlsSharedPageDescriptor.hxx"
#include "view/SlsILayerPainter.hxx"
#include "controller/SlsTransferableData.hxx"
#include "sdxfer.hxx"

#include <tools/gen.hxx>
#include <vcl/bitmapex.hxx>
#include <boost/scoped_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <vector>

class OutputDevice;
class SdPage;

namespace sd { namespace slidesorter {
class SlideSorter;
} }

namespace sd { namespace slidesorter { namespace model {
class PageEnumeration;
} } }

namespace sd { namespace slidesorter { namespace controller {
class Transferable;
} } }

namespace sd { namespace slidesorter { namespace view {

class FramePainter;
class LayeredDevice;

/** The insertion indicator is painted as a vertical or horizontal bar
    in the space between slides.
*/
class InsertionIndicatorOverlay
    : public ILayerPainter,
      public ::boost::enable_shared_from_this<InsertionIndicatorOverlay>
{
public:
    InsertionIndicatorOverlay (SlideSorter& rSlideSorter);
    virtual ~InsertionIndicatorOverlay (void);

    virtual void SetLayerInvalidator (const SharedILayerInvalidator& rpInvalidator);

    void Create (const SdTransferable* pTransferable);

    /** Given a position in model coordinates this method calculates the
        insertion marker both as an index in the document and as a location
        used for drawing the insertion indicator.
    */
    void SetLocation (const Point& rPosition);

    Size GetSize (void) const;

    virtual void Paint (
        OutputDevice& rDevice,
        const Rectangle& rRepaintArea);

    bool IsVisible (void) const;
    void Hide (void);
    void Show (void);

    Rectangle GetBoundingBox (void) const;

private:
    SlideSorter& mrSlideSorter;
    bool mbIsVisible;
    const sal_Int32 mnLayerIndex;
    SharedILayerInvalidator mpLayerInvalidator;
    // Center of the insertion indicator.
    Point maLocation;
    BitmapEx maIcon;
    Point maIconOffset;
    ::boost::scoped_ptr<FramePainter> mpShadowPainter;

    void SetPositionAndSize (const Rectangle& rBoundingBox);
    void SelectRepresentatives (
        model::PageEnumeration& rSelection,
        ::std::vector<model::SharedPageDescriptor>& rDescriptors) const;
    Point PaintRepresentatives (
        OutputDevice& rContent,
        const Size aPreviewSize,
        const sal_Int32 nOffset,
        const ::std::vector<controller::TransferableData::Representative>& rPages) const;
    void PaintPageCount (
        OutputDevice& rDevice,
        const sal_Int32 nSelectionCount,
        const Size aPreviewSize,
        const Point aFirstPageOffset) const;
    /** Setup the insertion indicator by creating the icon.  It consists of
        scaled down previews of some of the selected pages.
    */
    void Create (
        const ::std::vector<controller::TransferableData::Representative>& rPages,
        const sal_Int32 nSelectionCount);
};



} } } // end of namespace ::sd::slidesorter::view

#endif
