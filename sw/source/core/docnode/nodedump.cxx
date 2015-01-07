/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "doc.hxx"
#include "drawdoc.hxx"
#include <IDocumentDrawModelAccess.hxx>
#include <IDocumentRedlineAccess.hxx>
#include <IDocumentFieldsAccess.hxx>
#include <IDocumentState.hxx>
#include <UndoManager.hxx>
#include "ndtxt.hxx"
#include "MarkManager.hxx"
#include "docary.hxx"
#include "switerator.hxx"
#include "fmtfld.hxx"
#include "docufld.hxx"
#include "txatbase.hxx"
#include "fmtautofmt.hxx"
#include "fmtcntnt.hxx"
#include "fmtornt.hxx"
#include "fmtfsize.hxx"
#include "charfmt.hxx"
#include "frmfmt.hxx"
#include "fmtanchr.hxx"
#include "fmtsrnd.hxx"
#include "paratr.hxx"
#include "redline.hxx"
#include "section.hxx"
#include "fmtclds.hxx"
#include "fmtpdsc.hxx"
#include "pagedesc.hxx"
#include "fchrfmt.hxx"
#include "fmtfollowtextflow.hxx"
#include "fmtwrapinfluenceonobjpos.hxx"
#include <swmodule.hxx>
#include <svl/itemiter.hxx>
#include <svl/intitem.hxx>
#include <editeng/charrotateitem.hxx>
#include <editeng/rsiditem.hxx>
#include <editeng/fontitem.hxx>
#include <editeng/fhgtitem.hxx>
#include <editeng/editobj.hxx>
#include <editeng/outlobj.hxx>
#include <editeng/postitem.hxx>
#include <editeng/wghtitem.hxx>
#include <svx/xdef.hxx>
#include <svx/svdpage.hxx>
#include <svx/svdmodel.hxx>
#include <svx/xfillit0.hxx>
#include <svx/xflclit.hxx>
#include <svx/xbtmpit.hxx>
#include <tools/datetimeutils.hxx>

#include <libxml/encoding.h>
#include <libxml/xmlwriter.h>
#include <boost/optional.hpp>
#include <rtl/strbuf.hxx>
#include <comphelper/anytostring.hxx>

using namespace com::sun::star;

namespace
{

// Small helper class to ensure that we write to nodes.xml if nothing
// has been explicitly specified.
// Always use at the beginning of dumpAsXml().
// Also, there are some functions to save typing.
class WriterHelper
{
public:
    WriterHelper( xmlTextWriterPtr );
    ~WriterHelper();
    operator xmlTextWriterPtr() { return writer;}
    void startElement( const char* element );
    void endElement();
    void writeFormatAttribute( const char* attribute, const char* format, ... )
#ifdef LIBXML_ATTR_FORMAT
        LIBXML_ATTR_FORMAT(3,4)
#endif
        ;
private:
    xmlTextWriterPtr writer;
    bool owns;
};

WriterHelper::WriterHelper( xmlTextWriterPtr w )
    : writer( w )
    , owns( false )
{
    if( writer == NULL )
    {
        writer = xmlNewTextWriterFilename( "nodes.xml", 0 );
        xmlTextWriterStartDocument( writer, NULL, NULL, NULL );
        owns = true;
    }
}

WriterHelper::~WriterHelper()
{
    if( owns )
    {
        xmlTextWriterEndDocument( writer );
        xmlFreeTextWriter( writer );
    }
}


void WriterHelper::startElement( const char* element )
{
    xmlTextWriterStartElement( writer, BAD_CAST( element ));
}

void WriterHelper::endElement()
{
    xmlTextWriterEndElement( writer );
}

void WriterHelper::writeFormatAttribute( const char* attribute, const char* format, ... )
{
    va_list va;
    va_start( va, format );
    xmlTextWriterWriteVFormatAttribute( writer, BAD_CAST( attribute ), format, va );
    va_end( va );
}

// Hack: somehow conversion from "..." to va_list does
// bomb on two string litterals in the format.
static const char* TMP_FORMAT = "%" SAL_PRIuUINTPTR;
static const char* TMP_FORMAT_I32 = "%" SAL_PRIdINT32;

}

void lcl_dumpSfxItemSet(WriterHelper& writer, const SfxItemSet* pSet);

