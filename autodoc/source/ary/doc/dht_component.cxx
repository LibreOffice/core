/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: dht_component.cxx,v $
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
