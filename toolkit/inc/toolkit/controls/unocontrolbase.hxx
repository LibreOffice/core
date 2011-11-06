/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



#ifndef _TOOLKIT_AWT_UNOCONTROLBASE_HXX_
#define _TOOLKIT_AWT_UNOCONTROLBASE_HXX_

#include <com/sun/star/awt/Size.hpp>

#include <toolkit/controls/unocontrol.hxx>

//  ----------------------------------------------------
//  class UnoControlBase
//  ----------------------------------------------------

class TOOLKIT_DLLPUBLIC UnoControlBase : public UnoControl
{
protected:
    UnoControlBase();

protected:
    UnoControlBase( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& i_factory )
        :UnoControl( i_factory )
    {
    }

    sal_Bool                    ImplHasProperty( sal_uInt16 nProp );
    sal_Bool                    ImplHasProperty( const ::rtl::OUString& aPropertyName );
    void                        ImplSetPropertyValue( const ::rtl::OUString& aPropertyName, const ::com::sun::star::uno::Any& aValue, sal_Bool bUpdateThis );
    void                        ImplSetPropertyValues( const ::com::sun::star::uno::Sequence< ::rtl::OUString >& aPropertyNames, const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any >& aValues, sal_Bool bUpdateThis );
    ::com::sun::star::uno::Any  ImplGetPropertyValue( const ::rtl::OUString& aPropertyName );

    sal_Bool        ImplGetPropertyValue_BOOL( sal_uInt16 nProp );
    sal_Int16       ImplGetPropertyValue_INT16( sal_uInt16 nProp );
    sal_uInt16      ImplGetPropertyValue_UINT16( sal_uInt16 nProp );
    sal_Int32       ImplGetPropertyValue_INT32( sal_uInt16 nProp );
    sal_uInt32      ImplGetPropertyValue_UINT32( sal_uInt16 nProp );
    double          ImplGetPropertyValue_DOUBLE( sal_uInt16 nProp );
    ::rtl::OUString ImplGetPropertyValue_UString( sal_uInt16 nProp );

    // XLayoutConstrains (nur wenn das Control es unterstuetzt!)
    ::com::sun::star::awt::Size Impl_getMinimumSize();
    ::com::sun::star::awt::Size Impl_getPreferredSize();
    ::com::sun::star::awt::Size Impl_calcAdjustedSize( const ::com::sun::star::awt::Size& rNewSize );

    // XTextLayoutConstrains (nur wenn das Control es unterstuetzt!)
    ::com::sun::star::awt::Size Impl_getMinimumSize( sal_Int16 nCols, sal_Int16 nLines );
    void                        Impl_getColumnsAndLines( sal_Int16& nCols, sal_Int16& nLines );
};



#endif // _TOOLKIT_AWT_UNOCONTROLBASE_HXX_

