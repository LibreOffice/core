/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: FilterConfigItem.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-04-11 19:10:35 $
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


#ifndef _FILTER_CONFIG_ITEM_HXX_
#define _FILTER_CONFIG_ITEM_HXX_

#ifndef INCLUDED_SVTDLLAPI_H
#include "svtools/svtdllapi.h"
#endif

#ifndef _STRING_HXX
#include <tools/string.hxx>
#endif
#ifndef _COM_SUN_STAR_AWT_SIZE_HPP_
#include <com/sun/star/awt/Size.hpp>
#endif

#ifndef _COM_SUN_STAR_UNO_ANY_H_
#include <com/sun/star/uno/Any.h>
#endif
#ifndef _COM_SUN_STAR_UNO_REFERENCE_H_
#include <com/sun/star/uno/Reference.h>
#endif
#ifndef _COM_SUN_STAR_UNO_XINTERFACE_HPP_
#include <com/sun/star/uno/XInterface.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSET_HPP_
#include <com/sun/star/beans/XPropertySet.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYACCESS_HPP_
#include <com/sun/star/beans/XPropertyAccess.hpp>
#endif
#ifndef _COM_SUN_STAR_TASK_XSTATUSINDICATOR_HPP_
#include <com/sun/star/task/XStatusIndicator.hpp>
#endif

class SVT_DLLPUBLIC FilterConfigItem
{
        ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > xUpdatableView;
        ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet > xPropSet;
        ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue > aFilterData;

        sal_Bool    bModified;

        SVT_DLLPRIVATE sal_Bool ImplGetPropertyValue( ::com::sun::star::uno::Any& rAny,
                        const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& rXPropSet,
                            const ::rtl::OUString& rPropName,
                                sal_Bool bTestPropertyAvailability );

        SVT_DLLPRIVATE void     ImpInitTree( const String& rTree );


        SVT_DLLPRIVATE static ::com::sun::star::beans::PropertyValue* GetPropertyValue(
                    ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& rPropSeq,
                        const ::rtl::OUString& rName );
        SVT_DLLPRIVATE static  sal_Bool WritePropertyValue(
                    ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& rPropSeq,
                        const ::com::sun::star::beans::PropertyValue& rPropValue );

    public :

        FilterConfigItem( const ::rtl::OUString& rSubTree );
        FilterConfigItem( ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >* pFilterData );
        FilterConfigItem( const ::rtl::OUString& rSubTree, ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >* pFilterData );
        ~FilterConfigItem();

        // all read methods are trying to return values in following order:
        // 1. FilterData PropertySequence
        // 2. configuration
        // 3. given default
        sal_Bool    ReadBool( const ::rtl::OUString& rKey, sal_Bool bDefault );
        sal_Int32   ReadInt32( const ::rtl::OUString& rKey, sal_Int32 nDefault );
        ::com::sun::star::awt::Size
                    ReadSize( const ::rtl::OUString& rKey, const ::com::sun::star::awt::Size& rDefault );
        ::rtl::OUString
                    ReadString( const ::rtl::OUString& rKey, const ::rtl::OUString& rDefault );
        ::com::sun::star::uno::Any
                    ReadAny( const ::rtl::OUString& rKey, const ::com::sun::star::uno::Any& rDefault );

        // try to store to configuration
        // and always stores into the FilterData sequence
        void        WriteBool( const ::rtl::OUString& rKey, sal_Bool bValue );
        void        WriteInt32( const ::rtl::OUString& rKey, sal_Int32 nValue );
        void        WriteSize( const ::rtl::OUString& rKey, const ::com::sun::star::awt::Size& rSize );
        void        WriteString( const ::rtl::OUString& rKey, const ::rtl::OUString& rString );
        void        WriteAny( const rtl::OUString& rKey, const ::com::sun::star::uno::Any& rAny );

        ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue > GetFilterData() const;

        // GetStatusIndicator is returning the "StatusIndicator" property of the FilterData sequence
        ::com::sun::star::uno::Reference< ::com::sun::star::task::XStatusIndicator > GetStatusIndicator() const;
};

#endif  // _FILTER_CONFIG_ITEM_HXX_

