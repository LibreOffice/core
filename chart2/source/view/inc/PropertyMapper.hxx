/*************************************************************************
 *
 *  $RCSfile: PropertyMapper.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: bm $ $Date: 2003-10-17 14:50:27 $
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
 *  Copyright: 2003 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/
#ifndef _CHART2_PROPERTYMAPPER_HXX
#define _CHART2_PROPERTYMAPPER_HXX

#include "InlineContainer.hxx"

#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSET_HPP_
#include <com/sun/star/beans/XPropertySet.hpp>
#endif
#ifndef _COM_SUN_STAR_DRAWING_XSHAPE_HPP_
#include <com/sun/star/drawing/XShape.hpp>
#endif

//.............................................................................
namespace chart
{
//.............................................................................

//-----------------------------------------------------------------------------
/**
*/

typedef ::std::map< ::rtl::OUString, ::rtl::OUString >            tPropertyNameMap;
typedef ::comphelper::MakeMap< ::rtl::OUString, ::rtl::OUString > tMakePropertyNameMap;

typedef ::std::map< ::rtl::OUString, ::com::sun::star::uno::Any >            tPropertyNameValueMap;
typedef ::comphelper::MakeMap< ::rtl::OUString, ::com::sun::star::uno::Any > tMakePropertyNameValueMap;

typedef ::com::sun::star::uno::Sequence< rtl::OUString > tNameSequence;
typedef ::comphelper::MakeSequence< rtl::OUString >      tMakeNameSequence;

typedef ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any > tAnySequence;
typedef ::comphelper::MakeSequence< ::com::sun::star::uno::Any >      tMakeAnySequence;

class PropertyMapper
{
public:
    static void setMappedProperties(
          const ::com::sun::star::uno::Reference<
                ::com::sun::star::beans::XPropertySet >& xTarget
        , const ::com::sun::star::uno::Reference<
                ::com::sun::star::beans::XPropertySet >& xSource
        , const tPropertyNameMap& rMap );

    static void getValueMap(
          tPropertyNameValueMap& rValueMap
        , const tPropertyNameMap& rNameMap
        , const ::com::sun::star::uno::Reference<
                ::com::sun::star::beans::XPropertySet >& xSourceProp
        );

    static void getMultiPropertyLists(
                  tNameSequence& rNames
                , tAnySequence&  rValues
                , const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& xProp
                , const tPropertyNameMap& rMap
                );

    static void getMultiPropertyListsFromValueMap(
                  tNameSequence& rNames
                , tAnySequence&  rValues
                , const tPropertyNameValueMap& rValueMap
                );

    static ::com::sun::star::uno::Any*
                getValuePointer( tAnySequence& rPropValues
                         , const tNameSequence& rPropNames
                         , const rtl::OUString& rPropName );

    static void setMultiProperties(
                  const tNameSequence& rNames
                , const tAnySequence&  rValues
                , const ::com::sun::star::uno::Reference<
                    ::com::sun::star::beans::XPropertySet >& xTarget );

    static const tMakePropertyNameMap& getPropertyNameMapForCharacterProperties();
    static const tMakePropertyNameMap& getPropertyNameMapForParagraphProperties();
    static const tMakePropertyNameMap& getPropertyNameMapForFillProperties();
    static const tMakePropertyNameMap& getPropertyNameMapForLineProperties();

    static const tMakePropertyNameMap& getPropertyNameMapForFilledSeriesProperties();
    static const tMakePropertyNameMap& getPropertyNameMapForLineSeriesProperties();
};

//.............................................................................
} //namespace chart
//.............................................................................
#endif
