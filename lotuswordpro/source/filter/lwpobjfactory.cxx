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
 Jan 2005           Created
 ************************************************************************/

#include "lwpobjfactory.hxx"
#include "lwpdefs.hxx"
#include "lwpdoc.hxx"
#include "lwpstory.hxx"
#include "lwplayout.hxx"
#include "lwppara.hxx"
#include "lwpparastyle.hxx"
#include "lwpcharacterstyle.hxx"
#include "lwppiece.hxx"
#include "lwptabrack.hxx"
#include "lwpsilverbullet.hxx"
#include "lwpdivinfo.hxx"
#include "lwpholder.hxx"
#include "lwpdivopts.hxx"
#include "lwplaypiece.hxx"
#include "lwpsection.hxx"
#include "lwppagehint.hxx"
#include "lwpoleobject.hxx"
#include "lwpgrfobj.hxx"
#include "lwptable.hxx"
#include "lwptblcell.hxx"
#include "lwpmarker.hxx"
#include "lwpproplist.hxx"
#include <new>
#include "lwpframelayout.hxx"
#include "lwptablelayout.hxx"
#include "lwppagelayout.hxx"
#include "lwptblformula.hxx"
#include "lwpvpointer.hxx"
#include "lwpfootnote.hxx"
#include "lwpnumericfmt.hxx"
#include "lwpfnlayout.hxx"
#include "lwptoc.hxx"
#include "lwpdocdata.hxx"
#include "lwpnotes.hxx"
#include "lwpverdocument.hxx"

//LwpObjectFactory* LwpObjectFactory::m_pMgr = NULL;

LwpObjectFactory::LwpObjectFactory(LwpSvStream* pSvStream)
    : m_nNumObjs(0), m_pSvStream(pSvStream)
{
    m_IdToObjList.clear();
}

LwpObjectFactory::~LwpObjectFactory()
{
    if(!m_IdToObjList.empty())
        ClearObjectMap();
}

/**
 * @descr       clear object map and delete all objects
*/
void LwpObjectFactory::ClearObjectMap()
{
    LwpIdToObjMap::iterator it = m_IdToObjList.begin();
    while( it!=m_IdToObjList.end() )
    {
        delete it->second;
        it->second = NULL;
        ++it;
    }
    m_IdToObjList.clear();
}
/**
 * @descr       read the index manager
*/
void LwpObjectFactory::ReadIndex(LwpSvStream* pStrm)
{
    m_IndexMgr.Read(pStrm);
}

