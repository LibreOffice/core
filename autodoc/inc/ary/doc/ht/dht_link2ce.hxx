/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: dht_link2ce.hxx,v $
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

#ifndef ARY_DOC_DHT_LINK2CE_HXX
#define ARY_DOC_DHT_LINK2CE_HXX

// BASE CLASSES
#include <ary/doc/ht/dht_interpreter.hxx>




namespace ary
{
namespace doc
{
namespace ht
{
    class Component;


/** Link to an ->::ary::Entity.
*/
class Link2Ce : public Interpreter
{
  public:
    virtual             ~Link2Ce();

    static Component    Create_Component_(
                            const String &      i_ceAbsolutePath,
                            const String &      i_display );
    static void         Resolve_(
                            String &            o_absolutePath,
                            String &            o_display,
                            const String &      i_data);
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
