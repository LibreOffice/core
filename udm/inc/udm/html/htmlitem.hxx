/*************************************************************************
 *
 *  $RCSfile: htmlitem.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: np $ $Date: 2002-06-25 15:15:59 $
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

#ifndef CSI_HTML_HTMLITEM_HXX
#define CSI_HTML_HTMLITEM_HXX

// USED SERVICES
    // BASE CLASSES
#include <udm/xml/xmlitem.hxx>
    // COMPONENTS
    // PARAMETERS


#define COMPATIBLE_NETSCAPE_47

namespace csi
{
namespace html
{



/** <BODY>
*/
class Body : public csi::xml::AnElement
{
  public:
                        Body()
                                :   csi::xml::AnElement( "body" ) {}
  private:
    virtual bool        LineBreakAfterBeginTag() const;
};


#ifdef COMPATIBLE_NETSCAPE_47
/** <BR>
*/
class HorizontalLine : public xml::XmlCode
{
  public:
                        HorizontalLine()
                                :   xml::XmlCode("<hr>\n") {}
};

#else
/** <HR>
*/
class HorizontalLine : public csi::xml::AnEmptyElement
{
  public:
                        HorizontalLine()
                                :   csi::xml::AnEmptyElement("hr") {}
  private:
    virtual bool        LineBreakAfterBeginTag() const;
};
#endif


/** <IMG .... >
*/
class Image : public csi::xml::AnEmptyElement
{
  public:
                        Image(
                            const ::csv::String & i_sSrc,
                            const ::csv::String & i_sWidth,
                            const ::csv::String & i_sHeight,
                            const ::csv::String & i_sAlign,
                            const ::csv::String & i_sBorder );
};

/** <A name="">
*/
class Label : public csi::xml::AnElement
{
  public:
                        Label(
                            const ::csv::String & i_sLabel )
                                :   csi::xml::AnElement("a")
                                { *this << new csi::xml::AnAttribute(String("name"), i_sLabel); }
  private:
    virtual bool        LineBreakAfterEndTag() const;
};

/** <p>
*/
class Paragraph : public csi::xml::AnElement
{
  public:
                        Paragraph()
                                :   csi::xml::AnElement("p") {}
  private:
    virtual bool        LineBreakAfterEndTag() const;
};

/** <H1-6>
*/
class Headline : public csi::xml::APureElement
{
  public:
                        Headline(
                            int                 i_nNr )
                                :   csi::xml::APureElement(sTags[i_nNr-1]) { csv_assert(1 <= i_nNr AND i_nNr < 7); }
  private:
    static const char * sTags[6];
    virtual bool        LineBreakAfterEndTag() const;
};

#ifdef COMPATIBLE_NETSCAPE_47
/** <BR>
*/
class LineBreak : public xml::XmlCode
{
  public:
                        LineBreak()
                                :   xml::XmlCode("<br>\n") {}
};

#else
/** <BR>
*/
class LineBreak : public csi::xml::ASglTag
{
  public:
                        LineBreak()
                                :   csi::xml::ASglTag("br") {}
  private:
    virtual bool        LineBreakAfterBeginTag() const;
};
#endif


/** <b>
*/
class Bold : public csi::xml::APureElement
{
  public:
                        Bold()
                                :   csi::xml::APureElement("b") {}
};

/** <i>
*/
class Italic : public csi::xml::APureElement
{
  public:
                        Italic()
                                :   csi::xml::APureElement("i") {}
};

/** <strong>
*/
class Strong : public csi::xml::APureElement
{
  public:
                        Strong()
                                :   csi::xml::APureElement("strong") {}
};

/** <em>
*/
class Emphasized : public csi::xml::APureElement
{
  public:
                        Emphasized()
                                :   csi::xml::APureElement("em") {}
};

/** <font>
*/
class Font : public csi::xml::AnElement
{
  public:
                        Font()
                                :   csi::xml::AnElement("font") {}
};


/** <A href="">
*/
class Link : public csi::xml::AnElement
{
  public:
                        Link(
                            const ::csv::String &i_sDestinaton )
                                :   csi::xml::AnElement("a")
                                { *this << new csi::xml::AnAttribute(String("href"), i_sDestinaton); }
};

class TableCell : public csi::xml::AnElement
{
  public:
                        TableCell()
                                :   csi::xml::AnElement("td") {}
  private:
    virtual bool        LineBreakAfterEndTag() const;
};

