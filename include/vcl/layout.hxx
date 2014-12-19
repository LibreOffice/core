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

#include <vcl/dllapi.h>
#include <vcl/button.hxx>
#include <vcl/dialog.hxx>
#include <vcl/fixed.hxx>
#include <vcl/scrbar.hxx>
#include <vcl/vclmedit.hxx>
#include <vcl/window.hxx>
#include <boost/multi_array.hpp>
#include <set>

class VCL_DLLPUBLIC VclContainer : public vcl::Window
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

    void markLayoutDirty()
    {
        m_bLayoutDirty = true;
    }

    virtual void queue_resize(StateChangedType eReason = StateChangedType::LAYOUT) SAL_OVERRIDE;
protected:
    //these are the two that need to be implemented by
    //containers, figure out how much space you want...
    virtual Size calculateRequisition() const = 0;
    //..and decide what to do when set to this size
    virtual void setAllocation(const Size &rAllocation) = 0;

    virtual sal_uInt16 getDefaultAccessibleRole() const SAL_OVERRIDE;
public:
    //you don't want to override these
    virtual Size GetOptimalSize() const SAL_OVERRIDE;
    virtual void SetPosSizePixel(const Point& rNewPos, const Size& rNewSize) SAL_OVERRIDE;
    virtual void SetPosPixel(const Point& rAllocPos) SAL_OVERRIDE;
    virtual void SetSizePixel(const Size& rAllocation) SAL_OVERRIDE;
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
    bool get_homogeneous() const
    {
        return m_bHomogeneous;
    }
    virtual bool set_property(const OString &rKey, const OString &rValue) SAL_OVERRIDE;
protected:
    virtual sal_uInt16 getDefaultAccessibleRole() const SAL_OVERRIDE;
    void accumulateMaxes(const Size &rChildSize, Size &rSize) const;
    Size finalizeMaxes(const Size &rSize, sal_uInt16 nVisibleChildren) const;

    virtual Size calculateRequisition() const SAL_OVERRIDE;
    virtual void setAllocation(const Size &rAllocation) SAL_OVERRIDE;

    virtual long getPrimaryDimension(const Size &rSize) const = 0;
    virtual void setPrimaryDimension(Size &rSize, long) const = 0;
    virtual long getPrimaryCoordinate(const Point &rPos) const = 0;
    virtual void setPrimaryCoordinate(Point &rPos, long) const = 0;
    virtual long getSecondaryDimension(const Size &rSize) const = 0;
    virtual void setSecondaryDimension(Size &rSize, long) const = 0;
    virtual long getSecondaryCoordinate(const Point &rPos) const = 0;
    virtual void setSecondaryCoordinate(Point &rPos, long) const = 0;

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
    virtual long getPrimaryDimension(const Size &rSize) const SAL_OVERRIDE
    {
        return rSize.getHeight();
    }
    virtual void setPrimaryDimension(Size &rSize, long nHeight) const SAL_OVERRIDE
    {
        rSize.setHeight(nHeight);
    }
    virtual long getPrimaryCoordinate(const Point &rPos) const SAL_OVERRIDE
    {
        return rPos.getY();
    }
    virtual void setPrimaryCoordinate(Point &rPos, long nPos) const SAL_OVERRIDE
    {
        rPos.setY(nPos);
    }
    virtual long getSecondaryDimension(const Size &rSize) const SAL_OVERRIDE
    {
        return rSize.getWidth();
    }
    virtual void setSecondaryDimension(Size &rSize, long nWidth) const SAL_OVERRIDE
    {
        rSize.setWidth(nWidth);
    }
    virtual long getSecondaryCoordinate(const Point &rPos) const SAL_OVERRIDE
    {
        return rPos.getX();
    }
    virtual void setSecondaryCoordinate(Point &rPos, long nPos) const SAL_OVERRIDE
    {
        rPos.setX(nPos);
    }
    virtual bool getPrimaryDimensionChildExpand(const vcl::Window &rWindow) const SAL_OVERRIDE
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
    virtual long getPrimaryDimension(const Size &rSize) const SAL_OVERRIDE
    {
        return rSize.getWidth();
    }
    virtual void setPrimaryDimension(Size &rSize, long nWidth) const SAL_OVERRIDE
    {
        rSize.setWidth(nWidth);
    }
    virtual long getPrimaryCoordinate(const Point &rPos) const SAL_OVERRIDE
    {
        return rPos.getX();
    }
    virtual void setPrimaryCoordinate(Point &rPos, long nPos) const SAL_OVERRIDE
    {
        rPos.setX(nPos);
    }
    virtual long getSecondaryDimension(const Size &rSize) const SAL_OVERRIDE
    {
        return rSize.getHeight();
    }
    virtual void setSecondaryDimension(Size &rSize, long nHeight) const SAL_OVERRIDE
    {
        rSize.setHeight(nHeight);
    }
    virtual long getSecondaryCoordinate(const Point &rPos) const SAL_OVERRIDE
    {
        return rPos.getY();
    }
    virtual void setSecondaryCoordinate(Point &rPos, long nPos) const SAL_OVERRIDE
    {
        rPos.setY(nPos);
    }
    virtual bool getPrimaryDimensionChildExpand(const vcl::Window &rWindow) const SAL_OVERRIDE
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
    VclButtonBox(vcl::Window *pParent, int nSpacing)
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
    virtual bool set_property(const OString &rKey, const OString &rValue) SAL_OVERRIDE;
    void sort_native_button_order();
