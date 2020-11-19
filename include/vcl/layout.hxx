/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_VCL_LAYOUT_HXX
#define INCLUDED_VCL_LAYOUT_HXX

#include <config_options.h>
#include <vcl/dllapi.h>
#include <vcl/ctrl.hxx>
#include <vcl/help.hxx>
#include <vcl/svapp.hxx>
#include <vcl/window.hxx>
#include <vcl/settings.hxx>
#include <vcl/event.hxx>
#include <vcl/transfer.hxx>
#include <vcl/vclptr.hxx>
#include <vcl/IContext.hxx>
#include <vcl/commandevent.hxx>
#include <set>

class ScrollBar;
class ScrollBarBox;
class Splitter;

class VCL_DLLPUBLIC VclContainer : public vcl::Window,
                                   public vcl::IContext
{
public:
    VclContainer(vcl::Window *pParent, WinBits nStyle = WB_HIDE | WB_CLIPCHILDREN);

    //These take into account the external margins of the rWindow widget
    //while GetOptimalSize/get_preferred_size and SetPosSizePixel are
    //oblivious to them
    static Size getLayoutRequisition(const vcl::Window &rWindow);
    static void setLayoutPosSize(vcl::Window &rWindow, const Point &rPos, const Size &rSize);

    //applies the allocation pos and size onto rWindow via setLayoutPosSize taking into account
    //the rWindows alignment desires within that allocation
    static void setLayoutAllocation(vcl::Window &rWindow, const Point &rPos, const Size &rSize);

    virtual void queue_resize(StateChangedType eReason = StateChangedType::Layout) override;
protected:
    //these are the two that need to be implemented by
    //containers, figure out how much space you want...
    virtual Size calculateRequisition() const = 0;
    //..and decide what to do when set to this size
    virtual void setAllocation(const Size &rAllocation) = 0;

    virtual sal_uInt16 getDefaultAccessibleRole() const override;

    // support for screenshot context menu
    virtual void Command(const CommandEvent& rCEvt) override;

public:
    //you don't want to override these
    virtual Size GetOptimalSize() const override;
    virtual void SetPosSizePixel(const Point& rNewPos, const Size& rNewSize) override;
    virtual void SetPosPixel(const Point& rAllocPos) override;
    virtual void SetSizePixel(const Size& rAllocation) override;
private:
    bool m_bLayoutDirty;
};

class VCL_DLLPUBLIC VclBox : public VclContainer
{
protected:
    bool m_bHomogeneous;
    bool m_bVerticalContainer;
    int m_nSpacing;
public:
    VclBox(vcl::Window *pParent, bool bHomogeneous, int nSpacing)
        : VclContainer(pParent)
        , m_bHomogeneous(bHomogeneous)
        , m_bVerticalContainer(false)
        , m_nSpacing(nSpacing)
    {
    }
    void set_spacing(int nSpacing)
    {
        m_nSpacing = nSpacing;
    }
    int get_spacing() const
    {
        return m_nSpacing;
    }
    void set_homogeneous(bool bHomogeneous)
    {
        m_bHomogeneous = bHomogeneous;
    }
    bool get_orientation() const
    {
        return m_bVerticalContainer;
    }
    virtual bool set_property(const OString &rKey, const OUString &rValue) override;
    virtual void DumpAsPropertyTree(tools::JsonWriter&) override;
protected:
    virtual sal_uInt16 getDefaultAccessibleRole() const override;
    void accumulateMaxes(const Size &rChildSize, Size &rSize) const;
    Size finalizeMaxes(const Size &rSize, sal_uInt16 nVisibleChildren) const;

    virtual Size calculateRequisition() const override;
    virtual void setAllocation(const Size &rAllocation) override;

    virtual tools::Long getPrimaryDimension(const Size &rSize) const = 0;
    virtual void setPrimaryDimension(Size &rSize, tools::Long) const = 0;
    virtual tools::Long getPrimaryCoordinate(const Point &rPos) const = 0;
    virtual void setPrimaryCoordinate(Point &rPos, tools::Long) const = 0;
    virtual tools::Long getSecondaryDimension(const Size &rSize) const = 0;
    virtual void setSecondaryDimension(Size &rSize, tools::Long) const = 0;

