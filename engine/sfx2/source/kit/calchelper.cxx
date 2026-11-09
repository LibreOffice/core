/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the Collabora Office project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sal/config.h>
#include <sal/log.hxx>

#include <cmath>
#include <sstream>
#include <string_view>

#include <boost/property_tree/json_parser.hpp>
#include <boost/property_tree/ptree.hpp>

#include <sfx2/ipclient.hxx>
#include <sfx2/kit/callback.hxx>
#include <sfx2/kit/componenthelpers.hxx>
#include <sfx2/kit/helper.hxx>
#include <sfx2/objsh.hxx>
#include <sfx2/frame.hxx>
#include <sfx2/viewfrm.hxx>
#include <sfx2/bindings.hxx>
#include <sfx2/viewsh.hxx>

#include <comphelper/dispatchcommand.hxx>
#include <comphelper/kit.hxx>
#include <comphelper/scopeguard.hxx>
#include <o3tl/string_view.hxx>
#include <rtl/strbuf.hxx>
#include <sot/storage.hxx>
#include <tools/UnitConversion.hxx>
#include <vcl/ITiledRenderable.hxx>
#include <vcl/window.hxx>

#include <com/sun/star/embed/XEmbeddedObject.hpp>
#include <com/sun/star/frame/XDispatchResultListener.hpp>
#include <com/sun/star/frame/XFrame.hpp>
#include <com/sun/star/frame/XFramesSupplier.hpp>
#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>

namespace
{
/// The model of the object, if it is a spreadsheet.
cpo::uno::Reference<css::frame::XModel> spreadsheetModelOf(const SfxInPlaceClient& rIPClient)
{
    const cpo::uno::Reference<css::embed::XEmbeddedObject>& xEmbObj = rIPClient.GetObject();
    if (!xEmbObj)
        return {};
    cpo::uno::Reference<css::lang::XServiceInfo> xComp(xEmbObj->getComponent(),
                                                       cpo::uno::UNO_QUERY);
    if (!xComp || !xComp->supportsService(u"com.sun.star.sheet.SpreadsheetDocument"_ustr))
        return {};
    return cpo::uno::Reference<css::frame::XModel>(xComp, cpo::uno::UNO_QUERY);
}

/// The number of spreadsheet views that have the callback of KitCalcHelper::CreateViewCallback.
int g_nInPlaceCalcViews = 0;

/// Whether the document of the view is a spreadsheet.
bool isSpreadsheet(const SfxViewShell& rViewShell)
{
    SfxObjectShell* pObjectShell = const_cast<SfxViewShell&>(rViewShell).GetObjectShell();
    if (!pObjectShell)
        return false;
    cpo::uno::Reference<css::lang::XServiceInfo> xModel(pObjectShell->GetModel(),
                                                        cpo::uno::UNO_QUERY);
    return xModel && xModel->supportsService(u"com.sun.star.sheet.SpreadsheetDocument"_ustr);
}

/// Makes the view current while it lives, and the view that was current again after. The cell
/// editing of a spreadsheet works with the current view. The view is looked up by its id at the
/// end, as the event may have closed it.
class CurrentViewGuard
{
    int mnView;
    int mnPreviousView;
    bool mbUpdateStates;

public:
    /// bUpdateStates: the states of the commands of the view are updated before it stops being
    /// current, because the view gives its states only while it is current.
    CurrentViewGuard(SfxViewShell& rViewShell, bool bUpdateStates)
        : mnView(KitHelper::getView(rViewShell))
        , mnPreviousView(KitHelper::getCurrentView())
        , mbUpdateStates(bUpdateStates)
    {
        KitHelper::setView(mnView);
    }
    ~CurrentViewGuard()
    {
        if (mnPreviousView < 0 || !KitHelper::getViewOfId(mnPreviousView))
            return;
        if (SfxViewShell* pViewShell = KitHelper::getViewOfId(mnView);
            pViewShell && mbUpdateStates)
            pViewShell->GetViewFrame().GetBindings().Update();
        KitHelper::setView(mnPreviousView);
    }
};
}

