/*************************************************************************
 *
 *  $RCSfile: xmlitem.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: np $ $Date: 2002-06-25 15:16:00 $
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
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#include <precomp.h>
#include <udm/xml/xmlitem.hxx>

// NOT FULLY DECLARED SERVICES
#include <cosv/file.hxx>


namespace csi
{
namespace xml
{

char cReplacable[256] =
    {
      0,  0,  0,  0,  0,    0,  0,  0,  0,  0,  // 0 - 49
      0,  0,  0,  0,  0,    0,  0,  0,  0,  0,
      0,  0,  0,  0,  0,    0,  0,  0,  0,  0,
      0,  0,  0,  0,  1,    0,  0,  0,  1,  0,      // ", &
      0,  0,  0,  0,  0,    0,  0,  0,  0,  0,

      0,  0,  0,  0,  0,    0,  0,  0,  0,  0,  // 50 - 99
      1,  0,  1,  0,  0,    0,  0,  0,  0,  0,      // <, >
      0,  0,  0,  0,  0,    0,  0,  0,  0,  0,
      0,  0,  0,  0,  0,    0,  0,  0,  0,  0,
      0,  0,  0,  0,  0,    0,  0,  0,  0,  0,

      0,  0,  0,  0,  0,    0,  0,  0,  0,  0,  // 100 - 149
      0,  0,  0,  0,  0,    0,  0,  0,  0,  0,
      0,  0,  0,  0,  0,    0,  0,  0,  0,  0,
      0,  0,  0,  0,  0,    0,  0,  0,  0,  0,
      0,  0,  0,  0,  0,    0,  0,  0,  0,  0,

      0,  0,  0,  0,  0,    0,  0,  0,  0,  0,
      0,  0,  0,  0,  0,    0,  0,  0,  0,  0,
      0,  0,  0,  0,  0,    0,  0,  0,  0,  0,
      0,  0,  0,  0,  0,    0,  0,  0,  0,  0,
      0,  0,  0,  0,  0,    0,  0,  0,  0,  0,

      0,  0,  0,  0,  0,    0,  0,  0,  0,  0,
      0,  0,  0,  0,  0,    0,  0,  0,  0,  0,
      0,  0,  0,  0,  0,    0,  0,  0,  0,  0,
      0,  0,  0,  0,  0,    0,  0,  0,  0,  0,
      0,  0,  0,  0,  0,    0,  0,  0,  0,  0,

      0,  0,  0,  0,  0,    1                   // &nbsp;
    };


class MultiItem : public Item
{
  public:
                        MultiItem();
                        ~MultiItem();

    void                Add(
                            DYN Item *          let_dpDatum )
                                                    { csv_assert( let_dpDatum != 0 );
                                                      aItems.push_back(let_dpDatum); }
    void                Erase()                     { aItems.erase_all(); }

  private:
    virtual void        do_WriteOut(
                            csv::bostream &         io_aFile ) const;
    // DATA
    ItemList            aItems;
};


void                    StreamOut(
                            Dyn< Item >  &          o_rContent,
                            DYN Item *              let_dpItem );
inline void
StreamOut( AttrList &               o_rAttrs,
           DYN Attribute *          let_dpAttr )
{
    csv_assert( let_dpAttr != 0 );
    o_rAttrs.push_back( let_dpAttr );
}


inline void
Impl_SetContent( Dyn< Item >  &         o_rContent,
                 DYN Item *             let_dpItem )
{
    o_rContent = let_dpItem;
}


//*********************     Attribute       ****************************//

const String   attrValueBegin("=\"");
const String   attrValueEnd("\"");

void
Attribute::WriteOut( csv::bostream & io_aFile ) const
{
    io_aFile.write( Name() );
    if ( Value().length() > 0 )
    {
        io_aFile.write( attrValueBegin );
        io_aFile.write( Value() );
        io_aFile.write( attrValueEnd );
    }
}



//************************      Element     ****************************//

const String    newline("\n");
const String    space(" ");
const String    beginTagBegin("<");
const String    endTagBegin("</");
const String    tagEnd(">");
const String    emptyTagEnd("/>");