    virtual bool getPrimaryDimensionChildExpand(const vcl::Window &rWindow) const = 0;
};

class VCL_DLLPUBLIC VclVBox : public VclBox
{
public:
    VclVBox(vcl::Window *pParent, bool bHomogeneous = false, int nSpacing = 0)
        : VclBox(pParent, bHomogeneous, nSpacing)
    {
        m_bVerticalContainer = true;
    }
protected:
    virtual tools::Long getPrimaryDimension(const Size &rSize) const override
    {
        return rSize.getHeight();
    }
    virtual void setPrimaryDimension(Size &rSize, tools::Long nHeight) const override
    {
        rSize.setHeight(nHeight);
    }
    virtual tools::Long getPrimaryCoordinate(const Point &rPos) const override
    {
        return rPos.getY();
    }
    virtual void setPrimaryCoordinate(Point &rPos, tools::Long nPos) const override
    {
        rPos.setY(nPos);
    }
    virtual tools::Long getSecondaryDimension(const Size &rSize) const override
    {
        return rSize.getWidth();
    }
    virtual void setSecondaryDimension(Size &rSize, tools::Long nWidth) const override
    {
        rSize.setWidth(nWidth);
    }
    virtual bool getPrimaryDimensionChildExpand(const vcl::Window &rWindow) const override
    {
        return rWindow.get_expand() || rWindow.get_vexpand();
    }
};

class VCL_DLLPUBLIC VclHBox : public VclBox
{
public:
    VclHBox(vcl::Window *pParent, bool bHomogeneous = false, int nSpacing = 0)
        : VclBox(pParent, bHomogeneous, nSpacing)
    {
        m_bVerticalContainer = false;
    }
protected:
    virtual tools::Long getPrimaryDimension(const Size &rSize) const override
    {
        return rSize.getWidth();
    }
    virtual void setPrimaryDimension(Size &rSize, tools::Long nWidth) const override
    {
        rSize.setWidth(nWidth);
    }
    virtual tools::Long getPrimaryCoordinate(const Point &rPos) const override
    {
        return rPos.getX();
    }
    virtual void setPrimaryCoordinate(Point &rPos, tools::Long nPos) const override
    {
        rPos.setX(nPos);
    }
    virtual tools::Long getSecondaryDimension(const Size &rSize) const override
    {
        return rSize.getHeight();
    }
    virtual void setSecondaryDimension(Size &rSize, tools::Long nHeight) const override
    {
        rSize.setHeight(nHeight);
    }
    virtual bool getPrimaryDimensionChildExpand(const vcl::Window &rWindow) const override
    {
        return rWindow.get_expand() || rWindow.get_hexpand();
    }
};

enum class VclButtonBoxStyle
{
    Default,
    Spread,
    Edge,
    Start,
    End,
    Center
};

class VCL_DLLPUBLIC VclButtonBox : public VclBox
{
public:
    VclButtonBox(vcl::Window *pParent)
        : VclBox(pParent, false, Application::GetSettings().GetStyleSettings().GetDialogStyle().button_spacing)
        , m_eLayoutStyle(VclButtonBoxStyle::Default)
    {
    }
    virtual bool set_property(const OString &rKey, const OUString &rValue) override;
protected:
    virtual Size calculateRequisition() const override;
    virtual void setAllocation(const Size &rAllocation) override;
    Size addSpacing(const Size &rSize, sal_uInt16 nVisibleChildren) const;
private:
    VclButtonBoxStyle m_eLayoutStyle;
    struct Requisition
    {
        std::vector<tools::Long> m_aMainGroupDimensions;
        std::vector<tools::Long> m_aSubGroupDimensions;
        Size m_aMainGroupSize;
        Size m_aSubGroupSize;
    };
    Requisition calculatePrimarySecondaryRequisitions() const;
    Size addReqGroups(const VclButtonBox::Requisition &rReq) const;
};

