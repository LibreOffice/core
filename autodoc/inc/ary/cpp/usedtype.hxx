/*************************************************************************
 *
 *  $RCSfile: usedtype.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: np $ $Date: 2002-03-08 14:45:15 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
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

