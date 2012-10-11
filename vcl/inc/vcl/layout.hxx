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
#include <vcl/window.hxx>
#include <boost/multi_array.hpp>

class VCL_DLLPUBLIC VclContainer : public Window
{
public:
    VclContainer(Window *pParent);
    virtual Size GetOptimalSize(WindowSizeType eType) const;
    virtual void SetPosSizePixel(const Point& rNewPos, const Size& rNewSize);
    virtual void SetPosPixel(const Point& rAllocPos);
    virtual void SetSizePixel(const Size& rAllocation);

    void markLayoutDirty()
    {
        m_bLayoutDirty = true;
    }

    //These take into account the external margins of the rWindow widget
    //while GetOptimalSize/get_preferred_size and SetPosSizePixel are
    //oblivious to them
    static Size getLayoutRequisition(const Window &rWindow);
    static void setLayoutAllocation(Window &rWindow, const Point &rPos, const Size &rSize);

protected:
    virtual Size calculateRequisition() const = 0;
    virtual void setAllocation(const Size &rAllocation) = 0;
private:
    bool m_bLayoutDirty;
};

class VCL_DLLPUBLIC VclBox : public VclContainer
{
protected:
    bool m_bHomogeneous;
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
    virtual bool set_property(const rtl::OString &rKey, const rtl::OString &rValue);
protected:
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
};

class VCL_DLLPUBLIC VclVBox : public VclBox
{
public:
    VclVBox(Window *pParent, bool bHomogeneous = false, int nSpacing = 0)
        : VclBox(pParent, bHomogeneous, nSpacing)
    {
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
};

class VCL_DLLPUBLIC VclHBox : public VclBox
{
public:
    VclHBox(Window *pParent, bool bHomogeneous = false, int nSpacing = 0)
        : VclBox(pParent, bHomogeneous, nSpacing)
    {
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
        : VclBox(pParent, true, nSpacing)
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
    virtual bool set_property(const rtl::OString &rKey, const rtl::OString &rValue);
protected:
    virtual Size calculateRequisition() const;
    virtual void setAllocation(const Size &rAllocation);
private:
    VclButtonBoxStyle m_eLayoutStyle;
};

class VCL_DLLPUBLIC VclVButtonBox : public VclButtonBox
{
public:
    VclVButtonBox(Window *pParent, int nSpacing = 0)
        : VclButtonBox(pParent, nSpacing)
    {
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
};

class VCL_DLLPUBLIC VclHButtonBox : public VclButtonBox
{
public:
    VclHButtonBox(Window *pParent, int nSpacing = 0)
        : VclButtonBox(pParent, nSpacing)
    {
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
    virtual bool set_property(const rtl::OString &rKey, const rtl::OString &rValue);
};

VCL_DLLPUBLIC void setGridAttach(Window &rWidget, sal_Int32 nLeft, sal_Int32 nTop,
    sal_Int32 nWidth = 1, sal_Int32 nHeight = 1);

class VCL_DLLPUBLIC VclBin : public VclContainer
{
public:
    VclBin(Window *pParent) : VclContainer(pParent) {}
    virtual Window *get_child();
    virtual const Window *get_child() const;
    virtual Size calculateRequisition() const;
    virtual void setAllocation(const Size &rAllocation);
};

class VCL_DLLPUBLIC VclFrame : public VclBin
{
public:
    VclFrame(Window *pParent) : VclBin(pParent) {}
    void set_label(const rtl::OUString &rLabel);
    Window *get_label_widget();
    const Window *get_label_widget() const;
protected:
    virtual Size calculateRequisition() const;
    virtual void setAllocation(const Size &rAllocation);
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
    virtual bool set_property(const rtl::OString &rKey, const rtl::OString &rValue);
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
    virtual bool set_property(const rtl::OString &rKey, const rtl::OString &rValue);
protected:
    virtual Size calculateRequisition() const;
    virtual void setAllocation(const Size &rAllocation);
private:
    bool m_bResizeTopLevel;
    DisclosureButton m_aDisclosureButton;
    DECL_DLLPRIVATE_LINK(ClickHdl, DisclosureButton* pBtn);
};


// retro-fitting utilities //

//Get a Size which is large enough to contain all children with
//an equal amount of space at top left and bottom right
Size getLegacyBestSizeForChildren(const Window &rWindow);

//Get first parent which is not a layout widget
Window* getNonLayoutParent(Window *pParent);

//Get first real parent which is not a layout widget
Window* getNonLayoutRealParent(Window *pParent);

//return true if this window and its stack of containers are all shown
bool isVisibleInLayout(const Window *pWindow);

//return true if this window and its stack of containers are all enabled
bool isEnabledInLayout(const Window *pWindow);

//Get next window after pChild of a pTopLevel window as
//if any intermediate layout widgets didn't exist
//i.e. acts like pChild = pChild->GetWindow(WINDOW_NEXT);
//in a flat hierarchy where dialogs only have one layer
//of children
Window* nextLogicalChildOfParent(Window *pTopLevel, Window *pChild);
Window* prevLogicalChildOfParent(Window *pTopLevel, Window *pChild);

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
