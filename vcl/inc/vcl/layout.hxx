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

enum VclPackType
{
    VCL_PACK_START = 0,
    VCL_PACK_END = 1
};

class VCL_DLLPUBLIC Box : public Window
{
protected:
    bool m_bHomogeneous;
    int m_nSpacing;
public:
    Box(Window *pParent, bool bHomogeneous, int nSpacing)
        : Window(pParent)
        , m_bHomogeneous(bHomogeneous)
        , m_nSpacing(nSpacing)
    {
        Show();
    }
public:
    virtual Size GetOptimalSize(WindowSizeType eType) const;
    using Window::SetPosSizePixel;
    virtual void SetPosSizePixel(const Point& rNewPos, const Size& rNewSize);
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

class VCL_DLLPUBLIC VBox : public Box
{
public:
    VBox(Window *pParent, bool bHomogeneous = false, int nSpacing = 0)
        : Box(pParent, bHomogeneous, nSpacing)
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

class VCL_DLLPUBLIC HBox : public Box
{
public:
    HBox(Window *pParent, bool bHomogeneous = false, int nSpacing = 0)
        : Box(pParent, bHomogeneous, nSpacing)
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

class VCL_DLLPUBLIC ButtonBox : public Box
{
public:
    ButtonBox(Window *pParent, int nSpacing)
        : Box(pParent, true, nSpacing)
    {
    }
protected:
    virtual Size calculateRequisition() const;
    virtual void setAllocation(const Size &rAllocation);
};

class VCL_DLLPUBLIC VButtonBox : public ButtonBox
{
public:
    VButtonBox(Window *pParent, int nSpacing = 0)
        : ButtonBox(pParent, nSpacing)
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

class VCL_DLLPUBLIC HButtonBox : public ButtonBox
{
public:
    HButtonBox(Window *pParent, int nSpacing = 0)
        : ButtonBox(pParent, nSpacing)
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

class VCL_DLLPUBLIC Grid : public Window
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

    Size calculateRequisition() const;
    void setAllocation(const Size &rAllocation);
public:
    Grid(Window *pParent)
        : Window(pParent)
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
public:
    virtual Size GetOptimalSize(WindowSizeType eType) const;
    using Window::SetPosSizePixel;
    virtual void SetPosSizePixel(const Point& rNewPos, const Size& rNewSize);
};

VCL_DLLPUBLIC void setGridAttach(Window &rWidget, sal_Int32 nLeft, sal_Int32 nTop,
    sal_Int32 nWidth = 1, sal_Int32 nHeight = 1);

//Get a Size which is large enough to contain all children with
//an equal amount of space at top left and bottom right
Size getLegacyBestSizeForChildren(const Window &rWindow);

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
