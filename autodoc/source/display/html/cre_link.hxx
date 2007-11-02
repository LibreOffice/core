/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: cre_link.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: hr $ $Date: 2007-11-02 16:24:37 $
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

#ifndef ADC_DISPLAY_CRE_LINK_HXX
#define ADC_DISPLAY_CRE_LINK_HXX


// USED SERVICES
    // BASE CLASSES
#include <cosv/tpl/processor.hxx>
    // COMPONENTS
    // PARAMETERS

namespace ary
{
namespace cpp
{
     class CodeEntity;
    class Namespace;
    class Class;
    class Enum;
    class Typedef;
    class Function;
    class Variable;
    class EnumValue;
    class Define;
    class Macro;
}
}


class OuputPage_Environment;



/** Displays links to ->{ary::cpp::CodeEntity CodeEntites}.
*/
class LinkCreator : public csv::ProcessorIfc,
                    public csv::ConstProcessor<ary::cpp::Namespace>,
                    public csv::ConstProcessor<ary::cpp::Class>,
                    public csv::ConstProcessor<ary::cpp::Enum>,
                    public csv::ConstProcessor<ary::cpp::Typedef>,
                    public csv::ConstProcessor<ary::cpp::Function>,
                    public csv::ConstProcessor<ary::cpp::Variable>,
                    public csv::ConstProcessor<ary::cpp::EnumValue>,
                    public csv::ConstProcessor<ary::cpp::Define>,
                    public csv::ConstProcessor<ary::cpp::Macro>
{
  public:
                        LinkCreator(
                            char *              o_rOutput,
                            uintt               i_nOutputSize );
                        ~LinkCreator();


    void                SetEnv(
                            const OuputPage_Environment &
                                                i_rEnv );
  private:
    void                Create_PrePath(
                            const ary::cpp::CodeEntity &
                                                i_rData );
    // Interface csv::ConstProcessor<>
    virtual void        do_Process(
                            const ary::cpp::Namespace &
                                                i_rData );
    virtual void        do_Process(
                            const ary::cpp::Class &
                                                i_rData );
    virtual void        do_Process(
                            const ary::cpp::Enum &
                                                i_rData );
    virtual void        do_Process(
                            const ary::cpp::Typedef &
                                                i_rData );
    virtual void        do_Process(
                            const ary::cpp::Function &
                                                i_rData );
    virtual void        do_Process(
                            const ary::cpp::Variable &
                                                i_rData );
    virtual void        do_Process(
                            const ary::cpp::EnumValue &
                                                i_rData );
    virtual void        do_Process(
                            const ary::cpp::Define &
                                                i_rData );
    virtual void        do_Process(
                            const ary::cpp::Macro &
                                                i_rData );
    // DATA
    char *              pOut;
    uintt               nOutMaxSize;
    const OuputPage_Environment *
                        pEnv;
};




// IMPLEMENTATION
inline void
LinkCreator::SetEnv( const OuputPage_Environment & i_rEnv )
    { pEnv = &i_rEnv; }




#endif