void SwDoc::dumpAsXml( xmlTextWriterPtr w ) const
{
    WriterHelper writer( w );
    writer.startElement( "doc" );
    writer.writeFormatAttribute( "ptr", "%p", this );
    m_pNodes->dumpAsXml( writer );
    mpMarkManager->dumpAsXml( writer );
    m_pUndoManager->dumpAsXml(writer);
    getIDocumentFieldsAccess().GetFldTypes()->dumpAsXml( writer );
    mpTxtFmtCollTbl->dumpAsXml( writer );
    mpCharFmtTbl->dumpAsXml( writer );
    mpFrmFmtTbl->dumpAsXml( writer, "frmFmtTbl" );
    mpSpzFrmFmtTbl->dumpAsXml( writer, "spzFrmFmtTbl" );
    mpSectionFmtTbl->dumpAsXml( writer );
    mpNumRuleTbl->dumpAsXml( writer );
    getIDocumentRedlineAccess().GetRedlineTbl().dumpAsXml( writer );
    getIDocumentRedlineAccess().GetExtraRedlineTbl().dumpAsXml( writer );
    if (const SdrModel* pModel = getIDocumentDrawModelAccess().GetDrawModel())
        pModel->dumpAsXml(writer);

    writer.startElement("mbModified");
    writer.writeFormatAttribute("value", TMP_FORMAT, static_cast<int>(getIDocumentState().IsModified()));
    writer.endElement();

    writer.endElement();
}

namespace sw {
namespace mark {
void MarkManager::dumpAsXml( xmlTextWriterPtr w ) const
{
    WriterHelper writer(w);
    writer.startElement("markManager");
    if (!m_vBookmarks.empty())
    {
        writer.startElement("bookmarks");
        for (const_iterator_t it = m_vBookmarks.begin(); it != m_vBookmarks.end(); ++it)
        {
            pMark_t pMark = *it;
            writer.startElement("bookmark");
            writer.writeFormatAttribute("startNode", TMP_FORMAT, pMark->GetMarkStart().nNode.GetIndex());
            writer.writeFormatAttribute("startOffset", TMP_FORMAT_I32, pMark->GetMarkStart().nContent.GetIndex());
            writer.writeFormatAttribute("endNode", TMP_FORMAT, pMark->GetMarkEnd().nNode.GetIndex());
            writer.writeFormatAttribute("endOffset", TMP_FORMAT_I32, pMark->GetMarkEnd().nContent.GetIndex());
            OString txt8 = OUStringToOString(pMark->GetName(), RTL_TEXTENCODING_UTF8);
            writer.writeFormatAttribute("name", "%s", BAD_CAST( txt8.getStr()));
            writer.endElement();
        }
        writer.endElement();
    }

    if (!m_vFieldmarks.empty())
    {
        writer.startElement("fieldmarks");
        for (const_iterator_t it = m_vFieldmarks.begin(); it != m_vFieldmarks.end(); ++it)
        {
            pMark_t pMark = *it;
            writer.startElement("fieldmark");
            writer.writeFormatAttribute("startNode", TMP_FORMAT, pMark->GetMarkStart().nNode.GetIndex());
            writer.writeFormatAttribute("startOffset", TMP_FORMAT_I32, pMark->GetMarkStart().nContent.GetIndex());
            writer.writeFormatAttribute("endNode", TMP_FORMAT, pMark->GetMarkEnd().nNode.GetIndex());
            writer.writeFormatAttribute("endOffset", TMP_FORMAT_I32, pMark->GetMarkEnd().nContent.GetIndex());
            OString txt8 = OUStringToOString(pMark->GetName(), RTL_TEXTENCODING_UTF8);
            writer.writeFormatAttribute("name", "%s", BAD_CAST( txt8.getStr()));

            if (sw::mark::IFieldmark* pFieldmark = dynamic_cast<sw::mark::IFieldmark*>(pMark.get()))
            {
                sw::mark::IFieldmark::parameter_map_t* pParameters = pFieldmark->GetParameters();
                if (pParameters)
                {
                    writer.startElement("parameters");
                    for (sw::mark::IFieldmark::parameter_map_t::iterator parameter = pParameters->begin(); parameter != pParameters->end(); ++parameter)
                    {
                        writer.startElement("parameter");
                        OString aName = OUStringToOString(parameter->first, RTL_TEXTENCODING_UTF8);
                        writer.writeFormatAttribute("name", "%s", BAD_CAST(aName.getStr()));
                        OString aValue = OUStringToOString(comphelper::anyToString(parameter->second), RTL_TEXTENCODING_UTF8);
                        writer.writeFormatAttribute("value", "%s", BAD_CAST(aValue.getStr()));
                        writer.endElement();
                    }
                    writer.endElement();
                }
            }

            writer.endElement();
        }
        writer.endElement();
    }

    if (!m_vAnnotationMarks.empty())
    {
        writer.startElement("annotationmarks");
        for (const_iterator_t it = m_vAnnotationMarks.begin(); it != m_vAnnotationMarks.end(); ++it)
        {
            pMark_t pMark = *it;
            writer.startElement("annotationmark");
            writer.writeFormatAttribute("startNode", TMP_FORMAT, pMark->GetMarkStart().nNode.GetIndex());
            writer.writeFormatAttribute("startOffset", TMP_FORMAT_I32, pMark->GetMarkStart().nContent.GetIndex());
            writer.writeFormatAttribute("endNode", TMP_FORMAT, pMark->GetMarkEnd().nNode.GetIndex());
            writer.writeFormatAttribute("endOffset", TMP_FORMAT_I32, pMark->GetMarkEnd().nContent.GetIndex());
            OString txt8 = OUStringToOString(pMark->GetName(), RTL_TEXTENCODING_UTF8);
            writer.writeFormatAttribute("name", "%s", BAD_CAST( txt8.getStr()));
            writer.endElement();
        }
        writer.endElement();
    }
    writer.endElement();
}
} // namespace mark
} // namespace sw