class VCL_DLLPUBLIC VclVButtonBox : public VclButtonBox
{
public:
    VclVButtonBox(vcl::Window *pParent)
        : VclButtonBox(pParent)
    {
        m_bVerticalContainer = true;
    }
protected:
    virtual tools::Long getPrimaryDimension(const Size &rSize) const override
    {
        return rSize.getHeight();
    }
    virtual void setPrimaryDimension(Size &rSize, tools::Long nHeight) const override
    {
        rSize.setHeight(nHeight);
    }
    virtual tools::Long getPrimaryCoordinate(const Point &rPos) const override
    {
        return rPos.getY();
    }
    virtual void setPrimaryCoordinate(Point &rPos, tools::Long nPos) const override
    {
        rPos.setY(nPos);
    }
    virtual tools::Long getSecondaryDimension(const Size &rSize) const override
    {
        return rSize.getWidth();
    }
    virtual void setSecondaryDimension(Size &rSize, tools::Long nWidth) const override
    {
        rSize.setWidth(nWidth);
    }
    virtual bool getPrimaryDimensionChildExpand(const vcl::Window &rWindow) const override
    {
        return rWindow.get_expand() || rWindow.get_vexpand();
    }
};

class VCL_DLLPUBLIC VclHButtonBox final : public VclButtonBox
{
public:
    VclHButtonBox(vcl::Window *pParent)
        : VclButtonBox(pParent)
    {
        m_bVerticalContainer = false;
    }
private:
    virtual tools::Long getPrimaryDimension(const Size &rSize) const override
    {
        return rSize.getWidth();
    }
    virtual void setPrimaryDimension(Size &rSize, tools::Long nWidth) const override
    {
        rSize.setWidth(nWidth);
    }
    virtual tools::Long getPrimaryCoordinate(const Point &rPos) const override
    {
        return rPos.getX();
    }
    virtual void setPrimaryCoordinate(Point &rPos, tools::Long nPos) const override
    {
        rPos.setX(nPos);
    }
    virtual tools::Long getSecondaryDimension(const Size &rSize) const override
    {
        return rSize.getHeight();
    }
    virtual void setSecondaryDimension(Size &rSize, tools::Long nHeight) const override
    {
        rSize.setHeight(nHeight);
    }
    virtual bool getPrimaryDimensionChildExpand(const vcl::Window &rWindow) const override
    {
        return rWindow.get_expand() || rWindow.get_hexpand();
    }
};

class VCL_DLLPUBLIC VclGrid final : public VclContainer
{
private:
    bool m_bRowHomogeneous;
    bool m_bColumnHomogeneous;
    int m_nRowSpacing;
    int m_nColumnSpacing;

public:
    struct Value
    {
        tools::Long m_nValue;
        bool m_bExpand;
        Value() : m_nValue(0), m_bExpand(false) {}
    };
private:

    Size calculateRequisitionForSpacings(sal_Int32 nRowSpacing, sal_Int32 nColSpacing) const;
    virtual Size calculateRequisition() const override;
    virtual void setAllocation(const Size &rAllocation) override;
    virtual void DumpAsPropertyTree(tools::JsonWriter&) override;
public:
    VclGrid(vcl::Window *pParent)
        : VclContainer(pParent)
        , m_bRowHomogeneous(false), m_bColumnHomogeneous(false)
        , m_nRowSpacing(0), m_nColumnSpacing(0)
    {
    }
    bool get_row_homogeneous() const
    {
        return m_bRowHomogeneous;
    }
    bool get_column_homogeneous() const
    {
        return m_bColumnHomogeneous;
    }
    void set_row_spacing(int nSpacing)
    {
        m_nRowSpacing = nSpacing;
    }
    void set_column_spacing(int nSpacing)
    {
        m_nColumnSpacing = nSpacing;
    }
    int get_row_spacing() const
    {
        return m_nRowSpacing;
    }
    int get_column_spacing() const
    {
        return m_nColumnSpacing;
    }
    virtual bool set_property(const OString &rKey, const OUString &rValue) override;
};

