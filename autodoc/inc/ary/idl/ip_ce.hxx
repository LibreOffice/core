/*************************************************************************
 *
 *  $RCSfile: ip_ce.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2004-07-12 15:10:46 $
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

#ifndef ARY_IDL_IP_CE_HXX
#define ARY_IDL_IP_CE_HXX


// USED SERVICES
    // BASE CLASSES
    // COMPONENTS
    // PARAMETERS
#include <ary/idl/i_language.hxx>
#include <ary/idl/i_property.hxx>


namespace ary
{

class QualifiedName;

namespace idl
{
    class Module;

    class ConstantsGroup;
    class Enum;
    class Exception;
    class Interface;
    class Service;
    class SglIfcService;
    class Singleton;
    class SglIfcSingleton;
    class Struct;
    class Typedef;

    class Attribute;
    class Constant;
    class EnumValue;
    class Function;
    class Property;
    class StructElement;
    class Variable;

    class NameLookup;

/** @resp
    Provides the access logic for all code entities.

    @interface
*/
class CePilot
{
  public:
    // LIFECYCLE
    virtual             ~CePilot() {}

    // OPERATIONS
    Module &            CheckIn_Module(
                            Ce_id               i_nParentId,
                            const String &      i_sName );
    Service &           Store_Service(
                            Ce_id               i_nOwner,
                            const String &      i_sName );
    virtual SglIfcService &
                        Store_SglIfcService(
                            Ce_id               i_nOwner,
                            const String &      i_sName,
                            Type_id             i_nBaseInterface ) = 0;
    Interface &         Store_Interface(
                            Ce_id               i_nOwner,
                            const String &      i_sName,
                            Type_id             i_nBase );
    Struct &            Store_Struct(
                            Ce_id               i_nOwner,
                            const String &      i_sName,
                            Type_id             i_nBase );
    Exception &         Store_Exception(
                            Ce_id               i_nOwner,
                            const String &      i_sName,
                            Type_id             i_nBase );
    Enum &              Store_Enum(
                            Ce_id               i_nOwner,
                            const String &      i_sName );
    Typedef &           Store_Typedef(
                            Ce_id               i_nOwner,
                            const String &      i_sName,
                            Type_id             i_nDefiningType );
    ConstantsGroup &    Store_ConstantsGroup(
                            Ce_id               i_nOwner,
                            const String &      i_sName );
    Singleton &         Store_Singleton(
                            Ce_id               i_nOwner,
                            const String &      i_sName );
    virtual SglIfcSingleton &
                        Store_SglIfcSingleton(
                            Ce_id               i_nOwner,
                            const String &      i_sName,
                            Type_id             i_nBaseInterface ) = 0;

    Constant &          Store_Constant(
                            Ce_id               i_nOwner,
                            const String &      i_sName,
                            Type_id             i_nType,
                            const String &      i_sValue );
    Property &          Store_Property(
                            Ce_id               i_nOwner,
                            const String &      i_sName,
                            Type_id             i_nType,
                            Property::Stereotypes
                                                i_stereotypes );
    Function &          Store_Function(
                            Ce_id               i_nOwner,
                            const String &      i_sName,
                            Type_id             i_nReturnType,
                            bool                i_bOneWay );
    virtual Function &  Store_ServiceConstructor(
                            Ce_id               i_nOwner,
                            const String &      i_sName ) = 0;
    StructElement &     Store_StructMember(
                            Ce_id               i_nOwner,
                            const String &      i_sName,
                            Type_id             i_nType );
    StructElement &     Store_ExceptionMember(
                            Ce_id               i_nOwner,
                            const String &      i_sName,
                            Type_id             i_nType );
    EnumValue &         Store_EnumValue(
                            Ce_id               i_nOwner,
                            const String &      i_sName,
                            const String &      i_sValue );
    Attribute &         Store_Attribute(
                            Ce_id               i_nOwner,
                            const String &      i_sName,
                            Type_id             i_nType,
                            bool                i_bReadOnly,
                            bool                i_bBound );
    // INQUIRY
    const Module &      GlobalNamespace() const;
    const CodeEntity &  Find_Ce(
                            Ce_id               i_nId ) const;

    const Module &      Find_Module(
                            Ce_id               i_nId ) const;
    const Module *      Search_Module(
                            Ce_id               i_nId ) const;
    const Function &    Find_Function(
                            Ce_id               i_nId ) const;
    const Property &    Find_Property(
                            Ce_id               i_nId ) const;
    const EnumValue &   Find_EnumValue(
                            Ce_id               i_nId ) const;
    const Constant &    Find_Constant(
                            Ce_id               i_nId ) const;
    const StructElement &
                        Find_StructElement(
                            Ce_id               i_nId ) const;
    void                Get_Text(
                            StringVector &      o_module,
                            String &            o_ce,
                            String &            o_member,
                            const CodeEntity &  i_ce ) const;
    const NameLookup &  NameDictionary() const;

