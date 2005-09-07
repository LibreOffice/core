/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: luxenum.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 18:09:20 $
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

#ifndef UDM_LUXENUM_HXX
#define UDM_LUXENUM_HXX



// USED SERVICES
    // BASE CLASSES
    // COMPONENTS
    // PARAMETERS
#include <map>
#include <algorithm>


namespace lux
{

typedef std::map< intt, udmstri > EnumValueMap;


template <class DIFF>
class Enum // : public Template_Base
{
  public:
    // TYPES
    typedef Enum< DIFF >    self;

    // LIFECYCLE
                        Enum(
                            DIFF                i_nValue,
                            const char *        i_sText )
                                                :   nValue(i_nValue) { Values_()[nValue] = i_sText;
                                                                       // Sequence_().insert(
                                                                       //       std::lower_bound( Sequence_().begin(), Sequence_().end(), i_nValue ),
                                                                       //       i_nValue );
                                                                     }
                        Enum(
                            DIFF                i_nValue )
                                                :   nValue(i_nValue) { ; }
                        Enum(
                            intt                i_nValue = 0 )
                                                :   nValue(i_nValue) { if ( NOT CheckIntt(i_nValue) ) { csv_assert(false); } }
                        Enum(
                            const self &        i_rEnum )
                                                :   nValue(i_rEnum.nValue) {;}

    self &              operator=(
                            DIFF                i_nValue )
                                                { nValue = i_nValue; return *this; }
    self &              operator=(
                            intt                i_nValue )
                                                { if ( CheckIntt(i_nValue) ) nValue = DIFF(i_nValue);
                                                  else csv_assert(false); return *this; }
    self &              operator=(
                            const self &        i_rEnum )
                                                { nValue = i_rEnum.nValue; return *this; }
                        operator DIFF() const   { return DIFF(nValue); }

//  self &              operator++()            { if ( uintt(nValue) < Sequence_().size() - 1) nValue = Sequence_()[nValue+1]; return *this; }
//  self &              operator--()            { if ( uintt(nValue) > 0) nValue = Sequence_()[nValue-1]; }


    DIFF                operator()() const      { return nValue; }
    const udmstri &     Text() const            { return Values_()[nValue]; }
//  virtual const Agent &
//                      GetSpecialAgent() const;

  private:
    static EnumValueMap &
                        Values_();
//  static std::vector< DIFF > &
//                      Sequence_();
    bool                CheckIntt(
                            intt                i_nNumber )
                                                { return Values_().find(nValue) != Values_().end(); }

    // DATA
    intt                nValue;
};


// IMPLEMENTATION

/*
// Dummy - has t be overwritten by specialisation, if used:
template <class DIFF>
const Agent &
Enum<DIFF>::GetSpecialAgent() const
{
    csv_assert(false);
    return *(const udm::Agent*)0;
}
*/

}   // namespace lux


#endif