class UNLESS_MERGELIBS(VCL_DLLPUBLIC) VclBin : public VclContainer
{
public:
    VclBin(vcl::Window *pParent, WinBits nStyle = WB_HIDE | WB_CLIPCHILDREN)
        : VclContainer(pParent, nStyle)
    {
    }
    virtual vcl::Window *get_child();
    virtual const vcl::Window *get_child() const;
    virtual Size calculateRequisition() const override;
    virtual void setAllocation(const Size &rAllocation) override;
};

class VclPaned : public VclContainer
{
protected:
    VclPtr<Splitter> m_pSplitter;
    tools::Long m_nPosition;

    VclPaned(vcl::Window *pParent, bool bVertical);
public:
    virtual ~VclPaned() override;
    virtual void dispose() override;
    tools::Long get_position() const { return m_nPosition; }
    virtual void set_position(tools::Long nPosition) { m_nPosition = nPosition; }
};

class VclVPaned final : public VclPaned
{
private:
    DECL_LINK(SplitHdl, Splitter*, void);
    void arrange(const Size& rAllocation, tools::Long nFirstHeight, tools::Long nSecondHeight);

public:
    VclVPaned(vcl::Window *pParent);
    virtual ~VclVPaned() override;
    virtual Size calculateRequisition() const override;
    virtual void setAllocation(const Size &rAllocation) override;
    virtual void set_position(tools::Long nPosition) override;
};

class VclHPaned final : public VclPaned
{
private:
    DECL_LINK(SplitHdl, Splitter*, void);
    void arrange(const Size& rAllocation, tools::Long nFirstHeight, tools::Long nSecondHeight);

public:
    VclHPaned(vcl::Window *pParent);
    virtual ~VclHPaned() override;
    virtual Size calculateRequisition() const override;
    virtual void setAllocation(const Size &rAllocation) override;
    virtual void set_position(tools::Long nPosition) override;
};

class VclFrame final : public VclBin
{
private:
    VclPtr<vcl::Window> m_pLabel;
private:
    friend class VclBuilder;
    void designate_label(vcl::Window *pWindow);
    DECL_LINK(WindowEventListener, VclWindowEvent&, void);
public:
    VclFrame(vcl::Window *pParent)
        : VclBin(pParent)
        , m_pLabel(nullptr)
    {
    }
    virtual ~VclFrame() override;
    virtual void dispose() override;
    void set_label(const OUString &rLabel);
    OUString get_label() const;
    virtual vcl::Window *get_child() override;
    virtual const vcl::Window *get_child() const override;
    vcl::Window *get_label_widget();
    const vcl::Window *get_label_widget() const;
    virtual void DumpAsPropertyTree(tools::JsonWriter&) override;
private:
    virtual Size calculateRequisition() const override;
    virtual void setAllocation(const Size &rAllocation) override;
    virtual OUString getDefaultAccessibleName() const override;
};

class UNLESS_MERGELIBS(VCL_DLLPUBLIC) VclAlignment final : public VclBin
{
public:
    VclAlignment(vcl::Window *pParent)
        : VclBin(pParent)
        , m_nBottomPadding(0)
        , m_nLeftPadding(0)
        , m_nRightPadding(0)
        , m_nTopPadding(0)
    {
    }
    virtual bool set_property(const OString &rKey, const OUString &rValue) override;
private:
    virtual Size calculateRequisition() const override;
    virtual void setAllocation(const Size &rAllocation) override;
    sal_Int32 m_nBottomPadding;
    sal_Int32 m_nLeftPadding;
    sal_Int32 m_nRightPadding;
    sal_Int32 m_nTopPadding;
};

class DisclosureButton;
class CheckBox;