KitCalcHelper::KitCalcHelper(const SfxViewShell* pViewShell)
    : mpViewShell(pViewShell)
{
    if (!mpViewShell)
        return;

    const SfxInPlaceClient* pIPClient = mpViewShell->GetIPClient();
    if (!pIPClient || isSpreadsheet(*mpViewShell))
        return;

    cpo::uno::Reference<css::frame::XModel> xModel = spreadsheetModelOf(*pIPClient);
    if (!xModel)
        return;

    if (SfxViewShell* pCalcViewShell = SfxViewShell::Get(xModel->getCurrentController()))
    {
        mpIPClient = pIPClient;
        mpCalcViewShell = pCalcViewShell;
    }
}

vcl::Window* KitCalcHelper::GetWindow()
{
    if (!mpCalcViewShell)
        return nullptr;

    // the document finds the window of its view, which is its only one
    vcl::ITiledRenderable* pDoc
        = dynamic_cast<vcl::ITiledRenderable*>(mpCalcViewShell->GetCurrentDocument().get());
    return pDoc ? pDoc->getDocWindow().get() : nullptr;
}

tools::Rectangle KitCalcHelper::GetBoundingBox() const
{
    if (!mpCalcViewShell)
        return {};

    const tools::Rectangle aVisArea(
        mpCalcViewShell->GetObjectShell()->GetVisArea(ASPECT_CONTENT));
    return ToDocument(o3tl::convert(aVisArea, o3tl::Length::mm100, o3tl::Length::twip));
}

namespace
{
/// The area of the object in the document, in twips, and the scale the object is shown at in it.
void objectArea(const SfxViewShell& rViewShell, const SfxInPlaceClient& rIPClient,
                tools::Rectangle& rArea, double& rScaleX, double& rScaleY)
{
    rArea = rIPClient.GetObjArea();
    if (SfxObjectShell* pObjShell = const_cast<SfxViewShell&>(rViewShell).GetObjectShell())
    {
        const o3tl::Length eUnit = MapToO3tlLength(pObjShell->GetMapUnit());
        if (eUnit != o3tl::Length::twip && eUnit != o3tl::Length::invalid)
            rArea = o3tl::convert(rArea, eUnit, o3tl::Length::twip);
    }
    rScaleX = rIPClient.GetScaleWidth() > 0.0 ? rIPClient.GetScaleWidth() : 1.0;
    rScaleY = rIPClient.GetScaleHeight() > 0.0 ? rIPClient.GetScaleHeight() : 1.0;
}
}

Point KitCalcHelper::ToSheet(const Point& rInDocument) const
{
    if (!mpCalcViewShell)
        return rInDocument;

    tools::Rectangle aArea;
    double fScaleX;
    double fScaleY;
    objectArea(*mpViewShell, *mpIPClient, aArea, fScaleX, fScaleY);
    // the object shows its visible area, from its upper left corner on, at the scale of its frame
    const Point aVisAreaTopLeft(o3tl::convert(
        mpCalcViewShell->GetObjectShell()->GetVisArea(ASPECT_CONTENT).TopLeft(),
        o3tl::Length::mm100, o3tl::Length::twip));
    return Point(aVisAreaTopLeft.X() + std::round((rInDocument.X() - aArea.Left()) / fScaleX),
                 aVisAreaTopLeft.Y() + std::round((rInDocument.Y() - aArea.Top()) / fScaleY));
}

tools::Rectangle KitCalcHelper::ToDocument(const tools::Rectangle& rInSheet) const
{
    if (!mpCalcViewShell)
        return rInSheet;

    tools::Rectangle aArea;
    double fScaleX;
    double fScaleY;
    objectArea(*mpViewShell, *mpIPClient, aArea, fScaleX, fScaleY);
    const Point aVisAreaTopLeft(o3tl::convert(
        mpCalcViewShell->GetObjectShell()->GetVisArea(ASPECT_CONTENT).TopLeft(),
        o3tl::Length::mm100, o3tl::Length::twip));
    return tools::Rectangle(
        Point(aArea.Left() + std::round((rInSheet.Left() - aVisAreaTopLeft.X()) * fScaleX),
              aArea.Top() + std::round((rInSheet.Top() - aVisAreaTopLeft.Y()) * fScaleY)),
        Size(std::round(rInSheet.GetWidth() * fScaleX),
             std::round(rInSheet.GetHeight() * fScaleY)));
}

