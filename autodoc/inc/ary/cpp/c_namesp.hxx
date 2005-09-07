/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: c_namesp.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 16:00:09 $
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

#ifndef ARY_CPP_C_NAMESP_HXX
#define ARY_CPP_C_NAMESP_HXX



// USED SERVICES
    // BASE CLASSES
#include <ary/ce.hxx>
#include <ary/arygroup.hxx>
    // COMPONENTS
#include <ary/cessentl.hxx>
#include <ary/opertype.hxx>
#include <ary/cpp/c_idlist.hxx>
    // PARAMETERS
#include <ary/idlists.hxx>



namespace ary
{
namespace cpp
{


class Namespace : public CodeEntity,
                  public AryGroup
{
  public:
    enum E_Slots
    {
        SLOT_SubNamespaces = 1,
        SLOT_Classes,
        SLOT_Enums,
        SLOT_Typedefs,
        SLOT_Operations,
        SLOT_Variables,
        SLOT_Constants
    };
                        Namespace();
                        Namespace(              /// Used only for the global namespace.
                            Rid                 i_nId );
                        Namespace(
                            Cid                 i_nId,
                            const udmstri &     i_sName,
                            Namespace &         i_rParent );
                        ~Namespace();
    // OPERATIONS
    void                Add_LocalNamespace(
                            Namespace &         io_rLocalNamespace );
    void                Add_LocalClass(
                            const udmstri &     i_sLocalName,
                            Cid                 i_nId );
    void                Add_LocalEnum(
                            const udmstri &     i_sLocalName,
                            Cid                 i_nId );
    void                Add_LocalTypedef(
                            const udmstri &     i_sLocalName,
                            Cid                 i_nId );
    void                Add_LocalOperation(
                            const udmstri &     i_sLocalName,
                            OSid                i_nOS,
                            Cid                 i_nId );
    void                Add_LocalVariable(
                            const udmstri &     i_sLocalName,
                            Cid                 i_nId );
    void                Add_LocalConstant(
                            const udmstri &     i_sLocalName,
                            Cid                 i_nId );

    // INQUIRY
    static RCid         RC_()                   { return 0x1001; }
    virtual uintt       Depth() const;
    Namespace *         Parent() const;

    Namespace *         Search_LocalNamespace(
                            const udmstri &     i_sLocalName ) const;
    uintt               Get_SubNamespaces(
                            std::vector< const Namespace* > &
                                                o_rResultList ) const;
    Rid                 Search_LocalClass(
                            const String &      i_sName ) const;
    Rid                 Search_LocalOperation(
                            const String &      i_sName,
                            OSid                i_nSignature ) const;
  private:
    // Interface CodeEntity
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
        // Interface AryGroup
    virtual Gid         inq_Id_Group() const;
    virtual const RepositoryEntity &
                        inq_RE_Group() const;
    virtual const ary::group::SlotList &
                        inq_Slots() const;
    virtual DYN Slot *  inq_Create_Slot(
                            SlotAccessId        i_nSlot ) const;

    // Local

    // DATA
    CeEssentials        aEssentials;

    Map_NamespacePtr    aLocalNamespaces;
    Map_LocalCe         aLocalClasses;
    Map_LocalCe         aLocalEnums;
    Map_LocalCe         aLocalTypedefs;
    Set_LocalOperation  aLocalOperations;
    Map_LocalCe         aLocalVariables;
    Map_LocalCe         aLocalConstants;

    Namespace *         pParent;
    uintt               nDepth;
};


}   // namespace cpp
}   // ary


#endif

