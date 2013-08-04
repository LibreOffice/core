/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef _VCLLAYOUT_HXX
#define _VCLLAYOUT_HXX

#include <vcl/dllapi.h>
#include <vcl/button.hxx>
#include <vcl/dialog.hxx>
#include <vcl/fixed.hxx>
#include <vcl/scrbar.hxx>
#include <vcl/vclmedit.hxx>
#include <vcl/window.hxx>
#include <boost/multi_array.hpp>
#include <set>

class VCL_DLLPUBLIC VclContainer : public Window
{
public:
    VclContainer(Window *pParent, WinBits nStyle = WB_HIDE);

    //These take into account the external margins of the rWindow widget
    //while GetOptimalSize/get_preferred_size and SetPosSizePixel are
    //oblivious to them
    static Size getLayoutRequisition(const Window &rWindow);
    static void setLayoutPosSize(Window &rWindow, const Point &rPos, const Size &rSize);

    //applies the allocation pos and size onto rWindow via setLayoutPosSize taking into account
    //the rWindows alignment desires within that allocation
    static void setLayoutAllocation(Window &rWindow, const Point &rPos, const Size &rSize);

    void markLayoutDirty()
    {
        m_bLayoutDirty = true;
    }
protected:
    //these are the two that need to be implemented by
    //containers, figure out how much space you want...
    virtual Size calculateRequisition() const = 0;
    //..and decide what to do when set to this size
    virtual void setAllocation(const Size &rAllocation) = 0;

    virtual sal_uInt16 getDefaultAccessibleRole() const;
public:
    //you don't want to override these
    virtual Size GetOptimalSize() const;
    virtual void SetPosSizePixel(const Point& rNewPos, const Size& rNewSize);
    virtual void SetPosPixel(const Point& rAllocPos);
    virtual void SetSizePixel(const Size& rAllocation);
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
    VclBox(Window *pParent, bool bHomogeneous, int nSpacing)
        : VclContainer(pParent)
        , m_bHomogeneous(bHomogeneous)
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
    bool get_homogeneous() const
    {
        return m_bHomogeneous;
    }
    virtual bool set_property(const OString &rKey, const OString &rValue);
protected:
    virtual sal_uInt16 getDefaultAccessibleRole() const;
    void accumulateMaxes(const Size &rChildSize, Size &rSize) const;
    Size finalizeMaxes(const Size &rSize, sal_uInt16 nVisibleChildren) const;

    virtual Size calculateRequisition() const;
    virtual void setAllocation(const Size &rAllocation);

    virtual long getPrimaryDimension(const Size &rSize) const = 0;
    virtual void setPrimaryDimension(Size &rSize, long) const = 0;
    virtual long getPrimaryCoordinate(const Point &rPos) const = 0;
    virtual void setPrimaryCoordinate(Point &rPos, long) const = 0;
    virtual long getSecondaryDimension(const Size &rSize) const = 0;
    virtual void setSecondaryDimension(Size &rSize, long) const = 0;
    virtual long getSecondaryCoordinate(const Point &rPos) const = 0;
    virtual void setSecondaryCoordinate(Point &rPos, long) const = 0;

    virtual bool getPrimaryDimensionChildExpand(const Window &rWindow) const = 0;
};

class VCL_DLLPUBLIC VclVBox : public VclBox
{
public:
    VclVBox(Window *pParent, bool bHomogeneous = false, int nSpacing = 0)
        : VclBox(pParent, bHomogeneous, nSpacing)
    {
        m_bVerticalContainer = true;
    }
protected:
    virtual long getPrimaryDimension(const Size &rSize) const
    {
        return rSize.getHeight();
    }
    virtual void setPrimaryDimension(Size &rSize, long nHeight) const
    {
        rSize.setHeight(nHeight);
    }
    virtual long getPrimaryCoordinate(const Point &rPos) const
    {
        return rPos.getY();
    }
    virtual void setPrimaryCoordinate(Point &rPos, long nPos) const
    {
        rPos.setY(nPos);
    }
    virtual long getSecondaryDimension(const Size &rSize) const
    {
        return rSize.getWidth();
    }
    virtual void setSecondaryDimension(Size &rSize, long nWidth) const
    {
        rSize.setWidth(nWidth);
    }
    virtual long getSecondaryCoordinate(const Point &rPos) const
    {
        return rPos.getX();
    }
    virtual void setSecondaryCoordinate(Point &rPos, long nPos) const
    {
        rPos.setX(nPos);
    }
    virtual bool getPrimaryDimensionChildExpand(const Window &rWindow) const
    {
        return rWindow.get_expand() || rWindow.get_vexpand();
    }
};

