/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: cpp_pe.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: hr $ $Date: 2007-11-02 16:49:07 $
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

#ifndef ADC_CPP_CPP_PE_HXX
#define ADC_CPP_CPP_PE_HXX



// USED SERVICES
    // BASE CLASSES
#include <semantic/parseenv.hxx>
#include "tokintpr.hxx"
    // COMPONENTS
#include "pev.hxx"
    // PARAMETERS
#include <ary/cpp/c_types4cpp.hxx>


namespace cpp {

class Cpp_PE : public ::ParseEnvironment,
               public TokenInterpreter
{
  public:
    typedef cpp::PeEnvironment  EnvData;

    void                SetTokenResult(
                            E_TokenDone         i_eDone,
                            E_EnvStackAction    i_eWhat2DoWithEnvStack,
                            ParseEnvironment *  i_pParseEnv2Push = 0 );

    virtual Cpp_PE *    Handle_ChildFailure();  // Defaulted to 0.

  protected:
                        Cpp_PE(
                            Cpp_PE *            io_pParent );
                        Cpp_PE(
                            EnvData &           i_rEnv );

    EnvData &           Env() const;

    void                StdHandlingOfSyntaxError(
                            const char *        i_sText );

  private:
    // DATA
    EnvData &           rMyEnv;
};

inline  Cpp_PE::EnvData &
Cpp_PE::Env() const
    { return rMyEnv; }

}   // namespace cpp

#endif

