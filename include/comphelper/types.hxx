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
    /** compare two FontDescriptor's
    */
    COMPHELPER_DLLPUBLIC bool   operator ==(const css::awt::FontDescriptor& _rLeft, const css::awt::FontDescriptor& _rRight);
    inline  bool    operator !=(const css::awt::FontDescriptor& _rLeft, const css::awt::FontDescriptor& _rRight)
    {
        return !(_rLeft == _rRight);
    }


    /// returns sal_True if objects of the types given are "compatible"
    COMPHELPER_DLLPUBLIC bool isAssignableFrom(const css::uno::Type& _rAssignable, const css::uno::Type& _rFrom);

    /** ask the given object for an XComponent interface and dispose on it
    */
    template <class TYPE>
    void disposeComponent(css::uno::Reference<TYPE>& _rxComp)
    {
        css::uno::Reference<css::lang::XComponent> xComp(_rxComp, css::uno::UNO_QUERY);
        if (xComp.is())
        {
            xComp->dispose();
            _rxComp = NULL;
        }
    }

    template <class TYPE>
    bool getImplementation(TYPE*& _pObject, const css::uno::Reference< css::uno::XInterface >& _rxIFace)
    {
        _pObject = NULL;
        css::uno::Reference< css::lang::XUnoTunnel > xTunnel(_rxIFace, css::uno::UNO_QUERY);
        if (xTunnel.is())
            _pObject = reinterpret_cast< TYPE* >(xTunnel->getSomething(TYPE::getUnoTunnelImplementationId()));

        return (_pObject != NULL);
    }



    /** get a css::awt::FontDescriptor that is fully initialized with
        the XXX_DONTKNOW enum values (which isn't the case if you instantiate it
        via the default constructor)
    */
    COMPHELPER_DLLPUBLIC css::awt::FontDescriptor    getDefaultFont();

    /** examine a sequence for the com.sun.star.uno::Type of it's elements.
    */
    COMPHELPER_DLLPUBLIC css::uno::Type getSequenceElementType(const css::uno::Type& _rSequenceType);


//= replacement of the former UsrAny.getXXX methods

    // may be used if you need the return value just as temporary, else it's may be too inefficient ....

    // no, we don't use templates here. This would lead to a lot of implicit uses of the conversion methods,
    // which would be difficult to trace ...

    COMPHELPER_DLLPUBLIC sal_Int64      getINT64(const css::uno::Any& _rAny);
    COMPHELPER_DLLPUBLIC sal_Int32      getINT32(const css::uno::Any& _rAny);
    COMPHELPER_DLLPUBLIC sal_Int16      getINT16(const css::uno::Any& _rAny);
    COMPHELPER_DLLPUBLIC double         getDouble(const css::uno::Any& _rAny);
    COMPHELPER_DLLPUBLIC float          getFloat(const css::uno::Any& _rAny);
    COMPHELPER_DLLPUBLIC OUString       getString(const css::uno::Any& _rAny);
    COMPHELPER_DLLPUBLIC bool           getBOOL(const css::uno::Any& _rAny);

    COMPHELPER_DLLPUBLIC sal_Int32      getEnumAsINT32(const css::uno::Any& _rAny) throw(css::lang::IllegalArgumentException);

//= replacement of some former UsrAny.setXXX methods - can be used with rvalues
    inline void setBOOL(css::uno::Any& _rAny, bool _b)
    { _rAny.setValue(&_b, cppu::UnoType<bool>::get()); }

//= extension of ::cppu::makeAny()
    inline css::uno::Any makeBoolAny(bool _b)
    { return css::uno::Any(&_b, cppu::UnoType<bool>::get()); }


}   // namespace comphelper


#endif // INCLUDED_COMPHELPER_TYPES_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