class VCL_DLLPUBLIC VclHBox : public VclBox
{
public:
    VclHBox(Window *pParent, bool bHomogeneous = false, int nSpacing = 0)
        : VclBox(pParent, bHomogeneous, nSpacing)
    {
        m_bVerticalContainer = false;
    }
protected:
    virtual long getPrimaryDimension(const Size &rSize) const
    {
        return rSize.getWidth();
    }
    virtual void setPrimaryDimension(Size &rSize, long nWidth) const
    {
        rSize.setWidth(nWidth);
    }
    virtual long getPrimaryCoordinate(const Point &rPos) const
    {
        return rPos.getX();
    }
    virtual void setPrimaryCoordinate(Point &rPos, long nPos) const
    {
        rPos.setX(nPos);
    }
    virtual long getSecondaryDimension(const Size &rSize) const
    {
        return rSize.getHeight();
    }
    virtual void setSecondaryDimension(Size &rSize, long nHeight) const
    {
        rSize.setHeight(nHeight);
    }
    virtual long getSecondaryCoordinate(const Point &rPos) const
    {
        return rPos.getY();
    }
    virtual void setSecondaryCoordinate(Point &rPos, long nPos) const
    {
        rPos.setY(nPos);
    }
    virtual bool getPrimaryDimensionChildExpand(const Window &rWindow) const
    {
        return rWindow.get_expand() || rWindow.get_hexpand();
    }
};

enum VclButtonBoxStyle
{
    VCL_BUTTONBOX_DEFAULT_STYLE,
    VCL_BUTTONBOX_SPREAD,
    VCL_BUTTONBOX_EDGE,
    VCL_BUTTONBOX_START,
    VCL_BUTTONBOX_END,
    VCL_BUTTONBOX_CENTER
};

class VCL_DLLPUBLIC VclButtonBox : public VclBox
{
public:
    VclButtonBox(Window *pParent, int nSpacing)
        : VclBox(pParent, false, nSpacing)
        , m_eLayoutStyle(VCL_BUTTONBOX_DEFAULT_STYLE)
    {
    }
    void set_layout(VclButtonBoxStyle eStyle)
    {
        m_eLayoutStyle = eStyle;
    }
    VclButtonBoxStyle get_layout() const
    {
        return m_eLayoutStyle;
    }
    virtual bool set_property(const OString &rKey, const OString &rValue);
    void sort_native_button_order();
protected:
    virtual Size calculateRequisition() const;
    virtual void setAllocation(const Size &rAllocation);
    Size addSpacing(const Size &rSize, sal_uInt16 nVisibleChildren) const;
private:
    VclButtonBoxStyle m_eLayoutStyle;
    struct Requisition
    {
        std::vector<long> m_aMainGroupDimensions;
        std::vector<long> m_aSubGroupDimensions;
        Size m_aMainGroupSize;
        Size m_aSubGroupSize;
    };
    Requisition calculatePrimarySecondaryRequisitions() const;
    Size addReqGroups(const VclButtonBox::Requisition &rReq) const;
};

