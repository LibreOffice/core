/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 * 
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
