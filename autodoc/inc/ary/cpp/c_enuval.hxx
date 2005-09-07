/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: c_enuval.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 15:58:16 $
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

#ifndef ARY_CPP_C_ENUVAL_HXX
#define ARY_CPP_C_ENUVAL_HXX


// USED SERVICES
    // BASE CLASSES
#include <ary/ce.hxx>
    // COMPONENTS
#include <ary/cessentl.hxx>
#include <ary/cpp/c_etypes.hxx>
    // PARAMETERS



namespace ary
{
namespace cpp
{


class EnumValue : public CodeEntity
{
  public:
    // LIFECYCLE
                        EnumValue();
                        EnumValue(
                            Cid                 i_nId,
                            const udmstri &     i_sLocalName,
                            Cid                 i_nOwner,
                            udmstri             i_sInitialisation );
                        ~EnumValue();
    // INQUIRY
    static RCid         RC_()                   { return 0x1007; }

    const udmstri &     Initialisation() const;


  private:
    // Interface ary::CodeEntity
    virtual Cid         inq_Id() const;
    virtual const udmstri &
                        inq_LocalName() const;
    virtual Cid         inq_Owner() const;
    virtual Lid         inq_Location() const;

    // Interface ary::RepositoryEntity
    virtual void        do_StoreAt(
                            ary::Display &      o_rOut ) const;
    virtual RCid        inq_RC() const;
    virtual const ary::Documentation &
                        inq_Info() const;
    virtual void        do_Add_Documentation(
                            DYN ary::Documentation &
                                                let_drInfo );

    // DATA
    CeEssentials        aEssentials;
    udmstri             sInitialisation;
};



// IMPLEMENTATION

inline const udmstri &
EnumValue::Initialisation() const
    { return sInitialisation; }


}   // namespace cpp
}   // namespace ary


#endif

