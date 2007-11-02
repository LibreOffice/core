/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: dht_component.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: hr $ $Date: 2007-11-02 15:01:54 $
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