bool KitCalcHelper::postMouseEvent(COKitMouseEventType eType, int nX, int nY, int nCount,
                                   int nButtons, int nModifier)
{
    if (!mpCalcViewShell || !GetBoundingBox().Contains(Point(nX, nY)))
        return false;

    const Point aInSheet(ToSheet(Point(nX, nY)));
    // the cell editing of a spreadsheet works with the current view; a move changes no state
    CurrentViewGuard aGuard(*mpCalcViewShell, eType != COKitMouseEventType::MOVE);
    vcl::ITiledRenderable* pDoc
        = dynamic_cast<vcl::ITiledRenderable*>(mpCalcViewShell->GetCurrentDocument().get());
    if (!pDoc)
        return false;
    pDoc->postMouseEvent(eType, aInSheet.X(), aInSheet.Y(), nCount, nButtons, nModifier);
    return true;
}

bool KitCalcHelper::Dispatch(
    const OUString& cmd, const cpo::uno::Sequence<css::beans::PropertyValue>& rArguments,
    const cpo::uno::Reference<css::frame::XDispatchResultListener>& xListener) const
{
    if (!mpCalcViewShell)
        return false;

    const cpo::uno::Reference<css::frame::XFrame> xFrame
        = mpCalcViewShell->GetViewFrame().GetFrame().GetFrameInterface();
    // the commands of a spreadsheet work with the current view, as its cell editing does
    CurrentViewGuard aGuard(*mpCalcViewShell, true);
    return xFrame && comphelper::dispatchCommand(cmd, xFrame, rArguments, xListener);
}

bool KitCalcHelper::AnyInPlaceView() { return g_nInPlaceCalcViews > 0; }

SfxViewShell* KitCalcHelper::FindCalcViewShell(const vcl::Window* pWindow)
{
    if (!pWindow || !AnyInPlaceView())
        return nullptr;

    for (SfxViewShell* pViewShell = SfxViewShell::GetFirst(false); pViewShell;
         pViewShell = SfxViewShell::GetNext(*pViewShell, false))
    {
        if (!pViewShell->GetIPClient())
            continue;
        SfxViewShell* pCalcViewShell = KitCalcHelper(pViewShell).GetCalcViewShell();
        if (!pCalcViewShell)
            continue;
        const vcl::Window* pCalcWindow = pCalcViewShell->GetWindow();
        if (pCalcWindow && (pCalcWindow == pWindow || pCalcWindow->IsWindowOrChild(pWindow, true)))
            return pCalcViewShell;
    }
    return nullptr;
}

SfxViewShell* KitCalcHelper::FindContainerViewShell(const SfxViewShell& rCalcViewShell)
{
    if (!AnyInPlaceView())
        return nullptr;
    for (SfxViewShell* pViewShell = SfxViewShell::GetFirst(false); pViewShell;
         pViewShell = SfxViewShell::GetNext(*pViewShell, false))
    {
        if (pViewShell != &rCalcViewShell && pViewShell->GetIPClient()
            && KitCalcHelper(pViewShell).GetCalcViewShell() == &rCalcViewShell)
            return pViewShell;
    }
    return nullptr;
}

bool KitCalcHelper::HitAny(const SfxViewShell& rViewShell, const Point& rPosition)
{
    if (!AnyInPlaceView())
        return false;
    for (SfxViewShell* pView = SfxViewShell::GetFirst(false); pView;
         pView = SfxViewShell::GetNext(*pView, false))
    {
        if (pView == &rViewShell || !pView->GetIPClient()
            || pView->GetDocId() != rViewShell.GetDocId()
            || pView->getPart() != rViewShell.getPart())
            continue;
        const KitCalcHelper aHelper(pView);
        if (aHelper.GetCalcViewShell() && aHelper.GetBoundingBox().Contains(rPosition))
            return true;
    }
    return false;
}

