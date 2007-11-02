/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: prs_cpp.hxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: hr $ $Date: 2007-11-02 17:03:30 $
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

#ifndef ADC_CPP_PRS_CPP_HXX
#define ADC_CPP_PRS_CPP_HXX



// USED SERVICES
    // BASE CLASSES
#include <autodoc/prs_code.hxx>
    // COMPONENTS
    // PARAMETERS

namespace cpp
{

struct S_RunningData;

class Cpluplus_Parser : public autodoc::CodeParser_Ifc
{
  public:
                        Cpluplus_Parser();
    virtual             ~Cpluplus_Parser();


    virtual void        Setup(
                            ary::Repository &   o_rRepository,
                            const autodoc::DocumentationParser_Ifc &
                                                i_rDocumentationInterpreter );

    virtual void        Run(
                            const autodoc::FileCollector_Ifc &
                                                i_rFiles );
  private:
    Dyn<S_RunningData>  pRunningData;
};




}   // namespace cpp
#endif