class TableRow  : public csi::xml::AnElement
{
  public:
                        TableRow()
                                :   csi::xml::AnElement("tr") {}

    TableCell &         AddCell(
                            DYN csi::xml::Item* let_dpItem = 0 );
  private:
    virtual bool        LineBreakAfterBeginTag() const;
};

/** <table ....>
*/
class Table : public csi::xml::AnElement
{
  public:
                        Table()
                                :   csi::xml::AnElement("table") {}
                        Table(
                            const ::csv::String & i_sBorder,
                            const ::csv::String & i_sWidth,
                            const ::csv::String & i_sCellPadding,
                            const ::csv::String & i_sCellSpacing  );
    TableRow &          AddRow();
  private:
    virtual bool        FinishEmptyTag_XmlStyle() const;
    virtual bool        LineBreakAfterBeginTag() const;
};



class DefListTerm : public csi::xml::AnElement
{
  public:
                        DefListTerm()
                                :   csi::xml::AnElement("dt") {}
  private:
    virtual bool        LineBreakAfterEndTag() const;
};

class DefListDefinition : public csi::xml::AnElement
{
  public:
                        DefListDefinition()
                                :   csi::xml::AnElement("dd") {}
  private:
    virtual bool        LineBreakAfterEndTag() const;
};

/** <DL>
*/
class DefList : public csi::xml::AnElement
{
  public:
                        DefList()
                                :   csi::xml::AnElement("dl") {}

    DefListTerm &       AddTerm(
                            DYN csi::xml::Item* let_dpItem = 0 );
    DefListDefinition & AddDefinition(
                            DYN csi::xml::Item* let_dpItem = 0 );
  private:
    virtual bool        LineBreakAfterBeginTag() const;
};


class ListItem : public csi::xml::AnElement
{
  public:
                        ListItem()
                                :   csi::xml::AnElement("li") {}
  private:
    virtual bool        LineBreakAfterEndTag() const;
};

/** <OL>
*/
class NumeratedList : public csi::xml::AnElement
{
  public:
                        NumeratedList()
                                :   csi::xml::AnElement("ol") {}

    ListItem &          AddItem(
                            DYN csi::xml::Item* let_dpItem = 0 );
  private:
    virtual bool        LineBreakAfterBeginTag() const;
};

/** <UL>
*/
class SimpleList : public csi::xml::AnElement
{
  public:
                        SimpleList()
                                :   csi::xml::AnElement("ul") {}

    ListItem &          AddItem(
                            DYN csi::xml::Item* let_dpItem = 0 );
  private:
    virtual bool        LineBreakAfterBeginTag() const;
};


class ClassAttr : public csi::xml::AnAttribute
{
  public:
                        ClassAttr(
                            const ::csv::String & i_sValue )
                            :   csi::xml::AnAttribute( String("class"), i_sValue ) {}
};

class StyleAttr : public csi::xml::AnAttribute
{
  public:
                        StyleAttr(
                            const ::csv::String & i_sValue )
                            :   csi::xml::AnAttribute( String("style"), i_sValue ) {}
};

class SizeAttr : public csi::xml::AnAttribute
{
  public:
                        SizeAttr(
                            const ::csv::String & i_sValue )
                            :   csi::xml::AnAttribute( String("size"), i_sValue ) {}
};

class ColorAttr : public csi::xml::AnAttribute
{
  public:
                        ColorAttr(
                            const ::csv::String & i_sValue )
                            :   csi::xml::AnAttribute( String("color"), i_sValue ) {}
};

class BgColorAttr : public csi::xml::AnAttribute
{
  public:
                        BgColorAttr(
                            const ::csv::String & i_sValue )
                            :   csi::xml::AnAttribute( String("bgcolor"), i_sValue ) {}
};

class AlignAttr : public csi::xml::AnAttribute
{
  public:
                        AlignAttr(
                            const ::csv::String & i_sValue )
                            :   csi::xml::AnAttribute( String("align"), i_sValue ) {}
};

class WidthAttr : public csi::xml::AnAttribute
{
  public:
                        WidthAttr(
                            const ::csv::String & i_sValue )
                            :   csi::xml::AnAttribute( String("width"), i_sValue ) {}
};



class Sbr : public csi::xml::XmlCode  // Sourcefile lineBreak
{
  public:
                        Sbr()
                            :   csi::xml::XmlCode("\n") {}
};


}   // namespace html
}   // namespace csi

namespace Html = csi::html;

#endif


