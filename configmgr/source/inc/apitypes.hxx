/*************************************************************************
 *
 *  $RCSfile: apitypes.hxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: kr $ $Date: 2001-09-12 12:07:26 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef CONFIGMGR_API_APITYPES_HXX_
#define CONFIGMGR_API_APITYPES_HXX_

#ifndef _COM_SUN_STAR_UNO_TYPE_HXX_
#include <com/sun/star/uno/Type.hxx>
#endif
#ifndef _COM_SUN_STAR_UNO_REFERENCE_HXX_
#include <com/sun/star/uno/Reference.hxx>
#endif
#ifndef _COM_SUN_STAR_UNO_ANY_HXX_
#include <com/sun/star/uno/Any.hxx>
#endif
#ifndef _COM_SUN_STAR_UNO_SEQUENCE_HXX_
#include <com/sun/star/uno/Sequence.hxx>
#endif
#ifndef _COM_SUN_STAR_UNO_XINTERFACE_HPP_
#include <com/sun/star/uno/XInterface.hpp>
#endif

#ifndef _RTL_USTRING_HXX_
#include <rtl/ustring.hxx>
#endif

#ifndef INCLUDED_VECTOR
#include <vector>
#define INCLUDED_VECTOR
#endif  INCLUDED_VECTOR

namespace configmgr
{
    namespace css = ::com::sun::star;
    namespace uno = ::com::sun::star::uno;

    using ::std::vector;
    using ::rtl::OUString;

    namespace configapi
    {
        inline
        uno::Type getAnyType( )
        {
            return ::getCppuType( static_cast< uno::Any const * >(0) );
        }

        inline
        uno::Type getUnoInterfaceType( )
        {
            return ::getCppuType( static_cast< uno::Reference< uno::XInterface > const * >(0) );
        }

        template <typename Interface>
        inline
        uno::Type getReferenceType( Interface const* )
        {
            return ::getCppuType( static_cast< uno::Reference<Interface> const * >(0) );
        }

        template <typename Type>
        inline
        uno::Type getSequenceType( Type const* )
        {
            return ::getCppuType( static_cast< uno::Sequence<Type> const * >(0) );
        }

        template <typename T>
        inline
        uno::Sequence<T> makeSequence(vector<T> const& aVector)
        {
            if (aVector.empty())
                return uno::Sequence<T>();
            return uno::Sequence<T>(&aVector[0],aVector.size());
        }
    }

    // and one on the side
    class NotCopyable
    {
    protected:
        NotCopyable() {}
        ~NotCopyable() {}
    private:
        NotCopyable     (NotCopyable& notImplemented);
        void operator=  (NotCopyable& notImplemented);
    };

}

#endif // CONFIGMGR_API_APITYPES_HXX_


