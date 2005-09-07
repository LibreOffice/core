/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: usedtype.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 16:05:43 $
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

#ifndef ARY_CPP_USEDTYPE_HXX
#define ARY_CPP_USEDTYPE_HXX


// USED SERVICES
    // BASE CLASSES
#include <ary/cpp/ca_type.hxx>
    // COMPONENTS
    // PARAMETERS


namespace ary
{
namespace cpp
{
    namespace ut
    {
        class List_TplParameter;
    }

    class DisplayGate;
    class Gate;

/** This class represents a type in textual form, like it is parsed out of
    source code as a variable type or function return type.
*/
class UsedType : public Type
{
  public:
                        UsedType();
                        ~UsedType();
    // OPERATORS
    bool                operator<(
                            const UsedType &    i_rType ) const;
    // OPERATIONS

        // Operations to build up the used type from parsing:
    void                Set_Id(
                            Tid                 i_nId );
    void                Set_Absolute();         /// "::" is in front.
    void                Add_NameSegment(
                            const char *        i_sSeg );
    ut::List_TplParameter &
                        Enter_Template();
    void                LeaveTemplate();

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
    */
    Rid                 Connect2Ce(
                            const Gate &        i_rGate );

    // INQUIRY
    /** @return True, if type consists of one built-in typename and
                nothing else.
    */
    bool                IsBuiltInType() const;
    /** @return the full local name, including template instantiation, but without
        '*','&' or modifiers.
    */
    const udmstri &     LocalName() const;
    E_TypeSpecialisation
                        TypeSpecialisation() const;

  private:
    // Forbidden functions
                        UsedType(
                            const UsedType &    i_rType );
    bool                operator=(
                            const UsedType &    i_rType );

    // Interface ary::cpp::Type
    virtual Tid         inq_Id_Type() const;
    virtual Rid         inq_RelatedCe() const;
    virtual bool        inq_IsConst() const;
    virtual void        inq_Get_Text(
                            StreamStr &         o_rPreName,
                            StreamStr &         o_rName,
                            StreamStr &         o_rPostName,
                            const ary::cpp::DisplayGate &
                                                i_rGate ) const;
    // Local
    struct CheshireCat;

    bool                DoesMatch_Ce(
                            Rid                 i_nId,
                            const Gate &        i_rGate ) const;

    // DATA
    Dyn<CheshireCat>    pi;
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
                            Tid                 i_nType );
    void                AddParam_Constant(
                            const udmstri &     i_sConst );

    const_iterator      Begin() const;
    const_iterator      End() const;

    /// puts "< " TemplateArgumentList " >" to o_rOut.
    void                Get_Text(
                            StreamStr &         o_rOut,
                            const ary::cpp::DisplayGate &
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

