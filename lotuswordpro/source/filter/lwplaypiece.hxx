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
#include "lwpbasetype.hxx"

class LwpRotor
{
public:
    LwpRotor();
    ~LwpRotor();
    void Read(LwpObjectStream *pStrm);
private:
    sal_Int16 m_nRotation;  //angle
};

class LwpLayoutGeometry : public LwpVirtualPiece
{
public:
    LwpLayoutGeometry(LwpObjectHeader& objHdr, LwpSvStream* pStrm);
    virtual void Parse(IXFStream* pOutputStream) override;
    sal_Int32 GetWidth() { return m_nWidth; }
    sal_Int32 GetHeight() { return m_nHeight; }
    LwpPoint GetOrigin() { return m_Origin; }
    LwpPoint GetAbsoluteOrigin() { return m_AbsoluteOrigin; }
    sal_uInt8 GetContentOrientation(){ return m_ContentOrientation;}

protected:
    virtual void Read() override;
protected:
    sal_Int32 m_nWidth;
    sal_Int32 m_nHeight;
    LwpPoint m_Origin;
    LwpPoint m_AbsoluteOrigin;
    LwpRotor m_ContainerRotor;
    sal_uInt8 m_ContentOrientation;
private:
    virtual ~LwpLayoutGeometry();
};

class LwpLayoutScale : public LwpVirtualPiece
{
public:
    enum    {ORIGINAL_SIZE = 1, FIT_IN_FRAME = 2, PERCENTAGE = 4,
             CUSTOM = 8, MAINTAIN_ASPECT_RATIO = 16};

    enum    {CENTERED = 1, TILED = 2};
    LwpLayoutScale(LwpObjectHeader& objHdr, LwpSvStream* pStrm);
    virtual void Parse(IXFStream* pOutputStream) override;
    sal_uInt16 GetScaleMode(){return m_nScaleMode;}
    sal_uInt32 GetScalePercentage(){return m_nScalePercentage;}
    sal_Int32 GetScaleWidth(){return m_nScaleWidth;}
    sal_Int32 GetScaleHeight(){return m_nScaleHeight;}
    sal_uInt16 GetPlacement(){return m_nPlacement;}
    inline LwpPoint& GetOffset() {return m_Offset;}
protected:
    virtual void Read() override;
protected:
    sal_uInt16 m_nScaleMode;
    sal_uInt32 m_nScalePercentage;
    sal_Int32 m_nScaleWidth;
    sal_Int32 m_nScaleHeight;
    sal_uInt16 m_nContentRotation;
    LwpPoint m_Offset;
    sal_uInt16 m_nPlacement;
private:
    virtual ~LwpLayoutScale();
};

#include "lwpmargins.hxx"

class LwpLayoutMargins : public LwpVirtualPiece
{
public:
    LwpLayoutMargins(LwpObjectHeader& objHdr, LwpSvStream* pStrm);
    virtual void Parse(IXFStream* pOutputStream) override;
    LwpMargins& GetMargins() { return m_Margins; }
    LwpMargins& GetExtMargins(){ return m_ExtMargins;}
protected:
    virtual void Read() override;
protected:
    LwpMargins m_Margins;
    LwpMargins m_ExtMargins;
    LwpMargins m_ExtraMargins;
private:
    virtual ~LwpLayoutMargins();
};

#include "lwpborderstuff.hxx"
class LwpLayoutBorder : public LwpVirtualPiece
{
public:
    LwpLayoutBorder(LwpObjectHeader& objHdr, LwpSvStream* pStrm);
    virtual void Parse(IXFStream* pOutputStream) override;
    LwpBorderStuff& GetBorderStuff(){ return m_BorderStuff;}
protected:
    virtual void Read() override;
protected:
    LwpBorderStuff m_BorderStuff;
private:
    virtual ~LwpLayoutBorder();
};

class LwpLayoutBackground : public LwpVirtualPiece
{
public:
    LwpLayoutBackground(LwpObjectHeader& objHdr, LwpSvStream* pStrm);
    virtual void Parse(IXFStream* pOutputStream) override;
    LwpBackgroundStuff& GetBackgoudStuff(){return m_BackgroundStuff;}
protected:
    virtual void Read() override;
protected:
    LwpBackgroundStuff m_BackgroundStuff;
private:
    virtual ~LwpLayoutBackground();
};