namespace
{
/// Maps "x, y, w, h[, more]" rectangles, separated by semicolons, from the sheet into the
/// document. Fields after the fourth, such as the column and row of a cell cursor, stay.
OString mapRectangles(const KitCalcHelper& rHelper, std::string_view rPayload)
{
    if (rPayload.empty() || rPayload == "EMPTY")
        return OString(rPayload);

    // The text being edited in a cell gives its rectangles relative to a point of the sheet, after
    // "::". The rectangles in the document are absolute.
    Point aReference;
    if (const size_t nDelimiter = rPayload.find("::"); nDelimiter != std::string_view::npos)
    {
        const std::string_view aPoint = rPayload.substr(nDelimiter + 2);
        sal_Int32 nIndex = 0;
        const sal_Int64 nX = o3tl::toInt64(o3tl::trim(o3tl::getToken(aPoint, 0, ',', nIndex)));
        const sal_Int64 nY
            = nIndex >= 0 ? o3tl::toInt64(o3tl::trim(o3tl::getToken(aPoint, 0, ',', nIndex))) : 0;
        aReference = Point(nX, nY);
        rPayload = rPayload.substr(0, nDelimiter);
    }

    OStringBuffer aResult;
    sal_Int32 nRectIndex = 0;
    do
    {
        const std::string_view aRect = o3tl::trim(o3tl::getToken(rPayload, 0, ';', nRectIndex));
        if (aRect.empty())
            continue;
        std::vector<std::string_view> aFields;
        sal_Int32 nFieldIndex = 0;
        do
            aFields.push_back(o3tl::trim(o3tl::getToken(aRect, 0, ',', nFieldIndex)));
        while (nFieldIndex >= 0);
        if (aFields.size() < 4)
            return OString(rPayload);

        // the size of a rectangle is the size in the payload, see tools::Rectangle::toString
        const tools::Rectangle aInSheet(
            Point(aReference.X() + o3tl::toInt64(aFields[0]),
                  aReference.Y() + o3tl::toInt64(aFields[1])),
            Size(o3tl::toInt64(aFields[2]), o3tl::toInt64(aFields[3])));
        const tools::Rectangle aInDocument(rHelper.ToDocument(aInSheet));
        if (!aResult.isEmpty())
            aResult.append("; ");
        aResult.append(OString::number(aInDocument.Left()) + ", "
                       + OString::number(aInDocument.Top()) + ", "
                       + OString::number(aInDocument.GetWidth()) + ", "
                       + OString::number(aInDocument.GetHeight()));
        for (size_t i = 4; i < aFields.size(); ++i)
            aResult.append(OString::Concat(", ") + aFields[i]);
    } while (nRectIndex >= 0);
    return aResult.makeStringAndClear();
}

/// The callback of a spreadsheet view that is active in place. It sends the cursors and the
/// selections of the spreadsheet to the container view, in the coordinates of the document, as
/// reports of the container view. The rest only concerns a client of the spreadsheet itself.
class InPlaceCalcViewCallback final : public CoKitCallbackInterface
{
    SfxViewShell& mrCalcViewShell;
    cpo::uno::WeakReference<css::frame::XFrame> mxFrame;

    /// The container view, while the spreadsheet is active in it.
    SfxViewShell* containerViewShell() const
    {
        cpo::uno::Reference<css::frame::XFrame> xFrame(mxFrame);
        if (!xFrame)
            return nullptr;
        cpo::uno::Reference<css::frame::XFrame> xCreatorFrame(xFrame->getCreator());
        SfxViewShell* pViewShell
            = xCreatorFrame ? SfxViewShell::Get(xCreatorFrame->getController()) : nullptr;
        if (!pViewShell)
            pViewShell = KitCalcHelper::FindContainerViewShell(mrCalcViewShell);
        if (!pViewShell || KitCalcHelper(pViewShell).GetCalcViewShell() != &mrCalcViewShell)
            return nullptr;
        return pViewShell;
    }

    static bool isForwarded(COKitCallbackType eType)
    {
        switch (eType)
        {
            // the selected cells are also given as a text selection, which the container shows
            case COKitCallbackType::CELL_CURSOR:
            case COKitCallbackType::TEXT_SELECTION:
            case COKitCallbackType::TEXT_SELECTION_START:
            case COKitCallbackType::TEXT_SELECTION_END:
            case COKitCallbackType::INVALIDATE_VISIBLE_CURSOR:
            case COKitCallbackType::CURSOR_VISIBLE:
            case COKitCallbackType::MOUSE_POINTER:
            // the commands go to the spreadsheet while it is active, so their states are its own
            case COKitCallbackType::STATE_CHANGED:
                return true;
            default:
                return false;
        }
    }