class VCL_DLLPUBLIC VclVButtonBox : public VclButtonBox
{
public:
    VclVButtonBox(Window *pParent, int nSpacing = 0)
        : VclButtonBox(pParent, nSpacing)
    {
        m_bVerticalContainer = true;
    }
protected:
    virtual long getPrimaryDimension(const Size &rSize) const
    {
        return rSize.getHeight();
    }
    virtual void setPrimaryDimension(Size &rSize, long nHeight) const
    {
        rSize.setHeight(nHeight);
    }
    virtual long getPrimaryCoordinate(const Point &rPos) const
    {
        return rPos.getY();
    }
    virtual void setPrimaryCoordinate(Point &rPos, long nPos) const
    {
        rPos.setY(nPos);
    }
    virtual long getSecondaryDimension(const Size &rSize) const
    {
        return rSize.getWidth();
    }
    virtual void setSecondaryDimension(Size &rSize, long nWidth) const
    {
        rSize.setWidth(nWidth);
    }
    virtual long getSecondaryCoordinate(const Point &rPos) const
    {
        return rPos.getX();
    }
    virtual void setSecondaryCoordinate(Point &rPos, long nPos) const
    {
        rPos.setX(nPos);
    }
    virtual bool getPrimaryDimensionChildExpand(const Window &rWindow) const
    {
        return rWindow.get_expand() || rWindow.get_vexpand();
    }
};

class VCL_DLLPUBLIC VclHButtonBox : public VclButtonBox
{
public:
    VclHButtonBox(Window *pParent, int nSpacing = 0)
        : VclButtonBox(pParent, nSpacing)
    {
        m_bVerticalContainer = false;
    }
protected:
    virtual long getPrimaryDimension(const Size &rSize) const
    {
        return rSize.getWidth();
    }
    virtual void setPrimaryDimension(Size &rSize, long nWidth) const
    {
        rSize.setWidth(nWidth);
    }
    virtual long getPrimaryCoordinate(const Point &rPos) const
    {
        return rPos.getX();
    }
    virtual void setPrimaryCoordinate(Point &rPos, long nPos) const
    {
        rPos.setX(nPos);
    }
    virtual long getSecondaryDimension(const Size &rSize) const
    {
        return rSize.getHeight();
    }
    virtual void setSecondaryDimension(Size &rSize, long nHeight) const
    {
        rSize.setHeight(nHeight);
    }
    virtual long getSecondaryCoordinate(const Point &rPos) const
    {
        return rPos.getY();
    }
    virtual void setSecondaryCoordinate(Point &rPos, long nPos) const
    {
        rPos.setY(nPos);
    }
    virtual bool getPrimaryDimensionChildExpand(const Window &rWindow) const
    {
        return rWindow.get_expand() || rWindow.get_hexpand();
    }
};

class VCL_DLLPUBLIC VclGrid : public VclContainer
{
private:
    bool m_bRowHomogeneous;
    bool m_bColumnHomogeneous;
    int m_nRowSpacing;
    int m_nColumnSpacing;

    struct GridEntry
    {
        Window *pChild;
        sal_Int32 nSpanWidth;
        sal_Int32 nSpanHeight;
        GridEntry()
            : pChild(0)
            , nSpanWidth(0)
            , nSpanHeight(0)
        {
        }
    };

    typedef boost::multi_array<GridEntry, 2> array_type;

    struct ExtendedGridEntry : GridEntry
    {
        int x;
        int y;
        ExtendedGridEntry()
            : x(-1)
            , y(-1)
        {
        }
    };

    typedef boost::multi_array<ExtendedGridEntry, 2> ext_array_type;

    array_type assembleGrid() const;
    bool isNullGrid(const array_type& A) const;
public:
    struct Value
    {
        long m_nValue;
        bool m_bExpand;
        Value() : m_nValue(0), m_bExpand(false) {}
    };
private:
    void calcMaxs(const array_type &A, std::vector<Value> &rWidths, std::vector<Value> &rHeights) const;