void
Element::do_WriteOut( csv::bostream & io_aFile ) const
{
    io_aFile.write( beginTagBegin );
    io_aFile.write( inq_TagName() );

    const AttrList * pAttrs = inq_Attrs();
    if ( pAttrs != 0 )
    {
        for ( AttrList::iterator it = pAttrs->begin();
              it != pAttrs->end();
              ++it )
        {

            io_aFile.write( space );
            (*it)->WriteOut( io_aFile );
        }
    }

    const Item * pContent = inq_Content();
    if ( pContent != 0 )
        io_aFile.write( tagEnd );
    else
    {
         if (FinishEmptyTag_XmlStyle())
            io_aFile.write( emptyTagEnd );
        else
        {
            io_aFile.write( tagEnd );
            io_aFile.write( endTagBegin );
            io_aFile.write( inq_TagName() );
            io_aFile.write( tagEnd );
        }
    }
    if ( LineBreakAfterBeginTag() )
        io_aFile.write( newline );
    if ( pContent == 0 )
        return;

    pContent->WriteOut( io_aFile );
    io_aFile.write( endTagBegin );
    io_aFile.write( inq_TagName() );
    io_aFile.write( tagEnd );
    if ( LineBreakAfterEndTag() )
        io_aFile.write( newline );
}

bool
Element::FinishEmptyTag_XmlStyle() const
{
     return true;
}

bool
Element::LineBreakAfterBeginTag() const
{
    return false;
}

bool
Element::LineBreakAfterEndTag() const
{
    return LineBreakAfterBeginTag();
}


//************************      EmptyElement        ****************************//

void
EmptyElement::op_streamout( DYN Item *  )
{
    // Does nothing.
}

void
EmptyElement::op_streamout( DYN Attribute * let_dpAttr )
{
    StreamOut( inq_RefAttrs(), let_dpAttr );
}

void
EmptyElement::do_SetContent( DYN Item * )
{
    // Does nothing.
}

const Item *
EmptyElement::inq_Content() const
{
    return 0;
}

const AttrList *
EmptyElement::inq_Attrs() const
{
    return &inq_RefAttrs();
}


//************************      PureElement     ****************************//

void
PureElement::op_streamout( DYN Item * let_dpItem )
{
    StreamOut( inq_RefContent(), let_dpItem );
}

void
PureElement::op_streamout( DYN Attribute * )
{
    // Does nothing.
}

void
PureElement::do_SetContent( DYN Item *  let_dpItem )
{
    Impl_SetContent( inq_RefContent(), let_dpItem );
}

const Item *
PureElement::inq_Content() const
{
    return inq_RefContent().Ptr();
}

const AttrList *
PureElement::inq_Attrs() const
{
    return 0;
}


//***************************       SglTag      **************************//

void
SglTag::op_streamout( DYN Item * )
{
    // Does nothing.
}

void
SglTag::op_streamout( DYN Attribute * )
{
    // Does nothing.
}

void
SglTag::do_SetContent( DYN Item *)
{
    // Does nothing.
}

const Item *
SglTag::inq_Content() const
{
    return 0;
}

const AttrList *
SglTag::inq_Attrs() const
{
    return 0;
}


//***************************       AnElement       **************************//

AnElement::AnElement( const String &   i_sTagName )
    :   sTagName( i_sTagName )
        // pContent,
        // aAttrs
{
}

AnElement::AnElement( const char * i_sTagName )
    :   sTagName( i_sTagName )
        // pContent,
        // aAttrs
{
}

AnElement::~AnElement()
{
}

void
AnElement::op_streamout( DYN Item * let_dpItem )
{
    StreamOut( pContent, let_dpItem );
}

void
AnElement::op_streamout( DYN Attribute * let_dpAttr )
{
    StreamOut( aAttrs, let_dpAttr );
}

void
AnElement::do_SetContent( DYN Item * let_dpItem )
{
    Impl_SetContent( pContent, let_dpItem );
}

const String &
AnElement::inq_TagName() const
{
    return sTagName;
}

const Item *
AnElement::inq_Content() const
{
    return pContent.Ptr();
}

const AttrList *
AnElement::inq_Attrs() const
{
    return &aAttrs;
}


//***************************     AnEmptyElement    **************************//

AnEmptyElement::AnEmptyElement( const String &   i_sTagName )
    :   sTagName( i_sTagName )
        // aAttrs
{
}

AnEmptyElement::AnEmptyElement( const char * i_sTagName )
    :   sTagName( i_sTagName )
        // aAttrs
{
}

