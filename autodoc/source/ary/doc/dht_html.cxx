/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: dht_html.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: hr $ $Date: 2007-11-02 15:40:13 $
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
#include <ary/doc/ht/dht_html.hxx>


// NOT FULLY DEFINED SERVICES
#include <ary/doc/ht/dht_component.hxx>
#include <ary/doc/ht/dht_processor.hxx>


namespace ary
{
namespace doc
{
namespace ht
{



HtmlText::~HtmlText()
{
}

Component
HtmlText::Create_Component_( const String & i_text )
{
    static const HtmlText aTheInstance_;
    return Component(aTheInstance_, i_text);
}

void
HtmlText::do_Accept( Processor &         io_processor,
                    const String &      i_data ) const
{
    io_processor.Process(*this, i_data);
}



}   // namespace ht
}   // namespace doc
}   // namespace ary
