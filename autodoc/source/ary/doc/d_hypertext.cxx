/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: d_hypertext.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: hr $ $Date: 2007-11-02 15:37:20 $
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
#include <ary/doc/d_hypertext.hxx>


// NOT FULLY DEFINED SERVICES
#include <cosv/tpl/funcall.hxx>
#include <ary/doc/ht/dht_component.hxx>
#include <ary/doc/ht/dht_processor.hxx>


namespace ary
{
namespace doc
{



void
Accept( const HyperText &   i_text,
        csv::ProcessorIfc & io_processor )
{
    csv::CheckedCall(io_processor, i_text);
}


namespace ht
{

void
Processor::do_Process( const HyperText & i_client )
{
    csv::for_each_in( i_client,
                      csv::make_func(&Component::Accept, *this) );
}


}   // namespace ht




const HyperText &
NullText()
{
    static const HyperText aNullText_;
    return aNullText_;
}



}   // namespace doc
}   // namespace ary
