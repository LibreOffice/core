/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: dht_component.hxx,v $
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

#ifndef ARY_DHT_COMPONENT_HXX
#define ARY_DHT_COMPONENT_HXX




namespace ary
{
namespace doc
{
namespace ht
{
    class Interpreter;
    class Processor;


/** Interface for components of a ->HyperText.

    This may be a text, whitespace or link etc.
*/
class Component
{
  // COPYABLE
  public:
                        Component();
    explicit            Component(
                            const Interpreter & i_interpreter );
                        Component(
                            const Interpreter & i_interpreter,
                            const String &      i_data );
                        ~Component();

    void                Accept(
                            Processor &         io_processor ) const;

    const Interpreter & AssociatedInterpreter() const;
    const String &      Data() const;

  private:
    // DATA
    String              aData;
    const Interpreter * pInterpreter;
};




// IMPLEMENTATION

inline const Interpreter &
Component::AssociatedInterpreter() const
{
    csv_assert(pInterpreter != 0);
    return *pInterpreter;
}

inline const String &
Component::Data() const
{
    return aData;
}




}   // namespace ht
}   // namespace doc
}   // namespace ary
#endif