void SwFldTypes::dumpAsXml( xmlTextWriterPtr w ) const
{
    WriterHelper writer(w);
    writer.startElement("swfldtypes");
    sal_uInt16 nCount = size();
    for (sal_uInt16 nType = 0; nType < nCount; ++nType)
    {
        const SwFieldType *pCurType = (*this)[nType];
        SwIterator<SwFmtFld, SwFieldType> aIter(*pCurType);
        for (const SwFmtFld* pCurFldFmt = aIter.First(); pCurFldFmt; pCurFldFmt = aIter.Next())
        {
            writer.startElement("swfmtfld");
            writer.writeFormatAttribute("ptr", "%p", pCurFldFmt);
            writer.writeFormatAttribute("pTxtAttr", "%p", pCurFldFmt->GetTxtFld());
            const char* name = "FIXME_unhandledfield";
            switch(pCurFldFmt->GetField()->GetTyp()->Which())
            {
                case RES_PAGENUMBERFLD: name = "swpagenumberfield"; break;
                case RES_POSTITFLD: name = "swpostitfield"; break;
                case RES_DATETIMEFLD: name = "swdatetimefield"; break;
                default:
                    SAL_INFO("sw.core", "unhandled field type " << pCurFldFmt->GetField()->GetTyp()->Which());
                    break;
            }
            writer.startElement(name);
            writer.writeFormatAttribute("ptr", "%p", pCurFldFmt->GetField());
            const SwPostItField* pField = NULL;
            if (pCurFldFmt->GetField()->GetTyp()->Which() == RES_POSTITFLD)
                pField = dynamic_cast<const SwPostItField*>(pCurFldFmt->GetField());
            if (pField)
            {
                OString txt8 = OUStringToOString(pField->GetName(), RTL_TEXTENCODING_UTF8);
                writer.writeFormatAttribute("name", "%s", BAD_CAST( txt8.getStr()));
            }
            writer.endElement();
            writer.endElement();
        }
    }
    writer.endElement();
}

void SwNodes::dumpAsXml( xmlTextWriterPtr w ) const
{
    WriterHelper writer( w );
    writer.startElement( "swnodes" );
    writer.writeFormatAttribute( "ptr", "%p", this );
    for( unsigned int i = 0; i < Count(); ++i )
    {
        ( *this )[ i ]->dumpAsXml( writer );
    }
    writer.endElement();
}

void SwNode::dumpAsXml( xmlTextWriterPtr w ) const
{
    WriterHelper writer( w );
    const char* name = "???";
    switch( GetNodeType())
    {
        case ND_ENDNODE:
            name = "end";
            break;
        case ND_STARTNODE:
        case ND_TEXTNODE:
            abort(); // overridden
        case ND_TABLENODE:
            name = "table";
            break;
        case ND_GRFNODE:
            name = "grf";
            break;
        case ND_OLENODE:
            name = "ole";
            break;
    }
    writer.startElement( name );
    writer.writeFormatAttribute( "ptr", "%p", this );
    writer.writeFormatAttribute( "type", "0x%04x", GetNodeType() );
    writer.writeFormatAttribute( "index", TMP_FORMAT, GetIndex() );
    writer.endElement();
    if( GetNodeType() == ND_ENDNODE )
        writer.endElement(); // end start node
}

void SwStartNode::dumpAsXml( xmlTextWriterPtr w ) const
{
    WriterHelper writer( w );
    const char* name = "???";
    switch( GetNodeType() )
    {
        case ND_TABLENODE:
            name = "table";
            break;
        case ND_SECTIONNODE:
            name = "section";
            break;
        default:
            switch( GetStartNodeType())
            {
                case SwNormalStartNode:
                    name = "start";
                    break;
                case SwTableBoxStartNode:
                    name = "tablebox";
                    break;
                case SwFlyStartNode:
                    name = "fly";
                    break;
                case SwFootnoteStartNode:
                    name = "footnote";
                    break;
                case SwHeaderStartNode:
                    name = "header";
                    break;
                case SwFooterStartNode:
                    name = "footer";
                    break;
            }
            break;
    }
    writer.startElement( name );
    writer.writeFormatAttribute( "ptr", "%p", this );
    writer.writeFormatAttribute( "type", "0x%04x", GetNodeType() );
    writer.writeFormatAttribute( "index", TMP_FORMAT, GetIndex() );

    if (IsTableNode())
    {
        writer.startElement("attrset");
        const SwAttrSet& rAttrSet = GetTableNode()->GetTable().GetFrmFmt()->GetAttrSet();
        lcl_dumpSfxItemSet(writer, &rAttrSet);
        writer.endElement();
    }

    // writer.endElement(); - it is a start node, so don't end, will make xml better nested
}

