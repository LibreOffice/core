/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/

#ifndef _SVX_PAGECTRL_HXX
#define _SVX_PAGECTRL_HXX

#include <vcl/window.hxx>
#include "svx/svxdllapi.h"
#include <svx/sdr/attribute/sdrallfillattributeshelper.hxx>

class SvxBoxItem;

// class SvxPageWindow ---------------------------------------------------

class SVX_DLLPUBLIC SvxPageWindow : public Window
{
    using Window::GetBorder;

private:
    Size        aWinSize;
    Size        aSize;

    long        nTop;
    long        nBottom;
    long        nLeft;
    long        nRight;

    SvxBoxItem*     pBorder;
    sal_Bool        bResetBackground;
    sal_Bool        bFrameDirection;
    sal_Int32       nFrameDirection;

    long        nHdLeft;
    long        nHdRight;
    long        nHdDist;
    long        nHdHeight;
    SvxBoxItem* pHdBorder;

    long        nFtLeft;
    long        nFtRight;
    long        nFtDist;
    long        nFtHeight;
    SvxBoxItem* pFtBorder;

    //UUUU
    drawinglayer::attribute::SdrAllFillAttributesHelperPtr  maHeaderFillAttributes;
    drawinglayer::attribute::SdrAllFillAttributesHelperPtr  maFooterFillAttributes;
    drawinglayer::attribute::SdrAllFillAttributesHelperPtr  maPageFillAttributes;

    sal_Bool        bFooter : 1;
    sal_Bool        bHeader : 1;
    sal_Bool        bTable : 1;
    sal_Bool        bHorz : 1;
    sal_Bool        bVert : 1;

    sal_uInt16      eUsage;

    String      aLeftText;
    String      aRightText;

protected:
    virtual void Paint(const Rectangle& rRect);

    virtual void DrawPage(const Point& rPoint,
        const sal_Bool bSecond,
        const sal_Bool bEnabled);

    //UUUU
    void drawFillAttributes(
        const drawinglayer::attribute::SdrAllFillAttributesHelperPtr& rFillAttributes,
        const Rectangle& rPaintRange,
        const Rectangle& rDefineRange);

public:
    SvxPageWindow(Window* pParent,const ResId& rId);
    ~SvxPageWindow();

    //UUUU
    void setHeaderFillAttributes(const drawinglayer::attribute::SdrAllFillAttributesHelperPtr& rFillAttributes) { maHeaderFillAttributes = rFillAttributes; }
    void setFooterFillAttributes(const drawinglayer::attribute::SdrAllFillAttributesHelperPtr& rFillAttributes) { maFooterFillAttributes = rFillAttributes; }
    void setPageFillAttributes(const drawinglayer::attribute::SdrAllFillAttributesHelperPtr& rFillAttributes) { maPageFillAttributes = rFillAttributes; }
    const drawinglayer::attribute::SdrAllFillAttributesHelperPtr& getPageFillAttributes() const { return maPageFillAttributes; }

    void        SetWidth(long nW)                       { aSize.Width() = nW; }
    void        SetHeight(long nH)                  { aSize.Height() = nH; }

    void        SetSize(const Size& rSz)                { aSize = rSz; }
    const Size& GetSize() const                         { return aSize; }

    void        SetTop(long nNew)                       { nTop = nNew; }
    void        SetBottom(long nNew)                    { nBottom = nNew; }
    void        SetLeft(long nNew)                  { nLeft = nNew; }
    void        SetRight(long nNew)                 { nRight = nNew; }
    void        SetBorder(const SvxBoxItem& rNew);

    long        GetTop() const                          { return nTop; }
    long        GetBottom() const                       { return nBottom; }
    long        GetLeft() const                         { return nLeft; }
    long        GetRight() const                        { return nRight; }

    const SvxBoxItem&   GetBorder() const;

    void        SetHdLeft(long nNew)                    { nHdLeft = nNew; }
    void        SetHdRight(long nNew)                   { nHdRight = nNew; }
    void        SetHdDist(long nNew)                    { nHdDist = nNew; }
    void        SetHdHeight(long nNew)              { nHdHeight = nNew; }
    void        SetHdBorder(const SvxBoxItem& rNew);

    long        GetHdLeft() const                       { return nHdLeft; }
    long        GetHdRight() const                      { return nHdRight; }
    long        GetHdDist() const                       { return nHdDist; }
    long        GetHdHeight() const                     { return nHdHeight; }

    const SvxBoxItem&   GetHdBorder() const             { return *pHdBorder; }

    void        SetFtLeft(long nNew)                    { nFtLeft = nNew; }
    void        SetFtRight(long nNew)                   { nFtRight = nNew; }
    void        SetFtDist(long nNew)                    { nFtDist = nNew; }
    void        SetFtHeight(long nNew)              { nFtHeight = nNew; }
    void        SetFtBorder(const SvxBoxItem& rNew);

    long        GetFtLeft() const                       { return nFtLeft; }
    long        GetFtRight() const                      { return nFtRight; }
    long        GetFtDist() const                       { return nFtDist; }
    long        GetFtHeight() const                     { return nFtHeight; }

    const SvxBoxItem&   GetFtBorder() const             { return *pFtBorder; }

    void        SetUsage(sal_uInt16 eU)                 { eUsage = eU; }
    sal_uInt16      GetUsage() const                        { return eUsage; }

    void        SetHeader(sal_Bool bNew)                { bHeader = bNew; }
    sal_Bool        GetHeader() const                       { return bHeader; }
    void        SetFooter(sal_Bool bNew)                    { bFooter = bNew; }
    sal_Bool        GetFooter() const                       { return bFooter; }

    void        SetTable(sal_Bool bNew)                 { bTable = bNew; }
    sal_Bool        GetTable() const                        { return bTable; }
    void        SetHorz(sal_Bool bNew)                  { bHorz = bNew; }
    sal_Bool        GetHorz() const                         { return bHorz; }
    void        SetVert(sal_Bool bNew)                  { bVert = bNew; }
    sal_Bool        GetVert() const                         { return bVert; }

    void        EnableFrameDirection(sal_Bool bEnable);
    //uses enum SvxFrameDirection
    void        SetFrameDirection(sal_Int32 nDirection);

    void        ResetBackground();
};

#endif // #ifndef _SVX_PAGECTRL_HXX

//eof
