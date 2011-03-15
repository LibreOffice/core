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
#include "precompiled_sw.hxx"

#include "frame.hxx"
#include "txtfrm.hxx"
#include "porlin.hxx"
#include "porlay.hxx"
#include "portxt.hxx"
#include <libxml/xmlwriter.h>
#include <SwPortionHandler.hxx>

class XmlPortionDumper:public SwPortionHandler
{
  private:
    xmlTextWriterPtr writer;
    sal_uInt16 ofs;
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
                       sal_uInt16 nType )
    {
        ofs += nLength;
        xmlTextWriterStartElement( writer, BAD_CAST( "Text" ) );
        xmlTextWriterWriteFormatAttribute( writer,
                                           BAD_CAST( "nLength" ),
                                           "%i", ( int ) nLength );
        xmlTextWriterWriteFormatAttribute( writer,
                                           BAD_CAST( "nType" ),
                                           "%i", ( int ) nType );
        xmlTextWriterEndElement( writer );
    }

    /**
        @param nLength
                length of this portion in the model string
        @param rText
                text which is painted on-screen
        @param nType
                type of this portion
      */
    virtual void Special( sal_uInt16 nLength,
                          const String & rText,
                          sal_uInt16 nType )
    {
        xmlTextWriterStartElement( writer, BAD_CAST( "Special" ) );
        xmlTextWriterWriteFormatAttribute( writer,
                                           BAD_CAST( "nLength" ),
                                           "%i", ( int ) nLength );
        xmlTextWriterWriteFormatAttribute( writer,
                                           BAD_CAST( "nType" ),
                                           "%i", ( int ) nType );
        rtl::OUString sText( rText );
        rtl::OString sText8 =::rtl::OUStringToOString( sText,
                                                       RTL_TEXTENCODING_UTF8 );
        xmlTextWriterWriteFormatAttribute( writer, BAD_CAST( "rText" ),
                                           "%s", sText8.getStr(  ) );

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


void SwTxtPortion::dumpPortionAsXml( xub_StrLen ofs, XubString & /*aText */,
                                     xmlTextWriterPtr writer )
{
    xmlTextWriterStartElement( writer, BAD_CAST( "SwTxtPortion" ) );
    xmlTextWriterWriteFormatAttribute( writer, BAD_CAST( "ofs" ), "%i", ofs );
    xmlTextWriterWriteFormatAttribute( writer, BAD_CAST( "len" ), "%i",
                                       ( int ) this->GetLen(  ) );

    xmlTextWriterEndElement( writer );
}

void SwLinePortion::dumpPortionAsXml( xub_StrLen ofs, XubString & /*aText */,
                                      xmlTextWriterPtr writer )
{
    xmlTextWriterStartElement( writer, BAD_CAST( "SwLinePortion" ) );
    xmlTextWriterWriteFormatAttribute( writer,
                                       BAD_CAST( "nWhichPor" ),
                                       "%04X",
                                       ( int ) this->GetWhichPor(  ) );
    xmlTextWriterWriteFormatAttribute( writer, BAD_CAST( "ofs" ), "%i", ofs );
    xmlTextWriterWriteFormatAttribute( writer, BAD_CAST( "len" ), "%i",
                                       ( int ) this->GetLen(  ) );
    xmlTextWriterEndElement( writer );
}

void SwLineLayout::dumpLineAsXml( xmlTextWriterPtr writer,
                                  xub_StrLen & ofs, XubString & aText )
{                               // not used any longer...
    xmlTextWriterStartElement( writer, BAD_CAST( "SwLineLayout" ) );
    SwLinePortion *portion = this;
    while ( portion != NULL )
    {
        portion->dumpPortionAsXml( ofs, aText, writer );
        ofs += portion->GetLen(  );
        portion = portion->GetPortion(  );
    }
    xmlTextWriterEndElement( writer );
}


void SwParaPortion::dumpAsXml( xmlTextWriterPtr writer, SwTxtFrm * pTxtFrm )
{
    xmlTextWriterStartElement( writer, BAD_CAST( "SwParaPortion" ) );
    SwParaPortion *pPara = this;

    if ( pPara && pTxtFrm )
    {
        xub_StrLen ofs = 0;
        XubString & aText = ( String & ) pTxtFrm->GetTxt(  );
        if ( pTxtFrm->IsFollow(  ) )
            ofs += pTxtFrm->GetOfst(  );

        SwLineLayout *pLine = pPara;
        while ( pLine )
        {
            xmlTextWriterStartElement( writer, BAD_CAST( "line" ) );
            SwLinePortion *pPor = pLine->GetFirstPortion(  );
            while ( pPor )
            {
                pPor->dumpPortionAsXml( ofs, aText, writer );
                ofs += pPor->GetLen(  );
                pPor = pPor->GetPortion(  );
            }

            xmlTextWriterEndElement( writer );  // line
            pLine = pLine->GetNext(  );
        }
    }
    xmlTextWriterEndElement( writer );
}


void SwFrm::dumpAsXml( xmlTextWriterPtr writer )
{
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
        name = "txt";
        break;
    };

    if ( name != NULL )
    {
        xmlTextWriterStartElement( writer, ( const xmlChar * ) name );

        dumpAsXmlAttributes( writer );

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
}

void SwFrm::dumpAsXmlAttributes( xmlTextWriterPtr writer )
{
    xmlTextWriterWriteFormatAttribute( writer, BAD_CAST( "ptr" ), "%p", this );
    xmlTextWriterWriteFormatAttribute( writer, BAD_CAST( "next" ), "%p", GetNext() );
    xmlTextWriterWriteFormatAttribute( writer, BAD_CAST( "prev" ), "%p", GetPrev() );
    xmlTextWriterWriteFormatAttribute( writer, BAD_CAST( "upper" ), "%p", this->GetUpper() );
    xmlTextWriterWriteFormatAttribute( writer, BAD_CAST( "lower" ), "%p", this->GetLower() );
}

void SwFrm::dumpChildrenAsXml( xmlTextWriterPtr writer )
{
    SwFrm *pFrm = GetLower(  );
    for ( ; pFrm != NULL; pFrm = pFrm->GetNext(  ) )
    {
        pFrm->dumpAsXml( writer );
    }
}

void SwTxtFrm::dumpAsXmlAttributes( xmlTextWriterPtr writer )
{
    SwFrm::dumpAsXmlAttributes( writer );
    if ( HasFollow() )
        xmlTextWriterWriteFormatAttribute( writer, BAD_CAST( "follow" ), "%p", GetFollow() );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