protected:
    virtual Size calculateRequisition() const SAL_OVERRIDE;
    virtual void setAllocation(const Size &rAllocation) SAL_OVERRIDE;
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
    VclVButtonBox(vcl::Window *pParent, int nSpacing = 0)
        : VclButtonBox(pParent, nSpacing)
    {
        m_bVerticalContainer = true;
    }
protected:
    virtual long getPrimaryDimension(const Size &rSize) const SAL_OVERRIDE
    {
        return rSize.getHeight();
    }
    virtual void setPrimaryDimension(Size &rSize, long nHeight) const SAL_OVERRIDE
    {
        rSize.setHeight(nHeight);
    }
    virtual long getPrimaryCoordinate(const Point &rPos) const SAL_OVERRIDE
    {
        return rPos.getY();
    }
    virtual void setPrimaryCoordinate(Point &rPos, long nPos) const SAL_OVERRIDE
    {
        rPos.setY(nPos);
    }
    virtual long getSecondaryDimension(const Size &rSize) const SAL_OVERRIDE
    {
        return rSize.getWidth();
    }
    virtual void setSecondaryDimension(Size &rSize, long nWidth) const SAL_OVERRIDE
    {
        rSize.setWidth(nWidth);
    }
    virtual long getSecondaryCoordinate(const Point &rPos) const SAL_OVERRIDE
    {
        return rPos.getX();
    }
    virtual void setSecondaryCoordinate(Point &rPos, long nPos) const SAL_OVERRIDE
    {
        rPos.setX(nPos);
    }
    virtual bool getPrimaryDimensionChildExpand(const vcl::Window &rWindow) const SAL_OVERRIDE
    {
        return rWindow.get_expand() || rWindow.get_vexpand();
    }
};

class VCL_DLLPUBLIC VclHButtonBox : public VclButtonBox
{
public:
    VclHButtonBox(vcl::Window *pParent, int nSpacing = 0)
        : VclButtonBox(pParent, nSpacing)
    {
        m_bVerticalContainer = false;
    }
protected:
    virtual long getPrimaryDimension(const Size &rSize) const SAL_OVERRIDE
    {
        return rSize.getWidth();
    }
    virtual void setPrimaryDimension(Size &rSize, long nWidth) const SAL_OVERRIDE
    {
        rSize.setWidth(nWidth);
    }
    virtual long getPrimaryCoordinate(const Point &rPos) const SAL_OVERRIDE
    {
        return rPos.getX();
    }
    virtual void setPrimaryCoordinate(Point &rPos, long nPos) const SAL_OVERRIDE
    {
        rPos.setX(nPos);
    }
    virtual long getSecondaryDimension(const Size &rSize) const SAL_OVERRIDE
    {
        return rSize.getHeight();
    }
    virtual void setSecondaryDimension(Size &rSize, long nHeight) const SAL_OVERRIDE
    {
        rSize.setHeight(nHeight);
    }
    virtual long getSecondaryCoordinate(const Point &rPos) const SAL_OVERRIDE
    {
        return rPos.getY();
    }
    virtual void setSecondaryCoordinate(Point &rPos, long nPos) const SAL_OVERRIDE
    {
        rPos.setY(nPos);
    }
    virtual bool getPrimaryDimensionChildExpand(const vcl::Window &rWindow) const SAL_OVERRIDE
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
        vcl::Window *pChild;
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
    virtual Size calculateRequisition() const SAL_OVERRIDE;
    virtual void setAllocation(const Size &rAllocation) SAL_OVERRIDE;
public:
    VclGrid(vcl::Window *pParent)
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
    virtual bool set_property(const OString &rKey, const OString &rValue) SAL_OVERRIDE;
};

