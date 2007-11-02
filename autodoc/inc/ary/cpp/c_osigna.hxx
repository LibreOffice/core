/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: c_osigna.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: hr $ $Date: 2007-11-02 14:50:56 $
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

#ifndef ARY_CPP_C_OSIGNA_HXX
#define ARY_CPP_C_OSIGNA_HXX

// USED SERVICES
    // BASE CLASSES
    // OTHER
#include <ary/cpp/c_types4cpp.hxx>

namespace ary
{
namespace cpp
{
    class Gate;
}
}



namespace ary
{
namespace cpp
{


/** The signature of a C++ function. That is: parameter types and
    const/volatile modifiers.
*/
class OperationSignature
{
  public:
    typedef std::vector<Type_id>    ParameterTypeList;

                        OperationSignature();
                        OperationSignature(
                            ParameterTypeList   i_parameterTypes,  // Non const, because it will be swapped with aParameterTypes.
                            E_ConVol            i_conVol );

    bool                operator==(
                            const OperationSignature &
                                                i_rSig ) const;
    bool                operator<(
                            const OperationSignature &
                                                i_rSig ) const;

    // INQUIRY
    const ParameterTypeList &
                        Parameters() const;
    E_ConVol            ConVol() const;

    /** Compares the signatures by length an then by ids of
        parameter types. So the result is not always human
        reconstructable.
        @return like in strcmp().
    */
    int                 Compare(
                            const OperationSignature &
                                                i_rSig ) const;
  private:
    // DATA
    ParameterTypeList   aParameterTypes;
    E_ConVol            eConVol;
};




// IMPLEMENTATION
inline bool
OperationSignature::operator==( const OperationSignature & i_rSign ) const
{
    return Compare(i_rSign) == 0;
}

inline bool
OperationSignature::operator<( const OperationSignature & i_rSign ) const
{
    return Compare(i_rSign) < 0;
}

inline const OperationSignature::ParameterTypeList &
OperationSignature::Parameters() const
{
    return aParameterTypes;
}

inline E_ConVol
OperationSignature::ConVol() const
{
    return eConVol;
}



} // namespace cpp
} // namespace ary
#endif
