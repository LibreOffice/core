/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: icprivow.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 18:21:31 $
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

#ifndef ARY_CPP_ICPRIVOW_HXX
#define ARY_CPP_ICPRIVOW_HXX



// USED SERVICES
    // BASE CLASSES
#include <ary/cpp/inpcontx.hxx>
    // COMPONENTS
    // PARAMETERS

namespace cpp
{



using ary::Cid;
using ary::OSid;

class Owner_Namespace : public ary::cpp::InputContext::Owner
{
  public:
                        Owner_Namespace();
    void                SetAnotherNamespace(
                            ary::cpp::Namespace &
                                                io_rScope );
    virtual bool        HasClass(
                                const udmstri &     i_sLocalName );
    virtual bool        HasOperation(
                                const udmstri &     i_sLocalName,
                                OSid                i_nSignature );

  private:
    virtual void        do_Add_Class(
                            const udmstri &     i_sLocalName,
                            Cid                 i_nId );
    virtual void        do_Add_Enum(
                            const udmstri &     i_sLocalName,
                            Cid                 i_nId );
    virtual void        do_Add_Typedef(
                            const udmstri &     i_sLocalName,
                            Cid                 i_nId );
    virtual void        do_Add_Operation(
                                const udmstri &     i_sLocalName,
                                OSid                i_nSignature,
                                Cid                 i_nId,
                                bool                i_bIsStatic );
    virtual void        do_Add_Variable(
                                const udmstri &     i_sLocalName,
                                Cid                 i_nId,
                                bool                i_bIsConst,
                                bool                i_bIsStatic );
    virtual Cid         inq_Id() const;

    // DATA
    ary::cpp::Namespace *
                        pScope;
};

class Owner_Class : public ary::cpp::InputContext::Owner
{
  public:
                        Owner_Class();
    void                SetAnotherClass(
                            ary::cpp::Class &   io_rScope );

    /** @attention Only a dummy for use at ary::cpp::Gate!
        Will work nerver!
    */
    virtual bool        HasClass(
                                const udmstri &     i_sLocalName );
    /** @attention Only a dummy for use at ary::cpp::Gate!
        Will work nerver!
    */
    virtual bool        HasOperation(
                                const udmstri &     i_sLocalName,
                                OSid                i_nSignature );
  private:
    virtual void        do_Add_Class(
                            const udmstri &     i_sLocalName,
                            Cid                 i_nId );
    virtual void        do_Add_Enum(
                            const udmstri &     i_sLocalName,
                            Cid                 i_nId );
    virtual void        do_Add_Typedef(
                            const udmstri &     i_sLocalName,
                            Cid                 i_nId );
    virtual void        do_Add_Operation(
                                const udmstri &     i_sLocalName,
                                OSid                i_nSignature,
                                Cid                 i_nId,
                                bool                i_bIsStatic );
    virtual void        do_Add_Variable(
                                const udmstri &     i_sLocalName,
                                Cid                 i_nId,
                                bool                i_bIsConst,
                                bool                i_bIsStatic );
    virtual Cid         inq_Id() const;

    // DATA
    ary::cpp::Class *   pScope;
};



}   // namespace cpp


#endif