VCL_DLLPUBLIC void setGridAttach(vcl::Window &rWidget, sal_Int32 nLeft, sal_Int32 nTop,
    sal_Int32 nWidth = 1, sal_Int32 nHeight = 1);

class VCL_DLLPUBLIC VclBin : public VclContainer
{
public:
    VclBin(vcl::Window *pParent, WinBits nStyle = WB_HIDE | WB_CLIPCHILDREN)
        : VclContainer(pParent, nStyle)
    {
    }
    virtual vcl::Window *get_child();
    virtual const vcl::Window *get_child() const;
    virtual Size calculateRequisition() const SAL_OVERRIDE;
    virtual void setAllocation(const Size &rAllocation) SAL_OVERRIDE;
};

class VCL_DLLPUBLIC VclFrame : public VclBin
{
private:
    vcl::Window *m_pLabel;
private:
    friend class VclBuilder;
    void designate_label(vcl::Window *pWindow);
    DECL_LINK(WindowEventListener, VclSimpleEvent*);
public:
    VclFrame(vcl::Window *pParent)
        : VclBin(pParent)
        , m_pLabel(NULL)
    {
    }
    void set_label(const OUString &rLabel);
    OUString get_label() const;
    virtual vcl::Window *get_child() SAL_OVERRIDE;
    virtual const vcl::Window *get_child() const SAL_OVERRIDE;
    vcl::Window *get_label_widget();
    const vcl::Window *get_label_widget() const;
protected:
    virtual Size calculateRequisition() const SAL_OVERRIDE;
    virtual void setAllocation(const Size &rAllocation) SAL_OVERRIDE;
    virtual OUString getDefaultAccessibleName() const SAL_OVERRIDE;
};

class VCL_DLLPUBLIC VclAlignment : public VclBin
{
public:
    VclAlignment(vcl::Window *pParent)
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
    virtual bool set_property(const OString &rKey, const OString &rValue) SAL_OVERRIDE;
protected:
    virtual Size calculateRequisition() const SAL_OVERRIDE;
    virtual void setAllocation(const Size &rAllocation) SAL_OVERRIDE;
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
    VclExpander(vcl::Window *pParent)
        : VclBin(pParent)
        , m_bResizeTopLevel(true)
        , m_pDisclosureButton(new DisclosureButton(this))
    {
        m_pDisclosureButton->SetToggleHdl(LINK(this, VclExpander, ClickHdl));
        m_pDisclosureButton->Show();
    }
    virtual vcl::Window *get_child() SAL_OVERRIDE;
    virtual const vcl::Window *get_child() const SAL_OVERRIDE;
    virtual bool set_property(const OString &rKey, const OString &rValue) SAL_OVERRIDE;
    bool get_expanded() const
    {
        return m_pDisclosureButton->IsChecked();
    }
    void set_expanded(bool bExpanded)
    {
        m_pDisclosureButton->Check(bExpanded);
    }
    void set_label(const OUString& rLabel)
    {
        m_pDisclosureButton->SetText(rLabel);
    }
    OUString get_label() const
    {
        return m_pDisclosureButton->GetText();
    }
    virtual void StateChanged(StateChangedType nType) SAL_OVERRIDE;
    void  SetExpandedHdl( const Link& rLink ) { maExpandedHdl = rLink; }
    const Link& GetExpandedHdl() const { return maExpandedHdl; }
protected:
    virtual Size calculateRequisition() const SAL_OVERRIDE;
    virtual void setAllocation(const Size &rAllocation) SAL_OVERRIDE;
    void dispose() SAL_OVERRIDE { m_pDisclosureButton.disposeAndClear(); VclBin::dispose(); }
private:
    bool m_bResizeTopLevel;
    DisclosureButtonPtr m_pDisclosureButton;
    Link maExpandedHdl;
    DECL_DLLPRIVATE_LINK(ClickHdl, DisclosureButton* pBtn);
};

