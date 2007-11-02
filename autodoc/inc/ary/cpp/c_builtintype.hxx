/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: c_builtintype.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: hr $ $Date: 2007-11-02 14:45:31 $
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

#ifndef ARY_CPP_C_BUILTINTYPE_HXX
#define ARY_CPP_C_BUILTINTYPE_HXX



// USED SERVICES
    // BASE CLASSES
#include <ary/cpp/c_type.hxx>
    // COMPONENTS
    // PARAMETERS


namespace ary
{
namespace cpp
{


/** Represents types like void, int, double.
*/
class BuiltInType : public Type
{
  public:
    enum E_ClassId { class_id = 1200 };

                        BuiltInType(
                            const String  &     i_sName,
                            E_TypeSpecialisation
                                                i_Specialisation );

    String              SpecializedName() const;

    static String       SpecializedName_(
                            const char *        i_sName,
                            E_TypeSpecialisation
                                                i_eTypeSpecialisation );
  private:
    // Interface csv::ConstProcessorClient
    virtual void        do_Accept(
                            csv::ProcessorIfc & io_processor ) const;
    // Interface Object:
    virtual ClassId     get_AryClass() const;

    // Interface Type:
    virtual bool        inq_IsConst() const;
    virtual void        inq_Get_Text(
                            StreamStr &         o_rPreName,
                            StreamStr &         o_rName,
                            StreamStr &         o_rPostName,
                            const Gate &        i_rGate ) const;
    // DATA
    String              sName;
    E_TypeSpecialisation
                        eSpecialisation;
};



inline String
BuiltInType::SpecializedName() const
{
    return SpecializedName_(sName, eSpecialisation);
}




}   // namespace cpp
}   // namespace ary
#endif
