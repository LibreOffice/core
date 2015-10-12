/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: IBM Corporation
 *
 *  Copyright: 2008 by IBM Corporation
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/
/*************************************************************************
 * @file
 * Frame object for OOo.This is the basic object for all Shape Layer.
 * You can reference to the XFFrame object.
 ************************************************************************/
#ifndef INCLUDED_LOTUSWORDPRO_SOURCE_FILTER_XFILTER_XFFRAME_HXX
#define INCLUDED_LOTUSWORDPRO_SOURCE_FILTER_XFILTER_XFFRAME_HXX

#include "xfglobal.hxx"
#include "xfcontent.hxx"
#include "xfrect.hxx"
#include "xfcontentcontainer.hxx"

#define     XFFRAME_FLAG_HEIGHT         0x00000001
#define     XFFRAME_FLAG_MINHEIGHT      0x00000002
#define     XFFRAME_FLAG_MAXHEIGHT      0x00000004
/**
 * @brief
 *  Base class for all frame object,include drawing,image,text-box.
 *
 *  Use SetX(),SetY(),SetWidth(),SetHeight() to set position for the frame.
 *  Only if you set anchor type to enumXFAnchorPage, the SetAnchorPage functions.
 */
class XFFrame : public XFContentContainer
{
public:
    XFFrame();
    explicit XFFrame(bool isTextBox);

    virtual ~XFFrame();

public:
    using XFContentContainer::Add;

    /**
     * @descr   override the add function to adjust z-index.
     */
    virtual void    Add(XFContent *pContent) override;

    /**
     * @descr:  Set the anchor type for the frame object.
     */
    void    SetAnchorType(enumXFAnchor type);

    /**
     * @descr:  if it's page anchor,set the page number.
     */
    void    SetAnchorPage(sal_Int32 page);

    /**
     * @descr   Set frame name.
     */
    void    SetName(const OUString& name);

    /**
     * @descr   Set z-index of the frame.
     */
    void    SetZIndex(sal_uInt32 zIndex);
    /**
     * @descr   Set frame position X.
     */
    void    SetX(double x);

    /**
     * @descr   Set frame position Y.
     */
    void    SetY(double y);

    /**
     * @descr   Set frame width.
     */
    void    SetWidth(double width);

    /**
     * @descr   Set frame height. Be careful SetHeight and SetMinHeight will override each other.
     */
    void    SetHeight(double height);

    /**
     * @descr   Set frame min-height. Be careful SetHeight and SetMinHeight will override each other.
     */
    void    SetMinHeight(double minHeight);

    /**
     * @descr   Set frame position.
     */
    void    SetPosition(double x, double y, double width, double height);

    /**
     * @descr   Set frame position.
     */
    void    SetPosition(const XFRect& rect);

    /**
     * @descr:  Link the text content of the two frame.
     */
    void    SetNextLink(const OUString& next);

    /**
     * @descr   Get the frame type. image, drawing or text-box.
     */
    enumXFFrameType GetFrameType();

    /**
     * @descr   Get content type, enumXFContentFrame.
     */
    virtual enumXFContent   GetContentType() override;

    /**
     * @descr   serialize.
     */
    virtual void    ToXml(IXFStream *pStrm) override;

private:
    void    StartFrame(IXFStream *pStrm);

    static void EndFrame(IXFStream *pStrm);

    void    AdjustZIndex();

protected:
    enumXFAnchor    m_eAnchor;
    sal_Int32       m_nAnchorPage;
    OUString   m_strName;
    sal_uInt32      m_nZIndex;
    XFRect          m_aRect;
    double          m_fMinHeight;
    double          m_fMaxHeight;
    OUString   m_strNextLink;
    enumXFFrameType m_eType;
    sal_uInt32      m_nFlag;
    bool            m_isTextBox;
};

inline void XFFrame::SetAnchorType(enumXFAnchor anchor)
{
    m_eAnchor = anchor;
}

inline void XFFrame::SetName(const OUString& name)
{
    m_strName = name;
}

inline void XFFrame::SetX(double x)
{
    m_aRect.SetX(x);
}

inline void XFFrame::SetY(double y)
{
    m_aRect.SetY(y);
}

inline void XFFrame::SetWidth(double width)
{
    m_aRect.SetWidth(width);
}

inline void XFFrame::SetHeight(double height)
{
    m_aRect.SetHeight(height);
    m_nFlag |= XFFRAME_FLAG_HEIGHT;
    m_nFlag &= XFFRAME_FLAG_HEIGHT;
}

inline void XFFrame::SetMinHeight(double minHeight)
{
    m_fMinHeight = minHeight;
    m_nFlag |= XFFRAME_FLAG_MINHEIGHT;
    m_nFlag &= XFFRAME_FLAG_MINHEIGHT;
}

inline void XFFrame::SetPosition(double x, double y, double width, double height)
{
    m_aRect.SetStartPoint(XFPoint(x,y));
    m_aRect.SetSize(width,height);
    m_nFlag |= XFFRAME_FLAG_HEIGHT;
}

inline void XFFrame::SetPosition(const XFRect& rect)
{
    m_aRect = rect;
}

inline void XFFrame::SetNextLink(const OUString& next)
{
    m_strNextLink = next;
}

inline void XFFrame::SetAnchorPage(sal_Int32 page)
{
    m_nAnchorPage = page;
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
