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
/**
 * @file
  * The file declares the LwpDrawObjcts and associated class like LwpDrawGroup, LwpDrawRectange
 *	and so on.
 */

#ifndef _LWPDRAWOBJ_HXX
#define _LWPDRAWOBJ_HXX

#include "lwpsdwdrawheader.hxx"

class SvStream;
class XFFrame;
class XFDrawStyle;

/**
 * @brief
 * Lwp-base-draw object.
 */
class LwpDrawObj
{
protected:
    DrawObjectType m_eType;
    SvStream* m_pStream;
    SdwDrawObjHeader m_aObjHeader;
    SdwClosedObjStyleRec m_aClosedObjStyleRec;
    DrawingOffsetAndScale* m_pTransData;

public:
    LwpDrawObj(SvStream* pStream, DrawingOffsetAndScale* pTransData = NULL);
    virtual ~LwpDrawObj() {};

private:
    void ReadObjHeaderRecord();

protected:
    void ReadClosedObjStyle();
    void SetFillStyle(XFDrawStyle* pStyle);
    void SetLineStyle(XFDrawStyle* pStyle, sal_uInt8 nWidth, sal_uInt8 nLineStyle,
        const SdwColor& rColor);
    void SetPosition(XFFrame* pObj);
    void SetArrowHead(XFDrawStyle* pOpenedObjStyle, sal_uInt8 nArrowFlag, sal_uInt8 nLineWidth);
    rtl::OUString GetArrowName(sal_uInt8 nArrowStyle);

protected:
    /**
     * @descr	read out the record of a draw object.
     */
    virtual void Read() = 0;

    /**
     * @descr	register styles of a draw object according to the saved records data.
     * @return	the style name which has been registered.
     */
    virtual rtl::OUString RegisterStyle() = 0;

    /**
     * @descr	create XF-draw object and assign the style name to it.
     * @param	style name.
     * @return	pointer of the created XF-draw object.
     */
    virtual XFFrame* CreateDrawObj(const rtl::OUString& rStyleName) = 0;

    /**
     * @descr	create XF-draw object and assign the style name to it.
     * @param	style name.
     * @return	pointer of the created XF-draw object.
     */
    virtual XFFrame* CreateStandardDrawObj(const rtl::OUString& rStyleName) = 0;

public:
    /**
     * @descr	create a completed XF-draw object(read data, register styles and create XF-draw object)
     * @return	pointer of the created competed XF-draw object.
     */
    XFFrame* CreateXFDrawObject();

    /**
     * @param	type of the object.
     * @descr	set the type to the draw object.
     */
    inline void SetObjectType(DrawObjectType eType) { m_eType = eType; }

    /**
     * @descr	get the type of the draw object.
     * @return	the type of the object.
     */
    inline  DrawObjectType GetObjectType() const { return m_eType; }
};


/**
 * @brief
 * Lwp-draw-group object.
 */
class LwpDrawGroup : public LwpDrawObj
{
public:
    LwpDrawGroup(SvStream* pStream) : LwpDrawObj(pStream) {}
    virtual ~LwpDrawGroup() {}

protected:
    virtual void Read() {}
    virtual rtl::OUString RegisterStyle() { return rtl::OUString::createFromAscii(""); }
    virtual XFFrame* CreateDrawObj(const rtl::OUString& /*rStyleName*/) { return NULL; }
    virtual XFFrame* CreateStandardDrawObj(const rtl::OUString& /*rStyleName*/) { return NULL; }
};


/**
 * @brief
 * Lwp-draw-line object.
 */
class LwpDrawLine : public LwpDrawObj
{
private:
    SdwLineRecord m_aLineRec;

public:
    LwpDrawLine(SvStream * pStream, DrawingOffsetAndScale* pTransData);
    virtual ~LwpDrawLine() {};

protected:
    virtual void Read();
    virtual rtl::OUString RegisterStyle();
    virtual XFFrame* CreateDrawObj(const rtl::OUString& rStyleName);
    virtual XFFrame* CreateStandardDrawObj(const rtl::OUString& rStyleName);
};

/**
 * @brief
 * Lwp-draw-polyline object.
 */
class LwpDrawPolyLine : public LwpDrawObj
{
private:
    SdwPolyLineRecord m_aPolyLineRec;
    SdwPoint* m_pVector;

public:
    LwpDrawPolyLine(SvStream * pStream, DrawingOffsetAndScale* pTransData);
    virtual ~LwpDrawPolyLine();

protected:
    virtual void Read();
    virtual rtl::OUString RegisterStyle();
    virtual XFFrame* CreateDrawObj(const rtl::OUString& rStyleName);
    virtual XFFrame* CreateStandardDrawObj(const rtl::OUString& rStyleName);
};


 /**
 * @brief
 * Lwp-draw-polygon object.
 */