class VclExpander final : public VclBin
{
public:
    VclExpander(vcl::Window *pParent);
    virtual ~VclExpander() override;
    virtual void dispose() override;
    virtual vcl::Window *get_child() override;
    virtual const vcl::Window *get_child() const override;
    virtual bool set_property(const OString &rKey, const OUString &rValue) override;
    bool get_expanded() const;
    void set_expanded(bool bExpanded);
    void set_label(const OUString& rLabel);
    OUString get_label() const;
    vcl::Window *get_label_widget();
    const vcl::Window *get_label_widget() const;
    virtual void StateChanged(StateChangedType nType) override;
    void  SetExpandedHdl( const Link<VclExpander&,void>& rLink ) { maExpandedHdl = rLink; }
private:
    virtual Size calculateRequisition() const override;
    virtual void setAllocation(const Size &rAllocation) override;
    bool m_bResizeTopLevel;
    VclPtr<DisclosureButton> m_pDisclosureButton;
    Link<VclExpander&,void> maExpandedHdl;
    DECL_LINK(ClickHdl, CheckBox&, void);
};

class VCL_DLLPUBLIC VclScrolledWindow final : public VclBin
{
public:
    VclScrolledWindow(vcl::Window *pParent );
    virtual ~VclScrolledWindow() override;
    virtual void dispose() override;
    virtual vcl::Window *get_child() override;
    virtual const vcl::Window *get_child() const override;
    virtual bool set_property(const OString &rKey, const OUString &rValue) override;
    virtual void Paint(vcl::RenderContext& rRenderContext, const tools::Rectangle& rRect) override;
    bool HasVisibleBorder() const { return m_eDrawFrameStyle != DrawFrameStyle::NONE; }
    ScrollBar& getVertScrollBar() { return *m_pVScroll; }
    ScrollBar& getHorzScrollBar() { return *m_pHScroll; }
    Size getVisibleChildSize() const;
    //set to true to disable the built-in scrolling callbacks to allow the user
    //to override it
    void setUserManagedScrolling(bool bUserManagedScrolling) { m_bUserManagedScrolling = bUserManagedScrolling;}
    void doSetAllocation(const Size &rAllocation, bool bRetryOnFailure);
private:
    virtual Size calculateRequisition() const override;
    virtual void setAllocation(const Size &rAllocation) override;
    // sets new border size and adapts scrollbar and child widget position/size as needed
    void updateBorderWidth(tools::Long nBorderWidth);
    DECL_LINK(ScrollBarHdl, ScrollBar*, void);
    void InitScrollBars(const Size &rRequest);
    virtual bool EventNotify(NotifyEvent& rNEvt) override;
    bool m_bUserManagedScrolling;
    tools::Long m_nBorderWidth;
    DrawFrameStyle m_eDrawFrameStyle;
    DrawFrameFlags m_eDrawFrameFlags;
    VclPtr<ScrollBar> m_pVScroll;
    VclPtr<ScrollBar> m_pHScroll;
    VclPtr<ScrollBarBox> m_aScrollBarBox;
};

class VclViewport final : public VclBin
{
public:
    VclViewport(vcl::Window *pParent)
        : VclBin(pParent, WB_HIDE | WB_CLIPCHILDREN)
        , m_bInitialAllocation(true)
    {
    }
private:
    virtual void setAllocation(const Size &rAllocation) override;
    bool m_bInitialAllocation;
};

//Enforces that its children are always the same size as itself.
//Intercepts any Commands intended for its children.
//
//by default the Commands are discarded, inherit from this
//and implement "Command" to get them
class VclEventBox final : public VclBin
{
private:
    //Any Commands an EventBoxHelper receives are forwarded to its parent
    //The VclEventBox ensures that m_aEventBoxHelper is the
    //first child and is transparent, but covers the rest of the children
    class EventBoxHelper : public vcl::Window
    {
    public:
        EventBoxHelper(vcl::Window* pParent)
            : Window(pParent, 0)
        {
            SetSizePixel(pParent->GetSizePixel());
            EnableChildTransparentMode();
            SetPaintTransparent(true);
            SetBackground();
        }
        virtual void Command(const CommandEvent& rCEvt) override
        {
            GetParent()->Command(rCEvt);
        }
    };

