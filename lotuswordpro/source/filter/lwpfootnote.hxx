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
 *  footnote
 */
/*************************************************************************
 * Change History
 Mar 2005		 	Created
 ************************************************************************/
#ifndef _LWPFRIBFOOTNOTE_HXX_
#define _LWPFRIBFOOTNOTE_HXX_

#include "lwpfrib.hxx"
#include "lwpobj.hxx"
#include "lwpsection.hxx"
#include "lwpborderstuff.hxx"

// Footnote types are built up from these numbers
#define FN_MASK_ENDNOTE			0x80
#define FN_MASK_SEPARATE		0x40
#define FN_MASK_DEACTIVATED		0x20
#define FN_MASK_BASE			(0x0f | FN_MASK_ENDNOTE)
#define FN_BASE_DONTCARE		0
#define FN_BASE_FOOTNOTE		1
#define FN_BASE_DIVISION		(2 | FN_MASK_ENDNOTE)
#define FN_BASE_DIVISIONGROUP	(3 | FN_MASK_ENDNOTE)
#define FN_BASE_DOCUMENT		(4 | FN_MASK_ENDNOTE)

// Here are the real footnote types
#define FN_DONTCARE					(FN_BASE_DONTCARE)
#define FN_FOOTNOTE					(FN_BASE_FOOTNOTE)
#define FN_DIVISION					(FN_BASE_DIVISION)
#define FN_DIVISION_SEPARATE		(FN_BASE_DIVISION | FN_MASK_SEPARATE)
#define FN_DIVISIONGROUP			(FN_BASE_DIVISIONGROUP)
#define FN_DIVISIONGROUP_SEPARATE	(FN_BASE_DIVISIONGROUP | FN_MASK_SEPARATE)
#define FN_DOCUMENT					(FN_BASE_DOCUMENT)
#define FN_DOCUMENT_SEPARATE 		(FN_BASE_DOCUMENT | FN_MASK_SEPARATE)
#define STRID_FOOTCONTINUEDFROM		"Continued from previous page..."
#define STRID_FOOTCONTINUEDON		"Continued on next page..."
//Footnote table types, the string may have to do with local language
#define STR_DivisionFootnote			"DivisionFootnote"
#define STR_DivisionEndnote			"DivisionEndnote"
#define STR_DivisionGroupEndnote	"DivisionGroupEndnote"
#define STR_DocumentEndnote		"DocumentEndnote"

/**
 * @brief	Footnote frib object
*/
class LwpFootnote;
class LwpFribFootnote: public LwpFrib
{

public:
    LwpFribFootnote(LwpPara* pPara );
    ~LwpFribFootnote(){};
    void Read(LwpObjectStream* pObjStrm, sal_uInt16 len);
    void RegisterStyle();
    void XFConvert(XFContentContainer* pCont);
    LwpFootnote* GetFootnote();

private:
    LwpObjectID m_Footnote;
};

/**
 * @brief	VO_FOOTNOTE object
*/
class LwpSuperTableLayout;
class LwpEnSuperTableLayout;
class LwpFnSuperTableLayout;
class LwpCellLayout;
class LwpDocument;
class LwpTable;
class LwpFootnote : public LwpOrderedObject
{
public:
    LwpFootnote(LwpObjectHeader &objHdr, LwpSvStream* pStrm);
    ~LwpFootnote();
    void RegisterStyle();
    void XFConvert(XFContentContainer * pCont);
protected:
    void Read();
private:
    sal_uInt16 m_nType;
    sal_uInt16 m_nRow;
    LwpObjectID m_Content;
public:
    sal_uInt16 GetType(){ return m_nType;}
private:
    LwpCellLayout* GetCellLayout();
    LwpEnSuperTableLayout* GetEnSuperTableLayout();
    LwpDocument* GetFootnoteTableDivision();
    LwpDocument* GetEndnoteDivision(LwpDocument* pPossible);
    LwpEnSuperTableLayout* FindFootnoteTableLayout();
    LwpTable* FindFootnoteTable();
    LwpTable* GetFootnoteTable(LwpEnSuperTableLayout* pLayout);
    LwpContent* FindFootnoteContent();
    OUString GetTableClass();
};

/**
 * @brief VO_FOOTNOTETABLE object
*/
#include "lwptable.hxx"
class LwpFootnoteTable : public LwpTable
{
public:
    LwpFootnoteTable(LwpObjectHeader &objHdr, LwpSvStream* pStrm);
    ~LwpFootnoteTable(){};
protected:
    void Read();
};

