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
 Mar 2005           Created
 ************************************************************************/


#ifndef _LWPMARKER_HXX_
#define _LWPMARKER_HXX_

#include "lwpobj.hxx"
#include "lwpobjid.hxx"
#include "lwpdlvlist.hxx"
#include "lwpfrib.hxx"
#include "xfilter/xftextspan.hxx"

class LwpMarker : public LwpDLNFPVList
{
public:
    LwpMarker(LwpObjectHeader &objHdr, LwpSvStream *pStrm);
    ~LwpMarker(){}
    void Read();
    OUString GetNamedProperty(OUString name);
protected:
    enum{
        MARKER_START=1,
        MARKER_END=2
    };
private:
    sal_uInt16 m_nFlag;
    sal_uInt16 m_nPageNumber;
    sal_uInt16 m_nNeedUpdate;
    LwpObjectID m_objLayout;
    LwpObjectID m_objMarkerList;
    LwpObjectID m_objContent;

};

class LwpFribRange
{
public:
    LwpFribRange(){}
    ~LwpFribRange(){}
    void Read(LwpObjectStream* pObjStrm);
private:
    LwpObjectID m_StartPara;
    LwpObjectID m_EndPara;
};

class LwpStoryMarker : public LwpMarker
{
public:
    LwpStoryMarker(LwpObjectHeader &objHdr, LwpSvStream *pStrm);
    ~LwpStoryMarker(){}
    void Read();
private:
    LwpFribRange m_Range;
    sal_uInt16 m_nFlag;
};

class LwpCHBlkMarker : public LwpStoryMarker
{
public:
    LwpCHBlkMarker(LwpObjectHeader &objHdr, LwpSvStream *pStrm);
    ~LwpCHBlkMarker(){}
    void Read();
    sal_uInt16 GetAction(){return m_nAction;}
    void ConvertCHBlock(XFContentContainer* pXFPara,sal_uInt8 nType);
    enum{
        CLICKHERE_CHBEHAVIORTEXT=1,
        CLICKHERE_CHBEHAVIORTABLE=2,
        CLICKHERE_CHBEHAVIORPICTURE=3,
        CLICKHERE_CHBEHAVIOROLEOBJECT=4,
        CLICKHERE_CHBEHAVIORCHART=5,
        CLICKHERE_CHBEHAVIORDRAWING=6,
        CLICKHERE_CHBEHAVIORFILE=7,
        CLICKHERE_CHBEHAVIORGLOSSARY=8,
        CLICKHERE_CHBEHAVIOREQUATION=9,
        CLICKHERE_CHBEHAVIORINTERNETLINK=10,
        CLICKHERE_CHBEHAVIORSTRINGLIST=11,
        CLICKHERE_CHBEHAVIORDATETIME=12,
        CLICKHERE_CHBEHAVIORSYMBOL=13,
        CLICKHERE_CHBEHAVIORDOCFIELD=14,
        CLICKHERE_CHBEHAVIORPAGENUM=15
    };
private:
    void ProcessPlaceHolder(XFContentContainer* pXFPara,sal_uInt16 nAction,sal_uInt8 nType);
    void ProcessOtherCHB(XFContentContainer* pXFPara,sal_uInt8 nType);
    void ProcessKeylist(XFContentContainer* pXFPara,sal_uInt8 nType);
    sal_Bool IsHasFilled();
    sal_Bool IsBubbleHelp();
    OUString GetPromptText();
    void EnumAllKeywords();
private:
    enum{
        CHB_PROMPT = 0x01,
        CHB_EDIT = 0x02,
        CHB_HELP = 0x04,
        CHB_TAB = 0x08,
        CHB_HIDDEN = 0x10,
        CHB_ALLOWVALUESNOTINLIST = 0x20,
        CHB_ALLOWMULTIVALUES = 0x40,
        CHB_RETURN = 0x80,
        CHB_NOEARS = 0x100,
        CHB_MAGNETIC = 0x200,
        CHB_PERSISTENT = (CHB_PROMPT+CHB_HELP+CHB_TAB+CHB_HIDDEN
            +CHB_ALLOWVALUESNOTINLIST+CHB_ALLOWMULTIVALUES
            +CHB_RETURN+CHB_NOEARS+CHB_MAGNETIC)
    };
    LwpObjectID m_objPromptStory;
    sal_uInt32 m_nTab;
    sal_uInt16 m_nFlag;
    sal_uInt16 m_nAction;
    LwpAtomHolder m_Help;
    LwpAtomHolder m_Mirror;
    std::vector<OUString> m_Keylist;
};

