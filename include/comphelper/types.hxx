/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#ifndef INCLUDED_COMPHELPER_TYPES_HXX
#define INCLUDED_COMPHELPER_TYPES_HXX

#include <com/sun/star/uno/Any.hxx>
#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/uno/Sequence.hxx>
#include <com/sun/star/lang/IllegalArgumentException.hpp>
#include <com/sun/star/lang/XTypeProvider.hpp>
#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/lang/XUnoTunnel.hpp>
#include <comphelper/comphelperdllapi.h>
#include <cppu/unotype.hxx>

namespace com { namespace sun { namespace star { namespace awt {
    struct FontDescriptor;
} } } }


namespace comphelper
{


    namespace staruno       = ::com::sun::star::uno;
    namespace starawt       = ::com::sun::star::awt;
    namespace starlang      = ::com::sun::star::lang;

    typedef staruno::Reference< staruno::XInterface >           InterfaceRef;
    typedef staruno::Sequence< OUString >                StringSequence;


    /** compare the two given Anys
        The comparison is deep, means if one of the Any's contains an Any which contains an Any ..., this is resolved <br/>
        Other types recognized currently : FontDescriptor, ::com::sun::star::util::Date/Tim/DateTime, staruno::Sequence<sal_Int8>
    */
    COMPHELPER_DLLPUBLIC bool compare(const staruno::Any& rLeft, const staruno::Any& rRight);


    /** compare two FontDescriptor's
    */
    COMPHELPER_DLLPUBLIC bool   operator ==(const starawt::FontDescriptor& _rLeft, const starawt::FontDescriptor& _rRight);
    inline  bool    operator !=(const starawt::FontDescriptor& _rLeft, const starawt::FontDescriptor& _rRight)
    {
        return !(_rLeft == _rRight);
    }


    /// returns sal_True if objects of the types given are "compatible"
    COMPHELPER_DLLPUBLIC bool isAssignableFrom(const staruno::Type& _rAssignable, const staruno::Type& _rFrom);


    /** just a small shortcut ...
        check if a type you have at hand at runtime is equal to another type you have at compile time
        if all our compiler would accept function calls with explicit template arguments (like
        isA<classFoo>(runtimeType)), we wouldn't need the second parameter. But unfortunally at
        least the current solaris compiler doesn't allow this ....
        So this function is nearly senseless ....
    */
    template <class TYPE>
    bool isA(const staruno::Type& _rType, TYPE* pDummy)
    {
        return  _rType.equals(cppu::getTypeFavourUnsigned(pDummy));
    }


    /** check if a type you have at hand at runtime is equal to another type you have at compile time
        same comment as for the other isA ....
    */
    template <class TYPE>
    bool isA(const staruno::Any& _rVal, TYPE* pDummy)
    {
        return  _rVal.getValueType().equals(
            cppu::getTypeFavourUnsigned(pDummy));
    }


    /** check if a type you have at hand at runtime is equal to another type you have at compile time
    */
    template <class TYPE>
    bool isAReference(const staruno::Any& _rVal, TYPE*)
    {
        return  _rVal.getValueType().equals(
            cppu::getTypeFavourUnsigned(
                static_cast<staruno::Reference<TYPE>*>(NULL)));
    }


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

    template <class TYPE>
    bool getImplementation(TYPE*& _pObject, const staruno::Reference< staruno::XInterface >& _rxIFace)
    {
        _pObject = NULL;
        staruno::Reference< starlang::XUnoTunnel > xTunnel(_rxIFace, staruno::UNO_QUERY);
        if (xTunnel.is())
            _pObject = reinterpret_cast< TYPE* >(xTunnel->getSomething(TYPE::getUnoTunnelImplementationId()));

        return (_pObject != NULL);
    }



    /** get a com::sun::star::awt::FontDescriptor that is fully initialized with
        the XXX_DONTKNOW enum values (which isn't the case if you instantiate it
        via the default constructor)
    */
    COMPHELPER_DLLPUBLIC starawt::FontDescriptor    getDefaultFont();

    /** examine a sequence for the com.sun.star.uno::Type of it's elements.
    */
    COMPHELPER_DLLPUBLIC staruno::Type getSequenceElementType(const staruno::Type& _rSequenceType);


//= replacement of the former UsrAny.getXXX methods

    // may be used if you need the return value just as temporary, else it's may be too inefficient ....

    // no, we don't use templates here. This would lead to a lot of implicit uses of the conversion methods,
    // which would be difficult to trace ...

    COMPHELPER_DLLPUBLIC sal_Int64      getINT64(const staruno::Any& _rAny);
    COMPHELPER_DLLPUBLIC sal_Int32      getINT32(const staruno::Any& _rAny);
    COMPHELPER_DLLPUBLIC sal_Int16      getINT16(const staruno::Any& _rAny);
    COMPHELPER_DLLPUBLIC double         getDouble(const staruno::Any& _rAny);
    COMPHELPER_DLLPUBLIC float          getFloat(const staruno::Any& _rAny);
    COMPHELPER_DLLPUBLIC OUString    getString(const staruno::Any& _rAny);
    COMPHELPER_DLLPUBLIC bool       getBOOL(const staruno::Any& _rAny);

    COMPHELPER_DLLPUBLIC sal_Int32      getEnumAsINT32(const staruno::Any& _rAny) throw(starlang::IllegalArgumentException);

//= replacement of some former UsrAny.setXXX methods - can be used with rvalues
    inline void setBOOL(staruno::Any& _rAny, bool _b)
    { _rAny.setValue(&_b, ::getBooleanCppuType()); }

//= extension of ::cppu::makeAny()
    inline staruno::Any makeBoolAny(bool _b)
    { return staruno::Any(&_b, ::getBooleanCppuType()); }


}   // namespace comphelper


#endif // INCLUDED_COMPHELPER_TYPES_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
