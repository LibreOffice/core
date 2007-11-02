/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: c_funct.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: hr $ $Date: 2007-11-02 14:48:45 $
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
#include <ary/cpp/c_ce.hxx>
    // OTHER
#include <ary/cessentl.hxx>
#include <ary/cpp/c_types4cpp.hxx>
#include <ary/cpp/c_slntry.hxx>
#include <ary/cpp/c_vfflag.hxx>
#include <ary/cpp/c_osigna.hxx>




namespace ary
{
namespace cpp
{



/** A C++ function declaration.
*/
class Function : public CodeEntity
{
  public:
    enum E_ClassId { class_id = 1004 };

                        Function();
                        Function(
                            const String  &     i_sLocalName,
                            Ce_id               i_nOwner,
                            E_Protection        i_eProtection,
                            loc::Le_id          i_nFile,
                            Type_id             i_nReturnType,
                            const std::vector<S_Parameter> &
                                                i_parameters,
                            E_ConVol            i_conVol,
                            E_Virtuality        i_eVirtuality,
                            FunctionFlags       i_aFlags,
                            bool                i_bThrowExists,
                            const std::vector<Type_id> &
                                                i_rExceptions );
                        ~Function();


    // OPERATIONS
    void                Add_TemplateParameterType(
                            const String  &     i_sLocalName,
                            Type_id             i_nIdAsType );

    // INQUIRY
    const OperationSignature &
                        Signature() const;
    Type_id             ReturnType() const;
    E_Protection        Protection() const      { return eProtection; }
    E_Virtuality        Virtuality() const      { return eVirtuality; }
    const FunctionFlags &
                        Flags() const           { return aFlags; }
    const StringVector &
                        ParamInfos() const      { return aParameterInfos; }
    const std::vector<Type_id> *
                        Exceptions() const      { return pExceptions.Ptr(); }

    const List_TplParam &
                        TemplateParameters() const
                                                { return aTemplateParameterTypes; }
    bool                IsIdentical(
                            const Function &    i_f ) const;

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

    // Local Types
    typedef StringVector                ParameterInfoList;
    typedef std::vector<Type_id>        ExceptionTypeList;

    // DATA
    CeEssentials        aEssentials;
    List_TplParam       aTemplateParameterTypes;
    OperationSignature  aSignature;
    Type_id             nReturnType;
    E_Protection        eProtection;
    E_Virtuality        eVirtuality;
    FunctionFlags       aFlags;
    ParameterInfoList   aParameterInfos;
    Dyn<ExceptionTypeList>
                        pExceptions;            //  if (NOT pExceptions) there is no throw,
                                                //    else, there is one, but the list still may be empty.
};




// IMPLEMENTATION
inline const OperationSignature &
Function::Signature() const
    { return aSignature; }
inline Type_id
Function::ReturnType() const
    { return nReturnType; }




}   // namespace cpp
}   // namespace ary
#endif