    // ACCESS
    Module &            GlobalNamespace();
    CodeEntity &        Find_Ce(
                            Ce_id               i_nId );

  private:
    // Locals
    virtual Module &    do_CheckIn_Module(
                            Ce_id               i_nParentId,
                            const String &      i_sName ) = 0;
    virtual Service &   do_Store_Service(
                            Ce_id               i_nOwner,
                            const String &      i_sName ) = 0;
    virtual Interface & do_Store_Interface(
                            Ce_id               i_nOwner,
                            const String &      i_sName,
                            Type_id             i_nBase ) = 0;
    virtual Struct &    do_Store_Struct(
                            Ce_id               i_nOwner,
                            const String &      i_sName,
                            Type_id             i_nBase ) = 0;
    virtual Exception & do_Store_Exception(
                            Ce_id               i_nOwner,
                            const String &      i_sName,
                            Type_id             i_nBase ) = 0;
    virtual Enum &      do_Store_Enum(
                            Ce_id               i_nOwner,
                            const String &      i_sName ) = 0;
    virtual Typedef &   do_Store_Typedef(
                            Ce_id               i_nOwner,
                            const String &      i_sName,
                            Type_id             i_nDefiningType ) = 0;
    virtual ConstantsGroup &
                        do_Store_ConstantsGroup(
                            Ce_id               i_nOwner,
                            const String &      i_sName ) = 0;
    virtual Singleton & do_Store_Singleton(
                            Ce_id               i_nOwner,
                            const String &      i_sName ) = 0;

    virtual Constant &  do_Store_Constant(
                            Ce_id               i_nOwner,
                            const String &      i_sName,
                            Type_id             i_nType,
                            const String &      i_sValue ) = 0;
    virtual Property &  do_Store_Property(
                            Ce_id               i_nOwner,
                            const String &      i_sName,
                            Type_id             i_nType,
                            Property::Stereotypes
                                                i_stereotypes ) = 0;
    virtual Function &  do_Store_Function(
                            Ce_id               i_nOwner,
                            const String &      i_sName,
                            Type_id             i_nReturnType,
                            bool                i_bOneWay ) = 0;
    virtual StructElement &
                        do_Store_StructMember(
                            Ce_id               i_nOwner,
                            const String &      i_sName,
                            Type_id             i_nType ) = 0;
    virtual StructElement &
                        do_Store_ExceptionMember(
                            Ce_id               i_nOwner,
                            const String &      i_sName,
                            Type_id             i_nType ) = 0;
    virtual EnumValue & do_Store_EnumValue(
                            Ce_id               i_nOwner,
                            const String &      i_sName,
                            const String &      i_sValue ) = 0;
    virtual Attribute & do_Store_Attribute(
                            Ce_id               i_nOwner,
                            const String &      i_sName,
                            Type_id             i_nType,
                            bool                i_bReadOnly,
                            bool                i_bBound ) = 0;

    virtual const Module &
                        inq_GlobalNamespace() const = 0;
    virtual const CodeEntity &
                        inq_Find_Ce(
                            Ce_id               i_nId ) const = 0;
    virtual const Module &
                        inq_Find_Module(
                            Ce_id               i_nId ) const = 0;
    virtual const Module *
                        inq_Search_Module(
                            Ce_id               i_nId ) const = 0;
    virtual const Function &
                        inq_Find_Function(
                            Ce_id               i_nId ) const = 0;
    virtual const Property &
                        inq_Find_Property(
                            Ce_id               i_nId ) const = 0;
    virtual const EnumValue &
                        inq_Find_EnumValue(
                            Ce_id               i_nId ) const = 0;
    virtual const Constant &
                        inq_Find_Constant(
                            Ce_id               i_nId ) const = 0;
    virtual const StructElement &
                        inq_Find_StructElement(
                            Ce_id               i_nId ) const = 0;
    virtual void        inq_Get_Text(
                            StringVector &      o_module,
                            String &            o_ce,
                            String &            o_member,
                            const CodeEntity &  i_ce ) const = 0;
    virtual const NameLookup &
                        inq_NameDictionary() const = 0;

    // ACCESS
    virtual Module &    access_GlobalNamespace() = 0;
    virtual CodeEntity &
                        access_Find_Ce(
                            Ce_id               i_nId ) = 0;
};


// IMPLEMENTATION

inline Module &
CePilot::CheckIn_Module(    Ce_id               i_nParentId,
                            const String &      i_sName )
    { return do_CheckIn_Module(i_nParentId, i_sName); }

inline Service &
CePilot::Store_Service(     Ce_id               i_nOwner,
                            const String &      i_sName )
    { return do_Store_Service(i_nOwner,i_sName); }

inline Interface &
CePilot::Store_Interface(   Ce_id               i_nOwner,
                            const String &      i_sName,
                            Type_id             i_nBase )
    { return do_Store_Interface(i_nOwner,i_sName,i_nBase); }

