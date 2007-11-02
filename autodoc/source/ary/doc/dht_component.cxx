/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: dht_component.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: hr $ $Date: 2007-11-02 15:39:20 $
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
#include <ary/doc/ht/dht_component.hxx>


// NOT FULLY DEFINED SERVICES
#include <ary/doc/ht/dht_interpreter.hxx>
#include <ary/doc/ht/dht_processor.hxx>



namespace ary
{
namespace doc
{
namespace ht
{

class Null_Interpreter : public Interpreter
{
  public:
    virtual             ~Null_Interpreter();
  private:
    virtual void        do_Accept(
                            Processor &         io_processor,
                            const String &      i_data ) const;
};

namespace
{

static const Null_Interpreter
    C_aTheNullInterpreter_;
}


Component::Component()
    :   aData(String::Null_()),
        pInterpreter(&C_aTheNullInterpreter_)
{
}

Component::Component( const Interpreter & i_interpreter )
    :   aData(String::Null_()),         // Makes sure, no extra memory
                                        //   is used for this string.
        pInterpreter(&i_interpreter)
{
}

Component::Component( const Interpreter & i_interpreter,
                      const String &      i_data )
    :   aData(i_data),
        pInterpreter(&i_interpreter)
{
}

Component::~Component()
{
}

void
Component::Accept( Processor & io_processor ) const
{
    csv_assert(pInterpreter != 0);
    pInterpreter->Accept(io_processor, aData);
}



//*****************     Null_Interpreter    **********************//

Null_Interpreter::~Null_Interpreter()
{
}

void
Null_Interpreter::do_Accept( Processor & ,
                             const String & ) const
{
    // Does nothing.
}




}   // namespace ht
}   // namespace doc
}   // namespace ary