/**
 * @descr       create all kinds of objects except lwp7
*/
LwpObject* LwpObjectFactory::CreateObject(sal_uInt32 type, LwpObjectHeader &objHdr)
{
    LwpObject* newObj = NULL;
    m_nNumObjs++;
    assert(type<300);
    switch(type)
    {
        case VO_DOCUMENT:
        {
            newObj = new LwpDocument(objHdr, m_pSvStream);
            break;
        }
        case VO_DOCSOCK:
        {
            newObj = new LwpDocSock(objHdr, m_pSvStream);
            break;
        }
        case VO_DIVISIONINFO:
        {
            newObj = new LwpDivInfo(objHdr, m_pSvStream);
            break;
        }
        case VO_DIVOPTS:
        {
            newObj = new LwpDivisionOptions(objHdr, m_pSvStream);
            break;
        }
        case VO_HEADCONTENT:
        {
            newObj = new LwpHeadContent(objHdr, m_pSvStream);
            break;
        }
        case VO_HEADLAYOUT:
        {
            newObj = new LwpHeadLayout(objHdr, m_pSvStream);
            break;
        }
        case VO_PAGELAYOUT:
        {
            newObj = new LwpPageLayout(objHdr, m_pSvStream);
            break;
        }
        case VO_STORY:
        {
            newObj = new LwpStory(objHdr, m_pSvStream);
            break;
        }
        case VO_PARA:
        {
            newObj = new LwpPara(objHdr, m_pSvStream);
            break;
        }
        case VO_HEADERLAYOUT:
        {
            newObj = new LwpHeaderLayout(objHdr, m_pSvStream);
            break;
        }
        case VO_FOOTERLAYOUT:
        {
            newObj = new LwpFooterLayout(objHdr, m_pSvStream);
            break;
        }
        case VO_FRAMELAYOUT:
        {
            newObj = new LwpFrameLayout(objHdr, m_pSvStream);
            break;
        }
        case VO_LAYOUTGEOMETRY:
        {
            newObj = new LwpLayoutGeometry(objHdr, m_pSvStream);
            break;
        }
        case VO_LAYOUTSCALE:
        {
            newObj = new LwpLayoutScale(objHdr, m_pSvStream);
            break;
        }
        case VO_LAYOUTMARGINS:
        {
            newObj = new LwpLayoutMargins(objHdr, m_pSvStream);
            break;
        }
        case VO_LAYOUTBORDERSTUFF:
        {
            newObj = new LwpLayoutBorder(objHdr, m_pSvStream);
            break;
        }
        case VO_LAYOUTBACKGROUNDSTUFF:
        {
            newObj = new LwpLayoutBackground(objHdr, m_pSvStream);
            break;
        }
        case VO_LAYOUTEXTERNALBORDERSTUFF:
        {
            newObj = new LwpLayoutExternalBorder(objHdr, m_pSvStream);
            break;
        }
        case VO_LAYOUTCOLUMNS:
        {
            newObj = new LwpLayoutColumns(objHdr, m_pSvStream);
            break;
        }
        case VO_LAYOUTGUTTERSTUFF:
        {
            newObj = new LwpLayoutGutters(objHdr, m_pSvStream);
            break;
        }
        case VO_LAYOUTJOINSTUFF:
        {
            newObj = new LwpLayoutJoins(objHdr, m_pSvStream);
            break;
        }
        case VO_LAYOUTSHADOW:
        {
            newObj = new LwpLayoutShadow(objHdr, m_pSvStream);
            break;
        }
        // 01/12/2005
        case VO_PARASTYLE:
        {
            newObj = new LwpParaStyle(objHdr, m_pSvStream);
            break;
        }
        case VO_CHARACTERSTYLE:
        {
            newObj = new LwpCharacterStyle(objHdr, m_pSvStream);
            break;
        }
        case VO_SILVERBULLET:
        {
            newObj = new LwpSilverBullet(objHdr, m_pSvStream);
            break;
        }
        case VO_ALIGNMENTPIECE:
        {
            newObj = new LwpAlignmentPiece(objHdr, m_pSvStream);
            break;
        }
        case VO_INDENTPIECE:
        {
            newObj = new LwpIndentPiece(objHdr, m_pSvStream);
            break;
        }
        case VO_SPACINGPIECE:
        {
            newObj = new LwpSpacingPiece(objHdr, m_pSvStream);
            break;
        }
        case VO_CHARBORDERPIECE:
        {
            newObj = new LwpCharacterBorderPiece(objHdr, m_pSvStream);
            break;
        }
        case VO_AMIKAKEPIECE:
        {
            newObj = new LwpAmikakePiece(objHdr, m_pSvStream);
            break;
        }
        // end

        case VO_HEADHOLDER:
        {
            newObj = new LwpDLVListHeadHolder(objHdr, m_pSvStream);
            break;
        }

        // start
        case VO_PARABORDERPIECE:
        {
            newObj = new LwpParaBorderPiece(objHdr, m_pSvStream);
            break;
        }
        case VO_BREAKSPIECE:
        {
            newObj = new LwpBreaksPiece(objHdr, m_pSvStream);
            break;
        }
        case VO_NUMBERINGPIECE:
        {
            newObj = new LwpNumberingPiece(objHdr, m_pSvStream);
            break;
        }
        case VO_TABRACK:
        {
            newObj = new LwpTabRack(objHdr, m_pSvStream);
            break;
        }
        case VO_TABPIECE:
        {
            newObj = new LwpTabPiece(objHdr, m_pSvStream);
            break;
        }
        case VO_PARABACKGROUNDPIECE:    //perhaps wrong.
        {
            newObj = new LwpBackgroundPiece(objHdr, m_pSvStream);
            break;
        }
        // end.
        case VO_SECTION:
        {
            newObj = new LwpSection(objHdr, m_pSvStream);
            break;
        }
        case VO_INDEXSECTION:
        {
            newObj = new LwpIndexSection(objHdr, m_pSvStream);
            break;
        }
        case VO_HEADTAILHOLDER:
        {
            newObj = new LwpDLVListHeadTailHolder(objHdr, m_pSvStream);
            break;
        }
        case VO_PAGEHINT:
        {
            newObj = new LwpPageHint(objHdr, m_pSvStream);
            break;
        }
        case VO_OLEOBJECT:
        {
            newObj = new LwpOleObject(objHdr, m_pSvStream);
            break;
        }
        //
        case VO_GRAPHIC:
        {
            newObj = new LwpGraphicObject(objHdr, m_pSvStream);
            break;
        }
        case VO_DOCDATA:
        {
            newObj = new LwpDocData(objHdr, m_pSvStream);
            break;
        }
        //
        case VO_DROPCAPLAYOUT:
        {
            newObj = new LwpDropcapLayout(objHdr, m_pSvStream);
            break;
        }
        case VO_LAYOUTRELATIVITY:
        {
            newObj = new LwpLayoutRelativity(objHdr, m_pSvStream);
            break;
        }
        //  for table
        case VO_TABLE:
        {
            newObj = new LwpTable(objHdr, m_pSvStream);
            break;
        }
        case VO_TABLELAYOUT:
        {
            newObj = new LwpTableLayout(objHdr, m_pSvStream);
            break;
        }
        case VO_SUPERTABLE:
        {
            newObj = new LwpSuperTable(objHdr, m_pSvStream);
            break;
        }
        case VO_SUPERTABLELAYOUT:
        {
            newObj = new LwpSuperTableLayout(objHdr, m_pSvStream);
            break;
        }
        case VO_CELLLAYOUT:
        {
            newObj = new LwpCellLayout(objHdr, m_pSvStream);
            break;
        }
        case VO_ROWLAYOUT:
        {
            newObj = new LwpRowLayout(objHdr, m_pSvStream);
            break;
        }
        case VO_LAYOUTNUMERICS:
        {
            newObj = new LwpLayoutNumerics(objHdr, m_pSvStream);
            break;
        }
        case VO_NUMERICVALUE:
        {
            newObj = new LwpNumericValue(objHdr, m_pSvStream);
            break;
        }
        case VO_FORMULAINFO:
        {
            newObj = new LwpFormulaInfo(objHdr, m_pSvStream);
            break;
        }
        case VO_COLUMNLAYOUT:
        {
            newObj = new LwpColumnLayout(objHdr, m_pSvStream);
            break;
        }
        case VO_ROWLIST:
        {
            newObj = new LwpRowList(objHdr, m_pSvStream);
            break;
        }
        case VO_CELLLIST:
        {
            newObj = new LwpCellList(objHdr, m_pSvStream);
            break;
        }
        case VO_TABLERANGE:
        {
            newObj = new LwpTableRange(objHdr, m_pSvStream);
            break;
        }
        case VO_CELLRANGE:
        {
            newObj = new LwpCellRange(objHdr, m_pSvStream);
            break;
        }
        case VO_FOLDER:
        {
            newObj = new LwpFolder(objHdr, m_pSvStream);
            break;
        }
        case VO_DEPENDENT:
        {
            newObj = new LwpDependent(objHdr, m_pSvStream);
            break;
        }
        case VO_CONNECTEDCELLLAYOUT:
        {
            newObj = new LwpConnectedCellLayout(objHdr, m_pSvStream);
            break;
        }
        case VO_HIDDENCELLLAYOUT:
        {
            newObj = new LwpHiddenCellLayout(objHdr, m_pSvStream);
            break;
        }
        case VO_TABLEHEADINGLAYOUT:
        {
            newObj = new LwpTableHeadingLayout(objHdr, m_pSvStream);
            break;
        }
        case VO_ROWHEADINGLAYOUT:
        {
            newObj = new LwpRowHeadingLayout(objHdr, m_pSvStream);
            break;
        }
        case VO_TABLEHEADING:
        {
            newObj = new LwpTableHeading(objHdr, m_pSvStream);
            break;
        }
        case VO_CHBLKMARKER:
        {
            newObj = new LwpCHBlkMarker(objHdr, m_pSvStream);
            break;
        }
        case VO_PROPLIST:
        {
            newObj = new LwpPropListElement(objHdr, m_pSvStream);
            break;
        }
        case VO_BOOKMARK:
        {
            newObj = new LwpBookMark(objHdr, m_pSvStream);
            break;
        }
        case VO_FIELDMARKER:
        {
            newObj = new LwpFieldMark(objHdr, m_pSvStream);
            break;
        }
        case VO_OBJECTHOLDER:
        {
            newObj = new LwpObjectHolder(objHdr, m_pSvStream);
            break;
        }
        case VO_POBJECTHOLDER:
        {
            newObj = new LwpObjectHolder(objHdr, m_pSvStream);
            break;
        }
        case VO_VERGTR: //fall through
        case VO_VERQTR:
        {
            newObj = new LwpVersionedPointer(objHdr, m_pSvStream);
            break;
        }
        case VO_FOOTNOTE:
        {
            newObj = new LwpFootnote(objHdr, m_pSvStream);
            break;
        }
        case VO_FOOTNOTETABLE:
        {
            newObj = new LwpFootnoteTable(objHdr, m_pSvStream);
            break;
        }
        case VO_FOOTNOTEOPTS:
        {
            newObj = new LwpFootnoteOptions(objHdr, m_pSvStream);
            break;
        }
        case VO_FOOTNOTELAYOUT:
        {
            newObj = new LwpFootnoteLayout(objHdr, m_pSvStream);
            break;
        }
        case VO_FNROWLAYOUT:
        {
            newObj = new LwpFnRowLayout(objHdr, m_pSvStream);
            break;
        }
        case VO_FNCELLLAYOUT:
        {
            newObj = new LwpFnCellLayout(objHdr, m_pSvStream);
            break;
        }
        case VO_ENDNOTELAYOUT:
        {
            newObj = new LwpEndnoteLayout(objHdr, m_pSvStream);
            break;
        }
        case VO_ENSUPERTABLELAYOUT:
        {
            newObj = new LwpEnSuperTableLayout(objHdr, m_pSvStream);
            break;
        }
        case VO_FNSUPERTABLELAYOUT:
        {
            newObj = new LwpFnSuperTableLayout(objHdr, m_pSvStream);
            break;
        }
        case VO_CONTONLAYOUT:
        {
            newObj = new LwpContOnLayout(objHdr, m_pSvStream);
            break;
        }
        case VO_CONTFROMLAYOUT:
        {
            newObj = new LwpContFromLayout(objHdr, m_pSvStream);
            break;
        }
        case VO_GROUPLAYOUT:
        {
            newObj = new LwpGroupLayout(objHdr, m_pSvStream);
            break;
        }
        case VO_GROUPFRAME:
        {
            newObj = new LwpGroupFrame(objHdr, m_pSvStream);
            break;
        }
        case VO_TOCSUPERTABLELAYOUT:
        {
            newObj = new LwpTocSuperLayout(objHdr, m_pSvStream);
            break;
        }
        case VO_LISTLIST:
        {
            newObj = new LwpListList(objHdr, m_pSvStream);
            break;
        }
        case VO_TOCLEVELDATA:
        {
            newObj = new LwpTocLevelData(objHdr, m_pSvStream);
            break;
        }
        case VO_NOTELAYOUT:
        {
            newObj = new LwpNoteLayout(objHdr, m_pSvStream);
            break;
        }
        case VO_NOTEHEADERLAYOUT:
        {
            newObj = new LwpNoteHeaderLayout(objHdr, m_pSvStream);
            break;
        }
        case VO_NOTETEXTLAYOUT:
        {
            newObj = new LwpNoteTextLayout(objHdr, m_pSvStream);
            break;
        }
        case VO_VPLAYOUT:
        {
            newObj = new LwpViewportLayout(objHdr, m_pSvStream);
            break;
        }
        case VO_PCOLBLOCK:
        {
            newObj = new LwpParallelColumnsBlock(objHdr, m_pSvStream);
            break;
        }
        case VO_SUPERPARALLELCOLUMNLAYOUT:
        {
            newObj = new LwpSuperParallelColumnLayout(objHdr, m_pSvStream);
            break;
        }
        case VO_PCOLLAYOUT:
        {
            newObj = new LwpParallelColumnsLayout(objHdr, m_pSvStream);
            break;
        }
        case VO_PARALLELCOLUMNS:
        {
            newObj = new LwpParallelColumns(objHdr, m_pSvStream);
            break;
        }
        case VO_RUBYMARKER:
        {
            newObj = new LwpRubyMarker(objHdr, m_pSvStream);
            break;
        }
        case VO_RUBYLAYOUT:
        {
            newObj = new LwpRubyLayout(objHdr, m_pSvStream);
            break;
        }
        case VO_GLOSSARY:
        {
            newObj = new LwpGlossary(objHdr, m_pSvStream);
            break;
        }
        case VO_SUPERGLOSSARYLAYOUT:
        {
            newObj = new LwpSuperGlossaryLayout(objHdr, m_pSvStream);
            break;
        }
        case VO_VERDOCUMENT:
        {
            newObj = new LwpVerDocument(objHdr, m_pSvStream);
            break;
        }
        default:
        {
            //Unknown object type
            assert(false);
            newObj = NULL;
            break;
        }
    }
    if(newObj)
    {
        newObj->QuickRead();
        m_IdToObjList.insert(LwpIdToObjMap::value_type(*objHdr.GetID(), newObj));
    }

    return(newObj);
}
/**
 * @descr       query object by object id
 *          object is created if not in the factory
*/
LwpObject* LwpObjectFactory::QueryObject(const LwpObjectID &objID)
{
    LwpObject* obj = FindObject( objID );
    if(!obj)
    {
        //Read the object from file
        sal_uInt32 nStreamOffset = m_IndexMgr.GetObjOffset(objID);
        if(nStreamOffset == BAD_OFFSET) //does not find the offset in index manager
            return NULL;

        sal_Int64 nDesiredPos = nStreamOffset + LwpSvStream::LWP_STREAM_BASE;
        if (nDesiredPos != m_pSvStream->Seek(nDesiredPos))
            return NULL;
        LwpObjectHeader objHdr;
        if (!objHdr.Read(*m_pSvStream))
            return NULL;

        LwpObjectID* pId = objHdr.GetID();
        if (pId && (*pId != objID))
        {
            OSL_FAIL("apparently incorrect objid, invalidating");
            return NULL;
        }

        obj = CreateObject(objHdr.GetTag(), objHdr);
    }
    return obj;
}

/**
 * @descr       find object in the factory per the object id
*/
LwpObject* LwpObjectFactory::FindObject(const LwpObjectID &objID)
{
    LwpIdToObjMap::const_iterator it =  m_IdToObjList.find(objID);
    if (it != m_IdToObjList.end()) {
        return((*it).second);
    }
    else
    {
        return NULL;
    }
}
/**
 * @descr       release object in the factory per the object id
*/
void LwpObjectFactory::ReleaseObject(const LwpObjectID &objID)
{
    LwpObject* obj = FindObject( objID );
    m_IdToObjList.erase(objID);
    if( obj )
        delete obj;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