inline Struct &
CePilot::Store_Struct(      Ce_id               i_nOwner,
                            const String &      i_sName,
                            Type_id             i_nBase )
    { return do_Store_Struct(i_nOwner,i_sName,i_nBase); }

inline Exception &
CePilot::Store_Exception(   Ce_id               i_nOwner,
                            const String &      i_sName,
                            Type_id             i_nBase )
    { return do_Store_Exception(i_nOwner,i_sName,i_nBase); }

inline Enum &
CePilot::Store_Enum(        Ce_id               i_nOwner,
                            const String &      i_sName )
    { return do_Store_Enum(i_nOwner,i_sName); }

inline Typedef &
CePilot::Store_Typedef(     Ce_id               i_nOwner,
                            const String &      i_sName,
                            Type_id             i_nDefiningType )
    { return do_Store_Typedef(i_nOwner,i_sName,i_nDefiningType); }

inline ConstantsGroup &
CePilot::Store_ConstantsGroup(  Ce_id               i_nOwner,
                                const String &      i_sName )
    { return do_Store_ConstantsGroup(i_nOwner,i_sName); }

inline Singleton &
CePilot::Store_Singleton( Ce_id               i_nOwner,
                          const String &      i_sName )
    { return do_Store_Singleton(i_nOwner,i_sName); }

inline Constant &
CePilot::Store_Constant(    Ce_id               i_nOwner,
                            const String &      i_sName,
                            Type_id             i_nType,
                            const String &      i_sValue )
    { return do_Store_Constant(i_nOwner,i_sName,i_nType,i_sValue); }

inline Property &
CePilot::Store_Property(    Ce_id               i_nOwner,
                            const String &      i_sName,
                            Type_id             i_nType,
                            ary::idl::Property::Stereotypes
                                                i_stereotypes )
    { return do_Store_Property(i_nOwner,i_sName,i_nType,i_stereotypes); }

inline Function &
CePilot::Store_Function(    Ce_id               i_nOwner,
                            const String &      i_sName,
                            Type_id             i_nReturnType,
                            bool                i_bOneWay )
    { return do_Store_Function(i_nOwner,i_sName,i_nReturnType,i_bOneWay); }

inline StructElement &
CePilot::Store_StructMember(   Ce_id               i_nOwner,
                               const String &      i_sName,
                               Type_id             i_nType )
    { return do_Store_StructMember(i_nOwner,i_sName,i_nType); }

inline StructElement &
CePilot::Store_ExceptionMember( Ce_id               i_nOwner,
                                const String &      i_sName,
                                Type_id             i_nType )
    { return do_Store_ExceptionMember(i_nOwner,i_sName,i_nType); }

inline EnumValue &
CePilot::Store_EnumValue(   Ce_id               i_nOwner,
                            const String &      i_sName,
                            const String &      i_sValue )
    { return do_Store_EnumValue(i_nOwner,i_sName,i_sValue); }

inline Attribute &
CePilot::Store_Attribute( Ce_id               i_nOwner,
                          const String &      i_sName,
                          Type_id             i_nType,
                          bool                i_bReadOnly,
                          bool                i_bBound )
    { return do_Store_Attribute(i_nOwner, i_sName, i_nType, i_bReadOnly, i_bBound); }

inline const Module &
CePilot::GlobalNamespace() const
    { return inq_GlobalNamespace(); }
inline const CodeEntity &
CePilot::Find_Ce( Ce_id i_nId ) const
    { return inq_Find_Ce(i_nId); }
inline const Module &
CePilot::Find_Module( Ce_id i_nId ) const
    { return inq_Find_Module(i_nId); }
inline const Module *
CePilot::Search_Module( Ce_id i_nId ) const
    { return inq_Search_Module(i_nId); }
inline const Function &
CePilot::Find_Function( Ce_id i_nId ) const
    { return inq_Find_Function(i_nId); }
inline const Property &
CePilot::Find_Property( Ce_id i_nId ) const
    { return inq_Find_Property(i_nId); }
inline const EnumValue &
CePilot::Find_EnumValue( Ce_id i_nId ) const
    { return inq_Find_EnumValue(i_nId); }
inline const Constant &
CePilot::Find_Constant( Ce_id i_nId ) const
    { return inq_Find_Constant(i_nId); }

inline const StructElement &
CePilot::Find_StructElement( Ce_id i_nId ) const
    { return inq_Find_StructElement(i_nId); }
inline void
CePilot::Get_Text( StringVector &      o_module,
                   String &            o_ce,
                   String &            o_member,
                   const CodeEntity &  i_ce ) const
    { inq_Get_Text(o_module, o_ce, o_member, i_ce);  }

inline const NameLookup &
CePilot::NameDictionary() const
    { return inq_NameDictionary(); }

inline Module &
CePilot::GlobalNamespace()
    { return access_GlobalNamespace(); }
inline CodeEntity &
CePilot::Find_Ce( Ce_id i_nId )
    { return access_Find_Ce(i_nId); }


}   // namespace idl
}   // namespace ary


#endif


