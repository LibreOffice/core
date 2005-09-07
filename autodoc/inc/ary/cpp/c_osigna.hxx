/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: c_osigna.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 16:00:26 $
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
    // COMPONENTS
#include <ary/ids.hxx>
#include <ary/cpp/c_etypes.hxx>
    // PARAMETERS



namespace ary
{

namespace cpp
{

class OperationSignature
{
  public:
    typedef std::vector<Tid>    ParameterTypeList;

                        OperationSignature();
                        OperationSignature(
                            std::vector<Tid> &  i_rParameterTypes,  // Non const, because it will be swapped with aParameterTypes.
                            E_ConVol            i_eConVol );

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

#if 0 // Vielleicht noch gebraucht, vielleicht auch nicht. DO NOT delete!
struct S_VariableInfo
{
    udmstri             sName;
    udmstri             sSizeExpression;
    udmstri             sInitExpression;

    void                Empty()
                        { sName.clear();
                          sSizeExpression.clear();
                          sInitExpression.clear(); }
};
#endif


// IMPLEMENTATION

inline bool
OperationSignature::operator<( const OperationSignature & i_rSign ) const
    { return Compare(i_rSign) < 0; }
inline const OperationSignature::ParameterTypeList &
OperationSignature::Parameters() const
    { return aParameterTypes; }
inline E_ConVol
OperationSignature::ConVol() const
    { return eConVol; }


} // namespace cpp
} // namespace ary


#endif

