/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: c_funct.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 15:58:47 $
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

#ifndef ARY_CPP_C_FUNCT_HXX
#define ARY_CPP_C_FUNCT_HXX



// USED SERVICES
    // BASE CLASSES
#include <ary/ce.hxx>
    // COMPONENTS
#include <ary/ids.hxx>
#include <ary/cessentl.hxx>
#include <ary/cpp/c_etypes.hxx>
#include <ary/cpp/c_vfflag.hxx>
#include <ary/cpp/c_osigna.hxx>
#include <ary/cpp/c_idlist.hxx>
    // PARAMETERS



namespace ary
{
namespace cpp
{


class Function : public CodeEntity
{
  public:
                        Function();
                        Function(
                            Cid                 i_nId,
                            const udmstri &     i_sLocalName,
                            Cid                 i_nOwner,
                            E_Protection        i_eProtection,
                            Lid                 i_nFile,
                            Tid                 i_nReturnType,
                            OSid                i_nSignature,
                            StringVector &
                                                i_rNonType_ParameterInfos,      /// Is non const, because the contents are swap'ped with aParameterInfos.
                            E_Virtuality        i_eVirtuality,
                            FunctionFlags       i_aFlags,
                            bool                i_bThrowExists,
                            const std::vector<Tid> &
                                                i_rExceptions );
                        ~Function();

    // OPERATIONS
    void                Add_TemplateParameterType(
                            const udmstri &     i_sLocalName,
                            Tid                 i_nIdAsType );

    // INQUIRY
    static RCid         RC_()                   { return 0x1005; }

    OSid                Signature() const;
    Tid                 ReturnType() const;
    E_Protection        Protection() const      { return eProtection; }
    E_Virtuality        Virtuality() const      { return eVirtuality; }
    const FunctionFlags &
                        Flags() const           { return aFlags; }
    const StringVector &
                        ParamInfos() const      { return aParameterInfos; }
    const std::vector<Tid> *
                        Exceptions() const      { return pExceptions.Ptr(); }

    const List_TplParam &
                        TemplateParameters() const
                                                { return aTemplateParameterTypes; }

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
    virtual const Documentation &
                        inq_Info() const;
    virtual void        do_Add_Documentation(
                            DYN Documentation & let_drInfo );

    // Local Types
    typedef StringVector                ParameterInfoList;
    typedef std::vector<Tid>                    ExceptionTypeList;

    // DATA
    CeEssentials        aEssentials;

    List_TplParam       aTemplateParameterTypes;

    OSid                nSignature;
    Tid                 nReturnType;
    E_Protection        eProtection;
    E_Virtuality        eVirtuality;
    FunctionFlags       aFlags;
    ParameterInfoList   aParameterInfos;
    Dyn<ExceptionTypeList>
                        pExceptions;            //  if (NOT pExceptions) there is no throw,
                                                //    else, there is one, but the list still may be empty.
};



// IMPLEMENTATION
inline OSid
Function::Signature() const
    { return nSignature; }
inline Tid
Function::ReturnType() const
    { return nReturnType; }


}   // namespace cpp
}   // namespace ary


#endif