    Size calculateRequisitionForSpacings(sal_Int32 nRowSpacing, sal_Int32 nColSpacing) const;
    virtual Size calculateRequisition() const;
    virtual void setAllocation(const Size &rAllocation);
public:
    VclGrid(Window *pParent)
        : VclContainer(pParent)
        , m_bRowHomogeneous(false), m_bColumnHomogeneous(false)
        , m_nRowSpacing(0), m_nColumnSpacing(0)
    {
    }
    void set_row_homogeneous(bool bHomogeneous)
    {
        m_bRowHomogeneous = bHomogeneous;
    }
    void set_column_homogeneous(bool bHomogeneous)
    {
        m_bColumnHomogeneous = bHomogeneous;
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
    virtual bool set_property(const OString &rKey, const OString &rValue);
};

VCL_DLLPUBLIC void setGridAttach(Window &rWidget, sal_Int32 nLeft, sal_Int32 nTop,
    sal_Int32 nWidth = 1, sal_Int32 nHeight = 1);

class VCL_DLLPUBLIC VclBin : public VclContainer
{
public:
    VclBin(Window *pParent, WinBits nStyle = WB_HIDE)
        : VclContainer(pParent, nStyle)
    {
    }
    virtual Window *get_child();
    virtual const Window *get_child() const;
    virtual Size calculateRequisition() const;
    virtual void setAllocation(const Size &rAllocation);
};

class VCL_DLLPUBLIC VclFrame : public VclBin
{
private:
    Window *m_pLabel;
private:
    friend class VclBuilder;
    void designate_label(Window *pWindow);
public:
    VclFrame(Window *pParent)
        : VclBin(pParent)
        , m_pLabel(NULL)
    {
    }
    void set_label(const OUString &rLabel);
    OUString get_label() const;
    virtual Window *get_child();
    virtual const Window *get_child() const;
    Window *get_label_widget();
    const Window *get_label_widget() const;
protected:
    virtual Size calculateRequisition() const;
    virtual void setAllocation(const Size &rAllocation);
    virtual OUString getDefaultAccessibleName() const;
};

class VCL_DLLPUBLIC VclAlignment : public VclBin
{
public:
    VclAlignment(Window *pParent)
        : VclBin(pParent)
        , m_nBottomPadding(0)
        , m_nLeftPadding(0)
        , m_nRightPadding(0)
        , m_nTopPadding(0)
        , m_fXAlign(0.0)
        , m_fXScale(1.0)
        , m_fYAlign(0.0)
        , m_fYScale(1.0)
    {
    }
    virtual bool set_property(const OString &rKey, const OString &rValue);
protected:
    virtual Size calculateRequisition() const;
    virtual void setAllocation(const Size &rAllocation);
private:
    sal_Int32 m_nBottomPadding;
    sal_Int32 m_nLeftPadding;
    sal_Int32 m_nRightPadding;
    sal_Int32 m_nTopPadding;
    float m_fXAlign;
    float m_fXScale;
    float m_fYAlign;
    float m_fYScale;
};

class VCL_DLLPUBLIC VclExpander : public VclBin
{
public:
    VclExpander(Window *pParent)
        : VclBin(pParent)
        , m_bResizeTopLevel(true)
        , m_aDisclosureButton(this)
    {
        m_aDisclosureButton.SetToggleHdl(LINK(this, VclExpander, ClickHdl));
        m_aDisclosureButton.Show();
    }
    virtual Window *get_child();
    virtual const Window *get_child() const;
    virtual bool set_property(const OString &rKey, const OString &rValue);
    bool get_expanded() const
    {
        return m_aDisclosureButton.IsChecked();
    }
    void set_expanded(bool bExpanded)
    {
        m_aDisclosureButton.Check(bExpanded);
    }
    void set_label(const OUString& rLabel)
    {
        m_aDisclosureButton.SetText(rLabel);
    }
    OUString get_label() const
    {
        return m_aDisclosureButton.GetText();
    }
    virtual void StateChanged(StateChangedType nType);
    void  SetExpandedHdl( const Link& rLink ) { maExpandedHdl = rLink; }
    const Link& GetExpandedHdl() const { return maExpandedHdl; }
protected:
    virtual Size calculateRequisition() const;
    virtual void setAllocation(const Size &rAllocation);
private:
    bool m_bResizeTopLevel;
    DisclosureButton m_aDisclosureButton;
    Link maExpandedHdl;
    DECL_DLLPRIVATE_LINK(ClickHdl, DisclosureButton* pBtn);
};

//This is a work in progress, so if you want to put something inside a
//scrolled window that doesn't handle its own scrolling, then you may need to
//implement this fully
class VCL_DLLPUBLIC VclScrolledWindow : public VclBin
{
public:
    VclScrolledWindow(Window *pParent, WinBits nStyle = WB_HIDE | WB_AUTOHSCROLL | WB_AUTOVSCROLL)
        : VclBin(pParent, nStyle)
        , m_aVScroll(this, WB_HIDE | WB_VERT)
        , m_aHScroll(this, WB_HIDE | WB_HORZ)
    {
        SetType(WINDOW_SCROLLWINDOW);
    }
    virtual Window *get_child();
    virtual const Window *get_child() const;
    virtual bool set_property(const OString &rKey, const OString &rValue);
    ScrollBar& getVertScrollBar() { return m_aVScroll; }
    ScrollBar& getHorzScrollBar() { return m_aHScroll; }
    Size getVisibleChildSize() const;
protected:
    virtual Size calculateRequisition() const;
    virtual void setAllocation(const Size &rAllocation);
private:
    ScrollBar m_aVScroll;
    ScrollBar m_aHScroll;
};

//Enforces that its children are always the same size as itself.
//Intercepts any Commands intended for its children.
//
//by default the Commands are discarded, inherit from this
//and implement "Command" to get them
class VCL_DLLPUBLIC VclEventBox : public VclBin
{
private:
    //Any Commands an EventBoxHelper receives are forwarded to its parent
    //The VclEventBox ensures that m_aEventBoxHelper is the
    //first child and is transparent, but covers the rest of the children
    class EventBoxHelper : public Window
    {
    public:
        EventBoxHelper(Window* pParent)
            : Window(pParent, 0)
        {
            SetSizePixel(pParent->GetSizePixel());
            EnableChildTransparentMode();
            SetPaintTransparent(true);
            SetBackground();
        }
        virtual void Command(const CommandEvent& rCEvt)
        {
            GetParent()->Command(rCEvt);
        }
    };

