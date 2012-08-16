/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * Version: MPL 1.1 / GPLv3+ / LGPLv3+
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License. You may obtain a copy of the License at
 * http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * The Initial Developer of the Original Code is
 *       Novell Inc.
 * Portions created by the Initial Developer are Copyright (C) 2010 the
 * Initial Developer. All Rights Reserved.
 *
 * Contributor(s): Florian Reuter <freuter@novell.com>
 *                 Cedric Bosdonnat <cbosdonnat@novell.com>
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 3 or later (the "GPLv3+"), or
 * the GNU Lesser General Public License Version 3 or later (the "LGPLv3+"),
 * in which case the provisions of the GPLv3+ or the LGPLv3+ are applicable
 * instead of those above.
 */

#include "frame.hxx"
#include "frmfmt.hxx"
#include "sectfrm.hxx"
#include "tabfrm.hxx"
#include "txtfrm.hxx"
#include "hffrm.hxx"
#include "porlin.hxx"
#include "porlay.hxx"
#include "portxt.hxx"
#include "sortedobjs.hxx"
#include <anchoredobject.hxx>
#include <libxml/xmlwriter.h>
#include <SwPortionHandler.hxx>

class XmlPortionDumper:public SwPortionHandler
{
  private:
    xmlTextWriterPtr writer;
    sal_uInt16 ofs;

    const char* getTypeName( sal_uInt16 nType )
    {
        switch ( nType )
        {
            case POR_LIN: return "POR_LIN";
            case POR_FLYCNT: return "POR_FLYCNT";

            case POR_HOLE: return "POR_HOLE";
            case POR_TMPEND: return "POR_TMPEND";
            case POR_BRK: return "POR_BRK";
            case POR_KERN: return "POR_KERN";
            case POR_ARROW: return "POR_ARROW";
            case POR_MULTI: return "POR_MULTI";
            case POR_HIDDEN_TXT: return "POR_HIDDEN_TXT";
            case POR_CONTROLCHAR: return "POR_CONTROLCHAR";

            case POR_TXT: return "POR_TXT";
            case POR_LAY: return "POR_LAY";
            case POR_PARA: return "POR_PARA";
            case POR_URL: return "POR_URL";
            case POR_HNG: return "POR_HNG";

            case POR_DROP: return "POR_DROP";
            case POR_TOX: return "POR_TOX";
            case POR_ISOTOX: return "POR_ISOTOX";
            case POR_REF: return "POR_REF";
            case POR_ISOREF: return "POR_ISOREF";
            case POR_META: return "POR_META";

            case POR_EXP: return "POR_EXP";
            case POR_BLANK: return "POR_BLANK";
            case POR_POSTITS: return "POR_POSTITS";

            case POR_HYPH: return "POR_HYPH";
            case POR_HYPHSTR: return "POR_HYPHSTR";
            case POR_SOFTHYPH: return "POR_SOFTHYPH";
            case POR_SOFTHYPHSTR: return "POR_SOFTHYPHSTR";
            case POR_SOFTHYPH_COMP: return "POR_SOFTHYPH_COMP";

            case POR_FLD: return "POR_FLD";
            case POR_HIDDEN: return "POR_HIDDEN";
            case POR_QUOVADIS: return "POR_QUOVADIS";
            case POR_ERGOSUM: return "POR_ERGOSUM";
            case POR_COMBINED: return "POR_COMBINED";
            case POR_FTN: return "POR_FTN";

            case POR_FTNNUM: return "POR_FTNNUM";
            case POR_NUMBER: return "POR_NUMBER";
            case POR_BULLET: return "POR_BULLET";
            case POR_GRFNUM: return "POR_GRFNUM";

            case POR_GLUE: return "POR_GLUE";

            case POR_MARGIN: return "POR_MARGIN";

            case POR_FIX: return "POR_FIX";
            case POR_FLY: return "POR_FLY";

            case POR_TAB: return "POR_TAB";

            case POR_TABRIGHT: return "POR_TABRIGHT";
            case POR_TABCENTER: return "POR_TABCENTER";
            case POR_TABDECIMAL: return "POR_TABDECIMAL";

            case POR_TABLEFT: return "POR_TABLEFT";
            default:
                return "Unknown";
        }
    }

