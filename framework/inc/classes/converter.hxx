/*************************************************************************
 *
 *  $RCSfile: converter.hxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: hr $ $Date: 2003-04-04 16:01:27 $
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

#ifndef __FRAMEWORK_CLASSES_CONVERTER_HXX_
#define __FRAMEWORK_CLASSES_CONVERTER_HXX_

//_________________________________________________________________________________________________________________
//  my own includes
//_________________________________________________________________________________________________________________

#include <general.h>
#include <stdtypes.h>

//_________________________________________________________________________________________________________________
//  interface includes
//_________________________________________________________________________________________________________________

//_________________________________________________________________________________________________________________
//  other includes
//_________________________________________________________________________________________________________________

#ifndef _COM_SUN_STAR_UNO_SEQUENCE_HXX_
#include <com/sun/star/uno/Sequence.hxx>
#endif

#ifndef _COM_SUN_STAR_UNO_ANY_HXX_
#include <com/sun/star/uno/Any.hxx>
#endif

#ifndef _COM_SUN_STAR_BEANS_PROPERTYVALUE_HPP_
#include <com/sun/star/beans/PropertyValue.hpp>
#endif

#ifndef _COM_SUN_STAR_BEANS_NAMEDVALUE_HPP_
#include <com/sun/star/beans/NamedValue.hpp>
#endif

#ifndef _DATETIME_HXX
#include <tools/datetime.hxx>
#endif

//_________________________________________________________________________________________________________________
//  namespace
//_________________________________________________________________________________________________________________

namespace framework{

//_________________________________________________________________________________________________________________
//  exported const
//_________________________________________________________________________________________________________________

//_________________________________________________________________________________________________________________
//  exported definitions
//_________________________________________________________________________________________________________________

class Converter
{
    public:
        // Seq<Any> <=> Seq<beans.PropertyValue>
        static css::uno::Sequence< css::uno::Any >             convert_seqProp2seqAny          ( const css::uno::Sequence< css::beans::PropertyValue >& lSource );
        static css::uno::Sequence< css::beans::PropertyValue > convert_seqAny2seqProp          ( const css::uno::Sequence< css::uno::Any >&             lSource );

        // Seq<beans.NamedValue> <=> Seq<beans.PropertyValue>
        static css::uno::Sequence< css::beans::PropertyValue > convert_seqNamedVal2seqPropVal  ( const css::uno::Sequence< css::beans::NamedValue >&    lSource );
        static css::uno::Sequence< css::beans::NamedValue >    convert_seqPropVal2seqNamedVal  ( const css::uno::Sequence< css::beans::PropertyValue >& lSource );

        // Seq<String> => Vector<String>
        static OUStringList                                    convert_seqOUString2OUStringList( const css::uno::Sequence< ::rtl::OUString >&           lSource );
        static css::uno::Sequence< ::rtl::OUString >           convert_OUStringList2seqOUString( const OUStringList&                                    lSource );

        static css::uno::Sequence< css::beans::PropertyValue > convert_OUStringHash2seqProp    ( const OUStringHash&                                    lSource );
        static OUStringHash                                    convert_seqProp2OUStringHash    ( const css::uno::Sequence< css::beans::PropertyValue >& lSource );

        // String <=> tools.DateTime
        static DateTime                                        convert_String2DateTime         ( const ::rtl::OUString&                                 sSource );
        static ::rtl::OUString                                 convert_DateTime2String         ( const DateTime&                                        aSource );
};

}       // namespace framework

#endif  // #ifndef __FRAMEWORK_CLASSES_CONVERTER_HXX_
