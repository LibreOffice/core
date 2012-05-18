/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * Version: MPL 1.1 / GPLv3+ / LGPLv3+
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License. You may obtain a copy of the License at
 * http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * The Initial Developer of the Original Code is
 *        Caolán McNamara <caolanm@redhat.com> (Red Hat, Inc.)
 * Portions created by the Initial Developer are Copyright (C) 2011 the
 * Initial Developer. All Rights Reserved.
 *
 * Contributor(s): Caolán McNamara <caolanm@redhat.com>
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 3 or later (the "GPLv3+"), or
 * the GNU Lesser General Public License Version 3 or later (the "LGPLv3+"),
 * in which case the provisions of the GPLv3+ or the LGPLv3+ are applicable
 * instead of those above.
 */
#ifndef _VCLLAYOUT_HXX
#define _VCLLAYOUT_HXX

#include <vcl/dllapi.h>
#include <vcl/window.hxx>
#include <boost/multi_array.hpp>

class VCL_DLLPUBLIC VclContainer : public Window
{
public:
    VclContainer(Window *pParent) : Window(pParent), m_nBorderWidth(0) {}
    virtual Size GetOptimalSize(WindowSizeType eType) const;
    using Window::SetPosSizePixel;
    virtual void SetPosSizePixel(const Point& rNewPos, const Size& rNewSize);
    virtual bool set_property(const rtl::OString &rKey, const rtl::OString &rValue);

    void set_border_width(int nBorderWidth)
    {
        m_nBorderWidth = nBorderWidth;
    }
    int get_border_width() const
    {
        return m_nBorderWidth;
    }
protected:
    virtual Size calculateRequisition() const = 0;
    virtual void setAllocation(const Size &rAllocation) = 0;
private:
    int m_nBorderWidth;
};

enum VclPackType
{
    VCL_PACK_START = 0,
    VCL_PACK_END = 1
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
        Show();
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

class VCL_DLLPUBLIC VclButtonBox : public VclBox
{
public:
    VclButtonBox(Window *pParent, int nSpacing)
        : VclBox(pParent, true, nSpacing)
    {
    }
protected:
    virtual Size calculateRequisition() const;
    virtual void setAllocation(const Size &rAllocation);
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
    typedef boost::multi_array<Window*, 2> array_type;

    array_type assembleGrid() const;
    bool isNullGrid(const array_type& A) const;
    void calcMaxs(const array_type &A, std::vector<long> &rWidths, std::vector<long> &rHeights) const;

    virtual Size calculateRequisition() const;
    virtual void setAllocation(const Size &rAllocation);
public:
    VclGrid(Window *pParent)
        : VclContainer(pParent)
        , m_bRowHomogeneous(false), m_bColumnHomogeneous(false)
        , m_nRowSpacing(0), m_nColumnSpacing(0)
    {
        Show();
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
    Window *get_child();
    const Window *get_child() const;
};

class VCL_DLLPUBLIC VclFrame : public VclBin
{
public:
    VclFrame(Window *pParent) : VclBin(pParent) {}
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

/*
 * @return true if rValue is "True", "true", "1", etc.
 */
bool toBool(const rtl::OString &rValue);

// retro-fitting utilities //

//Get a Size which is large enough to contain all children with
//an equal amount of space at top left and bottom right
Size getLegacyBestSizeForChildren(const Window &rWindow);

//Get first parent which is not a layout widget
Window* getLegacyNonLayoutParent(Window *pParent);

//Get next window after pChild of a pTopLevel window as
//if any intermediate layout widgets didn't exist
//i.e. acts like pChild = pChild->GetWindow(WINDOW_NEXT);
//in a flat hierarchy where dialogs only have one layer
//of children
Window* nextLogicalChildOfParent(Window *pTopLevel, Window *pChild);

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