  public:

    XmlPortionDumper( xmlTextWriterPtr some_writer ):writer( some_writer ), ofs( 0 )
    {
    }

    virtual ~ XmlPortionDumper(  )
    {
    }

    /**
        @param nLength
                length of this portion in the model string
        @param rText
                text which is painted on-screen
      */
    virtual void Text( sal_uInt16 nLength,
                       sal_uInt16 nType,
                       sal_Int32 nHeight,
                       sal_Int32 nWidth)
    {
        ofs += nLength;
        xmlTextWriterStartElement( writer, BAD_CAST( "Text" ) );
        xmlTextWriterWriteFormatAttribute( writer,
                                           BAD_CAST( "nLength" ),
                                           "%i", ( int ) nLength );
        xmlTextWriterWriteFormatAttribute( writer,
                                           BAD_CAST( "nType" ),
                                           "%s", getTypeName( nType ) );
        if (nHeight > 0)
            xmlTextWriterWriteFormatAttribute(writer, BAD_CAST("nHeight"), "%i", (int)nHeight);
        if (nWidth > 0)
            xmlTextWriterWriteFormatAttribute(writer, BAD_CAST("nWidth"), "%i", (int)nWidth);

        xmlTextWriterEndElement( writer );
    }

    /**
        @param nLength
                length of this portion in the model string
        @param rText
                text which is painted on-screen
        @param nType
                type of this portion
        @param nHeight
                font size of the painted text
      */
    virtual void Special( sal_uInt16 nLength,
                          const String & rText,
                          sal_uInt16 nType,
                          sal_Int32 nHeight,
                          sal_Int32 nWidth )
    {
        xmlTextWriterStartElement( writer, BAD_CAST( "Special" ) );
        xmlTextWriterWriteFormatAttribute( writer,
                                           BAD_CAST( "nLength" ),
                                           "%i", ( int ) nLength );
        xmlTextWriterWriteFormatAttribute( writer,
                                           BAD_CAST( "nType" ),
                                           "%s", getTypeName( nType ) );
        rtl::OUString sText( rText );
        rtl::OString sText8 =::rtl::OUStringToOString( sText,
                                                       RTL_TEXTENCODING_UTF8 );
        xmlTextWriterWriteFormatAttribute( writer, BAD_CAST( "rText" ),
                                           "%s", sText8.getStr(  ) );

        if (nHeight > 0)
            xmlTextWriterWriteFormatAttribute(writer, BAD_CAST("nHeight"), "%i", (int)nHeight);

        if (nWidth > 0)
            xmlTextWriterWriteFormatAttribute(writer, BAD_CAST("nWidth"), "%i", (int)nWidth);

        xmlTextWriterEndElement( writer );
        ofs += nLength;
    }

    virtual void LineBreak(  )
    {
        xmlTextWriterStartElement( writer, BAD_CAST( "LineBreak" ) );
        xmlTextWriterEndElement( writer );
    }

    /**
      * @param nLength
      *         number of 'model string' characters to be skipped
      */
    virtual void Skip( sal_uInt16 nLength )
    {
        xmlTextWriterStartElement( writer, BAD_CAST( "Skip" ) );
        xmlTextWriterWriteFormatAttribute( writer,
                                           BAD_CAST( "nLength" ),
                                           "%i", ( int ) nLength );
        xmlTextWriterEndElement( writer );
        ofs += nLength;
    }

    virtual void Finish(  )
    {
        xmlTextWriterStartElement( writer, BAD_CAST( "Finish" ) );
        xmlTextWriterEndElement( writer );
    }

};

namespace
{
    xmlTextWriterPtr lcl_createDefaultWriter()
    {
        xmlTextWriterPtr writer = xmlNewTextWriterFilename( "layout.xml", 0 );
        xmlTextWriterStartDocument( writer, NULL, NULL, NULL );
        return writer;
    }

    void lcl_freeWriter( xmlTextWriterPtr writer )
    {
        xmlTextWriterEndDocument( writer );
        xmlFreeTextWriter( writer );
    }
}

