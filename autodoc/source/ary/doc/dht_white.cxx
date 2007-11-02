/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: dht_white.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: hr $ $Date: 2007-11-02 15:41:48 $
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