    VclPtr<EventBoxHelper> m_aEventBoxHelper;
    virtual void dispose() override;
    virtual ~VclEventBox() override;
public:
    VclEventBox(vcl::Window* pParent)
        : VclBin(pParent)
        , m_aEventBoxHelper(VclPtr<EventBoxHelper>::Create(this))
    {
        m_aEventBoxHelper->Show();
    }
    virtual vcl::Window *get_child() override;
    virtual const vcl::Window *get_child() const override;
    virtual Size calculateRequisition() const override;
    virtual void setAllocation(const Size &rAllocation) override;

    virtual void Command(const CommandEvent& rCEvt) override;
};

class VclSizeGroup
{
private:
    std::set< VclPtr<vcl::Window> > m_aWindows;
    bool m_bIgnoreHidden;
    VclSizeGroupMode m_eMode;

    void trigger_queue_resize();
public:
    VclSizeGroup()
        : m_bIgnoreHidden(false)
        , m_eMode(VclSizeGroupMode::Horizontal)
    {
    }
    void insert(vcl::Window *pWindow)
    {
        m_aWindows.insert(VclPtr<vcl::Window>(pWindow));
    }
    void erase(vcl::Window *pWindow)
    {
        m_aWindows.erase(VclPtr<vcl::Window>(pWindow));
    }
    const std::set< VclPtr<vcl::Window> >& get_widgets() const
    {
        return m_aWindows;
    }
    std::set< VclPtr<vcl::Window> >& get_widgets()
    {
        return m_aWindows;
    }
    void set_ignore_hidden(bool bIgnoreHidden);
    bool get_ignore_hidden() const
    {
        return m_bIgnoreHidden;
    }
    void set_mode(VclSizeGroupMode eMode);
    VclSizeGroupMode get_mode() const
    {
        return m_eMode;
    }
    void set_property(const OString &rKey, const OUString &rValue);
};

