/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#ifndef ARY_CPP_USEDTYPE_HXX
#define ARY_CPP_USEDTYPE_HXX

// USED SERVICES
    // BASE CLASSES
#include <ary/cpp/c_type.hxx>
    // OTHER
#include <ary/cpp/namechain.hxx>

namespace ary
{
namespace cpp
{
    class CePilot;

namespace ut
{
    class List_TplParameter;
}
}
}




namespace ary
{
namespace cpp
{


/** This class represents a type in textual form, like it is parsed out of
    source code as a variable type or function return type.
*/
class UsedType : public Type
{
  public:
    enum E_ClassId { class_id = 1203 };

    explicit            UsedType(
                            Ce_id               i_scope );
                        ~UsedType();
    // OPERATORS
    bool                operator<(
                            const UsedType &    i_rType ) const;
    // OPERATIONS

        // Operations to build up the used type from parsing:
    void                Set_Absolute();         /// "::" is in front.
    void                Add_NameSegment(
                            const char *        i_sSeg );
    ut::List_TplParameter &
                        Enter_Template();
    void                Set_Unsigned();
    void                Set_Signed();
    void                Set_BuiltIn(
                            const char *        i_sType );
    void                Set_Const();            /// Sets CV to the type or the present pointer level, whatever is highest.
    void                Set_Volatile();         /// Sets CV to the type or the present pointer level, whatever is highest.
    void                Add_PtrLevel();         /// For an '*'.
    void                Set_Reference();        /// For an '&'.

        // Operations to find the relating CodeEntity:
    /** This needs to be called only one time. After that
        RelatedCe() will return the value.

        When connectiing all parsed types, there are three steps:
            1. Find related types in the same scope and namespaces above.
            2. Then all classes can be connected to their base classes.
            3. Lastly types can be connected to Ces only known via their base
               classes. This is not possible at step 1.

        @see Connect2CeOnlyKnownViaBaseClass()
    */
    void                Connect2Ce(
                            const CePilot &     i_ces );

    /** @see Connect2Ce()
    */
    void                Connect2CeOnlyKnownViaBaseClass(
                            const Gate &        i_gate );

    // INQUIRY
    /** @return True, if type consists of one built-in typename and
                nothing else.
    */
    bool                IsBuiltInType() const;
    /** @return the full local name, including template instantiation, but without
        '*','&' or modifiers.
    */
    const String  &     LocalName() const;
    E_TypeSpecialisation
                        TypeSpecialisation() const;

  private:
    // Interface csv::ConstProcessorClient
    virtual void        do_Accept(
                            csv::ProcessorIfc & io_processor ) const;

    // Interface ary::Object:
    virtual ClassId     get_AryClass() const;

    // Interface ary::cpp::Type:
    virtual Rid         inq_RelatedCe() const;
    virtual bool        inq_IsConst() const;
    virtual void        inq_Get_Text(
                            StreamStr &         o_rPreName,
                            StreamStr &         o_rName,
                            StreamStr &         o_rPostName,
                            const ary::cpp::Gate &
                                                i_rGate ) const;
    // Local
    typedef std::vector< ary::cpp::E_ConVol >   PtrLevelVector;

    uintt               PtrLevel() const        { return uintt(aPtrLevels.size()); }
    Ce_id               RecursiveSearchCe_InBaseClassesOf(
                            const CodeEntity &  i_mayBeClass,
                            const StringVector &
                                                i_myQualification,
                            const String &      i_myName,
                            const Gate &        i_gate ) const;
    void                Get_NameParts(
                            StringVector &      o_qualification,
                            String &            o_name );

    // Forbidden functions
                        UsedType(
                            const UsedType &    i_rType );
    bool                operator=(
                            const UsedType &    i_rType );

    // DATA
    ut::NameChain       aPath;
    PtrLevelVector      aPtrLevels;
    ary::cpp::E_ConVol  eConVol_Type;
    bool                bIsReference;
    bool                bIsAbsolute;
    bool                bRefers2BuiltInType;
    E_TypeSpecialisation
                        eTypeSpecialisation;
    Ce_id               nRelatedCe;

    /// Namespace or class scope where the type occurred.
    Ce_id               nScope;
};


namespace ut
{
    class TemplateParameter;

class List_TplParameter
{
  public:
    typedef std::vector< DYN TemplateParameter * >::const_iterator  const_iterator;

                        List_TplParameter();
                        ~List_TplParameter();

    void                AddParam_Type(
                            Type_id             i_nType );
    /// puts "< " TemplateArgumentList " >" to o_rOut.
    void                Get_Text(
                            StreamStr &         o_rOut,
                            const ary::cpp::Gate &
                                                i_rGate ) const;
    /// @return as strcmp().
    intt                Compare(
                            const List_TplParameter &
                                                i_rOther ) const;

  private:
    typedef std::vector< DYN TemplateParameter * >  Vector_TplArgument;

    Vector_TplArgument  aTplParameters;
};

}   // namespace ut




}   // namespace cpp
}   // namespace ary
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