class LwpDrawPolygon : public LwpDrawObj
{
private:
    sal_uInt16 m_nNumPoints;
    SdwPoint* m_pVector;

public:
    LwpDrawPolygon(SvStream * pStream, DrawingOffsetAndScale* pTransData);
    virtual ~LwpDrawPolygon();

protected:
    virtual void Read();
    virtual rtl::OUString RegisterStyle();
    virtual XFFrame* CreateDrawObj(const rtl::OUString& rStyleName);
    virtual XFFrame* CreateStandardDrawObj(const rtl::OUString& rStyleName);
};

/**
 * @brief
 * Lwp-draw-rectangle(rounded-corner rectangle) object.
 */
class LwpDrawRectangle : public LwpDrawObj
{
private:
    SdwPoint m_aVector[16];

public:
    LwpDrawRectangle(SvStream* pStream, DrawingOffsetAndScale* pTransData);
    virtual ~LwpDrawRectangle(){}

protected:
    virtual void Read();
    virtual rtl::OUString RegisterStyle();
    virtual XFFrame* CreateDrawObj(const rtl::OUString& rStyleName);
    virtual XFFrame* CreateStandardDrawObj(const rtl::OUString& rStyleName);

private:
    XFFrame* CreateRoundedRect(const rtl::OUString& rStyleName);
};

/**
 * @brief
 * Lwp-draw-ellipse object.
 */
class LwpDrawEllipse : public LwpDrawObj
{
private:
    SdwPoint m_aVector[13];

public:
    LwpDrawEllipse(SvStream * pStream, DrawingOffsetAndScale* pTransData);
    virtual ~LwpDrawEllipse(){};

protected:
    virtual void Read();
    virtual rtl::OUString RegisterStyle();
    virtual XFFrame* CreateDrawObj(const rtl::OUString& rStyleName);
    virtual XFFrame* CreateStandardDrawObj(const rtl::OUString& rStyleName);
};


/**
 * @brief
 * Lwp-draw-arc object.
 */
class LwpDrawArc : public LwpDrawObj
{
private:
    SdwArcRecord m_aArcRec;
    SdwPoint m_aVector[4];

public:
    LwpDrawArc(SvStream * pStream, DrawingOffsetAndScale* pTransData);
    virtual ~LwpDrawArc() {};

protected:
    virtual void Read();
    virtual rtl::OUString RegisterStyle();
    virtual XFFrame* CreateDrawObj(const rtl::OUString& rStyleName);
    virtual XFFrame* CreateStandardDrawObj(const rtl::OUString& rStyleName);
};

/**
 * @brief
 * Lwp-draw-textbox object.
 */
class  XFFont;
class LwpDrawTextBox : public LwpDrawObj
{
private:
    SdwTextBoxRecord m_aTextRec;
    SdwPoint m_aVector;

public:
    LwpDrawTextBox(SvStream* pStream);
    virtual ~LwpDrawTextBox();
    static void SetFontStyle(XFFont* pFont, SdwTextBoxRecord* pRec);

protected:
    virtual void Read();
    virtual rtl::OUString RegisterStyle();
    virtual XFFrame* CreateDrawObj(const rtl::OUString& rStyleName);
    virtual XFFrame* CreateStandardDrawObj(const rtl::OUString& rStyleName);
};

/**
 * @brief
 * Lwp-draw-curved-text object.
 */
class XFDrawPath;
class LwpDrawTextArt : public LwpDrawObj
{
private:
    SdwTextArt m_aTextArtRec;
    SdwPoint m_aVector[4];

private:
    void CreateFWPath(XFDrawPath* pPath);

public:
    LwpDrawTextArt(SvStream* pStream, DrawingOffsetAndScale* pTransData);
    virtual ~LwpDrawTextArt();

protected:
    virtual void Read();
    virtual rtl::OUString RegisterStyle();
    virtual XFFrame* CreateDrawObj(const rtl::OUString& rStyleName);
    virtual XFFrame* CreateStandardDrawObj(const rtl::OUString& rStyleName);
};

/**
 * @brief
 * Lwp-draw-metafile object.
 */
class LwpDrawMetafile : public LwpDrawObj
{
public:
    LwpDrawMetafile(SvStream* pStream);
    virtual ~LwpDrawMetafile() {}

protected:
    virtual void Read();
    virtual rtl::OUString RegisterStyle() {return rtl::OUString::createFromAscii("");}
    virtual XFFrame* CreateDrawObj(const rtl::OUString& /*rStyleName*/){return NULL;}
    virtual XFFrame* CreateStandardDrawObj(const rtl::OUString& /*rStyleName*/){return NULL;}
};

/**
 * @brief
 * Lwp-draw-bitmap object.
 */
class LwpDrawBitmap : public LwpDrawObj
{
private:
    SdwBmpRecord m_aBmpRec;
    sal_uInt8* m_pImageData;
public:
    LwpDrawBitmap(SvStream* pStream);
    virtual ~LwpDrawBitmap();

protected:
    virtual void Read();
    virtual rtl::OUString RegisterStyle();
    virtual XFFrame* CreateDrawObj(const rtl::OUString& rStyleName);
    virtual XFFrame* CreateStandardDrawObj(const rtl::OUString& rStyleName);
};
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