void SwFrm::dumpAsXml( xmlTextWriterPtr writer )
{
    bool bCreateWriter = ( NULL == writer );
    if ( bCreateWriter )
        writer = lcl_createDefaultWriter();

    const char *name = NULL;

    switch ( GetType(  ) )
    {
    case FRM_ROOT:
        name = "root";
        break;
    case FRM_PAGE:
        name = "page";
        break;
    case FRM_COLUMN:
        name = "column";
        break;
    case FRM_HEADER:
        name = "header";
        break;
    case FRM_FOOTER:
        name = "footer";
        break;
    case FRM_FTNCONT:
        name = "ftncont";
        break;
    case FRM_FTN:
        name = "ftn";
        break;
    case FRM_BODY:
        name = "body";
        break;
    case FRM_FLY:
        name = "fly";
        break;
    case FRM_SECTION:
        name = "section";
        break;
    case FRM_UNUSED:
        name = "unused";
        break;
    case FRM_TAB:
        name = "tab";
        break;
    case FRM_ROW:
        name = "row";
        break;
    case FRM_CELL:
        name = "cell";
        break;
    case FRM_TXT:
        name = "txt";
        break;
    case FRM_NOTXT:
        name = "notxt";
        break;
    };

    if ( name != NULL )
    {
        xmlTextWriterStartElement( writer, ( const xmlChar * ) name );

        dumpAsXmlAttributes( writer );

        xmlTextWriterStartElement( writer, BAD_CAST( "infos" ) );
        dumpInfosAsXml( writer );
        xmlTextWriterEndElement( writer );

        // Dump Anchored objects if any
        SwSortedObjs* pAnchored = GetDrawObjs();
        if ( pAnchored && pAnchored->Count( ) > 0 )
        {
            xmlTextWriterStartElement( writer, BAD_CAST( "anchored" ) );

            for ( sal_uInt32 i = 0, len = pAnchored->Count(); i < len; i++ )
            {
                SwAnchoredObject* pObject = (*pAnchored)[i];
                pObject->dumpAsXml( writer );
            }

            xmlTextWriterEndElement( writer );
        }

        // Dump the children
        if ( IsTxtFrm(  ) )
        {
            SwTxtFrm *pTxtFrm = ( SwTxtFrm * ) this;
            rtl::OUString aTxt = pTxtFrm->GetTxt(  );
            for ( int i = 0; i < 32; i++ )
            {
                aTxt = aTxt.replace( i, '*' );
            }
            rtl::OString aTxt8 =::rtl::OUStringToOString( aTxt,
                                                          RTL_TEXTENCODING_UTF8 );
            xmlTextWriterWriteString( writer,
                                      ( const xmlChar * ) aTxt8.getStr(  ) );
            XmlPortionDumper pdumper( writer );
            pTxtFrm->VisitPortions( pdumper );

        }
        else
        {
            dumpChildrenAsXml( writer );
        }
        xmlTextWriterEndElement( writer );
    }

    if ( bCreateWriter )
        lcl_freeWriter( writer );
}

void SwFrm::dumpInfosAsXml( xmlTextWriterPtr writer )
{
    // output the Frm
    xmlTextWriterStartElement( writer, BAD_CAST( "bounds" ) );
    xmlTextWriterWriteFormatAttribute( writer, BAD_CAST( "left" ), "%ld", Frm().Left() );
    xmlTextWriterWriteFormatAttribute( writer, BAD_CAST( "top" ), "%ld", Frm().Top() );
    xmlTextWriterWriteFormatAttribute( writer, BAD_CAST( "width" ), "%ld", Frm().Width() );
    xmlTextWriterWriteFormatAttribute( writer, BAD_CAST( "height" ), "%ld", Frm().Height() );
    xmlTextWriterEndElement( writer );
}