void lcl_dumpSfxItemSet(WriterHelper& writer, const SfxItemSet* pSet)
{
    SfxItemIter aIter(*pSet);
    const SfxPoolItem* pItem = aIter.FirstItem();
    while (pItem)
    {
        bool bDone = true;
        switch (pItem->Which())
        {
            case RES_CHRATR_POSTURE:
            case RES_CHRATR_CJK_POSTURE:
            case RES_CHRATR_CTL_POSTURE:
                static_cast<const SvxPostureItem*>(pItem)->dumpAsXml(writer);
                break;
            case RES_CHRATR_WEIGHT:
            case RES_CHRATR_CJK_WEIGHT:
            case RES_CHRATR_CTL_WEIGHT:
                static_cast<const SvxWeightItem*>(pItem)->dumpAsXml(writer);
                break;
            case RES_CHRATR_RSID:
                static_cast<const SvxRsidItem*>(pItem)->dumpAsXml(writer);
                break;
            case RES_CHRATR_ROTATE:
                static_cast<const SvxCharRotateItem*>(pItem)->dumpAsXml(writer);
                break;
            case RES_PARATR_OUTLINELEVEL:
                static_cast<const SfxUInt16Item*>(pItem)->dumpAsXml(writer);
                break;
            case RES_PARATR_NUMRULE:
                static_cast<const SwNumRuleItem*>(pItem)->dumpAsXml(writer);
                break;
            case RES_CHRATR_FONT:
            case RES_CHRATR_CTL_FONT:
                static_cast<const SvxFontItem*>(pItem)->dumpAsXml(writer);
                break;
            case RES_CHRATR_BACKGROUND:
                static_cast<const SvxBrushItem*>(pItem)->dumpAsXml(writer);
                break;
            case RES_CHRATR_FONTSIZE:
                static_cast<const SvxFontHeightItem*>(pItem)->dumpAsXml(writer);
                break;
            case RES_CNTNT:
                static_cast<const SwFmtCntnt*>(pItem)->dumpAsXml(writer);
                break;
            case RES_FRM_SIZE:
                static_cast<const SwFmtFrmSize*>(pItem)->dumpAsXml(writer);
                break;
            case RES_VERT_ORIENT:
                static_cast<const SwFmtVertOrient*>(pItem)->dumpAsXml(writer);
                break;
            case RES_HORI_ORIENT:
                static_cast<const SwFmtHoriOrient*>(pItem)->dumpAsXml(writer);
                break;
            case RES_ANCHOR:
                static_cast<const SwFmtAnchor*>(pItem)->dumpAsXml(writer);
                break;
            case RES_SURROUND:
                static_cast<const SwFmtSurround*>(pItem)->dumpAsXml(writer);
                break;
            case RES_FOLLOW_TEXT_FLOW:
                static_cast<const SwFmtFollowTextFlow*>(pItem)->dumpAsXml(writer);
                break;
            case RES_WRAP_INFLUENCE_ON_OBJPOS:
                static_cast<const SwFmtWrapInfluenceOnObjPos*>(pItem)->dumpAsXml(writer);
                break;
            case RES_COL:
                static_cast<const SwFmtCol*>(pItem)->dumpAsXml(writer);
                break;
            case XATTR_FILLSTYLE:
                static_cast<const XFillStyleItem*>(pItem)->dumpAsXml(writer);
                break;
            case XATTR_FILLCOLOR:
                static_cast<const XFillColorItem*>(pItem)->dumpAsXml(writer);
                break;
            case XATTR_FILLBITMAP:
                static_cast<const XFillBitmapItem*>(pItem)->dumpAsXml(writer);
                break;
            default: bDone = false; break;
        }
        if (bDone)
        {
            pItem = aIter.NextItem();
            continue;
        }

        writer.startElement("item");
        writer.writeFormatAttribute("whichId", TMP_FORMAT, pItem->Which());
        const char* pWhich = 0;
        boost::optional<OString> oValue;
        switch (pItem->Which())
        {
            case XATTR_FILLTRANSPARENCE:
                pWhich = "fill transparence";
                break;
            case XATTR_FILLBMP_TILE:
                pWhich = "fill bitmap tile";
                break;
            case XATTR_FILLBMP_POS:
                pWhich = "fill bitmap position";
                break;
            case XATTR_FILLBMP_STRETCH:
                pWhich = "fill bitmap stretch";
                break;
            case RES_PROTECT:
                pWhich = "protect";
                break;
            case RES_EDIT_IN_READONLY:
                pWhich = "edit in read-only";
                break;
            case RES_COLUMNBALANCE:
                pWhich = "column balance";
                break;
            case RES_LR_SPACE:
                pWhich = "left-right space";
                break;
            case RES_UL_SPACE:
                pWhich = "upper-lower space";
                break;
            case RES_SHADOW:
                pWhich = "shadow";
                break;
            case RES_PAGEDESC:
            {
                pWhich = "page description";
                const SwFmtPageDesc* pFmt = static_cast<const SwFmtPageDesc*>(pItem);
                if (pFmt->GetPageDesc())
                    oValue = "name: " + OUStringToOString(pFmt->GetPageDesc()->GetName(), RTL_TEXTENCODING_UTF8);
                break;
            }
            case RES_FRMATR_STYLE_NAME:
            {
                pWhich = "frame style name";
                const SfxStringItem* pStringItem = static_cast<const SfxStringItem*>(pItem);
                oValue = "name: " + OUStringToOString(pStringItem->GetValue(), RTL_TEXTENCODING_UTF8);
                break;
            }
            case RES_FRMATR_CONDITIONAL_STYLE_NAME:
            {
                pWhich = "frame conditional style name";
                const SfxStringItem* pStringItem = static_cast<const SfxStringItem*>(pItem);
                oValue = "name: " + OUStringToOString(pStringItem->GetValue(), RTL_TEXTENCODING_UTF8);
                break;
            }
        }
        if (pWhich)
            writer.writeFormatAttribute("which", "%s", BAD_CAST(pWhich));
        if (oValue)
            writer.writeFormatAttribute("value", "%s", BAD_CAST(oValue->getStr()));
        pItem = aIter.NextItem();
        writer.endElement();
    }
}