/**
 * @brief footnote number options information
*/
class LwpFootnoteNumberOptions
{
public:
    LwpFootnoteNumberOptions(){};
    ~LwpFootnoteNumberOptions(){};
    void Read(LwpObjectStream* pObjStrm);
private:
    sal_uInt16 m_nFlag;
    sal_uInt16 m_nStartingNumber;
    LwpAtomHolder m_LeadingText;
    LwpAtomHolder m_TrailingText;
public:
    enum // for flag
    {
        RESET_DOCUMENT	= 0x00,		// i.e., never reset the numbering
        RESET_PAGE	= 0x01,
        RESET_DIVISION	= 0x02,
        RESET_DIVISIONGROUP	= 0x04,
        RESET_MASK = (RESET_PAGE | RESET_DIVISION | RESET_DIVISIONGROUP |
            RESET_DOCUMENT),
        SUPERSCRIPT_REFERENCE 	= 0x10
    };
public:
    sal_uInt16 GetStartingNumber(){ return m_nStartingNumber;}
    OUString GetLeadingText(){ return m_LeadingText.str();}
    OUString GetTrailingText(){ return m_TrailingText.str();}
    sal_uInt16 GetReset(){ return static_cast<sal_uInt16>(m_nFlag & RESET_MASK);}
};

/**
 * @brief footnote separator options information
*/
class LwpFootnoteSeparatorOptions
{
public:
    LwpFootnoteSeparatorOptions(){};
    ~LwpFootnoteSeparatorOptions(){};
    void Read(LwpObjectStream* pObjStrm);
private:
    sal_uInt16 m_nFlag;
    sal_uInt32 m_nLength;
    sal_uInt32 m_nIndent;
    sal_uInt32 m_nAbove;
    sal_uInt32 m_nBelow;
    LwpBorderStuff m_BorderStuff;
public:
    enum // for flag
    {
        HAS_SEPARATOR	= 0x01,
        CUSTOM_LENGTH	= 0x02
    };
public:
    sal_uInt32 GetFlag(){ return m_nFlag;}
    sal_uInt32 GetLength(){ return m_nLength;}
    sal_uInt32 GetIndent(){ return m_nIndent;}
    sal_uInt32 GetAbove(){ return m_nAbove;}
    sal_uInt32 GetBelow(){ return m_nBelow;}
    LwpBorderStuff* GetBorderStuff(){ return &m_BorderStuff;}
    sal_Bool HasCustomLength(){ return (m_nFlag & CUSTOM_LENGTH) != 0;}
    sal_Bool HasSeparator(){ return (m_nFlag & HAS_SEPARATOR) != 0;}
    float GetTopBorderWidth(){ return m_BorderStuff.GetSideWidth(LwpBorderStuff::TOP);}
    LwpColor GetTopBorderColor(){ return m_BorderStuff.GetSideColor(LwpBorderStuff::TOP);}
};

/**
 * @brief	VO_FOOTNOTEOPTS object
*/
class LwpFootnoteOptions : public LwpObject
{
public:
    LwpFootnoteOptions(LwpObjectHeader &objHdr, LwpSvStream* pStrm);
    ~LwpFootnoteOptions();
    void RegisterStyle();
protected:
    void Read();
    void RegisterFootnoteStyle();
    void RegisterEndnoteStyle();
private:
    sal_uInt16 m_nFlag;
    LwpFootnoteNumberOptions m_FootnoteNumbering;
    LwpFootnoteNumberOptions m_EndnoteDivisionNumbering;
    LwpFootnoteNumberOptions m_EndnoteDivisionGroupNumbering;
    LwpFootnoteNumberOptions m_EndnoteDocNumbering;
    LwpFootnoteSeparatorOptions m_FootnoteSeparator;
    LwpFootnoteSeparatorOptions m_FootnoteContinuedSeparator;
    LwpAtomHolder m_ContinuedOnMessage;
    LwpAtomHolder m_ContinuedFromMessage;
    OUString m_strMasterPage;
    enum
    {
        FO_REPEAT 			= 0x0001,
        FO_CONTINUEFROM 	= 0x0002,
        FO_CONTINUEON 		= 0x0004,
        FO_ON_CENTER		= 0x0008,
        FO_ON_RIGHT			= 0x0010,
        FO_ON_ALIGNMASK		= FO_ON_CENTER | FO_ON_RIGHT,
        FO_FROM_CENTER		= 0x0020,
        FO_FROM_RIGHT		= 0x0040,
        FO_FROM_ALIGNMASK	= FO_FROM_CENTER | FO_FROM_RIGHT
    };

public:
    LwpFootnoteNumberOptions* GetFootnoteNumbering(){ return &m_FootnoteNumbering;}
    LwpFootnoteNumberOptions* GetEndnoteDivisionNumbering(){ return &m_EndnoteDivisionNumbering;}
    LwpFootnoteNumberOptions* GetEndnoteDivisionGroupNumbering(){ return &m_EndnoteDivisionGroupNumbering;}
    LwpFootnoteNumberOptions* GetEndnoteDocNumbering(){ return &m_EndnoteDocNumbering;}
    LwpFootnoteSeparatorOptions* GetFootnoteSeparator(){ return &m_FootnoteSeparator;}
    LwpFootnoteSeparatorOptions* GetFootnoteContinuedSeparator(){ return &m_FootnoteContinuedSeparator;}
    sal_Bool GetContinuedFrom(){ return ((m_nFlag & FO_CONTINUEFROM) != 0);}
    sal_Bool GetContinuedOn(){ return ((m_nFlag & FO_CONTINUEON) != 0);}
    OUString GetContinuedOnMessage();
    OUString GetContinuedFromMessage();
    void SetMasterPage(OUString strMasterPage){ m_strMasterPage = strMasterPage;}
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
