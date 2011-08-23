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
/*************************************************************************
 * Change History
 Jan 2005			Created
 ************************************************************************/
#ifndef _LWPFRIBTEXT_HXX_
#define _LWPFRIBTEXT_HXX_

#include "lwpfrib.hxx"
#include "lwpstory.hxx"
class LwpFribText : public LwpFrib
{
public:
    LwpFribText( LwpPara* pPara, sal_Bool bNoUnicode=false);
    ~LwpFribText(){};
    void Read(LwpObjectStream* pObjStrm, sal_uInt16 len);
    void XFConvert(XFContentContainer* pXFPara,LwpStory* pStory);
private:
    OUString m_Content;
    sal_Bool m_bNoUnicode;
public:
    OUString GetText(){return m_Content;}
};

class LwpFribHardSpace : public LwpFrib
{
public:
    LwpFribHardSpace( LwpPara* pPara ) : LwpFrib(pPara){};
    ~LwpFribHardSpace(){};
};

class LwpFribSoftHyphen : public LwpFrib
{
public:
    LwpFribSoftHyphen( LwpPara* pPara ) : LwpFrib(pPara){};
    ~LwpFribSoftHyphen(){};
};

//add by , 02/03/2005
class LwpFribParaNumber : public LwpFrib
{
public:
    LwpFribParaNumber( LwpPara* pPara ) : LwpFrib(pPara),
        m_nStyleID(0), m_nNumberChar(0), m_nLevel(1), m_nStart(0){};
    void Read(LwpObjectStream* pObjStrm, sal_uInt16 len);

    inline sal_uInt16 GetStyleID() const;

    inline sal_uInt16 GetNumberChar() const;

    inline sal_uInt16 GetLevel() const;

    inline sal_uInt16 GetStart() const;

private:
    sal_uInt16 m_nStyleID;
    sal_uInt16 m_nNumberChar;
    sal_uInt16 m_nLevel;
    sal_uInt16 m_nStart;
};
inline sal_uInt16 LwpFribParaNumber::GetStyleID() const
{
    return m_nStyleID;
}
inline sal_uInt16 LwpFribParaNumber::GetNumberChar() const
{
    return m_nNumberChar;
}
inline sal_uInt16 LwpFribParaNumber::GetLevel() const
{
    return m_nLevel;
}
inline sal_uInt16 LwpFribParaNumber::GetStart() const
{
    return m_nStart;
}

class LwpFribDocVar : public LwpFrib
{
public:
    LwpFribDocVar( LwpPara* pPara );

    ~LwpFribDocVar();

    void Read(LwpObjectStream* pObjStrm, sal_uInt16 len);

    void RegisterStyle(LwpFoundry* pFoundry);//add by ,05/5/27

    void XFConvert(XFContentContainer* pXFPara);//add by ,05/5/27

//	inline sal_uInt16 GetType() const;

    enum {
        FILENAME = 0x02,
        PATH = 0x03,
        SMARTMASTER = 0x04,
        DESCRIPTION = 0x05,
        DATECREATED = 0x06,
        DATELASTREVISION = 0x07,
        TOTALEDITTIME = 0x08,
        NUMPAGES= 0x09,
        NUMWORDS= 0x0a,
        NUMCHARS = 0x0b,
        DOCSIZE = 0x0c,
        DIVISIONNAME = 0x0d,
        SECTIONNAME = 0x0e,
        VERSIONCREATEBY = 0x0f,
        VERSIONCREATEDATE = 0x10,
        VERSIONOTHEREDITORS = 0x11,
        VERSIONNAME = 0x12,
        VERSIONNUMBER = 0x13,
        ALLVERSIONNAME = 0x14,
        VERSIONREMARK = 0x15,
        DOCUMENTCATEGORY = 0x16,
        VERSIONLASTDATE = 0x17,
        VERSIONLASTEDITOR = 0x18,
        KEYWORDS = 0x19,
        CREATEDBY = 0x1a,
        LASTEDIT = 0x1b,
        OTHEREDITORS = 0x1c,
        NUMOFREVISION = 0x1d,
        };

private:
    sal_uInt16 m_nType;
    LwpAtomHolder* m_pName;
    OUString m_TimeStyle;
    void RegisterDefaultTimeStyle();
    void RegisterTotalTimeStyle();
};
//inline sal_uInt16 LwpFribDocVar::GetType() const
//{
//	return m_nType;
//}

class LwpFribTab : public LwpFrib
{
public:
    LwpFribTab( LwpPara* pPara ) : LwpFrib(pPara){};
    ~LwpFribTab(){};
};

class LwpFribUnicode: public LwpFrib
{
public:
    LwpFribUnicode( LwpPara* pPara ) : LwpFrib(pPara){};
    ~LwpFribUnicode(){};
    void Read(LwpObjectStream* pObjStrm, sal_uInt16 len);
    void XFConvert(XFContentContainer* pXFPara,LwpStory* pStory);
private:
    OUString m_Content;
public:
    OUString GetText(){return m_Content;}
};

class LwpFribPageNumber : public LwpFrib
{
public:
    LwpFribPageNumber(LwpPara* pPara) : LwpFrib(pPara),
        m_nNumStyle(0), m_nStartNum(1), m_nStartOnPage(1), m_nFlag(0){};
    ~LwpFribPageNumber(){};
    void Read(LwpObjectStream* pObjStrm, sal_uInt16 len);
    void XFConvert(XFContentContainer* pXFPara);
private:
    sal_uInt16 m_nNumStyle;
    LwpAtomHolder m_aBefText;
    LwpAtomHolder m_aAfterText;
    sal_uInt16 m_nStartNum;
    sal_uInt16 m_nStartOnPage;
    sal_uInt16 m_nFlag;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
