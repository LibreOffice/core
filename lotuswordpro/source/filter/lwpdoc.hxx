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
 *  LwpDocument header file
 ************************************************************************/
/*************************************************************************
 * Change History
 Jan 2005           Created
 Jun 2005           Code cleaning by change some members to local variables in Read()
 ************************************************************************/

#ifndef _LWPDOCUMENT_HXX
#define _LWPDOCUMENT_HXX


#include "lwpobj.hxx"
#include "lwpsortopt.hxx"
#include "lwpuidoc.hxx"
#include "lwplnopts.hxx"
#include "lwpusrdicts.hxx"
#include "lwpprtinfo.hxx"
#include "lwpdlvlist.hxx"
#include "lwpheader.hxx"
#include "lwpfoundry.hxx"

class IXFStream;
class LwpVirtualLayout;
/**
 * @brief   Document object, represent document and division
*/
class LwpDocument : public LwpDLNFPVList
{
public:
    LwpDocument(LwpObjectHeader &objHdr, LwpSvStream* pStrm);
    ~LwpDocument();

private:
    LwpFoundry* m_pOwnedFoundry;

    //Data members in file format
    LwpObjectID m_DocSockID;
    sal_uInt16 m_nFlags;
    sal_uInt32 m_nPersistentFlags;
    enum
    {
        DOC_PROTECTED = 0x00000004UL,
        DOC_CHILDDOC =  0x00000800UL
    };

    //Code cleaning by change some members to local variables in Read()
    //Reserve the comments for future use
    //LwpSortOption* m_pDocSort;
    //LwpUIDocument* m_pUIDoc;
    LwpLineNumberOptions* m_pLnOpts;
    //LwpUserDictFiles* m_pUsrDicts;
    //LwpPrinterInfo* m_pPrtInfo;

    LwpObjectID m_DivOpts;
    LwpObjectID m_FootnoteOpts;
    LwpObjectID m_DocData;
    LwpObjectID m_DivInfo;
    LwpAtomHolder m_Epoch;

    LwpObjectID m_WYSIWYGPageHints;
    LwpObjectID m_VerDoc;
    LwpObjectID m_STXInfo;

protected:
    void Read();
    void ReadPlug();
    void ParseDocContent(IXFStream* pOutputStream);
    void RegisterTextStyles();
    void RegisterLayoutStyles();

    void RegisterStylesInPara();
    void RegisterBulletStyles();
    void RegisterGraphicsStyles();
    void RegisterLinenumberStyles();
    void RegisterFootnoteStyles();
    void RegisterDefaultParaStyles();

public:
    void Parse(IXFStream* pOutputStream);
    void RegisterStyle();

    inline sal_Bool IsChildDoc();
    inline sal_Bool HonorProtection();
    inline LwpObjectID* GetContentList();
    inline LwpObjectID* GetDocData();
    inline LwpObjectID* GetSocket();

    inline LwpFoundry* GetFoundry();
    inline LwpObjectID* GetDivInfoID();
    inline LwpObjectID* GetPageHintsID();
    inline LwpObjectID* GetFootnoteOpts();
    inline LwpObjectID* GetVerDoc();
    LwpObjectID* GetValidFootnoteOpts();

    sal_uInt16 GetEndnoteType();
    LwpDocument* GetPreviousDivision();
    LwpDocument* GetNextDivision();
    LwpDocument* GetParentDivision();
    LwpDocument* GetPreviousInGroup();

    LwpDocument* GetNextInGroup();
    LwpDocument* GetPreviousDivisionWithContents();
    LwpDocument* GetLastDivisionWithContents();
    LwpDocument* GetLastInGroupWithContents();
    LwpDocument* GetRootDocument();
    LwpDocument* GetFirstDivisionWithContentsThatIsNotOLE();
    LwpDocument* GetLastDivisionThatHasEndnote();

    LwpDocument* GetLastDivision();
    LwpDocument* GetFirstDivision();
    LwpVirtualLayout* GetEnSuperTableLayout();
    sal_Bool GetNumberOfPages(LwpDocument* pEndDivision, sal_uInt16& nCount);

    sal_uInt16 GetNumberOfPagesBefore();
    void ParseFrameInPage(IXFStream* pOutputStream);

private:
    void MaxNumberOfPages(sal_uInt16& nNumPages);
    void XFConvertFrameInPage(XFContentContainer* pCont);
    void ChangeStyleName();
    sal_Bool IsSkippedDivision();
};

inline sal_Bool LwpDocument::IsChildDoc()
{
    return (sal_Bool) ((m_nPersistentFlags & DOC_CHILDDOC) ? sal_True : sal_False);
}
inline sal_Bool LwpDocument::HonorProtection()
{
    return m_nPersistentFlags & DOC_PROTECTED ? sal_True : sal_False;
}
inline LwpObjectID* LwpDocument::GetContentList()
{
    return(m_pFoundry->GetContentManager()->GetContentList());
}
inline LwpObjectID* LwpDocument::GetSocket()
{
    return &m_DocSockID;
}
inline LwpFoundry* LwpDocument::GetFoundry()
{
    return m_pFoundry;
}
inline LwpObjectID* LwpDocument::GetDivInfoID()
{
    return &m_DivInfo;
}
inline LwpObjectID* LwpDocument::GetPageHintsID()
{
    return &m_WYSIWYGPageHints;
}
inline LwpObjectID* LwpDocument::GetFootnoteOpts()
{
    return &m_FootnoteOpts;
}
inline LwpObjectID* LwpDocument::GetDocData()
{
    return &m_DocData;
}
inline LwpObjectID* LwpDocument::GetVerDoc()
{
    return &m_VerDoc;
}

/**
 * @brief   DocumentSock object, divisions are embedded by document socket object
*/
class LwpDocSock : public LwpDLNFVList
{
public:
    LwpDocSock(LwpObjectHeader &objHdr, LwpSvStream* pStrm);
    ~LwpDocSock(){}
private:
    LwpObjectID m_Doc;
protected:
    void Read();
public:
    void RegisterStyle();
    void Parse(IXFStream* pOutputStream);
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
