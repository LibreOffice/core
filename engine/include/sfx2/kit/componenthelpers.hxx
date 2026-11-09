/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <sfx2/dllapi.h>
#include <tools/gen.hxx>
#include <vcl/vclptr.hxx>
#include <vcl/window.hxx>

#include <COKit/COKit.hxx>

#include <memory>

namespace com::sun::star::beans { struct PropertyValue; }
namespace com::sun::star::frame { class XController; }
namespace com::sun::star::frame { class XFrame; }
namespace com::sun::star::frame { class XDispatchResultListener; }
namespace cpo::uno { template<class E> class Sequence; }

class CoKitCallbackInterface;
class SfxInPlaceClient;
class SfxViewShell;
class VirtualDevice;

/// A class for chart editing support via COKit.
class SFX2_DLLPUBLIC KitChartHelper
{
private:
    SfxViewShell* mpViewShell;
    mutable cpo::uno::Reference<css::frame::XController> mxController;
    VclPtr<vcl::Window> mpWindow;

public:
    KitChartHelper(SfxViewShell* pViewShell)
        : mpViewShell(pViewShell)
        , mpWindow(nullptr)
    {}

    cpo::uno::Reference<css::frame::XController>& GetXController() const;
    vcl::Window* GetWindow();
    /// Where the chart is on the tiles, or with bWithGridOffset false, in print twips: in Calc
    /// these differ by the grid offset.
    tools::Rectangle GetChartBoundingBox(bool bWithGridOffset = true);
    void Invalidate();

    void Dispatch(const OUString& cmd, const cpo::uno::Sequence<css::beans::PropertyValue>& rArguments) const;
    /// Dispatches the command to the chart, which tells the listener when it is done. Answers
    /// whether the chart takes the command.
    bool Dispatch(const OUString& cmd,
                  const cpo::uno::Sequence<css::beans::PropertyValue>& rArguments,
                  const cpo::uno::Reference<css::frame::XDispatchResultListener>& xListener) const;

    bool Hit(const Point& aPos);
    static bool HitAny(const Point& aPos);
    void PaintTile(VirtualDevice& rRenderContext, const tools::Rectangle& rTileRect);
    static void PaintAllChartsOnTile(VirtualDevice& rDevice,
                                     int nOutputWidth, int nOutputHeight,
                                     int nTilePosX, int nTilePosY,
                                     tools::Long nTileWidth, tools::Long nTileHeight,
                                     bool bNegativeX = false);
    bool postMouseEvent(COKitMouseEventType eType, int nX, int nY,
                        int nCount, int nButtons, int nModifier,
                        double fScaleX = 1.0, double fScaleY = 1.0);
    bool setTextSelection(COKitSetTextSelectionType eType, int nX, int nY);
    bool setGraphicSelection(COKitSetGraphicSelectionType eType, int nX, int nY,
                             double fScaleX = 1.0, double fScaleY = 1.0);
};

/// A class for math editing support via COKit.
class SFX2_DLLPUBLIC KitStarMathHelper
{
public:
    KitStarMathHelper(const SfxViewShell* pViewShell);

    vcl::Window* GetGraphicWindow();
    vcl::Window* GetWidgetWindow();
    const SfxViewShell* GetSmViewShell();
    tools::Rectangle GetBoundingBox() const;
    /// A rectangle of the Math view in the coordinates of the document: the formula shows at the
    /// scale its frame gives it, from the frame's upper left corner.
    tools::Rectangle ToDocument(const tools::Rectangle& rInView) const;

    static const SfxViewShell* FindContainerViewShell(const SfxViewShell* pSmViewShell);

    /// Sends the cursor and selection of a Math view to its COKit client.
    static void SendCaret(const SfxViewShell& rSmViewShell);

    void Dispatch(const OUString& cmd, const cpo::uno::Sequence<css::beans::PropertyValue>& rArguments) const;
    /// Dispatches the command to the formula, which tells the listener when it is done. Answers
    /// whether the formula takes the command.
    bool Dispatch(const OUString& cmd,
                  const cpo::uno::Sequence<css::beans::PropertyValue>& rArguments,
                  const cpo::uno::Reference<css::frame::XDispatchResultListener>& xListener) const;

    bool postMouseEvent(COKitMouseEventType eType, int nX, int nY, int nCount, int nButtons,
                        int nModifier,
                        double fPPTScaleX, double fPPTScaleY);