    /// Whether the payload of the type is a list of rectangles.
    static bool hasRectangles(COKitCallbackType eType)
    {
        switch (eType)
        {
            case COKitCallbackType::CELL_CURSOR:
            case COKitCallbackType::TEXT_SELECTION:
            case COKitCallbackType::TEXT_SELECTION_START:
            case COKitCallbackType::TEXT_SELECTION_END:
                return true;
            default:
                return false;
        }
    }

    static void dropped(COKitCallbackType eType)
    {
        SAL_INFO("sfx.view", "InPlaceCalcViewCallback: not sent on: "
                                 << kitCallbackTypeToString(eType));
    }

public:
    InPlaceCalcViewCallback(SfxViewShell& rCalcViewShell,
                            const cpo::uno::Reference<css::frame::XFrame>& rFrame)
        : mrCalcViewShell(rCalcViewShell)
        , mxFrame(rFrame)
    {
        ++g_nInPlaceCalcViews;
    }

    ~InPlaceCalcViewCallback() override { --g_nInPlaceCalcViews; }

    void viewCallback(COKitCallbackType eType, const OString& rPayload) override
    {
        if (!isForwarded(eType))
            return dropped(eType);
        // whether the document is modified, and saving it, are the container's
        if (eType == COKitCallbackType::STATE_CHANGED
            && (rPayload.indexOf(".uno:ModifiedStatus") >= 0 || rPayload.indexOf(".uno:Save=") >= 0
                || rPayload.indexOf("\".uno:Save\"") >= 0))
            return dropped(eType);
        SfxViewShell* pContainer = containerViewShell();
        if (!pContainer)
            return dropped(eType);
        if (hasRectangles(eType))
            pContainer->viewCallback(eType, mapRectangles(KitCalcHelper(pContainer), rPayload));
        else
            pContainer->viewCallback(eType, rPayload);
    }

    void viewCallbackWithViewId(COKitCallbackType eType, const OString& rPayload,
                                int nViewId) override
    {
        // only the own cursor of the spreadsheet view; the other views of the spreadsheet are
        // not known to the client
        if (nViewId != KitHelper::getView(mrCalcViewShell) || !isForwarded(eType))
            return dropped(eType);
        SfxViewShell* pContainer = containerViewShell();
        if (!pContainer)
            return dropped(eType);

        boost::property_tree::ptree aTree;
        try
        {
            std::stringstream aStream((std::string(rPayload)));
            boost::property_tree::read_json(aStream, aTree);
        }
        catch (const boost::property_tree::json_parser_error&)
        {
            return dropped(eType);
        }
        const KitCalcHelper aHelper(pContainer);
        const int nContainerViewId = KitHelper::getView(*pContainer);
        aTree.put("viewId", nContainerViewId);
        if (aTree.get_child_optional("part"))
            aTree.put("part", pContainer->getPart());
        // the cursor of a cell is also given relative to a point of the sheet, which the
        // document does not know
        aTree.erase("relrect");
        aTree.erase("refpoint");
        if (boost::optional<std::string> oRectangle
            = aTree.get_optional<std::string>("rectangle"))
            aTree.put("rectangle", std::string(mapRectangles(aHelper, *oRectangle)));
        if (boost::optional<std::string> oSelection
            = aTree.get_optional<std::string>("selection"))
            aTree.put("selection", std::string(mapRectangles(aHelper, *oSelection)));
        std::stringstream aStream;
        boost::property_tree::write_json(aStream, aTree, false);
        pContainer->viewCallbackWithViewId(eType, OString(o3tl::trim(aStream.str())),
                                           nContainerViewId);
    }

