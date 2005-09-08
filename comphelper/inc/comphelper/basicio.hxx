/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: basicio.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 02:27:43 $
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

#ifndef _COMPHELPER_BASIC_IO_HXX_
#define _COMPHELPER_BASIC_IO_HXX_

#ifndef _COM_SUN_STAR_IO_XPERSISTOBJECT_HPP_
#include <com/sun/star/io/XPersistObject.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_FONTDESCRIPTOR_HPP_
#include <com/sun/star/awt/FontDescriptor.hpp>
#endif

#ifndef INCLUDED_COMPHELPERDLLAPI_H
#include "comphelper/comphelperdllapi.h"
#endif

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

// ::rtl::OUString
COMPHELPER_DLLPUBLIC const staruno::Reference<stario::XObjectInputStream>& operator >> (const staruno::Reference<stario::XObjectInputStream>& _rxInStream, ::rtl::OUString& _rStr);
COMPHELPER_DLLPUBLIC const staruno::Reference<stario::XObjectOutputStream>& operator << (const staruno::Reference<stario::XObjectOutputStream>& _rxOutStream, const ::rtl::OUString& _rStr);

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

