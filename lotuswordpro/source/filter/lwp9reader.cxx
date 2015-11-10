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
#include "lwpglobalmgr.hxx"
#include "lwparrowstyles.hxx"
#include "lwpobjhdr.hxx"
#include "lwpdoc.hxx"
#include "xfilter/xfstylemanager.hxx"
#include "lwpdocdata.hxx"
#include "lwpbookmarkmgr.hxx"
#include "lwpchangemgr.hxx"
#include <tools/stream.hxx>

Lwp9Reader::Lwp9Reader (LwpSvStream* pInputStream, IXFStream* pStream)
    : m_pDocStream(pInputStream)
    , m_pStream(pStream)
    , m_pObjMgr(nullptr)
    , m_LwpFileHdr()
{}

/**
 * @descr   The entrance of Word Pro 9 import filter.
 **/
void Lwp9Reader::Read()
{
    LwpGlobalMgr* pGlobal = LwpGlobalMgr::GetInstance(m_pDocStream);
    try
    {
        m_pObjMgr = pGlobal->GetLwpObjFactory();

        ReadFileHeader();
        //Does not support Word Pro 96 and previous versions
        if(LwpFileHeader::m_nFileRevision>=0x000B)
        {
            ReadIndex();
            ParseDocument();
        }
    }
    catch(...)
    {
        LwpGlobalMgr::DeleteInstance();
        throw;
    }
    LwpGlobalMgr::DeleteInstance();
}

/**
 * @descr   Read the LWP7 object.
 */
void Lwp9Reader::ReadFileHeader()
{
    m_pDocStream->Seek(LwpSvStream::LWP_STREAM_BASE);

    //Remember to initialize the LwpFileHeader::m_nFileRevision first.
    LwpFileHeader::m_nFileRevision = 0;

    LwpObjectHeader objHdr;
    objHdr.Read(*m_pDocStream);
    sal_Int64 pos = m_pDocStream->Tell();
    m_LwpFileHdr.Read(m_pDocStream);
    m_pDocStream->Seek(pos+objHdr.GetSize());

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
void Lwp9Reader::ParseDocument()
{
    WriteDocHeader();

    //Get root document
    LwpDocument* doc = dynamic_cast<LwpDocument*> ( m_LwpFileHdr.GetDocID().obj().get() );

    if (!doc)
        return;

    //Parse Doc Data
    LwpDocData *pDocData = dynamic_cast<LwpDocData*>(doc->GetDocData().obj().get());
    if (pDocData!=nullptr)
        pDocData->Parse(m_pStream);

    //Register Styles
    RegisteArrowStyles();
    doc->RegisterStyle();
    XFStyleManager* pXFStyleManager = LwpGlobalMgr::GetInstance()->GetXFStyleManager();
    pXFStyleManager->ToXml(m_pStream);

    //Parse document content
    m_pStream->GetAttrList()->Clear();
    m_pStream->StartElement( "office:body" );

    //Parse change list
    LwpGlobalMgr* pGlobal = LwpGlobalMgr::GetInstance();
    LwpChangeMgr* pChangeMgr = pGlobal->GetLwpChangeMgr();
    pChangeMgr->ConvertAllChange(m_pStream);

    doc->Parse(m_pStream);
    m_pStream->EndElement("office:body");

    WriteDocEnd();
}

/**
 * @descr   Write xml document header
 */
void Lwp9Reader::WriteDocHeader()
{
    m_pStream->StartDocument();

    IXFAttrList *pAttrList = m_pStream->GetAttrList();

    pAttrList->AddAttribute( "xmlns:office", "http://openoffice.org/2000/office" );
    pAttrList->AddAttribute( "xmlns:style", "http://openoffice.org/2000/style" );
    pAttrList->AddAttribute( "xmlns:text", "http://openoffice.org/2000/text" );
    pAttrList->AddAttribute( "xmlns:table", "http://openoffice.org/2000/table" );
    pAttrList->AddAttribute( "xmlns:draw", "http://openoffice.org/2000/drawing" );

    pAttrList->AddAttribute( "xmlns:fo", "http://www.w3.org/1999/XSL/Format" );
    pAttrList->AddAttribute( "xmlns:xlink", "http://www.w3.org/1999/xlink" );
    pAttrList->AddAttribute( "xmlns:number", "http://openoffice.org/2000/datastyle" );
    pAttrList->AddAttribute( "xmlns:svg", "http://www.w3.org/2000/svg" );
    pAttrList->AddAttribute( "xmlns:chart", "http://openoffice.org/2000/chart" );

    pAttrList->AddAttribute( "xmlns:dr3d", "http://openoffice.org/2000/dr3d" );
    pAttrList->AddAttribute( "xmlns:math", "http://www.w3.org/1998/Math/MathML" );
    pAttrList->AddAttribute( "xmlns:form", "http://openoffice.org/2000/form" );
    pAttrList->AddAttribute( "xmlns:script", "http://openoffice.org/2000/script" );
    pAttrList->AddAttribute( "xmlns:dc", "http://purl.org/dc/elements/1.1/" );

    pAttrList->AddAttribute( "xmlns:meta", "http://openoffice.org/2000/meta" );
    pAttrList->AddAttribute( "office:class", "text");
    pAttrList->AddAttribute( "office:version", "1.0");

    m_pStream->StartElement( "office:document" );
    pAttrList->Clear();

}
/**
 * @descr   Write xml document end
 */
void Lwp9Reader::WriteDocEnd()
{
    m_pStream->EndElement("office:document");
    m_pStream->EndDocument();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