void SwFrmFmts::dumpAsXml(xmlTextWriterPtr w, const char* pName) const
{
    WriterHelper writer(w);
    if (size())
    {
        writer.startElement(pName);
        for (size_t i = 0; i < size(); ++i)
        {
            if (const SwFrmFmt* pFmt = GetFmt(i))
                pFmt->dumpAsXml(writer);
        }
        writer.endElement();
    }
}

void SwFrmFmt::dumpAsXml(xmlTextWriterPtr pWriter) const
{
    xmlTextWriterStartElement(pWriter, BAD_CAST("swFrmFmt"));
    xmlTextWriterWriteFormatAttribute(pWriter, BAD_CAST("ptr"), "%p", this);
    xmlTextWriterWriteAttribute(pWriter, BAD_CAST("name"), BAD_CAST(GetName().toUtf8().getStr()));
    xmlTextWriterWriteFormatAttribute(pWriter, BAD_CAST("whichId"), "%d", Which());

    const char* pWhich = 0;
    switch (Which())
    {
    case RES_FLYFRMFMT:
        pWhich = "fly frame format";
        break;
    case RES_DRAWFRMFMT:
        pWhich = "draw frame format";
        break;
    }
    if (pWhich)
        xmlTextWriterWriteAttribute(pWriter, BAD_CAST("which"), BAD_CAST(pWhich));

    WriterHelper w(pWriter);
    lcl_dumpSfxItemSet(w, &GetAttrSet());

    xmlTextWriterEndElement(pWriter);
}

void SwCharFmts::dumpAsXml(xmlTextWriterPtr w) const
{
    WriterHelper writer(w);
    if (size())
    {
        writer.startElement("swcharfmts");
        for (size_t i = 0; i < size(); ++i)
        {
            const SwCharFmt* pFmt = GetFmt(i);
            writer.startElement("swcharfmt");
            OString aName = OUStringToOString(pFmt->GetName(), RTL_TEXTENCODING_UTF8);
            writer.writeFormatAttribute("name", "%s", BAD_CAST(aName.getStr()));

            lcl_dumpSfxItemSet(writer, &pFmt->GetAttrSet());
            writer.endElement();
        }
        writer.endElement();
    }
}

void SwSectionFmts::dumpAsXml(xmlTextWriterPtr w) const
{
    WriterHelper writer(w);
    if (size())
    {
        writer.startElement("swsectionfmts");
        for (size_t i = 0; i < size(); ++i)
        {
            const SwSectionFmt* pFmt = GetFmt(i);
            writer.startElement("swsectionfmt");
            lcl_dumpSfxItemSet(writer, &pFmt->GetAttrSet());
            writer.endElement();
        }
        writer.endElement();
    }
}

void SwTxtFmtColls::dumpAsXml(xmlTextWriterPtr w) const
{
    WriterHelper writer(w);
    if (size())
    {
        writer.startElement("swtxtfmtcolls");
        for (size_t i = 0; i < size(); ++i)
        {
            const SwTxtFmtColl* pColl = GetFmt(i);
            writer.startElement("swtxtfmtcoll");
            OString aName = OUStringToOString(pColl->GetName(), RTL_TEXTENCODING_UTF8);
            writer.writeFormatAttribute("name", "%s", BAD_CAST(aName.getStr()));

            lcl_dumpSfxItemSet(writer, &pColl->GetAttrSet());
            writer.endElement();
        }
        writer.endElement();
    }
}

