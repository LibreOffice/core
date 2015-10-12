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

#ifndef INCLUDED_LOTUSWORDPRO_SOURCE_FILTER_LWPFRIBMARK_HXX
#define INCLUDED_LOTUSWORDPRO_SOURCE_FILTER_LWPFRIBMARK_HXX

#include "lwpfrib.hxx"
#include "lwpstory.hxx"
#include "lwpmarker.hxx"
#include "xfilter/xftextspan.hxx"
#include "xfilter/xfbookmark.hxx"

class LwpFribCHBlock : public LwpFrib
{
public:
    explicit LwpFribCHBlock( LwpPara* pPara )
        : LwpFrib(pPara)
        , m_nType(0)
        {}
    virtual ~LwpFribCHBlock(){}
    void Read(LwpObjectStream* pObjStrm, sal_uInt16 len) override;
    LwpCHBlkMarker* GetMarker();
    sal_uInt8 GetType(){return m_nType;}
    enum{MARKER_START=1,MARKER_END,MARKER_NONE};
    void XFConvert(XFContentContainer* pXFPara,LwpStory* pStory);
private:
    sal_uInt8 m_nType;
    LwpObjectID m_objMarker;
};

class LwpFribBookMark : public LwpFrib
{
public:
    explicit LwpFribBookMark( LwpPara* pPara );
    virtual ~LwpFribBookMark(){}
    void Read(LwpObjectStream* pObjStrm, sal_uInt16 len) override;
    LwpObjectID GetMarkerID(){return m_objMarker;}
    sal_uInt8 GetType(){return m_nType;}
    enum{MARKER_START=1,MARKER_END,MARKER_NONE};
    void XFConvert(XFContentContainer* pXFPara);
    void RegisterStyle(LwpFoundry* pFoundry) override;
private:
    sal_uInt8 m_nType;
    LwpObjectID m_objMarker;
    XFBookmarkStart* m_pStart;
    XFBookmarkEnd* m_pEnd;
};

class LwpFribField : public LwpFrib
{
public:
    explicit LwpFribField( LwpPara* pPara );
    virtual ~LwpFribField(){}
    void Read(LwpObjectStream* pObjStrm, sal_uInt16 len) override;
    LwpFieldMark* GetMarker();
    sal_uInt8 GetType(){return m_nType;}
    enum{MARKER_START=1,MARKER_END,MARKER_NONE};
    void XFConvert(XFContentContainer* pXFPara);
    void RegisterStyle(LwpFoundry* pFoundry) override;
    enum{
        SUBFIELD_INVALID = 0,
        SUBFIELD_DATETIME = 1,
        SUBFIELD_CROSSREF = 2,
        SUBFIELD_DOCPOWER = 3,
    };
private:
    sal_uInt8 m_nType;
    LwpObjectID m_objMarker;
    OUString m_TimeStyle;

    sal_uInt8 m_nSubType;//datetime/crossref/other
    OUString m_sFormula;

    sal_uInt8 m_nCrossRefType;
    sal_uInt8 m_nDateTimeType;
    sal_uInt8 m_nDocPowerType;

    void RegisterTimeField(LwpFieldMark* pFieldMark);
    void RegisterDateTimeStyle(const OUString& sFormula);
    void RegisterTotalTimeStyle();
    void CheckFieldType(LwpFieldMark* pFieldMark);
    void ConvertDocFieldStart(XFContentContainer* pXFPara,LwpFieldMark* pFieldMark);
    void ConvertDocFieldEnd(XFContentContainer* pXFPara,LwpFieldMark* pFieldMark);
    void ConvertDateTimeStart(XFContentContainer* pXFPara,LwpFieldMark* pFieldMark);
    void ConvertDateTimeEnd(XFContentContainer* pXFPara,LwpFieldMark* pFieldMark);
    static void ConvertCrossRefEnd(XFContentContainer* pXFPara,LwpFieldMark* pFieldMark);
    void ConvertCrossRefStart(XFContentContainer* pXFPara,LwpFieldMark* pFieldMark);
};

class LwpFribRubyMarker : public LwpFrib
{
public:
    explicit LwpFribRubyMarker( LwpPara* pPara );
    virtual ~LwpFribRubyMarker(){}
    void Read(LwpObjectStream* pObjStrm, sal_uInt16 len) override;
    LwpRubyMarker* GetMarker();
    sal_uInt8 GetType(){return m_nType;}
    enum{MARKER_START=1,MARKER_END,MARKER_NONE};
    void XFConvert(XFContentContainer* pXFPara);
    void RegisterStyle(LwpFoundry* pFoundry) override;
private:
    sal_uInt8 m_nType;
    LwpObjectID m_objMarker;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
