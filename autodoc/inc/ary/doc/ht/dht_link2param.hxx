/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: dht_link2param.hxx,v $
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

#ifndef ARY_DHT_LINK2PARAM_HXX
#define ARY_DHT_LINK2PARAM_HXX

// BASE CLASSES
#include <ary/doc/ht/dht_interpreter.hxx>

// USED SERVICES




namespace ary
{
namespace doc
{
namespace ht
{
    class Component;


/** Interpretes a link to a parameter of the just described function.
*/
class Link2Param : public Interpreter
{
  public:
    virtual             ~Link2Param();

    static Component    Create_Component_(
                            const String &      i_parameterName,
                            const String &      i_display );
  private:
    // Interface Interpreter:
    virtual void        do_Accept(
                            Processor &         io_processor,
                            const String &      i_data ) const;
};




}   // namespace ht
}   // namespace doc
}   // namespace ary
#endif
