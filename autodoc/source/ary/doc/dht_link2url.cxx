/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: dht_link2url.cxx,v $
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
#include <ary/doc/ht/dht_link2url.hxx>


// NOT FULLY DEFINED SERVICES
#include <ary/doc/ht/dht_component.hxx>
#include <ary/doc/ht/dht_processor.hxx>
#include "dht_helper.hxx"


namespace ary
{
namespace doc
{
namespace ht
{


const char  C_cSeparator = ' ';


Link2Url::~Link2Url()
{
}

Component
Link2Url::Create_Component_( const String &      i_url,
                             const String &      i_display )
{
    static const Link2Url
        aTheInstance_;
    return Create_Component(
                aTheInstance_,
                i_url,
                C_cSeparator,
                i_display );
}

void
Link2Url::Resolve_( String &            o_url,
                    String &            o_display,
                    const String &      i_data)
{
    Resolve_ComponentData(  o_url,
                            o_display,
                            C_cSeparator,
                            i_data );
}

void
Link2Url::do_Accept( Processor &         io_processor,
                     const String &      i_data ) const
{
    io_processor.Process(*this, i_data);
}



}   // namespace ht
}   // namespace doc
}   // namespace ary
