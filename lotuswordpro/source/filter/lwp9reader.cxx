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

#include "lwp9reader.hxx"
#include <lwpglobalmgr.hxx>
#include "lwparrowstyles.hxx"
#include <lwpobjhdr.hxx>
#include "lwpdoc.hxx"
#include <xfilter/xfstylemanager.hxx>
#include <lwpdocdata.hxx>
#include <lwpchangemgr.hxx>
#include <sal/log.hxx>

Lwp9Reader::Lwp9Reader(LwpSvStream* pInputStream, IXFStream* pStream)
    : m_pDocStream(pInputStream)
    , m_pStream(pStream)
    , m_pObjMgr(nullptr)
{
}

/**
 * @descr   The entrance of Word Pro 9 import filter.
 **/
bool Lwp9Reader::Read()
{
    bool bRet = true;
    LwpGlobalMgr* pGlobal = LwpGlobalMgr::GetInstance(m_pDocStream);
    try
    {
        m_pObjMgr = pGlobal->GetLwpObjFactory();

        //Does not support Word Pro 96 and previous versions
        if (ReadFileHeader() && LwpFileHeader::m_nFileRevision >= 0x000B)
        {
            ReadIndex();
            bRet = ParseDocument();
        }
        else
        {
            SAL_WARN("lwp", "Only Lotus Word Pro 97 (version 11) and later is supported.");
            SAL_WARN("lwp", "You are trying to open version: " << LwpFileHeader::m_nFileRevision);
            bRet = false;
        }
    }
    catch (...)
    {
        LwpGlobalMgr::DeleteInstance();
        throw;
    }
    LwpGlobalMgr::DeleteInstance();
    return bRet;
}

/**
 * @descr   Read the LWP7 object.
 */
bool Lwp9Reader::ReadFileHeader()
{
    if (!m_pDocStream->CheckSeek(LwpSvStream::LWP_STREAM_BASE))
        return false;

    //Remember to initialize the LwpFileHeader::m_nFileRevision first.
    LwpFileHeader::m_nFileRevision = 0;

    LwpObjectHeader objHdr;
    if (!objHdr.Read(*m_pDocStream))
        return false;

    sal_Int64 pos = m_pDocStream->Tell();
    m_LwpFileHdr.Read(m_pDocStream);
    return m_pDocStream->CheckSeek(pos + objHdr.GetSize());
}

/**
 * @descr  Read the index objects at the end of the WordProData stream
 */
void Lwp9Reader::ReadIndex()
{
    sal_Int64 oldpos = m_pDocStream->Tell();
    sal_uInt32 rootoffset = m_LwpFileHdr.GetRootIndexOffset();
    m_pDocStream->Seek(rootoffset + LwpSvStream::LWP_STREAM_BASE);
    m_pObjMgr->ReadIndex(m_pDocStream);
    m_pDocStream->Seek(oldpos);
}

/**
 * @descr       Parse all document content
*/
bool Lwp9Reader::ParseDocument()
{
    WriteDocHeader();

    //Get root document
    LwpDocument* doc = dynamic_cast<LwpDocument*>(m_LwpFileHdr.GetDocID().obj().get());

    if (!doc)
        return false;

    //Parse Doc Data
    LwpDocData* pDocData = dynamic_cast<LwpDocData*>(doc->GetDocData().obj().get());
    if (pDocData != nullptr)
        pDocData->Parse(m_pStream);

    //Register Styles
    RegisteArrowStyles();
    doc->DoRegisterStyle();
    XFStyleManager* pXFStyleManager = LwpGlobalMgr::GetInstance()->GetXFStyleManager();
    pXFStyleManager->ToXml(m_pStream);

    //Parse document content
    m_pStream->GetAttrList()->Clear();
    m_pStream->StartElement(u"office:body"_ustr);

    //Parse change list
    LwpGlobalMgr* pGlobal = LwpGlobalMgr::GetInstance();
    LwpChangeMgr* pChangeMgr = pGlobal->GetLwpChangeMgr();
    pChangeMgr->ConvertAllChange(m_pStream);

    doc->Parse(m_pStream);
    m_pStream->EndElement(u"office:body"_ustr);

    WriteDocEnd();
    return true;
}

/**
 * @descr   Write xml document header
 */
void Lwp9Reader::WriteDocHeader()
{
    m_pStream->StartDocument();

    IXFAttrList* pAttrList = m_pStream->GetAttrList();

    pAttrList->AddAttribute(u"xmlns:office"_ustr, u"http://openoffice.org/2000/office"_ustr);
    pAttrList->AddAttribute(u"xmlns:style"_ustr, u"http://openoffice.org/2000/style"_ustr);
    pAttrList->AddAttribute(u"xmlns:text"_ustr, u"http://openoffice.org/2000/text"_ustr);
    pAttrList->AddAttribute(u"xmlns:table"_ustr, u"http://openoffice.org/2000/table"_ustr);
    pAttrList->AddAttribute(u"xmlns:draw"_ustr, u"http://openoffice.org/2000/drawing"_ustr);

    pAttrList->AddAttribute(u"xmlns:fo"_ustr, u"http://www.w3.org/1999/XSL/Format"_ustr);
    pAttrList->AddAttribute(u"xmlns:xlink"_ustr, u"http://www.w3.org/1999/xlink"_ustr);
    pAttrList->AddAttribute(u"xmlns:number"_ustr, u"http://openoffice.org/2000/datastyle"_ustr);
    pAttrList->AddAttribute(u"xmlns:svg"_ustr, u"http://www.w3.org/2000/svg"_ustr);
    pAttrList->AddAttribute(u"xmlns:chart"_ustr, u"http://openoffice.org/2000/chart"_ustr);

    pAttrList->AddAttribute(u"xmlns:dr3d"_ustr, u"http://openoffice.org/2000/dr3d"_ustr);
    pAttrList->AddAttribute(u"xmlns:math"_ustr, u"http://www.w3.org/1998/Math/MathML"_ustr);
    pAttrList->AddAttribute(u"xmlns:form"_ustr, u"http://openoffice.org/2000/form"_ustr);
    pAttrList->AddAttribute(u"xmlns:script"_ustr, u"http://openoffice.org/2000/script"_ustr);
    pAttrList->AddAttribute(u"xmlns:dc"_ustr, u"http://purl.org/dc/elements/1.1/"_ustr);

    pAttrList->AddAttribute(u"xmlns:meta"_ustr, u"http://openoffice.org/2000/meta"_ustr);
    pAttrList->AddAttribute(u"office:class"_ustr, u"text"_ustr);
    pAttrList->AddAttribute(u"office:version"_ustr, u"1.0"_ustr);

    m_pStream->StartElement(u"office:document"_ustr);
    pAttrList->Clear();
}
/**
 * @descr   Write xml document end
 */
void Lwp9Reader::WriteDocEnd()
{
    m_pStream->EndElement(u"office:document"_ustr);
    m_pStream->EndDocument();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