class VCL_DLLPUBLIC VclDrawingArea final : public Control
                                         , public DragSourceHelper
{
private:
    FactoryFunction m_pFactoryFunction;
    void* m_pUserData;
    rtl::Reference<TransferDataContainer> m_xTransferHelper;
    sal_Int8 m_nDragAction;
    Link<std::pair<vcl::RenderContext&, const tools::Rectangle&>, void> m_aPaintHdl;
    Link<const Size&, void> m_aResizeHdl;
    Link<const MouseEvent&, bool> m_aMousePressHdl;
    Link<const MouseEvent&, bool> m_aMouseMotionHdl;
    Link<const MouseEvent&, bool> m_aMouseReleaseHdl;
    Link<const KeyEvent&, bool> m_aKeyPressHdl;
    Link<const KeyEvent&, bool> m_aKeyReleaseHdl;
    Link<VclDrawingArea&, void> m_aStyleUpdatedHdl;
    Link<const CommandEvent&, bool> m_aCommandHdl;
    Link<tools::Rectangle&, OUString> m_aQueryTooltipHdl;
    Link<OUString&, int> m_aGetSurroundingHdl;
    Link<const Selection&, bool> m_aDeleteSurroundingHdl;
    Link<VclDrawingArea*, bool> m_aStartDragHdl;

    virtual void Paint(vcl::RenderContext& rRenderContext, const tools::Rectangle& rRect) override
    {
        m_aPaintHdl.Call(std::pair<vcl::RenderContext&, const tools::Rectangle&>(rRenderContext, rRect));
    }
    virtual void Resize() override
    {
        m_aResizeHdl.Call(GetOutputSizePixel());
    }
    virtual void MouseMove(const MouseEvent& rMEvt) override
    {
        if (!m_aMouseMotionHdl.Call(rMEvt))
            Control::MouseMove(rMEvt);
    }
    virtual void KeyInput(const KeyEvent& rKEvt) override
    {
        if (!m_aKeyPressHdl.Call(rKEvt))
            Control::KeyInput(rKEvt);

    }
    virtual void KeyUp(const KeyEvent& rKEvt) override
    {
        if (!m_aKeyReleaseHdl.Call(rKEvt))
            Control::KeyUp(rKEvt);
    }
    virtual void StateChanged(StateChangedType nType) override
    {
        Control::StateChanged(nType);
        if (nType == StateChangedType::ControlForeground || nType == StateChangedType::ControlBackground)
        {
            m_aStyleUpdatedHdl.Call(*this);
            Invalidate();
        }
    }
    virtual void DataChanged(const DataChangedEvent& rDCEvt) override
    {
        Control::DataChanged(rDCEvt);
        if ((rDCEvt.GetType() == DataChangedEventType::SETTINGS) && (rDCEvt.GetFlags() & AllSettingsFlags::STYLE))
        {
            m_aStyleUpdatedHdl.Call(*this);
            Invalidate();
        }
    }
    virtual void Command(const CommandEvent& rEvent) override
    {
        if (m_aCommandHdl.Call(rEvent))
            return;
        Control::Command(rEvent);
    }
    virtual void RequestHelp(const HelpEvent& rHelpEvent) override
    {
        if (rHelpEvent.GetMode() & (HelpEventMode::QUICK | HelpEventMode::BALLOON))
        {
            Point aPos(ScreenToOutputPixel(rHelpEvent.GetMousePosPixel()));
            tools::Rectangle aHelpArea(aPos.X(), aPos.Y());
            OUString sHelpTip = m_aQueryTooltipHdl.Call(aHelpArea);
            if (sHelpTip.isEmpty())
                return;
            Point aPt = OutputToScreenPixel(aHelpArea.TopLeft());
            aHelpArea.SetLeft(aPt.X());
            aHelpArea.SetTop(aPt.Y());
            aPt = OutputToScreenPixel(aHelpArea.BottomRight());
            aHelpArea.SetRight(aPt.X());
            aHelpArea.SetBottom(aPt.Y());
            // tdf#125369 recover newline support of tdf#101779
            QuickHelpFlags eHelpWinStyle = sHelpTip.indexOf('\n') != -1 ? QuickHelpFlags::TipStyleBalloon : QuickHelpFlags::NONE;
            Help::ShowQuickHelp(this, aHelpArea, sHelpTip, eHelpWinStyle);
        }
    }
    virtual void StartDrag(sal_Int8 nAction, const Point& rPosPixel) override;
    virtual FactoryFunction GetUITestFactory() const override
    {
        if (m_pFactoryFunction)
            return m_pFactoryFunction;
        return Control::GetUITestFactory();
    }

public:
    VclDrawingArea(vcl::Window *pParent, WinBits nStyle)
        : Control(pParent, nStyle)
        , DragSourceHelper(this)
        , m_pFactoryFunction(nullptr)
        , m_pUserData(nullptr)
        , m_nDragAction(0)
    {
        SetBackground();
    }
    virtual void MouseButtonDown(const MouseEvent& rMEvt) override
    {
        if (!m_aMousePressHdl.Call(rMEvt))
            Control::MouseButtonDown(rMEvt);
    }
    virtual void MouseButtonUp(const MouseEvent& rMEvt) override
    {
        if (!m_aMouseReleaseHdl.Call(rMEvt))
            Control::MouseButtonUp(rMEvt);
    }
    virtual OUString GetSurroundingText() const override;
    virtual Selection GetSurroundingTextSelection() const override;
    virtual bool DeleteSurroundingText(const Selection& rSelection) override;
    void SetUITestFactory(FactoryFunction pFactoryFunction, void* pUserData)
    {
        m_pFactoryFunction = pFactoryFunction;
        m_pUserData = pUserData;
    }
    void* GetUserData() const
    {
        return m_pUserData;
    }
    void SetPaintHdl(const Link<std::pair<vcl::RenderContext&, const tools::Rectangle&>, void>& rLink)
    {
        m_aPaintHdl = rLink;
    }
    void SetResizeHdl(const Link<const Size&, void>& rLink)
    {
        m_aResizeHdl = rLink;
    }
    void SetMousePressHdl(const Link<const MouseEvent&, bool>& rLink)
    {
        m_aMousePressHdl = rLink;
    }
    void SetMouseMoveHdl(const Link<const MouseEvent&, bool>& rLink)
    {
        m_aMouseMotionHdl = rLink;
    }
    void SetMouseReleaseHdl(const Link<const MouseEvent&, bool>& rLink)
    {
        m_aMouseReleaseHdl = rLink;
    }
    void SetKeyPressHdl(const Link<const KeyEvent&, bool>& rLink)
    {
        m_aKeyPressHdl = rLink;
    }
    void SetKeyReleaseHdl(const Link<const KeyEvent&, bool>& rLink)
    {
        m_aKeyReleaseHdl = rLink;
    }
    void SetStyleUpdatedHdl(const Link<VclDrawingArea&, void>& rLink)
    {
        m_aStyleUpdatedHdl = rLink;
    }
    void SetCommandHdl(const Link<const CommandEvent&, bool>& rLink)
    {
        m_aCommandHdl = rLink;
    }
    void SetQueryTooltipHdl(const Link<tools::Rectangle&, OUString>& rLink)
    {
        m_aQueryTooltipHdl = rLink;
    }
    void SetGetSurroundingHdl(const Link<OUString&, int>& rLink)
    {
        m_aGetSurroundingHdl = rLink;
    }
    void SetDeleteSurroundingHdl(const Link<const Selection&, bool>& rLink)
    {
        m_aDeleteSurroundingHdl = rLink;
    }
    void SetStartDragHdl(const Link<VclDrawingArea*, bool>& rLink)
    {
        m_aStartDragHdl = rLink;
    }
    void SetDragHelper(const rtl::Reference<TransferDataContainer>& rHelper, sal_uInt8 eDNDConstants)
    {
        m_xTransferHelper = rHelper;
        m_nDragAction = eDNDConstants;
    }
    virtual void DumpAsPropertyTree(tools::JsonWriter&) override;
};

