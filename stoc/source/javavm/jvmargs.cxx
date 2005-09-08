/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: jvmargs.cxx,v $
 *
 *  $Revision: 1.16 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 07:59:29 $
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

#include "jvmargs.hxx"
#include <rtl/ustring.hxx>


#define OUSTR(x) rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( x ))

using namespace rtl;

namespace stoc_javavm {

JVM::JVM() throw()//: _enabled(sal_False)
{
}

void JVM::pushProp(const OUString & property)
{
    sal_Int32 index = property.indexOf((sal_Unicode)'=');
    if(index > 0)
    {
        OUString left = property.copy(0, index).trim();
        OUString right(property.copy(index + 1).trim());
        _props.push_back(property);
    }
    else
    { // no '=', could be -X
        _props.push_back(property);
    }
}


const ::std::vector< ::rtl::OUString > & JVM::getProperties() const
{
    return _props;
}

}
