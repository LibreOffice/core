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

#ifndef _COMPHELPER_BASIC_IO_HXX_
#define _COMPHELPER_BASIC_IO_HXX_

#include <com/sun/star/io/XPersistObject.hpp>
#include <com/sun/star/awt/FontDescriptor.hpp>
#include "comphelper/comphelperdllapi.h"

//.........................................................................
namespace comphelper
{
//.........................................................................

namespace stario    = ::com::sun::star::io;
namespace staruno   = ::com::sun::star::uno;
namespace starawt   = ::com::sun::star::awt;

// sal_Bool
COMPHELPER_DLLPUBLIC const staruno::Reference<stario::XObjectInputStream>& operator >> (const staruno::Reference<stario::XObjectInputStream>& _rxInStream, sal_Bool& _rVal);
COMPHELPER_DLLPUBLIC const staruno::Reference<stario::XObjectOutputStream>& operator << (const staruno::Reference<stario::XObjectOutputStream>& _rxOutStream, sal_Bool _bVal);

// OUString
COMPHELPER_DLLPUBLIC const staruno::Reference<stario::XObjectInputStream>& operator >> (const staruno::Reference<stario::XObjectInputStream>& _rxInStream, OUString& _rStr);
COMPHELPER_DLLPUBLIC const staruno::Reference<stario::XObjectOutputStream>& operator << (const staruno::Reference<stario::XObjectOutputStream>& _rxOutStream, const OUString& _rStr);

// sal_Int16
COMPHELPER_DLLPUBLIC const staruno::Reference<stario::XObjectInputStream>& operator >> (const staruno::Reference<stario::XObjectInputStream>& _rxInStream, sal_Int16& _rValue);
COMPHELPER_DLLPUBLIC const staruno::Reference<stario::XObjectOutputStream>& operator << (const staruno::Reference<stario::XObjectOutputStream>& _rxOutStream, sal_Int16 _nValue);

// sal_uInt16
COMPHELPER_DLLPUBLIC const staruno::Reference<stario::XObjectInputStream>& operator >> (const staruno::Reference<stario::XObjectInputStream>& _rxInStream, sal_uInt16& _rValue);
COMPHELPER_DLLPUBLIC const staruno::Reference<stario::XObjectOutputStream>& operator << (const staruno::Reference<stario::XObjectOutputStream>& _rxOutStream, sal_uInt16 _nValue);

// sal_uInt32
COMPHELPER_DLLPUBLIC const staruno::Reference<stario::XObjectInputStream>& operator >> (const staruno::Reference<stario::XObjectInputStream>& _rxInStream, sal_uInt32& _rValue);
COMPHELPER_DLLPUBLIC const staruno::Reference<stario::XObjectOutputStream>& operator << (const staruno::Reference<stario::XObjectOutputStream>& _rxOutStream, sal_uInt32 _nValue);

// sal_Int16
COMPHELPER_DLLPUBLIC const staruno::Reference<stario::XObjectInputStream>& operator >> (const staruno::Reference<stario::XObjectInputStream>& _rxInStream, sal_Int32& _rValue);
COMPHELPER_DLLPUBLIC const staruno::Reference<stario::XObjectOutputStream>& operator << (const staruno::Reference<stario::XObjectOutputStream>& _rxOutStream, sal_Int32 _nValue);

// FontDescriptor
COMPHELPER_DLLPUBLIC const staruno::Reference<stario::XObjectInputStream>& operator >> (const staruno::Reference<stario::XObjectInputStream>& InStream, starawt::FontDescriptor& rVal);
COMPHELPER_DLLPUBLIC const staruno::Reference<stario::XObjectOutputStream>& operator << (const staruno::Reference<stario::XObjectOutputStream>& OutStream, const starawt::FontDescriptor& rVal);

// sequences
template <class ELEMENT>
const staruno::Reference<stario::XObjectInputStream>& operator >> (const staruno::Reference<stario::XObjectInputStream>& _rxInStream, staruno::Sequence<ELEMENT>& _rSeq)
{
    sal_Int32 nLen = _rxInStream->readLong();
    _rSeq.realloc(nLen);
    if (nLen)
    {
        ELEMENT* pElement = _rSeq.getArray();
        for (sal_Int32 i=0; i<nLen; ++i, ++pElement)
            _rxInStream >> *pElement;
    }
    return _rxInStream;
}

template <class ELEMENT>
const staruno::Reference<stario::XObjectOutputStream>& operator << (const staruno::Reference<stario::XObjectOutputStream>& _rxOutStream, const staruno::Sequence<ELEMENT>& _rSeq)
{
    sal_Int32 nLen = _rSeq.getLength();
    _rxOutStream->writeLong(nLen);
    if (nLen)
    {
        const ELEMENT* pElement = _rSeq.getConstArray();
        for (sal_Int32 i = 0; i < nLen; ++i, ++pElement)
            _rxOutStream << *pElement;
    }
    return _rxOutStream;
}

//.........................................................................
}   // namespace comphelper
//.........................................................................

#endif // _COMPHELPER_BASIC_IO_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