class LwpBookMark : public LwpDLNFVList
{
public:
    LwpBookMark(LwpObjectHeader &objHdr, LwpSvStream *pStrm);
    ~LwpBookMark(){}
protected:
    void Read();
public:
    sal_Bool IsRightMarker(LwpObjectID objMarker);
    OUString GetName();
private:
    enum {  BKMK_NOTESFX = 0x0001,
        BKMK_OLDNOTESFX = 0x0002
        };
    LwpObjectID m_objMarker;
    sal_uInt16 m_nFlag;
};

class LwpFieldMark : public LwpStoryMarker
{
public:
    LwpFieldMark(LwpObjectHeader &objHdr, LwpSvStream *pStrm);
    ~LwpFieldMark(){}
    void Read();
    void ParseIndex(OUString& sKey1,OUString& sKey2);
    void ParseTOC(OUString& sLevel,OUString& sText);
    sal_uInt16 GetFieldType(){return m_nFieldType;}
    sal_Bool IsFormulaInsert();
    sal_Bool IsDateTimeField(sal_uInt8& type,OUString& formula);
    sal_Bool IsCrossRefField(sal_uInt8& nType, OUString& sMarkName);
    sal_Bool IsDocPowerField(sal_uInt8& nType,OUString& sFormula);
    OUString GetFormula(){return m_Formula.str();}
    void SetStyleFlag(sal_Bool bFalg){m_bHasStyle = bFalg;}
    sal_Bool GetStyleFlag(){return m_bHasStyle;}
    sal_Bool GetStart(){return m_bHasStart;}
    void SetStart(sal_Bool bFlag){m_bHasStart = bFlag;}
    LwpFrib* GetStartFrib(){return m_pFrib;}
    void SetStartFrib(LwpFrib* pFrib){m_pFrib = pFrib;}
    sal_Bool GetRevisionFlag(){return m_bRevisionFlag;}
    void SetRevisionFlag(sal_Bool bFlag){m_bRevisionFlag = bFlag;}

    enum{
        FLD_FIELD = 0x0003,
        FLD_INDEX = 0x0008,
        FLD_TOC = 0x000B
    };
    enum{
        FF_FORMULAINSERTED = 0X0008,
    };
    enum{
        CROSSREF_INVALID = 0,
        CROSSREF_TEXT = 1,
        CROSSREF_PAGE = 2,
        CROSSREF_PARANUMBER = 3
    };
    enum{
        DATETIME_SKIP = 0,
        DATETIME_NOW = 1,
        DATETIME_CREATE = 2,
        DATETIME_LASTEDIT = 3,
        DATETIME_TOTALTIME = 4
    };
    enum{
        DOC_DESCRIPTION = 1,
        DOC_NUMPAGES = 2,
        DOC_NUMWORDS = 3,
        DOC_NUMCHARS = 4,
    };
private:
    LwpObjectID m_objFormulaStory;
    LwpObjectID m_objResultContent;
    sal_uInt16 m_nFlag;
    sal_uInt16 m_nFieldType;
    LwpAtomHolder m_Formula;

    sal_Bool m_bHasStyle;
    sal_Bool m_bHasStart;

    LwpFrib* m_pFrib;
    sal_Bool m_bRevisionFlag;
};

class LwpRubyMarker : public LwpStoryMarker
{
public:
    LwpRubyMarker(LwpObjectHeader &objHdr, LwpSvStream *pStrm);
    ~LwpRubyMarker(){}
    void Read();
    OUString GetRubyText(){return m_strRubyText;}
    void SetRubyText(OUString sText){m_strRubyText = sText;}
    OUString GetTextStyleName(){return m_TextStye;}
    void SetTextStyleName(OUString sName){m_TextStye = sName;}
    OUString GetRubyStyleName(){return m_RubyStyle;}
    void SetRubyStyleName(OUString sName){m_RubyStyle = sName;}
private:
    LwpObjectID m_objLayout;
    OUString m_strRubyText;
    OUString m_RubyStyle;
    OUString m_TextStye;
};
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
