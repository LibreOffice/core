/*************************************************************************
 *
 *  $RCSfile: escher.hxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: rt $ $Date: 2003-09-25 07:40:09 $
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
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil -*- */

#ifndef _ESCHER_HXX
#define _ESCHER_HXX

#ifndef _ESCHEREX_HXX
#include <svx/escherex.hxx>
#endif

class SwFrmFmt;

class WinwordAnchoring : public EscherExClientRecord_Base
{
public:
    void WriteData(EscherEx& rEx) const;
    void SetAnchoring(const SwFrmFmt& rFmt, bool bBROKEN = false);
private:
    sal_uInt32 nXAlign;
    sal_uInt32 nYAlign;
    sal_uInt32 nXRelTo;
    sal_uInt32 nYRelTo;
};

class SwBasicEscherEx : public EscherEx
{
private:
    void Init();
protected:
    SwWW8Writer& rWrt;
    SvStream* pEscherStrm;
    SvStream* pPictStrm;
    long mnEmuMul, mnEmuDiv;

    virtual INT32 WriteFlyFrameAttr(const SwFrmFmt& rFmt, MSO_SPT eShapeType,
        EscherPropertyContainer& rPropOpt);
    void WriteBrushAttr(const SvxBrushItem &rBrush,
        EscherPropertyContainer& rPropOpt);
    void WritePicture(EscherPropertyContainer &rPropOpt,
        const Graphic &rGraphic, const SdrObject &rObj, sal_uInt32 nShapeId);
    void WriteGrfAttr(const SwNoTxtNode& rNd,EscherPropertyContainer& rPropOpt);

    INT32 DrawModelToEmu(INT32 nVal) const
        { return BigMulDiv(nVal, mnEmuMul, mnEmuDiv); }

    INT32 ToFract16(INT32 nVal, UINT32 nMax) const;

    SvStream* QueryPicStream();

    virtual void SetPicId(const SdrObject &, UINT32, EscherPropertyContainer &);
    SdrLayerID GetInvisibleHellId() const
        {return rWrt.pDoc->GetInvisibleHellId();}
public:
    SwBasicEscherEx(SvStream* pStrm, SwWW8Writer& rWrt, UINT32 nDrawings = 1);
    INT32 WriteGrfFlyFrame(const SwFrmFmt& rFmt, UINT32 nShapeId);
    INT32 WriteOLEFlyFrame(const SwFrmFmt& rFmt, UINT32 nShapeId);
    virtual void WriteFrmExtraData(const SwFrmFmt&);
    virtual void WritePictures();
    virtual ~SwBasicEscherEx();
private:
    //No copying
    SwBasicEscherEx(const SwBasicEscherEx&);
    SwBasicEscherEx& operator=(const SwBasicEscherEx&);
};

class SwEscherEx : public SwBasicEscherEx
{
private:
    SvULongs aFollowShpIds;
    std::vector<short> maDirections;
    SvPtrarr aSortFmts;
    EscherExHostAppData aHostData;
    WinwordAnchoring aWinwordAnchoring;
    WW8_WrPlcTxtBoxes *pTxtBxs;

    UINT32 GetFlyShapeId( const SwFrmFmt& rFmt );
    void MakeZOrderArrAndFollowIds(std::vector<DrawObj>& rSrcArr,
        std::vector<DrawObj*>& rDstArr);

    INT32 WriteFlyFrm(const SwFrmFmt& rFmt,UINT32 &rShapeId, short nDirection);
    INT32 WriteTxtFlyFrame(const SwFrmFmt& rFmt, UINT32 nShapeId,
        UINT32 nTxtBox, short nDirection);
    void WriteOCXControl(const SwFrmFmt& rFmt,UINT32 nShapeId);
    virtual INT32 WriteFlyFrameAttr(const SwFrmFmt& rFmt, MSO_SPT eShapeType,
        EscherPropertyContainer& rPropOpt);

    virtual UINT32 QueryTextID(
        const com::sun::star::uno::Reference<
        com::sun::star::drawing::XShape > &,UINT32);
    virtual void SetPicId(const SdrObject &rSdrObj, UINT32 nShapeId,
        EscherPropertyContainer &rPropOpt);
public:
    SwEscherEx( SvStream* pStrm, SwWW8Writer& rWW8Wrt );
    virtual ~SwEscherEx();
    void FinishEscher();
    virtual void WritePictures();

    virtual void WriteFrmExtraData(const SwFrmFmt& rFmt);

    EscherExHostAppData* StartShape(const com::sun::star::uno::Reference<
        com::sun::star::drawing::XShape > &) {return &aHostData;}
private:
    //No copying
    SwEscherEx(const SwEscherEx&);
    SwEscherEx &operator=(const SwEscherEx&);
};

#endif

/* vi:set tabstop=4 shiftwidth=4 expandtab: */