//Get first window of a pTopLevel window as
//if any intermediate layout widgets didn't exist
//i.e. acts like pChild = pChild->GetWindow(GetWindowType::FirstChild);
//in a flat hierarchy where dialogs only have one layer
//of children
vcl::Window* firstLogicalChildOfParent(const vcl::Window *pTopLevel);

//Get last window of a pTopLevel window as
//if any intermediate layout widgets didn't exist
//i.e. acts like pChild = pChild->GetWindow(GetWindowType::LastChild);
//in a flat hierarchy where dialogs only have one layer
//of children
vcl::Window* lastLogicalChildOfParent(const vcl::Window *pTopLevel);

//Get next window after pChild of a pTopLevel window as
//if any intermediate layout widgets didn't exist
//i.e. acts like pChild = pChild->GetWindow(GetWindowType::Next);
//in a flat hierarchy where dialogs only have one layer
//of children
vcl::Window* nextLogicalChildOfParent(const vcl::Window *pTopLevel, const vcl::Window *pChild);

//Get previous window before pChild of a pTopLevel window as
//if any intermediate layout widgets didn't exist
//i.e. acts like pChild = pChild->GetWindow(GetWindowType::Prev);
//in a flat hierarchy where dialogs only have one layer
//of children
vcl::Window* prevLogicalChildOfParent(const vcl::Window *pTopLevel, const vcl::Window *pChild);

//Returns true is the Window has a single child which is a container
VCL_DLLPUBLIC bool isLayoutEnabled(const vcl::Window *pWindow);

inline bool isContainerWindow(const vcl::Window &rWindow)
{
    WindowType eType = rWindow.GetType();
    return eType == WindowType::CONTAINER || eType == WindowType::SCROLLWINDOW ||
           (eType == WindowType::DOCKINGWINDOW && ::isLayoutEnabled(&rWindow));
}

inline bool isContainerWindow(const vcl::Window *pWindow)
{
    return pWindow && isContainerWindow(*pWindow);
}

// retro-fitting utilities

//Get a Size which is large enough to contain all children with
//an equal amount of space at top left and bottom right
Size getLegacyBestSizeForChildren(const vcl::Window &rWindow);

//Get first parent which is not a layout widget
vcl::Window* getNonLayoutParent(vcl::Window *pParent);

//Sort ok/cancel etc buttons in platform order
void sort_native_button_order(const VclBox& rContainer);

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