void SwNumRule::dumpAsXml(xmlTextWriterPtr w) const
{
     WriterHelper writer(w);
     writer.startElement("swnumrule");
     OString aName = OUStringToOString(GetName(), RTL_TEXTENCODING_UTF8);
     writer.writeFormatAttribute("name", "%s", BAD_CAST(aName.getStr()));
     writer.writeFormatAttribute("isautorule", TMP_FORMAT, IsAutoRule() ? 1 : 0);
     if (GetPoolFmtId() != USHRT_MAX)
         writer.writeFormatAttribute("poolfmtid", TMP_FORMAT, GetPoolFmtId());
     writer.endElement();
}

void SwNumRuleTbl::dumpAsXml(xmlTextWriterPtr w) const
{
    if (!empty())
    {
        WriterHelper writer(w);
        writer.startElement("swnumruletbl");
        for (size_t i = 0; i < size(); ++i)
            operator[](i)->dumpAsXml(w);
        writer.endElement();
    }
}

void SwTxtNode::dumpAsXml( xmlTextWriterPtr w ) const
{
    WriterHelper writer( w );
    writer.startElement( "text" );
    writer.writeFormatAttribute( "ptr", "%p", this );
    writer.writeFormatAttribute( "index", TMP_FORMAT, GetIndex() );
    OUString txt = GetTxt();
    for( int i = 0; i < 32; ++i )
        txt = txt.replace( i, '*' );
    OString txt8 = OUStringToOString( txt, RTL_TEXTENCODING_UTF8 );
    writer.startElement("inner_text");
    xmlTextWriterWriteString( writer, BAD_CAST( txt8.getStr()));
    writer.endElement( );

    if (GetFmtColl())
    {
        SwTxtFmtColl* pColl = static_cast<SwTxtFmtColl*>(GetFmtColl());
        writer.startElement("swtxtfmtcoll");
        OString aName = OUStringToOString(pColl->GetName(), RTL_TEXTENCODING_UTF8);
        writer.writeFormatAttribute("name", "%s", BAD_CAST(aName.getStr()));
        writer.endElement();
    }

    if (HasSwAttrSet())
    {
        writer.startElement("attrset");
        const SwAttrSet& rAttrSet = GetSwAttrSet();
        lcl_dumpSfxItemSet(writer, &rAttrSet);
        writer.endElement();
    }

    if (HasHints())
    {
        writer.startElement("hints");
        const SwpHints& rHints = GetSwpHints();
        for (size_t i = 0; i < rHints.Count(); ++i)
        {
            writer.startElement("hint");
            const SwTxtAttr* pHint = rHints.GetTextHint(i);

            if (pHint->GetStart())
                writer.writeFormatAttribute("start", TMP_FORMAT, pHint->GetStart());
            if (pHint->End())
                writer.writeFormatAttribute("end", TMP_FORMAT, *pHint->End());
            writer.writeFormatAttribute("whichId", TMP_FORMAT, pHint->Which());

            const char* pWhich = 0;
            boost::optional<OString> oValue;
            switch (pHint->Which())
            {
                case RES_TXTATR_AUTOFMT:
                    pWhich = "autofmt";
                    break;
                case RES_TXTATR_ANNOTATION:
                    pWhich = "annotation";
                    break;
                case RES_TXTATR_FLYCNT:
                    pWhich = "fly content";
                    break;
                case RES_TXTATR_CHARFMT:
                {
                    pWhich = "character format";
                    if (SwCharFmt* pCharFmt = pHint->GetCharFmt().GetCharFmt())
                        oValue = "name: " + OUStringToOString(pCharFmt->GetName(), RTL_TEXTENCODING_UTF8);
                    break;
                }
                default:
                    break;
            }
            if (pWhich)
                writer.writeFormatAttribute("which", "%s", BAD_CAST(pWhich));
            if (oValue)
                writer.writeFormatAttribute("value", "%s", BAD_CAST(oValue->getStr()));

            if (pHint->Which() == RES_TXTATR_AUTOFMT)
            {
                boost::shared_ptr<SfxItemSet> const pSet(pHint->GetAutoFmt().GetStyleHandle());
                writer.startElement("autofmt");
                lcl_dumpSfxItemSet(writer, pSet.get());
                writer.endElement();
            }

            writer.endElement();
        }
        writer.endElement();
    }
    if (GetNumRule())
        GetNumRule()->dumpAsXml(w);

    writer.endElement();
}

