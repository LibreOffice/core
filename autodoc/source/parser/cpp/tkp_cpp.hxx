/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: tkp_cpp.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: hr $ $Date: 2007-11-02 17:02:06 $
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

#ifndef ADC_TKP_CPP_HXX
#define ADC_TKP_CPP_HXX

// USED SERVICES
    // BASE CLASSES
#include <tokens/tkp.hxx>
    // COMPONENTS
    // PARAMETRS

namespace autodoc
{
    class TkpDocuContext;
}

namespace csv
{
    namespace ploc
    {
        class Path;
        class DirectoryChain;
    }
}


namespace cpp {

class Context_CppStd;
class DefineDescription;
class Distributor;


/** This is a TokenParser which is able to parse tokens from
    C++ source code.
*/
class TokenParser_Cpp : public TokenParser
{
  public:
    typedef std::map< String, DefineDescription* >     MacroMap;

    // LIFECYCLE
                        TokenParser_Cpp(
                            DYN autodoc::TkpDocuContext &
                                                let_drDocuContext );
    virtual             ~TokenParser_Cpp();

    // OPERATIONS
    void                AssignPartners(
                            CharacterSource &   io_rCharacterSource,
                            cpp::Distributor &  o_rDealer );
    void                StartNewFile(
                            const csv::ploc::Path &
                                                i_file );
  private:
    virtual void        SetStartContext();
    virtual void        SetCurrentContext(
                            TkpContext &        io_rContext );
    virtual TkpContext &
                        CurrentContext();
    // DATA
    Dyn<Context_CppStd> pBaseContext;
    TkpContext *        pCurContext;
    Distributor *       pDealer;
    CharacterSource *   pCharacterSource;
};


}   // namespace cpp


#endif


