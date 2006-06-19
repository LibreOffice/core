/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: crenum.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: hr $ $Date: 2006-06-20 00:00:54 $
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

#include "base.hxx"

namespace stoc_corefl
{

//==================================================================================================
class IdlEnumFieldImpl
    : public IdlMemberImpl
    , public XIdlField
    , public XIdlField2
{
    sal_Int32               _nValue;

public:
    IdlEnumFieldImpl( IdlReflectionServiceImpl * pReflection, const OUString & rName,
                      typelib_TypeDescription * pTypeDescr, sal_Int32 nValue )
        : IdlMemberImpl( pReflection, rName, pTypeDescr, pTypeDescr )
        , _nValue( nValue )
        {}
    virtual ~IdlEnumFieldImpl();

    // XInterface
    virtual Any SAL_CALL queryInterface( const Type & rType ) throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL acquire() throw();
    virtual void SAL_CALL release() throw();

    // XTypeProvider
    virtual Sequence< Type > SAL_CALL getTypes() throw (::com::sun::star::uno::RuntimeException);
    virtual Sequence< sal_Int8 > SAL_CALL getImplementationId() throw (::com::sun::star::uno::RuntimeException);

    // XIdlMember
    virtual Reference< XIdlClass > SAL_CALL getDeclaringClass() throw(::com::sun::star::uno::RuntimeException);
    virtual OUString SAL_CALL getName() throw(::com::sun::star::uno::RuntimeException);
    // XIdlField
    virtual Reference< XIdlClass > SAL_CALL getType() throw(::com::sun::star::uno::RuntimeException);
    virtual FieldAccessMode SAL_CALL getAccessMode() throw(::com::sun::star::uno::RuntimeException);
    virtual Any SAL_CALL get( const Any & rObj ) throw(::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL set( const Any & rObj, const Any & rValue ) throw(::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::lang::IllegalAccessException, ::com::sun::star::uno::RuntimeException);
    // XIdlField2: getType, getAccessMode and get are equal to XIdlField
    virtual void SAL_CALL set( Any & rObj, const Any & rValue ) throw(::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::lang::IllegalAccessException, ::com::sun::star::uno::RuntimeException);
};
//__________________________________________________________________________________________________
IdlEnumFieldImpl::~IdlEnumFieldImpl()
{
}

// XInterface
//__________________________________________________________________________________________________
Any IdlEnumFieldImpl::queryInterface( const Type & rType )
    throw(::com::sun::star::uno::RuntimeException)
{
    Any aRet( ::cppu::queryInterface( rType,
                                      static_cast< XIdlField * >( this ),
                                      static_cast< XIdlField2 * >( this ) ) );
    return (aRet.hasValue() ? aRet : IdlMemberImpl::queryInterface( rType ));
}
//__________________________________________________________________________________________________
void IdlEnumFieldImpl::acquire() throw()
{
    IdlMemberImpl::acquire();
}
//__________________________________________________________________________________________________
void IdlEnumFieldImpl::release() throw()
{
    IdlMemberImpl::release();
}

// XTypeProvider
//__________________________________________________________________________________________________
Sequence< Type > IdlEnumFieldImpl::getTypes()
    throw (::com::sun::star::uno::RuntimeException)
{
    static OTypeCollection * s_pTypes = 0;
    if (! s_pTypes)
    {
        MutexGuard aGuard( getMutexAccess() );
        if (! s_pTypes)
        {
            static OTypeCollection s_aTypes(
                ::getCppuType( (const Reference< XIdlField2 > *)0 ),
                ::getCppuType( (const Reference< XIdlField > *)0 ),
                IdlMemberImpl::getTypes() );
            s_pTypes = &s_aTypes;
        }
    }
    return s_pTypes->getTypes();
}
//__________________________________________________________________________________________________
Sequence< sal_Int8 > IdlEnumFieldImpl::getImplementationId()
    throw (::com::sun::star::uno::RuntimeException)
{
    static OImplementationId * s_pId = 0;
    if (! s_pId)
    {
        MutexGuard aGuard( getMutexAccess() );
        if (! s_pId)
        {
            static OImplementationId s_aId;
            s_pId = &s_aId;
        }
    }
    return s_pId->getImplementationId();
}

// XIdlMember
//__________________________________________________________________________________________________
Reference< XIdlClass > IdlEnumFieldImpl::getDeclaringClass()
    throw(::com::sun::star::uno::RuntimeException)
{
    return IdlMemberImpl::getDeclaringClass();
}
//__________________________________________________________________________________________________
OUString IdlEnumFieldImpl::getName()
    throw(::com::sun::star::uno::RuntimeException)
{
    return IdlMemberImpl::getName();
}

// XIdlField
//__________________________________________________________________________________________________
Reference< XIdlClass > IdlEnumFieldImpl::getType()
    throw(::com::sun::star::uno::RuntimeException)
{
    return getDeclaringClass();
}
//__________________________________________________________________________________________________
FieldAccessMode IdlEnumFieldImpl::getAccessMode()
    throw(::com::sun::star::uno::RuntimeException)
{
    return FieldAccessMode_READONLY;
}
//__________________________________________________________________________________________________
Any IdlEnumFieldImpl::get( const Any & )
    throw(::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException)
{
    return Any( &_nValue, getTypeDescr() );
}
//__________________________________________________________________________________________________
void IdlEnumFieldImpl::set( const Any &, const Any & )
    throw(::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::lang::IllegalAccessException, ::com::sun::star::uno::RuntimeException)
{
    throw IllegalAccessException(
        OUString( RTL_CONSTASCII_USTRINGPARAM("enum field is constant!") ),
        (XWeak *)(OWeakObject *)this );
}
//__________________________________________________________________________________________________
void IdlEnumFieldImpl::set( Any &, const Any & )
    throw(::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::lang::IllegalAccessException, ::com::sun::star::uno::RuntimeException)
{
    throw IllegalAccessException(
        OUString( RTL_CONSTASCII_USTRINGPARAM("enum field is constant!") ),
        (XWeak *)(OWeakObject *)this );
}

//##################################################################################################
//##################################################################################################
//##################################################################################################


//__________________________________________________________________________________________________
EnumIdlClassImpl::~EnumIdlClassImpl()
{
    delete _pFields;
}

// IdlClassImpl modifications
//__________________________________________________________________________________________________
Reference< XIdlField > EnumIdlClassImpl::getField( const OUString & rName )
    throw(::com::sun::star::uno::RuntimeException)
{
    if (! _pFields)
        getFields(); // init members

    const OUString2Field::const_iterator iFind( _aName2Field.find( rName ) );
    if (iFind != _aName2Field.end())
        return (*iFind).second;
    else
        return Reference< XIdlField >();
}
//__________________________________________________________________________________________________
Sequence< Reference< XIdlField > > EnumIdlClassImpl::getFields()
    throw(::com::sun::star::uno::RuntimeException)
{
    if (! _pFields)
    {
        MutexGuard aGuard( getMutexAccess() );
        if (! _pFields)
        {
            sal_Int32 nFields = getTypeDescr()->nEnumValues;
            Sequence< Reference< XIdlField > > * pFields =
                new Sequence< Reference< XIdlField > >( nFields );
            Reference< XIdlField > * pSeq = pFields->getArray();

            while (nFields--)
            {
                OUString aName( getTypeDescr()->ppEnumNames[nFields] );
                _aName2Field[aName] = pSeq[nFields] = new IdlEnumFieldImpl(
                    getReflection(), aName, IdlClassImpl::getTypeDescr(), getTypeDescr()->pEnumValues[nFields] );
            }

            _pFields = pFields;
        }
    }
    return *_pFields;
}
//__________________________________________________________________________________________________
void EnumIdlClassImpl::createObject( Any & rObj )
    throw(::com::sun::star::uno::RuntimeException)
{
    sal_Int32 eVal =
        ((typelib_EnumTypeDescription *)IdlClassImpl::getTypeDescr())->nDefaultEnumValue;
    rObj.setValue( &eVal, IdlClassImpl::getTypeDescr() );
}

}


