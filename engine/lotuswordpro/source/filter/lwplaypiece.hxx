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
 *  For LWP filter architecture prototype
 ************************************************************************/

#ifndef INCLUDED_LOTUSWORDPRO_SOURCE_FILTER_LWPLAYPIECE_HXX
#define INCLUDED_LOTUSWORDPRO_SOURCE_FILTER_LWPLAYPIECE_HXX

#include "lwppiece.hxx"
#include <lwpbasetype.hxx>
#include "lwpmargins.hxx"
#include "lwpborderstuff.hxx"
#include "lwpshadow.hxx"

class LwpRotor
{
public:
    LwpRotor();
    void Read(LwpObjectStream* pStrm);

private:
    sal_Int16 m_nRotation; //angle
};

class LwpLayoutGeometry final : public LwpVirtualPiece
{
public:
    LwpLayoutGeometry(LwpObjectHeader const& objHdr, LwpSvStream* pStrm);
    virtual void Parse(IXFStream* pOutputStream) override;
    sal_Int32 GetWidth() const { return m_nWidth; }
    sal_Int32 GetHeight() const { return m_nHeight; }
    const LwpPoint& GetOrigin() const { return m_Origin; }
    const LwpPoint& GetAbsoluteOrigin() const { return m_AbsoluteOrigin; }
    sal_uInt8 GetContentOrientation() const { return m_ContentOrientation; }

private:
    virtual void Read() override;
    virtual ~LwpLayoutGeometry() override;

    sal_Int32 m_nWidth;
    sal_Int32 m_nHeight;
    LwpPoint m_Origin;
    LwpPoint m_AbsoluteOrigin;
    LwpRotor m_ContainerRotor;
    sal_uInt8 m_ContentOrientation;
};

class LwpLayoutScale final : public LwpVirtualPiece
{
public:
    enum
    {
        ORIGINAL_SIZE = 1,
        FIT_IN_FRAME = 2,
        PERCENTAGE = 4,
        CUSTOM = 8,
        MAINTAIN_ASPECT_RATIO = 16
    };

    enum
    {
        CENTERED = 1,
        TILED = 2
    };
    LwpLayoutScale(LwpObjectHeader const& objHdr, LwpSvStream* pStrm);
    virtual void Parse(IXFStream* pOutputStream) override;
    sal_uInt16 GetScaleMode() const { return m_nScaleMode; }
    sal_uInt32 GetScalePercentage() const { return m_nScalePercentage; }
    sal_Int32 GetScaleWidth() const { return m_nScaleWidth; }
    sal_Int32 GetScaleHeight() const { return m_nScaleHeight; }
    sal_uInt16 GetPlacement() const { return m_nPlacement; }
    LwpPoint& GetOffset() { return m_Offset; }

private:
    virtual void Read() override;
    virtual ~LwpLayoutScale() override;

    sal_uInt16 m_nScaleMode;
    sal_uInt32 m_nScalePercentage;
    sal_Int32 m_nScaleWidth;
    sal_Int32 m_nScaleHeight;
    sal_uInt16 m_nContentRotation;
    LwpPoint m_Offset;
    sal_uInt16 m_nPlacement;
};

class LwpLayoutMargins final : public LwpVirtualPiece
{
public:
    LwpLayoutMargins(LwpObjectHeader const& objHdr, LwpSvStream* pStrm);
    virtual void Parse(IXFStream* pOutputStream) override;
    LwpMargins& GetMargins() { return m_Margins; }
    LwpMargins& GetExtMargins() { return m_ExtMargins; }

private:
    virtual void Read() override;
    virtual ~LwpLayoutMargins() override;

    LwpMargins m_Margins;
    LwpMargins m_ExtMargins;
    LwpMargins m_ExtraMargins;
};

class LwpLayoutBorder final : public LwpVirtualPiece
{
public:
    LwpLayoutBorder(LwpObjectHeader const& objHdr, LwpSvStream* pStrm);
    virtual void Parse(IXFStream* pOutputStream) override;
    LwpBorderStuff& GetBorderStuff() { return m_BorderStuff; }

private:
    virtual void Read() override;
    virtual ~LwpLayoutBorder() override;

    LwpBorderStuff m_BorderStuff;
};

class LwpLayoutBackground final : public LwpVirtualPiece
{
public:
    LwpLayoutBackground(LwpObjectHeader const& objHdr, LwpSvStream* pStrm);
    virtual void Parse(IXFStream* pOutputStream) override;
    LwpBackgroundStuff& GetBackgoudStuff() { return m_BackgroundStuff; }

private:
    virtual void Read() override;
    virtual ~LwpLayoutBackground() override;

    LwpBackgroundStuff m_BackgroundStuff;
};

class LwpExternalBorder
{
public:
    LwpExternalBorder();
    ~LwpExternalBorder();
    void Read(LwpObjectStream* pStrm);

private:
    LwpAtomHolder m_LeftName;
    LwpAtomHolder m_TopName;
    LwpAtomHolder m_RightName;
    LwpAtomHolder m_BottomName;
};

//It seems that this class is used for designer border. Only read now.
class LwpLayoutExternalBorder final : public LwpVirtualPiece
{
public:
    LwpLayoutExternalBorder(LwpObjectHeader const& objHdr, LwpSvStream* pStrm);
    virtual void Parse(IXFStream* pOutputStream) override;

private:
    virtual void Read() override;
    virtual ~LwpLayoutExternalBorder() override;

