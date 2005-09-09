/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: UCBDeadPropertyValue.hxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 16:14:52 $
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

#ifndef _UCBDEADPROPERTYVALUE_HXX_
#define _UCBDEADPROPERTYVALUE_HXX_

#ifndef _RTL_STRING_HXX_
#include <rtl/string.hxx>
#endif
#ifndef _COM_SUN_STAR_UNO_ANY_HXX_
#include <com/sun/star/uno/Any.hxx>
#endif

namespace webdav_ucp
{

class UCBDeadPropertyValue
{
private:
    static const rtl::OUString aTypeString;
    static const rtl::OUString aTypeLong;
    static const rtl::OUString aTypeShort;
    static const rtl::OUString aTypeBoolean;
    static const rtl::OUString aTypeChar;
    static const rtl::OUString aTypeByte;
    static const rtl::OUString aTypeHyper;
    static const rtl::OUString aTypeFloat;
    static const rtl::OUString aTypeDouble;

    static const rtl::OUString aXMLPre;
    static const rtl::OUString aXMLMid;
    static const rtl::OUString aXMLEnd;

public:
    static bool supportsType( const com::sun::star::uno::Type & rType );

    static bool createFromXML( const rtl::OString & rInData,
                               com::sun::star::uno::Any & rOutData );
    static bool toXML( const com::sun::star::uno::Any & rInData,
                       rtl::OUString & rOutData );
};

}

#endif /* _UCBDEADPROPERTYVALUE_HXX_ */
