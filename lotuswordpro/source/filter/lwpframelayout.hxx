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
 *  the class for VO_FrameLayout
 ************************************************************************/
/*************************************************************************
 * Change History
Mar 2005            Created
 ************************************************************************/

#ifndef _LWPFRAMELAYOUT_HXX
#define _LWPFRAMELAYOUT_HXX

#include "lwplayout.hxx"
#include "lwpstory.hxx"
#include "lwpmarker.hxx"
/**
 * @brief       For register frame style and parse frame
 *
 */

class LwpFrame
{
public:
    LwpFrame(LwpPlacableLayout* pLayout);
    ~LwpFrame();
    void RegisterStyle(XFFrameStyle* pFrameStyle);
    void Parse(XFFrame* pXFFrame, sal_Int32 nPageNo = 0);
    void XFConvert(XFContentContainer* pCont);
private:
    void ApplyWrapType(XFFrameStyle* pFrameStyle);
    void ApplyMargins(XFFrameStyle* pFrameStyle);
    void ApplyPadding(XFFrameStyle* pFrameStyle);
    void ApplyBorders(XFFrameStyle* pFrameStyle);
    void ApplyColumns(XFFrameStyle* pFrameStyle);
    void ApplyShadow(XFFrameStyle* pFrameStyle);
    void ApplyBackColor(XFFrameStyle* pFrameStyle);
    void ApplyProtect(XFFrameStyle* pFrameStyle);
    void ApplyTextDir(XFFrameStyle* pFrameStyle);
    void ApplyPosType(XFFrameStyle* pFrameStyle);
    void ApplyWatermark(XFFrameStyle* pFrameStyle);
    // add by , 06/01/2005
    void ApplyPatternFill(XFFrameStyle* pFrameStyle);
    void ApplyBackGround(XFFrameStyle* pFrameStyle);
    // end add
    void ParseAnchorType(XFFrame* pXFFrame);
    sal_Bool IsLeftWider();
private:
    LwpPlacableLayout* m_pLayout;
    OUString m_StyleName;
};

/**
 * @brief       Frame link information
 *
 */
class LwpFrameLink
{
public:
    LwpFrameLink();
    ~LwpFrameLink();
    void Read(LwpObjectStream* pStrm);
    LwpObjectID* GetNextLayout(){ return &m_NextLayout;}
    LwpObjectID* GetPreviousLayout(){ return &m_PreviousLayout;}
private:
    LwpObjectID m_PreviousLayout;
    LwpObjectID m_NextLayout;
};

/**
 * @brief       VO_FRAMELAYOUT object
 *
 */
class LwpFrameLayout: public LwpPlacableLayout
{
public:
    LwpFrameLayout(LwpObjectHeader &objHdr, LwpSvStream* pStrm);
    virtual ~LwpFrameLayout();
    virtual LWP_LAYOUT_TYPE GetLayoutType () { return LWP_FRAME_LAYOUT;}
    virtual void RegisterStyle();
    virtual void XFConvert(XFContentContainer* pCont);
    void XFConvertFrame(XFContentContainer* pCont, sal_Int32 nStart = 0, sal_Int32 nEnd = 0, sal_Bool bAll = sal_False);
    OUString GetNextLinkName();
    sal_Bool HasPreviousLinkLayout();
    sal_Bool IsForWaterMark();
    double GetWidth();
    void ApplyGraphicSize(XFFrame* pXFFrame);
protected:
    void Read();
private:
    double GetMaxWidth();

private:
    LwpFrameLink m_Link;
    LwpFrame* m_pFrame;
};

/**
 * @brief       VO_GROUPLAYOUT object , information for frame group layout
 *
 */
class LwpGroupLayout: public LwpPlacableLayout
{
public:
    LwpGroupLayout(LwpObjectHeader &objHdr, LwpSvStream* pStrm);
    virtual ~LwpGroupLayout();
    virtual LWP_LAYOUT_TYPE GetLayoutType () { return LWP_GROUP_LAYOUT;}
    virtual void RegisterStyle();
    virtual void XFConvert(XFContentContainer* pCont);
    void XFConvertFrame(XFContentContainer* pCont, sal_Int32 nStart = 0, sal_Int32 nEnd = 0, sal_Bool bAll = sal_False);
protected:
    void Read();
private:
    LwpFrame* m_pFrame;

};

/**
 * @brief       VO_GROUPFRAME object , information for frame group contents
 *
 */
class LwpGroupFrame: public LwpContent
{
public:
    LwpGroupFrame(LwpObjectHeader &objHdr, LwpSvStream* pStrm);
    ~LwpGroupFrame();
    virtual void RegisterStyle();
    virtual void XFConvert(XFContentContainer* pCont);
protected:
    void Read();
};

class LwpStory;
class LwpFoundry;
class LwpDropcapLayout : public LwpFrameLayout
{
public:
    LwpDropcapLayout(LwpObjectHeader &objHdr, LwpSvStream* pStrm);
    virtual ~LwpDropcapLayout(){}
    virtual LWP_LAYOUT_TYPE GetLayoutType () { return LWP_DROPCAP_LAYOUT;}
    virtual void Parse(IXFStream* pOutputStream);
    virtual void XFConvert(XFContentContainer* pCont);
    sal_uInt16 GetLines(){return m_nLines;}
    void SetChars(sal_uInt32 nChars){ m_nChars += nChars;}
    sal_uInt32 GetChars() const {return m_nChars;}
    LwpStory* GetContentStory();
    void RegisterStyle(LwpFoundry* pFoundry);
    void RegisterStyle();
protected:
    void Read();
private:
    sal_uInt16 m_nLines;
    sal_uInt32 m_nChars;
};

class LwpRubyLayout : public LwpFrameLayout
{
public:
    LwpRubyLayout(LwpObjectHeader &objHdr, LwpSvStream* pStrm);
    virtual ~LwpRubyLayout(){}
    LwpRubyMarker* GetMarker();
    void ConvertContentText();
    LwpStory* GetContentStory();
    sal_uInt8 GetPosition(){return m_nPlacement;}
    sal_uInt8 GetAlignment(){return m_nAlignment;}
    void RegisterStyle();
    enum{
    LEFT = 4,
    RIGHT = 5,
    CENTER = 2,
    TOP = 1,
    BOTTOM = 3
    };
protected:
    void Read();
private:
    sal_uInt8 m_nPlacement;
    sal_uInt8 m_nAlignment;
    sal_uInt16 m_nStateFlag;
    sal_Int32 m_nXOffset;
    sal_Int32 m_nYOffset;
    LwpObjectID m_objRubyMarker;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