void SwRedlineTbl::dumpAsXml( xmlTextWriterPtr w ) const
{
    WriterHelper writer( w );

    writer.startElement( "swredlinetbl" );
    writer.writeFormatAttribute( "ptr", "%p", this );

    const SwRedlineTbl& redlineTbl = (*this);

    for( sal_uInt16 nCurRedlinePos = 0; nCurRedlinePos < size(); ++nCurRedlinePos )
    {
        const SwRangeRedline* pRedline = redlineTbl[ nCurRedlinePos ];

        writer.startElement( "swredline" );
        writer.writeFormatAttribute( "ptr", "%p", pRedline );

        OString aId( OString::number( pRedline->GetSeqNo() ) );
        const OUString &rAuthor( SW_MOD()->GetRedlineAuthor( pRedline->GetAuthor() ) );
        OString aAuthor( OUStringToOString( rAuthor, RTL_TEXTENCODING_UTF8 ) );
        OString aDate( DateTimeToOString( pRedline->GetTimeStamp() ) );
        OString sRedlineType;
        switch( pRedline->GetType() )
        {
            case nsRedlineType_t::REDLINE_INSERT:
                sRedlineType = "REDLINE_INSERT";
                break;
            case nsRedlineType_t::REDLINE_DELETE:
                sRedlineType = "REDLINE_DELETE";
                break;
            case nsRedlineType_t::REDLINE_FORMAT:
                sRedlineType = "REDLINE_FORMAT";
                break;
            default:
                sRedlineType = "UNKNOWN";
                break;
        }
        writer.writeFormatAttribute( "id", "%s", BAD_CAST(aId.getStr()) );
        writer.writeFormatAttribute( "author", "%s", BAD_CAST(aAuthor.getStr()) );
        writer.writeFormatAttribute( "date", "%s", BAD_CAST(aDate.getStr()) );
        writer.writeFormatAttribute( "type", "%s", BAD_CAST(sRedlineType.getStr()) );
        {
            const SwPosition* pStart = pRedline->Start();

            writer.startElement( "swposition_start" );
            //writer.writeFormatAttribute( "ptr", "%p", pStart );
            {
                const SwNodeIndex pStartNodeIndex = pStart->nNode;
                //writer.startElement( "swnodeindex" );
                //writer.writeFormatAttribute( "ptr", "%p", &pStartNodeIndex );
                {
                    const SwNode&     pStartSwNode      = pStartNodeIndex.GetNode();
                    //writer.startElement( "swnode" );
                    //writer.writeFormatAttribute( "ptr", "%p", &pStartSwNode );
                    //writer.writeFormatAttribute( "type", "%d", pStartSwNode.GetNodeType() );
                    //writer.endElement( );    // swnode
                    writer.writeFormatAttribute( "swnode_type", TMP_FORMAT, pStartSwNode.GetNodeType() );

                    writer.writeFormatAttribute( "paragraph_index", "%d", (int)pStartNodeIndex.GetIndex() );

                    const SwIndex&    pStartContent   = pStart->nContent;
                    //writer.startElement( "swindex" );
                    //writer.writeFormatAttribute( "ptr", "%p", &pStartContent );
                    //writer.writeFormatAttribute( "content_index", "%d", pStartContent.GetIndex() );
                    //writer.endElement( );    // swindex
                    writer.writeFormatAttribute( "character_index", TMP_FORMAT_I32, pStartContent.GetIndex() );
                }
                //writer.endElement( );    // swnodeindex
            }
            writer.endElement( );    // swposition_start

            const SwPosition* pEnd;
            bool bEndIsMark = false;
            if ( pStart == pRedline->GetPoint() )
            {
                // End = Mark
                pEnd = pRedline->GetMark();
                bEndIsMark = true;
            }
            else
            {
                // End = Point
                pEnd = pRedline->GetPoint();
            }

            writer.startElement( "swposition_end" );
            //writer.writeFormatAttribute( "ptr", "%p", pStart );
            {
                const SwNodeIndex pEndNodeIndex = pEnd->nNode;
                //writer.startElement( "swnodeindex" );
                //writer.writeFormatAttribute( "ptr", "%p", &pEndNodeIndex );
                {
                    const SwNode&     pEndSwNode      = pEndNodeIndex.GetNode();
                    //writer.startElement( "swnode" );
                    //writer.writeFormatAttribute( "ptr", "%p", &pEndSwNode );
                    //writer.writeFormatAttribute( "type", "%d", pEndSwNode.GetNodeType() );
                    //writer.endElement( );    // swnode
                    writer.writeFormatAttribute( "swnode_type", TMP_FORMAT, pEndSwNode.GetNodeType() );

                    writer.writeFormatAttribute( "paragraph_index", "%d", (int)pEndNodeIndex.GetIndex() );

                    const SwIndex&    pEndContent   = pEnd->nContent;
                    //writer.startElement( "swindex" );
                    //writer.writeFormatAttribute( "ptr", "%p", &pEndContent );
                    //writer.writeFormatAttribute( "content_index", "%d", pEndContent.GetIndex() );
                    //writer.endElement( );    // swindex
                    writer.writeFormatAttribute( "character_index", TMP_FORMAT_I32, pEndContent.GetIndex() );
                }
                //writer.endElement( );    // swnodeindex
            }
            writer.writeFormatAttribute( "end_is", "%s", BAD_CAST(bEndIsMark ? "mark" : "point"));
            writer.endElement( );    // swposition_end

            //const SwRedlineData& aRedlineData = pRedline->GetRedlineData();
            const SwRedlineExtraData* pExtraRedlineData = pRedline->GetExtraData();
            writer.startElement( "extra_redline_data" );
            {
                const SwRedlineExtraData_FmtColl*           pExtraData_FmtColl           = dynamic_cast<const SwRedlineExtraData_FmtColl*>(pExtraRedlineData);
                const SwRedlineExtraData_Format*            pExtraData_Format            = dynamic_cast<const SwRedlineExtraData_Format*>(pExtraRedlineData);
                const SwRedlineExtraData_FormattingChanges* pExtraData_FormattingChanges = dynamic_cast<const SwRedlineExtraData_FormattingChanges*>(pExtraRedlineData);
                if (pExtraData_FmtColl)
                    writer.writeFormatAttribute( "extra_data_type", "%s", BAD_CAST( "fmt coll" ) );
                else if (pExtraData_Format)
                    writer.writeFormatAttribute( "extra_data_type", "%s", BAD_CAST( "format" ) );
                else if (pExtraData_FormattingChanges)
                    writer.writeFormatAttribute( "extra_data_type", "%s", BAD_CAST( "formatting changes" ) );
                else
                    writer.writeFormatAttribute( "extra_data_type", "%s", BAD_CAST( "UNKNOWN" ) );
            }
            writer.endElement( );    // extra_redline_data
        }

        writer.endElement( );    // extra_redline_data
    }

    writer.endElement( );    // swredlinetbl
}

