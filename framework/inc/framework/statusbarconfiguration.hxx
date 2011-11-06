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


#ifndef __FRAMEWORK_CLASSES_STATUSBARCONFIGURATION_HXX_
#define __FRAMEWORK_CLASSES_STATUSBARCONFIGURATION_HXX_

#include <framework/fwedllapi.h>
#include <svl/svarray.hxx>
#include <tools/string.hxx>
#include <tools/stream.hxx>
#include <vcl/status.hxx>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/io/XInputStream.hpp>
#include <com/sun/star/io/XOutputStream.hpp>
#include <com/sun/star/container/XIndexAccess.hpp>
#include <com/sun/star/container/XIndexContainer.hpp>

namespace framework
{

struct FWE_DLLPUBLIC StatusBarItemDescriptor
{
    String  aURL;                       // URL command to dispatch
    long    nItemBits;                  // properties for this statusbar item (WinBits)
    long    nWidth;                     // width of a statusbar item
    long    nOffset;                    // offset

    public:

        StatusBarItemDescriptor() : nItemBits( SIB_CENTER | SIB_IN )
                                    ,nWidth( 0 )
                                    ,nOffset( STATUSBAR_OFFSET ) {}
};

typedef StatusBarItemDescriptor* StatusBarItemDescriptorPtr;
SV_DECL_PTRARR_DEL( StatusBarDescriptor, StatusBarItemDescriptorPtr, 10, 2)

class FWE_DLLPUBLIC StatusBarConfiguration
{
    public:
        static sal_Bool LoadStatusBar(
            const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& xServiceFactory,
            SvStream& rInStream, StatusBarDescriptor& aItems );

        static sal_Bool StoreStatusBar(
            const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& xServiceFactory,
            SvStream& rOutStream, const StatusBarDescriptor& aItems );

        static sal_Bool LoadStatusBar(
            const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& xServiceFactory,
            const ::com::sun::star::uno::Reference< ::com::sun::star::io::XInputStream >& xInputStream,
            const ::com::sun::star::uno::Reference< ::com::sun::star::container::XIndexContainer >& rStatusbarConfiguration );

        static sal_Bool StoreStatusBar(
            const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& xServiceFactory,
            const ::com::sun::star::uno::Reference< ::com::sun::star::io::XOutputStream >& xOutputStream,
            const ::com::sun::star::uno::Reference< ::com::sun::star::container::XIndexAccess >& rStatusbarConfiguration );
};

} // namespace framework

#endif // __FRAMEWORK_CLASSES_STATUSBARCONFIGURATION_HXX_