class VCL_DLLPUBLIC VclScrolledWindow : public VclBin
{
public:
    VclScrolledWindow(vcl::Window *pParent, WinBits nStyle = WB_HIDE | WB_CLIPCHILDREN | WB_AUTOHSCROLL | WB_AUTOVSCROLL);
    virtual vcl::Window *get_child() SAL_OVERRIDE;
    virtual const vcl::Window *get_child() const SAL_OVERRIDE;
    virtual bool set_property(const OString &rKey, const OString &rValue) SAL_OVERRIDE;
    ScrollBar& getVertScrollBar() { return *m_pVScroll.get(); }
    ScrollBar& getHorzScrollBar() { return *m_pHScroll.get(); }
    Size getVisibleChildSize() const;
    //set to true to disable the built-in scrolling callbacks to allow the user
    //to override it
    void setUserManagedScrolling(bool bUserManagedScrolling) { m_bUserManagedScrolling = bUserManagedScrolling;}
protected:
    virtual Size calculateRequisition() const SAL_OVERRIDE;
    virtual void setAllocation(const Size &rAllocation) SAL_OVERRIDE;
    DECL_LINK(ScrollBarHdl, void *);
    void InitScrollBars(const Size &rRequest);
    virtual bool Notify(NotifyEvent& rNEvt) SAL_OVERRIDE;
    void dispose() SAL_OVERRIDE { m_pVScroll.disposeAndClear(); m_pHScroll.disposeAndClear(); VclBin::dispose(); }
private:
    bool m_bUserManagedScrolling;
    ScrollBarPtr m_pVScroll;
    ScrollBarPtr m_pHScroll;
    ScrollBarBox m_aScrollBarBox;
};

class VCL_DLLPUBLIC VclViewport : public VclBin
{
public:
    VclViewport(vcl::Window *pParent, WinBits nStyle = WB_HIDE | WB_CLIPCHILDREN)
        : VclBin(pParent, nStyle)
    {
    }
protected:
    virtual void setAllocation(const Size &rAllocation) SAL_OVERRIDE;
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
        virtual void Command(const CommandEvent& rCEvt) SAL_OVERRIDE
        {
            GetParent()->Command(rCEvt);
        }
    };

    EventBoxHelper m_aEventBoxHelper;
public:
    VclEventBox(vcl::Window* pParent)
        : VclBin(pParent)
        , m_aEventBoxHelper(this)
    {
        m_aEventBoxHelper.Show();
    }
    virtual vcl::Window *get_child() SAL_OVERRIDE;
    virtual const vcl::Window *get_child() const SAL_OVERRIDE;
    virtual Size calculateRequisition() const SAL_OVERRIDE;
    virtual void setAllocation(const Size &rAllocation) SAL_OVERRIDE;

    virtual void Command(const CommandEvent& rCEvt) SAL_OVERRIDE;
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
    std::set<vcl::Window*> m_aWindows;
    bool m_bIgnoreHidden;
    VclSizeGroupMode m_eMode;

    void trigger_queue_resize();
public:
    VclSizeGroup()
        : m_bIgnoreHidden(false)
        , m_eMode(VCL_SIZE_GROUP_HORIZONTAL)
    {
    }
    void insert(vcl::Window *pWindow)
    {
        m_aWindows.insert(pWindow);
    }
    void erase(vcl::Window *pWindow)
    {
        m_aWindows.erase(pWindow);
    }
    const std::set<vcl::Window*>& get_widgets() const
    {
        return m_aWindows;
    }
    std::set<vcl::Window*>& get_widgets()
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
    VclBox *m_pOwnedContentArea;
    VclButtonBox *m_pOwnedActionArea;
    VclGrid* m_pGrid;
    FixedImage* m_pImage;
    VclMultiLineEdit* m_pPrimaryMessage;
    VclMultiLineEdit* m_pSecondaryMessage;
    std::vector<PushButton*> m_aOwnedButtons;
    std::map<const vcl::Window*, short> m_aResponses;
    OUString m_sPrimaryString;
    OUString m_sSecondaryString;
    DECL_DLLPRIVATE_LINK(ButtonHdl, Button *);
    void setButtonHandlers(VclButtonBox *pButtonBox);
    short get_response(const vcl::Window *pWindow) const;
    void create_owned_areas();

    friend class VclBuilder;
    MessageDialog(vcl::Window* pParent, WinBits nStyle = WB_MOVEABLE | WB_3DLOOK | WB_CLOSEABLE);