class LwpExternalBorder
{
public:
    LwpExternalBorder();
    ~LwpExternalBorder();
    void Read(LwpObjectStream *pStrm);
private:
    LwpAtomHolder m_LeftName;
    LwpAtomHolder m_TopName;
    LwpAtomHolder m_RightName;
    LwpAtomHolder m_BottomName;
};

//It seems that this class is used for designer border. Only read now.
class LwpLayoutExternalBorder : public LwpVirtualPiece
{
public:
    LwpLayoutExternalBorder(LwpObjectHeader& objHdr, LwpSvStream* pStrm);
    virtual void Parse(IXFStream* pOutputStream) override;
protected:
    virtual void Read() override;
protected:
    LwpExternalBorder   m_ExtranalBorder;
private:
    virtual ~LwpLayoutExternalBorder();
};

class LwpColumnInfo
{
public:
    LwpColumnInfo();
    ~LwpColumnInfo();
    void Read(LwpObjectStream *pStrm);
    inline double GetWidth(){return LwpTools::ConvertFromUnitsToMetric(m_nWidth);}
    inline double GetGap(){return LwpTools::ConvertFromUnitsToMetric(m_nGap);}
private:
    sal_Int32 m_nWidth;
    sal_Int32 m_nGap;
};

class LwpLayoutColumns : public LwpVirtualPiece
{
public:
    LwpLayoutColumns(LwpObjectHeader& objHdr, LwpSvStream* pStrm);
    virtual void Parse(IXFStream* pOutputStream) override;
    inline sal_uInt16 GetNumCols(){return m_nNumCols;}
    double GetColWidth(sal_uInt16 nIndex);
    double GetColGap(sal_uInt16 nIndex);
protected:
    virtual void Read() override;
protected:
    sal_uInt16 m_nNumCols;
    LwpColumnInfo* m_pColumns;
private:
    virtual ~LwpLayoutColumns();
};

class LwpLayoutGutters : public LwpVirtualPiece
{
public:
    LwpLayoutGutters(LwpObjectHeader& objHdr, LwpSvStream* pStrm);
    virtual void Parse(IXFStream* pOutputStream) override;
    inline LwpBorderStuff& GetBorderStuff() { return m_BorderBuffer; }
protected:
    virtual void Read() override;
protected:
    LwpBorderStuff m_BorderBuffer;
private:
    virtual ~LwpLayoutGutters();
};

class LwpJoinStuff
{
public:
    LwpJoinStuff();
    ~LwpJoinStuff();
    void Read(LwpObjectStream *pStrm);
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

    sal_uInt16 m_nID;//JoinType
    sal_uInt16 m_nScaling;
    LwpColor m_Color;
};

class LwpLayoutJoins : public LwpVirtualPiece
{
public:
    LwpLayoutJoins(LwpObjectHeader& objHdr, LwpSvStream* pStrm);
    virtual void Parse(IXFStream* pOutputStream) override;
protected:
    virtual void Read() override;
protected:
    LwpJoinStuff m_JoinStuff;
private:
    virtual ~LwpLayoutJoins();
};

#include "lwpshadow.hxx"

class LwpLayoutShadow : public LwpVirtualPiece
{
public:
    LwpLayoutShadow(LwpObjectHeader& objHdr, LwpSvStream* pStrm);
    virtual void Parse(IXFStream* pOutputStream) override;
    LwpShadow& GetShadow() { return m_Shadow; }
protected:
    virtual void Read() override;
protected:
    LwpShadow m_Shadow;
private:
    virtual ~LwpLayoutShadow();
};

class LwpLayoutRelativityGuts
{
public:
    LwpLayoutRelativityGuts();
    void Read(LwpObjectStream *pStrm);
    sal_uInt8 GetRelativeType(){ return m_nRelType;}
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
    sal_uInt8   m_nRelType;
    sal_uInt8   m_nRelFromWhere;
    LwpPoint    m_RelDistance;
    sal_uInt8   m_nTether;
    sal_uInt8   m_nTetherWhere;
    sal_uInt8   m_nFlags;
};

class LwpLayoutRelativity: public LwpVirtualPiece
{
public:
    LwpLayoutRelativity(LwpObjectHeader& objHdr, LwpSvStream* pStrm);
    virtual void Parse(IXFStream* pOutputStream) override;
    LwpLayoutRelativityGuts& GetRelGuts() { return m_RelGuts; }
protected:
    virtual void Read() override;
protected:
    LwpLayoutRelativityGuts m_RelGuts;
private:
    virtual ~LwpLayoutRelativity();
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
