/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: c_macro.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 15:59:49 $
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

#ifndef ARY_CPP_C_MACRO_HXX
#define ARY_CPP_C_MACRO_HXX


// USED SERVICES
    // BASE CLASSES
#include <ary/cpp/cpp_defs.hxx>
    // COMPONENTS
    // PARAMETERS



namespace ary
{

namespace cpp
{



/** Describes a C/C++ #define MACRO(a,b, ...) statement.
*/
class Macro : public CppDefinition
{
  public:
                        Macro();
                        Macro(      /// Used for: #define DEFINE xyz
                            Did                 i_nId,
                            const udmstri &     i_sName,
                            const str_vector &  i_rParams,
                            const str_vector &  i_rDefinition,
                            Lid                 i_nDeclaringFile );
                        ~Macro();
    // INQUIRY
    void                GetText(
                            csv::StreamStr &    o_rText,
                            const StringVector &
                                                i_rGivenArguments ) const;
    const str_vector &  Params() const          { return aParams; }

  private:
    // Interface RepositoryEntity:
    virtual void        do_StoreAt(
                            ary::Display &      o_rOut ) const;
    virtual RCid        inq_RC() const;

    // Interface CppDefinition:
    virtual const str_vector &
                        inq_DefinitionText() const;

    // DATA
    str_vector          aParams;
    str_vector          aDefinition;
};




}   // namespace cpp
}   // namespace ary


#endif