void SwExtraRedlineTbl::dumpAsXml( xmlTextWriterPtr w ) const
{
    WriterHelper writer( w );

    writer.startElement( "swextraredlinetbl" );
    writer.writeFormatAttribute( "ptr", "%p", this );

    const SwExtraRedlineTbl& extraRedlineTbl = (*this);

    for( sal_uInt16 nCurExtraRedlinePos = 0; nCurExtraRedlinePos < GetSize(); ++nCurExtraRedlinePos )
    {
        const SwExtraRedline* pExtraRedline = extraRedlineTbl.GetRedline( nCurExtraRedlinePos );

        writer.startElement( "swextraredline" );
        {
            const SwTableRowRedline*           pTableRowRedline           = dynamic_cast<const SwTableRowRedline*>(pExtraRedline);
            const SwTableCellRedline*          pTableCellRedline          = dynamic_cast<const SwTableCellRedline*>(pExtraRedline);
            if (pTableRowRedline)
                writer.writeFormatAttribute( "extra_redline_type", "%s", BAD_CAST( "table row" ) );
            else if (pTableCellRedline)
                writer.writeFormatAttribute( "extra_redline_type", "%s", BAD_CAST( "table cell" ) );
            else
                writer.writeFormatAttribute( "extra_redline_type", "%s", BAD_CAST( "UNKNOWN" ) );
        }
        writer.endElement( );    // extra_redline_data
    }

    writer.endElement( );    // swextraredlinetbl
}

void lcl_dumpSfxUndoAction(WriterHelper& writer, SfxUndoAction* pAction)
{
    writer.startElement("undoAction");
    writer.writeFormatAttribute("symbol", "%s", BAD_CAST(typeid(*pAction).name()));
    writer.writeFormatAttribute("comment", "%s", BAD_CAST(OUStringToOString(pAction->GetComment(), RTL_TEXTENCODING_UTF8).getStr()));

    if (SfxListUndoAction* pList = dynamic_cast<SfxListUndoAction*>(pAction))
    {
        writer.startElement("list");
        writer.writeFormatAttribute("size", TMP_FORMAT, pList->aUndoActions.size());
        for (size_t i = 0; i < pList->aUndoActions.size(); ++i)
            lcl_dumpSfxUndoAction(writer, pList->aUndoActions.GetUndoAction(i));
        writer.endElement();
    }

    writer.endElement();
}

void sw::UndoManager::dumpAsXml(xmlTextWriterPtr w)
{
    WriterHelper writer(w);

    writer.startElement("m_pUndoManager");
    writer.writeFormatAttribute("nUndoActionCount", TMP_FORMAT, GetUndoActionCount());

    for (size_t i = 0; i < GetUndoActionCount(); ++i)
        lcl_dumpSfxUndoAction(writer, GetUndoAction(i));

    writer.endElement();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
