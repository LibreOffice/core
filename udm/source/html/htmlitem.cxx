/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: htmlitem.cxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: vg $ $Date: 2007-09-18 14:29:36 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#include <precomp.h>
#include <udm/html/htmlitem.hxx>

// NOT FULLY DECLARED SERVICES


namespace csi
{
namespace html
{

using namespace csi::xml;

template <class ELEM>
inline ELEM &
PushElem( Element &     i_rMain,
          DYN ELEM *    let_dpSub,
          DYN Item *    let_dpItem )
{
    i_rMain << let_dpSub;
    if ( let_dpItem != 0 )
        *let_dpSub << let_dpItem;
    return *let_dpSub;
}


bool
Body::LineBreakAfterBeginTag() const
{
     return true;
}

#ifndef COMPATIBLE_NETSCAPE_47
bool
HorizontalLine::LineBreakAfterBeginTag() const
{
     return true;
}
#endif


Image::Image( const String &   i_sSrc,
              const String &   i_sWidth,
              const String &   i_sHeight,
              const String &   i_sAlign,
              const String &   i_sBorder )
    :   AnEmptyElement( "img" )
{
    *this << new AnAttribute(String("src"),i_sSrc)
          << new AnAttribute(String("width"),i_sWidth)
          << new AnAttribute(String("height"),i_sHeight)
          << new AnAttribute(String("align"),i_sAlign)
          << new AnAttribute(String("border"),i_sBorder);
}

bool
Paragraph::LineBreakAfterEndTag() const
{
     return true;
}

const char *
Headline::sTags[6] = { "h1", "h2", "h3", "h4", "h5", "h6" };

bool
Headline::LineBreakAfterEndTag() const
{
     return true;
}

#ifndef COMPATIBLE_NETSCAPE_47
bool
LineBreak::LineBreakAfterBeginTag() const
{
     return true;
}
#endif


bool
TableCell::LineBreakAfterEndTag() const
{
     return true;
}



TableCell &
TableRow::AddCell( DYN Item * let_dpItem )
{
    return PushElem( *this, new TableCell, let_dpItem );
}

bool
TableRow::LineBreakAfterBeginTag() const
{
     return true;
}


Table::Table( const String &   i_sBorder,
              const String &   i_sWidth,
              const String &   i_sCellPadding,
              const String &   i_sCellSpacing  )
    :   csi::xml::AnElement("table")
{
    if ( i_sBorder.length() > 0 )
        *this << new AnAttribute(String("border"),i_sBorder);
    if ( i_sBorder.length() > 0 )
        *this << new AnAttribute(String("width"),i_sWidth);
    if ( i_sBorder.length() > 0 )
        *this << new AnAttribute(String("cellpadding"),i_sCellPadding);
    if ( i_sBorder.length() > 0 )
        *this << new AnAttribute(String("cellspacing"),i_sCellSpacing);
}

TableRow &
Table::AddRow()
{
    TableRow * ret = new TableRow;
    *this << ret;
    return *ret;
}

bool
Table::FinishEmptyTag_XmlStyle() const
{
     return false;
}

bool
Table::LineBreakAfterBeginTag() const
{
     return true;
}



bool
DefListTerm::LineBreakAfterEndTag() const
{
     return true;
}

bool
DefListDefinition::LineBreakAfterEndTag() const
{
     return true;
}





DefListTerm &
DefList::AddTerm( DYN csi::xml::Item* let_dpItem )
{
    return PushElem( *this, new DefListTerm, let_dpItem );
}

DefListDefinition &
DefList::AddDefinition( DYN csi::xml::Item* let_dpItem )
{
    return PushElem( *this, new DefListDefinition, let_dpItem );
}

bool
DefList::LineBreakAfterBeginTag() const
{
     return true;
}

bool
DefList::FinishEmptyTag_XmlStyle() const
{
     return false;
}

bool
ListItem::LineBreakAfterEndTag() const
{
     return true;
}




ListItem &
NumeratedList::AddItem( DYN csi::xml::Item* let_dpItem )
{
    return PushElem( *this, new ListItem, let_dpItem );
}

bool
NumeratedList::LineBreakAfterBeginTag() const
{
     return true;
}


ListItem &
SimpleList::AddItem( DYN csi::xml::Item* let_dpItem )
{
    return PushElem( *this, new ListItem, let_dpItem );
}

bool
SimpleList::LineBreakAfterBeginTag() const
{
     return true;
}



}   // namespace html
}   // namespace csi