    EventBoxHelper m_aEventBoxHelper;
public:
    VclEventBox(Window* pParent)
        : VclBin(pParent)
        , m_aEventBoxHelper(this)
    {
        m_aEventBoxHelper.Show();
    }
    virtual Window *get_child();
    virtual const Window *get_child() const;
    virtual Size calculateRequisition() const;
    virtual void setAllocation(const Size &rAllocation);

    virtual void Command(const CommandEvent& rCEvt);
};

enum VclSizeGroupMode
{
    VCL_SIZE_GROUP_NONE,
    VCL_SIZE_GROUP_HORIZONTAL,
    VCL_SIZE_GROUP_VERTICAL,
    VCL_SIZE_GROUP_BOTH
};

class VCL_DLLPUBLIC VclSizeGroup
{
private:
    std::set<Window*> m_aWindows;
    bool m_bIgnoreHidden;
    VclSizeGroupMode m_eMode;

    void trigger_queue_resize();
public:
    VclSizeGroup()
        : m_bIgnoreHidden(false)
        , m_eMode(VCL_SIZE_GROUP_HORIZONTAL)
    {
    }
    void insert(Window *pWindow)
    {
        m_aWindows.insert(pWindow);
    }
    void erase(Window *pWindow)
    {
        m_aWindows.erase(pWindow);
    }
    const std::set<Window*>& get_widgets() const
    {
        return m_aWindows;
    }
    std::set<Window*>& get_widgets()
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
    bool set_property(const OString &rKey, const OString &rValue);
};

enum VclButtonsType
{
    VCL_BUTTONS_NONE,
    VCL_BUTTONS_OK,
    VCL_BUTTONS_CLOSE,
    VCL_BUTTONS_CANCEL,
    VCL_BUTTONS_YES_NO,
    VCL_BUTTONS_OK_CANCEL
};

enum VclMessageType
{
    VCL_MESSAGE_INFO,
    VCL_MESSAGE_WARNING,
    VCL_MESSAGE_QUESTION,
    VCL_MESSAGE_ERROR
};

class VCL_DLLPUBLIC MessageDialog : public Dialog
{
private:
    VclButtonsType m_eButtonsType;
    VclMessageType m_eMessageType;
    VclGrid* m_pGrid;
    FixedImage* m_pImage;
    VclMultiLineEdit* m_pPrimaryMessage;
    VclMultiLineEdit* m_pSecondaryMessage;
    std::vector<PushButton*> m_aOwnedButtons;
    std::map<const Window*, short> m_aResponses;
    OUString m_sPrimaryString;
    OUString m_sSecondaryString;
    DECL_DLLPRIVATE_LINK(ButtonHdl, Button *);
    void setButtonHandlers(VclButtonBox *pButtonBox);
    short get_response(const Window *pWindow) const;
public:

    MessageDialog(Window* pParent, WinBits nStyle);
    MessageDialog(Window* pParent, const OString& rID, const OUString& rUIXMLDescription);
    virtual bool set_property(const OString &rKey, const OString &rValue);
    virtual short Execute();
    OUString get_primary_text() const;
    OUString get_secondary_text() const;
    void set_primary_text(const OUString &rPrimaryString);
    void set_secondary_text(const OUString &rSecondaryString);
    ~MessageDialog();
};

VCL_DLLPUBLIC Size bestmaxFrameSizeForScreenSize(const Size &rScreenSize);

//Get first window of a pTopLevel window as
//if any intermediate layout widgets didn't exist
//i.e. acts like pChild = pChild->GetWindow(WINDOW_FIRSTCHILD);
//in a flat hierarchy where dialogs only have one layer
//of children
VCL_DLLPUBLIC Window* firstLogicalChildOfParent(Window *pTopLevel);

//Get next window after pChild of a pTopLevel window as
//if any intermediate layout widgets didn't exist
//i.e. acts like pChild = pChild->GetWindow(WINDOW_NEXT);
//in a flat hierarchy where dialogs only have one layer
//of children
VCL_DLLPUBLIC Window* nextLogicalChildOfParent(Window *pTopLevel, Window *pChild);

//Get previous window before pChild of a pTopLevel window as
//if any intermediate layout widgets didn't exist
//i.e. acts like pChild = pChild->GetWindow(WINDOW_PREV);
//in a flat hierarchy where dialogs only have one layer
//of children
VCL_DLLPUBLIC Window* prevLogicalChildOfParent(Window *pTopLevel, Window *pChild);

//Returns true is the Window has a single child which is a container
VCL_DLLPUBLIC bool isLayoutEnabled(const Window *pWindow);

VCL_DLLPUBLIC inline bool isContainerWindow(const Window &rWindow)
{
    WindowType eType = rWindow.GetType();
    return (eType == WINDOW_CONTAINER || eType == WINDOW_SCROLLWINDOW);
}

VCL_DLLPUBLIC inline bool isContainerWindow(const Window *pWindow)
{
    return pWindow && isContainerWindow(*pWindow);
}

//Returns true if the containing dialog is doing its initial
//layout and isn't visible yet
VCL_DLLPUBLIC bool isInitialLayout(const Window *pWindow);

// retro-fitting utilities //

//Get a Size which is large enough to contain all children with
//an equal amount of space at top left and bottom right
Size getLegacyBestSizeForChildren(const Window &rWindow);

//Get first parent which is not a layout widget
VCL_DLLPUBLIC Window* getNonLayoutParent(Window *pParent);

//Get first real parent which is not a layout widget
Window* getNonLayoutRealParent(Window *pParent);

//return true if this window and its stack of containers are all shown
bool isVisibleInLayout(const Window *pWindow);

//return true if this window and its stack of containers are all enabled
bool isEnabledInLayout(const Window *pWindow);

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