public:

    MessageDialog(vcl::Window* pParent,
        const OUString &rMessage,
        VclMessageType eMessageType = VCL_MESSAGE_ERROR,
        VclButtonsType eButtonsType = VCL_BUTTONS_OK,
        WinBits nStyle = WB_MOVEABLE | WB_3DLOOK | WB_CLOSEABLE);
    MessageDialog(vcl::Window* pParent, const OString& rID, const OUString& rUIXMLDescription);
    virtual bool set_property(const OString &rKey, const OString &rValue) SAL_OVERRIDE;
    virtual short Execute() SAL_OVERRIDE;
    ///Emitted when an action widget is clicked
    virtual void response(short nResponseId);
    OUString get_primary_text() const;
    OUString get_secondary_text() const;
    void set_primary_text(const OUString &rPrimaryString);
    void set_secondary_text(const OUString &rSecondaryString);
    virtual ~MessageDialog();

    static void SetMessagesWidths(vcl::Window *pParent, VclMultiLineEdit *pPrimaryMessage,
        VclMultiLineEdit *pSecondaryMessage);
};

VCL_DLLPUBLIC Size bestmaxFrameSizeForScreenSize(const Size &rScreenSize);

//Get first window of a pTopLevel window as
//if any intermediate layout widgets didn't exist
//i.e. acts like pChild = pChild->GetWindow(WINDOW_FIRSTCHILD);
//in a flat hierarchy where dialogs only have one layer
//of children
VCL_DLLPUBLIC vcl::Window* firstLogicalChildOfParent(vcl::Window *pTopLevel);

//Get next window after pChild of a pTopLevel window as
//if any intermediate layout widgets didn't exist
//i.e. acts like pChild = pChild->GetWindow(WINDOW_NEXT);
//in a flat hierarchy where dialogs only have one layer
//of children
VCL_DLLPUBLIC vcl::Window* nextLogicalChildOfParent(vcl::Window *pTopLevel, vcl::Window *pChild);

//Get previous window before pChild of a pTopLevel window as
//if any intermediate layout widgets didn't exist
//i.e. acts like pChild = pChild->GetWindow(WINDOW_PREV);
//in a flat hierarchy where dialogs only have one layer
//of children
VCL_DLLPUBLIC vcl::Window* prevLogicalChildOfParent(vcl::Window *pTopLevel, vcl::Window *pChild);

//Returns true is the Window has a single child which is a container
VCL_DLLPUBLIC bool isLayoutEnabled(const vcl::Window *pWindow);

inline bool isContainerWindow(const vcl::Window &rWindow)
{
    WindowType eType = rWindow.GetType();
    return eType == WINDOW_CONTAINER || eType == WINDOW_SCROLLWINDOW ||
           (eType == WINDOW_DOCKINGWINDOW && ::isLayoutEnabled(&rWindow));
}

inline bool isContainerWindow(const vcl::Window *pWindow)
{
    return pWindow && isContainerWindow(*pWindow);
}

//Returns true if the containing dialog is doing its initial
//layout and isn't visible yet
VCL_DLLPUBLIC bool isInitialLayout(const vcl::Window *pWindow);

// retro-fitting utilities

//Get a Size which is large enough to contain all children with
//an equal amount of space at top left and bottom right
Size getLegacyBestSizeForChildren(const vcl::Window &rWindow);

//Get first parent which is not a layout widget
VCL_DLLPUBLIC vcl::Window* getNonLayoutParent(vcl::Window *pParent);

//Get first real parent which is not a layout widget
vcl::Window* getNonLayoutRealParent(vcl::Window *pParent);

//return true if this window and its stack of containers are all shown
bool isVisibleInLayout(const vcl::Window *pWindow);

//return true if this window and its stack of containers are all enabled
bool isEnabledInLayout(const vcl::Window *pWindow);

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
