/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: c_vari.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: hr $ $Date: 2007-11-02 14:52:45 $
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

#ifndef ARY_CPP_C_VARI_HXX
#define ARY_CPP_C_VARI_HXX


// USED SERVICES
    // BASE CLASSES
#include <ary/cpp/c_ce.hxx>
    // OTHER
#include <ary/cessentl.hxx>
#include <ary/cpp/c_types4cpp.hxx>
#include <ary/cpp/c_vfflag.hxx>



namespace ary
{
namespace cpp
{


/** A C++ variable or constant declaration.
*/
class Variable : public CodeEntity
{
  public:
    // LIFECYCLE
    enum E_ClassId { class_id = 1005 };

                        Variable();
                        Variable(
                            const String  &     i_sLocalName,
                            Ce_id               i_nOwner,
                            E_Protection        i_eProtection,
                            loc::Le_id          i_nFile,
                            Type_id             i_nType,
                            VariableFlags       i_aFlags,
                            const String  &     i_sArraySize,
                            const String  &     i_sInitValue );
                        ~Variable();


    // INQUIRY
    Type_id             Type() const;
    const String  &     ArraySize() const;
    const String  &     Initialisation() const;
    E_Protection        Protection() const      { return eProtection; }

  private:
    // Interface csv::ConstProcessorClient
    virtual void        do_Accept(
                            csv::ProcessorIfc & io_processor ) const;

    // Interface ary::cpp::CodeEntity
    virtual const String  &
                        inq_LocalName() const;
    virtual Cid         inq_Owner() const;
    virtual Lid         inq_Location() const;

    // Interface ary::cpp::CppEntity
    virtual ClassId     get_AryClass() const;

    // DATA
    CeEssentials        aEssentials;
    Type_id             nType;
    E_Protection        eProtection;
    VariableFlags       aFlags;
    String              sArraySize;
    String              sInitialisation;
};



// IMPLEMENTATION
inline Type_id
Variable::Type() const
    { return nType; }
inline const String  &
Variable::ArraySize() const
    { return sArraySize; }
inline const String  &
Variable::Initialisation() const
    { return sInitialisation; }



}   // namespace cpp
}   // namespace ary
#endif
