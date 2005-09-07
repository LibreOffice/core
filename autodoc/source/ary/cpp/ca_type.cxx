/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: ca_type.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 16:34:15 $
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


#include <precomp.h>
#include <ary/cpp/ca_type.hxx>


// NOT FULLY DEFINED SERVICES


namespace ary
{
namespace cpp
{




//**********************        Type        **************************//
Rid
Type::inq_RelatedCe() const
{
     return 0;
}


//**********************        BuiltInType        **************************//

BuiltInType::BuiltInType( Tid                   i_nId,
                          const udmstri &       i_sName,
                          E_TypeSpecialisation  i_eSpecialisation )
    :   nId( i_nId ),
        sName( i_sName ),
        eSpecialisation( i_eSpecialisation )
{
}

Tid
BuiltInType::inq_Id_Type() const
{
    return nId;
}

bool
BuiltInType::inq_IsConst() const
{
    return false;
}

void
BuiltInType::inq_Get_Text( StreamStr &          o_rPreName,
                           StreamStr &          o_rName,
                           StreamStr &          o_rPostName,
                           const DisplayGate &  i_rGate ) const
{
    switch (eSpecialisation)
    {
        case TYSP_unsigned: o_rName << "unsigned "; break;
        case TYSP_signed:   o_rName << "signed ";   break;

        default:            // Does nothing.
                            ;
    }
    o_rName << sName;
}


//**********************        NullType        **************************//

Tid
NullType::inq_Id_Type() const
{
    return 0;
}

bool
NullType::inq_IsConst() const
{
    return false;
}

void
NullType::inq_Get_Text( StreamStr &          o_rPreName,
                        StreamStr &          o_rName,
                        StreamStr &          o_rPostName,
                        const DisplayGate &  i_rGate ) const
{
}




#if 0
void
NamedType::GetText( StreamStr &     o_rOut,
                    const Gate &    i_rGate ) const
{
    i_rGate.Get_QualifiedName(o_rOut, Name(), "::");
}


BuiltInType::BuiltInType( const S_InitData &  i_rData )
    :   nId(i_rData.nId),
        aName(i_rData.aName)
{
}

Tid
BuiltInType::IdAsType() const
{
    return nId;
}

const QName &
BuiltInType::Name() const
{
    return aName;
}

#if 0
PredeclaredType::PredeclaredType( Tid                 i_nId,
                                  const char *        i_sName,
                                  Cid                 i_nOwner )
    :   nId(i_nId),
        aName(i_sName,i_nOwner)
{
}

Tid
PredeclaredType::IdAsType() const
{
    return nId;
}

const QName &
PredeclaredType::Name() const
{
    return aName;
}
#endif // 0

Tid
ReferingType::IdAsType() const
{
    return nId;
}

Tid
ReferingType::ReferedType() const
{
    return nReferedType;
}

ReferingType::ReferingType( Tid i_nId,
                            Tid i_nReferedType )
    :   nId(i_nId),
        nReferedType(i_nReferedType)
{
}

ConstType::ConstType( Tid                 nId,
                      Tid                 nReferedType )
    :   ReferingType(nId, nReferedType)
{
}

void
ConstType::GetText( ostream &           o_rOut,
                    const Gate &        i_rGate ) const
{
    i_rGate.Get_TypeText(o_rOut,ReferedType());
    o_rOut << " const";
}

VolatileType::VolatileType( Tid                 nId,
                            Tid                 nReferedType )
    :   ReferingType(nId, nReferedType)
{
}

void
VolatileType::GetText( ostream &           o_rOut,
                       const Gate &        i_rGate ) const
{
    i_rGate.Get_TypeText(o_rOut,ReferedType());
    o_rOut << " volatile";
}

PtrType::PtrType( Tid                 nId,
                  Tid                 nReferedType )
    :   ReferingType(nId, nReferedType)
{
}

void
PtrType::GetText( ostream &           o_rOut,
                    const Gate &      i_rGate ) const
{
    i_rGate.Get_TypeText(o_rOut,ReferedType());
    o_rOut << " *";
}

RefType::RefType( Tid                 nId,
                  Tid                 nReferedType )
    :   ReferingType(nId, nReferedType)
{
}

void
RefType::GetText( ostream &           o_rOut,
                  const Gate &        i_rGate ) const
{
    i_rGate.Get_TypeText(o_rOut,ReferedType());
    o_rOut << " &";
}

TemplateInstance::TemplateInstance( Tid                 i_nId,
                                    Cid                 i_nReferedClass,
                                    const char *        i_sInstantiation )
    :   nId(i_nId),
        nReferedClass(i_nReferedClass),
        sInstantiation(i_sInstantiation)
{
}

bool
TemplateInstance::operator<( const TemplateInstance & i_r ) const
{
    if ( nReferedClass < i_r.nReferedClass )
        return true;
    if ( nReferedClass == i_r.nReferedClass
         AND sInstantiation < i_r.sInstantiation )
        return true;
    return false;
}

Tid
TemplateInstance::IdAsType() const
{
    return nId;
}

void
TemplateInstance::GetText( ostream &        o_rOut,
                           const Gate &     i_rGate ) const
{
    i_rGate.Get_TypeText(o_rOut,nReferedClass);
    o_rOut << "< " << sInstantiation << " >";
}
#endif // 0


}   // namespace cpp
}   // namespace ary

