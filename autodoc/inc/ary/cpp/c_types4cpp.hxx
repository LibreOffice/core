/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: c_types4cpp.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: hr $ $Date: 2007-11-02 14:52:28 $
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

#ifndef ARY_CPP_C_TYPES4CPP_HXX
#define ARY_CPP_C_TYPES4CPP_HXX



// USED SERVICES
    // BASE CLASSES
    // OTHER
#include <cosv/tpl/range.hxx>
#include <ary/types.hxx>



namespace ary
{
namespace cpp
{

class Namespace;
class CodeEntity;
class Type;
class DefineEntity;
class Gate;
class CePilot;
class TypePilot;
class SecondariesPilot;
class Ce_Storage;
class Def_Storage;
class Type_Storage;
class UsedType;


typedef TypedId< ::ary::cpp::CodeEntity >       Ce_id;
typedef TypedId< ::ary::cpp::Type >             Type_id;
typedef TypedId< ::ary::cpp::DefineEntity >     De_id;


typedef std::vector<Ce_id>                      CesList;
typedef std::vector<De_id>                      DefsList;
typedef std::vector<Type_id>                    TypesList;

typedef std::vector<Ce_id>::const_iterator      CesConstIterator;
typedef std::vector<De_id>::const_iterator      DefsConstIterator;
typedef std::vector<Type_id>::const_iterator    TypesConstIterator;

typedef csv::range< CesConstIterator >          CesResultList;
typedef csv::range< DefsConstIterator >         DefsResultList;
typedef csv::range< TypesConstIterator >        TypesResultList;


typedef std::map<String, Ce_id>                 Map_LocalCe;



enum E_Protection
{
    PROTECT_global = 0,
    PROTECT_local,          /// within Functions
    PROTECT_public,
    PROTECT_protected,
    PROTECT_private
};

/** The sequence of E_Virtuality's values must not be changed,
    because they are used in int-comparisons.
*/
enum E_Virtuality
{
    VIRTUAL_none = 0,
    VIRTUAL_virtual,
    VIRTUAL_abstract
};

enum E_ClassKey
{
    CK_class,
    CK_struct,
    CK_union
};

enum E_TypeSpecialisation
{
    TYSP_none = 0,
    TYSP_unsigned,
    TYSP_signed
};

enum E_ConVol
{
    CONVOL_none = 0,
    CONVOL_const = 0x0001,
    CONVOL_volatile = 0x0002,
    CONVOL_both = 0x0003
};



// Backwards compatibility:
typedef Ce_id                           Cid;
typedef Type_id                         Tid;




}   //  namespace cpp
}   //  namespace ary
#endif
