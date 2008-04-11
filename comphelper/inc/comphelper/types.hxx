/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: types.hxx,v $
 * $Revision: 1.11 $
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

#ifndef _COMPHELPER_TYPES_HXX_
#define _COMPHELPER_TYPES_HXX_

#include <com/sun/star/uno/Any.hxx>
#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/uno/Sequence.hxx>
#include <com/sun/star/lang/IllegalArgumentException.hpp>
#include <com/sun/star/lang/XTypeProvider.hpp>
#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/lang/XUnoTunnel.hpp>
#include "comphelper/comphelperdllapi.h"
#include "cppu/unotype.hxx"

namespace com { namespace sun { namespace star { namespace awt {
    struct FontDescriptor;
} } } }

//.........................................................................
namespace comphelper
{
//.........................................................................

    namespace staruno       = ::com::sun::star::uno;
    namespace starawt       = ::com::sun::star::awt;
    namespace starlang      = ::com::sun::star::lang;

    typedef staruno::Reference< staruno::XInterface >           InterfaceRef;
    typedef staruno::Sequence< ::rtl::OUString >                StringSequence;

    //-------------------------------------------------------------------------
    /** compare the two given Anys
        The comparison is deep, means if one of the Any's contains an Any which contains an Any ..., this is resolved <br/>
        Other types recognized currently : FontDescriptor, ::com::sun::star::util::Date/Tim/DateTime, staruno::Sequence<sal_Int8>
    */
    COMPHELPER_DLLPUBLIC sal_Bool compare(const staruno::Any& rLeft, const staruno::Any& rRight);

    //-------------------------------------------------------------------------
    /** compare two FontDescriptor's
    */
    COMPHELPER_DLLPUBLIC sal_Bool   operator ==(const starawt::FontDescriptor& _rLeft, const starawt::FontDescriptor& _rRight);
    inline  sal_Bool    operator !=(const starawt::FontDescriptor& _rLeft, const starawt::FontDescriptor& _rRight)
    {
        return !(_rLeft == _rRight);
    }

    //-------------------------------------------------------------------------
    /// returns sal_True if objects of the types given are "compatible"
    COMPHELPER_DLLPUBLIC sal_Bool isAssignableFrom(const staruno::Type& _rAssignable, const staruno::Type& _rFrom);

    //-------------------------------------------------------------------------
    /** just a small shortcut ...
        check if a type you have at hand at runtime is equal to another type you have at compile time
        if all our compiler would accept function calls with explicit template arguments (like
        isA<classFoo>(runtimeType)), we wouldn't need the second parameter. But unfortunally at
        least the current solaris compiler doesn't allow this ....
        So this function is nearly senseless ....
    */
    template <class TYPE>
    sal_Bool isA(const staruno::Type& _rType, TYPE* pDummy)
    {
        return  _rType.equals(cppu::getTypeFavourUnsigned(pDummy));
    }

    //-------------------------------------------------------------------------
    /** check if a type you have at hand at runtime is equal to another type you have at compile time
        same comment as for the other isA ....
    */
    template <class TYPE>
    sal_Bool isA(const staruno::Any& _rVal, TYPE* pDummy)
    {
        return  _rVal.getValueType().equals(
            cppu::getTypeFavourUnsigned(pDummy));
    }

    //-------------------------------------------------------------------------
    /** check if a type you have at hand at runtime is equal to another type you have at compile time
    */
    template <class TYPE>
    sal_Bool isAReference(const staruno::Any& _rVal, TYPE* pDummy)
    {
        return  _rVal.getValueType().equals(
            cppu::getTypeFavourUnsigned(
                static_cast<staruno::Reference<TYPE>*>(NULL)));
    }

    //-------------------------------------------------------------------------
    /** ask the given object for an XComponent interface and dispose on it
    */
    template <class TYPE>
    void disposeComponent(staruno::Reference<TYPE>& _rxComp)
    {
        staruno::Reference<starlang::XComponent> xComp(_rxComp, staruno::UNO_QUERY);
        if (xComp.is())
        {
            xComp->dispose();
            _rxComp = NULL;
        }
    }
    //-------------------------------------------------------------------------
    template <class TYPE>
    sal_Bool getImplementation(TYPE*& _pObject, const staruno::Reference< staruno::XInterface >& _rxIFace)
    {
        _pObject = NULL;
        staruno::Reference< starlang::XUnoTunnel > xTunnel(_rxIFace, staruno::UNO_QUERY);
        if (xTunnel.is())
            _pObject = reinterpret_cast< TYPE* >(xTunnel->getSomething(TYPE::getUnoTunnelImplementationId()));

        return (_pObject != NULL);
    }


    //-------------------------------------------------------------------------
    /** get a com::sun::star::awt::FontDescriptor that is fully initialized with
        the XXX_DONTKNOW enum values (which isn't the case if you instantiate it
        via the default constructor)
    */
    COMPHELPER_DLLPUBLIC starawt::FontDescriptor    getDefaultFont();

    /** examine a sequence for the <type scope="com.sun.star.uno">Type</type> of it's elements.
    */
    COMPHELPER_DLLPUBLIC staruno::Type getSequenceElementType(const staruno::Type& _rSequenceType);

//=========================================================================
//= replacement of the former UsrAny.getXXX methods

    // may be used if you need the return value just as temporary, else it's may be too inefficient ....

    // no, we don't use templates here. This would lead to a lot of implicit uses of the conversion methods,
    // which would be difficult to trace ...

    COMPHELPER_DLLPUBLIC sal_Int32      getINT32(const staruno::Any& _rAny);
    COMPHELPER_DLLPUBLIC sal_Int16      getINT16(const staruno::Any& _rAny);
    COMPHELPER_DLLPUBLIC double         getDouble(const staruno::Any& _rAny);
    COMPHELPER_DLLPUBLIC float          getFloat(const staruno::Any& _rAny);
    COMPHELPER_DLLPUBLIC ::rtl::OUString    getString(const staruno::Any& _rAny);
    COMPHELPER_DLLPUBLIC sal_Bool       getBOOL(const staruno::Any& _rAny);

    COMPHELPER_DLLPUBLIC sal_Int32      getEnumAsINT32(const staruno::Any& _rAny) throw(starlang::IllegalArgumentException);

//= replacement of some former UsrAny.setXXX methods - can be used with rvalues
    inline void setBOOL(staruno::Any& _rAny, sal_Bool _b)
    { _rAny.setValue(&_b, ::getBooleanCppuType()); }

//= extension of ::cppu::makeAny()
    inline staruno::Any makeBoolAny(sal_Bool _b)
    { return staruno::Any(&_b, ::getBooleanCppuType()); }

//.........................................................................
}   // namespace comphelper
//.........................................................................

#endif // _COMPHELPER_TYPES_HXX_

