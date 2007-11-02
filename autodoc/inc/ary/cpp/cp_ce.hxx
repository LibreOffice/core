/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: cp_ce.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: hr $ $Date: 2007-11-02 14:53:59 $
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

#ifndef ARY_CPP_CP_CE_HXX
#define ARY_CPP_CP_CE_HXX


// USED SERVICES
    // BASE CLASSES
    // OTHER
#include <ary/cpp/c_types4cpp.hxx>

namespace ary
{
    class QualifiedName;

namespace cpp
{
    class Class;
    class CodeEntity;
    class Enum;
    class EnumValue;
    class Function;
    class InputContext;
    class Namespace;
    class OperationSignature;
    class Typedef;
    class Variable;

    struct FunctionFlags;
    struct S_Parameter;
    struct VariableFlags;
}
}





namespace ary
{
namespace cpp
{



/** Acess to all declared C++ code entites (types, variables, operations)
    in the repository.
*/
class CePilot
{
  public:
    // LIFECYCLE
    virtual             ~CePilot() {}

    // OPERATIONS
    virtual Namespace &
                        CheckIn_Namespace(
                            const InputContext &
                                                i_context,
                            const String  &     i_localName ) = 0;
    virtual Class &     Store_Class(
                            const InputContext &
                                                i_context,
                            const String  &     i_localName,
                            E_ClassKey          i_classKey ) = 0;
    virtual Enum &      Store_Enum(
                            const InputContext &
                                                i_context,
                            const String  &     i_localName ) = 0;
    virtual Typedef &   Store_Typedef(
                            const InputContext &
                                                i_context,
                            const String  &     i_localName,
                            Type_id             i_referredType ) = 0;

    /// @return 0, if the function is duplicate.
    virtual Function *  Store_Operation(
                            const InputContext &
                                                i_context,
                            const String  &     i_localName,
                            Type_id             i_returnType,
                            const std::vector<S_Parameter> &
                                                i_parameters,
                            E_Virtuality        i_virtuality,
                            E_ConVol            i_conVol,
                            FunctionFlags       i_flags,
                            bool                i_throwExists,
                            const std::vector<Tid> &
                                                i_exceptions ) = 0;
    virtual Variable &  Store_Variable(
                            const InputContext &
                                                i_context,
                            const String  &     i_localName,
                            Type_id             i_type,
                            VariableFlags       i_flags,
                            const String  &     i_arraySize,
                            const String  &     i_initValue ) = 0;
    virtual EnumValue & Store_EnumValue(
                            const InputContext &
                                                i_context,
                            const String  &     i_localName,
                            const String  &     i_initValue ) = 0;
    // INQUIRY
    virtual const Namespace &
                        GlobalNamespace() const = 0;
    virtual const CodeEntity &
                        Find_Ce(
                            Ce_id               i_id ) const = 0;
    virtual const CodeEntity *
                        Search_Ce(
                            Ce_id               i_id ) const = 0;

    /// It's assumed that i_rSearchedName is an absolute name.
    virtual const CodeEntity *
                        Search_CeAbsolute(
                            const CodeEntity &  i_curScope,
                            const QualifiedName &
                                                i_absoluteName ) const = 0;
    virtual const CodeEntity *
                        Search_CeLocal(
                            const String  &     i_relativeName,
                            bool                i_isFunction,
                            const Namespace &   i_curNamespace,
                            const Class *       i_curClass ) const = 0;
    virtual void        Get_QualifiedName(
                            StreamStr &         o_result,
                            const String  &     i_localName,
                            Ce_id               i_owner,
                            const char *        i_delimiter = "::" ) const = 0;
    virtual void        Get_SignatureText(
                            StreamStr &         o_rOut,
                            const OperationSignature &
                                                i_signature,
                            const StringVector *
                                                i_sParameterNames = 0 ) const = 0;
    virtual CesResultList
                        Search_TypeName(
                            const String  &     i_sName ) const = 0;
    // ACCESS
       virtual Namespace & GlobalNamespace() = 0;
};




}   // namespace cpp
}   // namespace ary
#endif
