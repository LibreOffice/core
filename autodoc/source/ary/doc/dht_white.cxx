/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: dht_white.cxx,v $
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
#include <ary/doc/ht/dht_white.hxx>


// NOT FULLY DEFINED SERVICES
#include <ary/doc/ht/dht_component.hxx>
#include <ary/doc/ht/dht_processor.hxx>


namespace ary
{
namespace doc
{
namespace ht
{

const String
    C_sMore(" (...)");

//**********************        Blank       *********************//

Blank::~Blank()
{
}

Component
Blank::Create_Component_()
{
    static const Blank aTheInstance_;
    return Component(aTheInstance_);
}

void
Blank::do_Accept( Processor &         io_processor,
                 const String &      ) const
{
    io_processor.Process(*this);
}





//********************        NewParagraph       ********************//

NewParagraph::~NewParagraph()
{
}

Component
NewParagraph::Create_Component_()
{
    static const NewParagraph aTheInstance_;
    return Component(aTheInstance_);
}

void
NewParagraph::do_Accept( Processor &         io_processor,
                         const String &      ) const
{
    io_processor.Process(*this);
}


//**************        Indentation       ****************//

Indentation::~Indentation()
{
}

Component
Indentation::Create_Component_(uintt i_size)
{
    static const Indentation     aTheInstance_;
    StreamLock sl(100);
    sl() << i_size;
    return Component( aTheInstance_,
                      String(sl().c_str()) );
}

void
Indentation::do_Accept( Processor &         io_processor,
                              const String &      i_data ) const
{
    io_processor.Process(*this, i_data);
}



//**********************        SummarySeparator       *********************//

SummarySeparator::~SummarySeparator()
{
}

Component
SummarySeparator::Create_Component_(bool i_interrupted)
{
    static const SummarySeparator aTheInstance_;
    return Component( aTheInstance_,
                      i_interrupted
                        ?   C_sMore
                        :   String::Null_() );
}

void
SummarySeparator::do_Accept( Processor &         io_processor,
                             const String &      ) const
{
    io_processor.Process(*this);
}





}   // namespace ht
}   // namespace doc
}   // namespace ary