    void viewInvalidateTilesCallback(const tools::Rectangle* pRect, int /*nPart*/,
                                     int /*nMode*/) override
    {
        SfxViewShell* pContainer = containerViewShell();
        if (!pContainer)
            return;
        const KitCalcHelper aHelper(pContainer);
        tools::Rectangle aArea(aHelper.GetBoundingBox());
        if (pRect && !pRect->IsEmpty())
            aArea.Intersection(aHelper.ToDocument(*pRect));
        if (aArea.IsEmpty())
            return;
        // every view of the document on the part, in the same mode, shows the spreadsheet as it
        // is being edited; the view of the spreadsheet has the id of that document too
        const int nPart = pContainer->getPart();
        const int nMode = pContainer->getEditMode();
        for (SfxViewShell* pView = SfxViewShell::GetFirst(false); pView;
             pView = SfxViewShell::GetNext(*pView, false))
        {
            if (pView->GetDocId() == pContainer->GetDocId() && pView->getPart() == nPart
                && pView->getEditMode() == nMode && !KitCalcHelper::IsInPlaceView(*pView))
                pView->viewInvalidateTilesCallback(&aArea, nPart, nMode);
        }
        // a container that draws from the objects shows the spreadsheet itself
        if (SfxInPlaceClient* pIPClient = pContainer->GetIPClient())
            pIPClient->ObjectContentChanged();
    }

    void viewUpdatedCallback(COKitCallbackType eType) override
    {
        // the spreadsheet view gives only its cell cursor as an updated payload
        if (eType != COKitCallbackType::CELL_CURSOR)
            return dropped(eType);
        if (std::optional<OString> oPayload
            = mrCalcViewShell.getKitPayload(eType, KitHelper::getView(mrCalcViewShell)))
            viewCallback(eType, *oPayload);
    }

    void viewUpdatedCallbackPerViewId(COKitCallbackType eType, int nViewId,
                                      int nSourceViewId) override
    {
        if (nViewId != KitHelper::getView(mrCalcViewShell)
            || nSourceViewId != KitHelper::getView(mrCalcViewShell)
            || eType != COKitCallbackType::CELL_CURSOR)
            return dropped(eType);
        if (std::optional<OString> oPayload = mrCalcViewShell.getKitPayload(eType, nViewId))
            viewCallbackWithViewId(eType, *oPayload, nViewId);
    }

    void viewVectorPartChanged(const OString& /*rPartId*/, int /*nMode*/) override {}

    void viewAddPendingInvalidateTiles() override
    {
        mrCalcViewShell.flushPendingKitInvalidateTiles();
    }

    void dumpState(rtl::OStringBuffer& rState) override
    {
        rState.append("\n        InPlaceCalcViewCallback");
    }
};
}

std::unique_ptr<CoKitCallbackInterface>
KitCalcHelper::CreateViewCallback(SfxViewShell& rCalcViewShell,
                                  const cpo::uno::Reference<css::frame::XFrame>& rFrame)
{
    return std::make_unique<InPlaceCalcViewCallback>(rCalcViewShell, rFrame);
}

bool KitCalcHelper::IsInPlaceView(const SfxViewShell& rViewShell)
{
    return dynamic_cast<InPlaceCalcViewCallback*>(rViewShell.getCOKitViewCallback()) != nullptr;
}

KitCalcHelper::ViewGuard::ViewGuard(const vcl::Window* pWindow)
{
    const SfxViewShell* pCalcViewShell = FindCalcViewShell(pWindow);
    if (!pCalcViewShell || pCalcViewShell == SfxViewShell::Current())
        return;
    mnPreviousView = KitHelper::getCurrentView();
    KitHelper::setView(KitHelper::getView(*pCalcViewShell));
}

KitCalcHelper::ViewGuard::~ViewGuard()
{
    if (mnPreviousView < 0 || !KitHelper::getViewOfId(mnPreviousView))
        return;
    // the states of the spreadsheet are given while its view is current
    if (SfxViewShell* pCalcViewShell = SfxViewShell::Current())
        pCalcViewShell->GetViewFrame().GetBindings().Update();
    KitHelper::setView(mnPreviousView);
}

void KitCalcHelper::SendCursor(const SfxViewShell& rCalcViewShell)
{
    if (CoKitCallbackInterface* pCallback = rCalcViewShell.getCOKitViewCallback())
        pCallback->viewUpdatedCallback(COKitCallbackType::CELL_CURSOR);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
