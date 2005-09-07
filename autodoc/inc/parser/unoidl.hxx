/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: unoidl.hxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 16:29:37 $
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

#ifndef ADC_UNOIDL_HXX
#define ADC_UNOIDL_HXX



// USED SERVICES
    // BASE CLASSES
#include <parser/parser.hxx>
#include <parser/parserinfo.hxx>
    // COMPONENTS
    // PARAMETERS


namespace ary
{
    namespace n22
    {
        class Repository;
    }
}
namespace autodoc
{
    class FileCollector_Ifc;
}

namespace autodoc
{


class IdlParser : public ::CodeParser,
                  public ::ParserInfo
{
  public:
                        IdlParser(
                            ary::n22::Repository &  io_rRepository );

    virtual void        Run(
                            const autodoc::FileCollector_Ifc &
                                                i_rFiles );

  private:
    // DATA
    ary::n22::Repository *  pRepository;
};



// IMPLEMENTATION


}   // namespace autodoc


#endif

