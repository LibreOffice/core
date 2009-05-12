/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: d_hypertext.cxx,v $
 * $Revision: 1.3 $
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
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