    LwpExternalBorder m_ExtranalBorder;
};

class LwpColumnInfo
{
public:
    LwpColumnInfo();
    void Read(LwpObjectStream* pStrm);
    double GetGap() { return LwpTools::ConvertFromUnits(m_nGap); }

private:
    sal_Int32 m_nWidth;
    sal_Int32 m_nGap;
};

class LwpLayoutColumns final : public LwpVirtualPiece
{
public:
    LwpLayoutColumns(LwpObjectHeader const& objHdr, LwpSvStream* pStrm);
    virtual void Parse(IXFStream* pOutputStream) override;
    sal_uInt16 GetNumCols() const { return m_nNumCols; }
    double GetColGap(sal_uInt16 nIndex);

private:
    virtual void Read() override;
    virtual ~LwpLayoutColumns() override;

    sal_uInt16 m_nNumCols;
    std::unique_ptr<LwpColumnInfo[]> m_pColumns;
};

class LwpLayoutGutters final : public LwpVirtualPiece
{
public:
    LwpLayoutGutters(LwpObjectHeader const& objHdr, LwpSvStream* pStrm);
    virtual void Parse(IXFStream* pOutputStream) override;
    LwpBorderStuff& GetBorderStuff() { return m_BorderBuffer; }

private:
    virtual void Read() override;
    virtual ~LwpLayoutGutters() override;

    LwpBorderStuff m_BorderBuffer;
};

class LwpJoinStuff
{
public:
    LwpJoinStuff();
    void Read(LwpObjectStream* pStrm);

private:
    sal_uInt16 m_nPercentage;
    sal_uInt16 m_nCorners;
    sal_Int32 m_nWidth;
    sal_Int32 m_nHeight;

    enum JoinType
    {
        MITRE = 1,
        NEGATE = 2,
        ROUNDED = 3,
        RECTANGLE = 4,
        SPECIAL = 5,
        DIAGONAL = 6,
        NEGATE_NO_CROSS = 7,
        DOG_EAR_PAGE = 8,
        DESKTOP = 9,
        BOX_HIGHLIGHT = 10,
        STAR = 11,
        ROPE = 12,
        DECO1 = 13,
        DECO2 = 14,
        RAIN = 15,
        PIN = 16,
        ROSE = 17,
        SUNF = 18,
        DECO3 = 19,
        WARNING = 20,
        BUBBLE = 21,
        GIRDER = 22,
        SMILE = 23,
        ARROW = 24,
        MAXJOIN = 24
    };

    sal_uInt16 m_nID; //JoinType
    sal_uInt16 m_nScaling;
    LwpColor m_Color;
};

class LwpLayoutJoins final : public LwpVirtualPiece
{
public:
    LwpLayoutJoins(LwpObjectHeader const& objHdr, LwpSvStream* pStrm);
    virtual void Parse(IXFStream* pOutputStream) override;

private:
    virtual void Read() override;
    virtual ~LwpLayoutJoins() override;

    LwpJoinStuff m_JoinStuff;
};

class LwpLayoutShadow final : public LwpVirtualPiece
{
public:
    LwpLayoutShadow(LwpObjectHeader const& objHdr, LwpSvStream* pStrm);
    virtual void Parse(IXFStream* pOutputStream) override;
    LwpShadow& GetShadow() { return m_Shadow; }

private:
    virtual void Read() override;
    virtual ~LwpLayoutShadow() override;

    LwpShadow m_Shadow;
};

class LwpLayoutRelativityGuts
{
public:
    LwpLayoutRelativityGuts();
    void Read(LwpObjectStream* pStrm);
    sal_uInt8 GetRelativeType() const { return m_nRelType; }
    enum RelativeType
    {
        LAY_PARENT_RELATIVE = 1,
        LAY_PARA_RELATIVE,
        LAY_INLINE,
        LAY_INLINE_NEWLINE,
        LAY_CONTENT_RELATIVE,
        LAY_INLINE_VERTICAL
    };
    enum WhereType
    {
        LAY_UPPERLEFT = 1,
        LAY_MIDDLETOP,
        LAY_UPPERRIGHT,
        LAY_MIDDLELEFT,
        LAY_MIDDLERIGHT,
        LAY_LOWERLEFT,
        LAY_MIDDLEBOTTOM,
        LAY_LOWERRIGHT,
        LAY_MIDDLE
    };
    enum TetherWhereType
    {
        LAY_INTERNAL = 1,
        LAY_EXTERNAL,
        LAY_BORDER
    };

private:
    sal_uInt8 m_nRelType;
    sal_uInt8 m_nRelFromWhere;
    LwpPoint m_RelDistance;
    sal_uInt8 m_nTether;
    sal_uInt8 m_nTetherWhere;
    sal_uInt8 m_nFlags;
};

class LwpLayoutRelativity final : public LwpVirtualPiece
{
public:
    LwpLayoutRelativity(LwpObjectHeader const& objHdr, LwpSvStream* pStrm);
    virtual void Parse(IXFStream* pOutputStream) override;
    LwpLayoutRelativityGuts& GetRelGuts() { return m_RelGuts; }

private:
    virtual void Read() override;
    virtual ~LwpLayoutRelativity() override;

    LwpLayoutRelativityGuts m_RelGuts;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