void SwFrm::dumpAsXmlAttributes( xmlTextWriterPtr writer )
{
    xmlTextWriterWriteFormatAttribute( writer, BAD_CAST( "ptr" ), "%p", this );
    xmlTextWriterWriteFormatAttribute( writer, BAD_CAST( "id" ), "%" SAL_PRIuUINT32, GetFrmId() );
    if ( GetNext( ) )
        xmlTextWriterWriteFormatAttribute( writer, BAD_CAST( "next" ), "%" SAL_PRIuUINT32, GetNext()->GetFrmId() );
    if ( GetPrev( ) )
        xmlTextWriterWriteFormatAttribute( writer, BAD_CAST( "prev" ), "%" SAL_PRIuUINT32, GetPrev()->GetFrmId() );
    if ( GetUpper( ) )
        xmlTextWriterWriteFormatAttribute( writer, BAD_CAST( "upper" ), "%" SAL_PRIuUINT32, GetUpper()->GetFrmId() );
    if ( GetLower( ) )
        xmlTextWriterWriteFormatAttribute( writer, BAD_CAST( "lower" ), "%" SAL_PRIuUINT32, GetLower()->GetFrmId() );
    if ( IsTxtFrm(  ) )
    {
        SwTxtFrm *pTxtFrm = ( SwTxtFrm * ) this;
        SwTxtNode *pTxtNode = pTxtFrm->GetTxtNode();
        xmlTextWriterWriteFormatAttribute( writer, BAD_CAST( "txtNodeIndex" ), "%lu", pTxtNode->GetIndex() );
    }
    if (IsHeaderFrm() || IsFooterFrm())
    {
        SwHeadFootFrm *pHeadFootFrm = (SwHeadFootFrm*)this;
        rtl::OUString aFmtName = pHeadFootFrm->GetFmt()->GetName();
        xmlTextWriterWriteFormatAttribute( writer, BAD_CAST( "fmtName" ), "%s", BAD_CAST(rtl::OUStringToOString(aFmtName, RTL_TEXTENCODING_UTF8).getStr()));
    }
}

void SwFrm::dumpChildrenAsXml( xmlTextWriterPtr writer )
{
    SwFrm *pFrm = GetLower(  );
    for ( ; pFrm != NULL; pFrm = pFrm->GetNext(  ) )
    {
        pFrm->dumpAsXml( writer );
    }
}

void SwAnchoredObject::dumpAsXml( xmlTextWriterPtr writer )
{
    bool bCreateWriter = ( NULL == writer );
    if ( bCreateWriter )
        writer = lcl_createDefaultWriter();

    xmlTextWriterStartElement( writer, BAD_CAST( getElementName() ) );
    xmlTextWriterWriteFormatAttribute( writer, BAD_CAST( "ptr" ), "%p", this );
    xmlTextWriterEndElement( writer );

    if ( bCreateWriter )
        lcl_freeWriter( writer );
}

void SwTxtFrm::dumpAsXmlAttributes( xmlTextWriterPtr writer )
{
    SwFrm::dumpAsXmlAttributes( writer );
    if ( HasFollow() )
        xmlTextWriterWriteFormatAttribute( writer, BAD_CAST( "follow" ), "%" SAL_PRIuUINT32, GetFollow()->GetFrmId() );

    if (m_pPrecede != NULL)
        xmlTextWriterWriteFormatAttribute( writer, BAD_CAST( "precede" ), "%" SAL_PRIuUINT32, static_cast<SwTxtFrm*>(m_pPrecede)->GetFrmId() );
}

void SwSectionFrm::dumpAsXmlAttributes( xmlTextWriterPtr writer )
{
    SwFrm::dumpAsXmlAttributes( writer );
    if ( HasFollow() )
        xmlTextWriterWriteFormatAttribute( writer, BAD_CAST( "follow" ), "%" SAL_PRIuUINT32, GetFollow()->GetFrmId() );

    if (m_pPrecede != NULL)
        xmlTextWriterWriteFormatAttribute( writer, BAD_CAST( "precede" ), "%" SAL_PRIuUINT32, static_cast<SwSectionFrm*>( m_pPrecede )->GetFrmId() );
}

void SwTabFrm::dumpAsXmlAttributes( xmlTextWriterPtr writer )
{
    SwFrm::dumpAsXmlAttributes( writer );
    if ( HasFollow() )
        xmlTextWriterWriteFormatAttribute( writer, BAD_CAST( "follow" ), "%" SAL_PRIuUINT32, GetFollow()->GetFrmId() );

    if (m_pPrecede != NULL)
        xmlTextWriterWriteFormatAttribute( writer, BAD_CAST( "precede" ), "%" SAL_PRIuUINT32, static_cast<SwTabFrm*>( m_pPrecede )->GetFrmId() );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