AnEmptyElement::~AnEmptyElement()
{

}

const String &
AnEmptyElement::inq_TagName() const
{
    return sTagName;
}

AttrList &
AnEmptyElement::inq_RefAttrs()
{
    return aAttrs;
}


//***************************     APureElement      **************************//

APureElement::APureElement( const String &   i_sTagName )
    :   sTagName( i_sTagName )
        // pContent
{
}

APureElement::APureElement( const char * i_sTagName )
    :   sTagName( i_sTagName )
        // pContent
{
}

APureElement::~APureElement()
{
}

const String &
APureElement::inq_TagName() const
{
    return sTagName;
}

Dyn< Item > &
APureElement::inq_RefContent()
{
    return pContent;
}



//***************************     ASglTag           **************************//

ASglTag::ASglTag( const String &   i_sTagName )
    :   sTagName( i_sTagName )
{
}

ASglTag::ASglTag( const char *  i_sTagName )
    :   sTagName( i_sTagName )
{
}

ASglTag::~ASglTag()
{
}

const String &
ASglTag::inq_TagName() const
{
    return sTagName;
}


//***************************       AnAttribute     **************************//
AnAttribute::AnAttribute( const String &   i_sName,
                          const String &   i_sValue )
    :   sName(i_sName),
        sValue(i_sValue)
{
}

AnAttribute::AnAttribute( const char * i_sName,
                          const char * i_sValue )
    :   sName(i_sName),
        sValue(i_sValue)
{
}

AnAttribute::~AnAttribute()
{
}

const String &
AnAttribute::inq_Name() const
{
    return sName;
}

const String &
AnAttribute::inq_Value() const
{
    return sValue;
}



//***************************       Text        **************************//

Text::Text( const String &   i_sText )
    :   sText(i_sText)
{
}

Text::Text( const char * i_sText )
    :   sText(i_sText)
{
}

Text::~Text()
{
}

void
Text::do_WriteOut( csv::bostream & io_aFile ) const
{
    const char * pStart = sText.c_str();
    const char * pOut = 0;

    for ( pOut = pStart ; *pOut != '\0'; ++pOut )
    {
        if ( cReplacable[*pOut] )
        {
            if ( pOut != pStart )
            {
                io_aFile.write( pStart, pOut-pStart );
            }

            switch (UINT8(*pOut))
            {
                case '<':   io_aFile.write("&lt;");     break;
                case '>':   io_aFile.write("&gt;");     break;
                case '"':   io_aFile.write("&quot;");   break;
                case '&':   io_aFile.write("&amp;");        break;
                case 255:   io_aFile.write("&nbsp;");   break;
            }

            pStart = pOut+1;
        }   // endif (cReplacable[*pOut])
    }   // end for

    if ( pOut != pStart )
    {
        io_aFile.write( pStart, pOut-pStart );
    }
}


//***************************       XmlCode     **************************//

XmlCode::XmlCode( const String &   i_sText )
    :   sText(i_sText)
{
}

XmlCode::XmlCode( const char *     i_sText )
    :   sText(i_sText)
{
}

XmlCode::~XmlCode()
{
}

void
XmlCode::do_WriteOut( csv::bostream & io_aFile ) const
{
    io_aFile.write(sText);
}


//***************************       MultiItem       **************************//

MultiItem::MultiItem()
{
}

MultiItem::~MultiItem()
{
}

void
MultiItem::do_WriteOut( csv::bostream & io_aFile ) const
{
    ItemList::iterator itEnd = aItems.end();

    for ( ItemList::iterator it = aItems.begin();
          it != itEnd;
          ++it )
    {
        (*it)->WriteOut( io_aFile );
    }

}



//***************************       Helpers     **************************//

void
StreamOut( Dyn< Item >  &           o_rContent,
           DYN Item *               let_dpItem )
{
    MultiItem * pContent = 0;
    if ( bool(o_rContent) )
    {
        pContent = static_cast< MultiItem* >( o_rContent.MutablePtr() );
        csv_assert( dynamic_cast< MultiItem* >( o_rContent.MutablePtr() ) != 0 );
    }
    else
    {
        pContent = new MultiItem;
        o_rContent = pContent;
    }

    csv_assert( let_dpItem != 0 );
    pContent->Add( let_dpItem );
}




}   // namespace xml
}   // namespace csi