    static void PaintAllInPlaceOnTile(VirtualDevice& rDevice, int nOutputWidth, int nOutputHeight,
                                      int nTilePosX, int nTilePosY, tools::Long nTileWidth,
                                      tools::Long nTileHeight);

private:
    void PaintTile(VirtualDevice& rDevice, const tools::Rectangle& rTileRect);

    const SfxViewShell* mpViewShell;
    const SfxInPlaceClient* mpIPClient = nullptr; // not nullptr when the object is valid
    cpo::uno::Reference<css::frame::XFrame> mxFrame; // not empty when the object is valid
    VclPtr<vcl::Window> mpGraphicWindow;
    VclPtr<vcl::Window> mpWidgetWindow;
};

/// A class for editing a spreadsheet object in place via COKit. The spreadsheet has a view of its
/// own; its presses are given to it in the coordinates of its sheet, and what its view reports is
/// sent on to the container view in the coordinates of the document. The container is a text
/// document, a presentation or a drawing; a spreadsheet in a spreadsheet is not edited this way.
class SFX2_DLLPUBLIC KitCalcHelper
{
public:
    KitCalcHelper(const SfxViewShell* pViewShell);

    /// The view of the spreadsheet that is active in place in the view, if there is one.
    SfxViewShell* GetCalcViewShell() const { return mpCalcViewShell; }
    /// The window the keys of the spreadsheet go to.
    vcl::Window* GetWindow();
    /// Where the spreadsheet is in the document, in twips.
    tools::Rectangle GetBoundingBox() const;
    /// A position in the document, in twips, as a position in the sheet, in twips from its corner.
    Point ToSheet(const Point& rInDocument) const;
    /// A rectangle in the sheet, in twips from its corner, as a rectangle in the document, in
    /// twips.
    tools::Rectangle ToDocument(const tools::Rectangle& rInSheet) const;

    bool postMouseEvent(COKitMouseEventType eType, int nX, int nY, int nCount, int nButtons,
                        int nModifier);

    /// Dispatches the command to the spreadsheet, which tells the listener when it is done.
    /// Answers whether the spreadsheet takes the command.
    bool Dispatch(const OUString& cmd,
                  const cpo::uno::Sequence<css::beans::PropertyValue>& rArguments,
                  const cpo::uno::Reference<css::frame::XDispatchResultListener>& xListener) const;

    /// The view of the spreadsheet active in place whose window holds pWindow, if there is one.
    static SfxViewShell* FindCalcViewShell(const vcl::Window* pWindow);

    /// The container view of the spreadsheet view rCalcViewShell, if it is active in place.
    static SfxViewShell* FindContainerViewShell(const SfxViewShell& rCalcViewShell);

    /// Whether any spreadsheet view has the callback of CreateViewCallback. It costs no search
    /// of the views, so it can answer for every event.
    static bool AnyInPlaceView();

    /// Whether the position, in twips, is on a spreadsheet that another view of the same
    /// document, on the same part, has active in place.
    static bool HitAny(const SfxViewShell& rViewShell, const Point& rPosition);

    /// Makes the callback of a spreadsheet view that is active in place: it sends what the view
    /// reports to the container view, in the coordinates of the document. rFrame is the frame of
    /// the spreadsheet; its creator is the frame of the container.
    static std::unique_ptr<CoKitCallbackInterface>
    CreateViewCallback(SfxViewShell& rCalcViewShell,
                       const cpo::uno::Reference<css::frame::XFrame>& rFrame);

    /// Whether rViewShell is the view of a spreadsheet active in place, with the callback of
    /// CreateViewCallback.
    static bool IsInPlaceView(const SfxViewShell& rViewShell);

    /// Makes the view of a spreadsheet active in place current while it lives, when the window
    /// is one of that spreadsheet, and the view that was current again after. The cell editing of
    /// a spreadsheet works with the current view, so an event handled at once, not posted, needs
    /// this.
    class SFX2_DLLPUBLIC ViewGuard
    {
        int mnPreviousView = -1;

    public:
        explicit ViewGuard(const vcl::Window* pWindow);
        ~ViewGuard();
    };

    /// Sends the cell cursor of the spreadsheet to the container view again, after the object
    /// moved or changed its size.
    static void SendCursor(const SfxViewShell& rCalcViewShell);

private:
    const SfxViewShell* mpViewShell;
    const SfxInPlaceClient* mpIPClient = nullptr; // not nullptr when the object is valid
    SfxViewShell* mpCalcViewShell = nullptr; // not nullptr when the object is valid
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
