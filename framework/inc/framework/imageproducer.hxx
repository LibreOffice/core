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



#ifndef __FRAMEWORK_HELPER_IMAGEPRODUCER_HXX_
#define __FRAMEWORK_HELPER_IMAGEPRODUCER_HXX_

//_________________________________________________________________________________________________________________
//  interface includes
//_________________________________________________________________________________________________________________
#include <sal/types.h>
#include <com/sun/star/frame/XFrame.hpp>
#include <vcl/image.hxx>
#include <rtl/ustring.hxx>
#include <framework/fwedllapi.h>

namespace framework
{

typedef Image ( *pfunc_getImage)( const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame >& rFrame, const ::rtl::OUString& aURL, sal_Bool bBig, sal_Bool bHiContrast );

pfunc_getImage FWE_DLLPUBLIC SAL_CALL SetImageProducer( pfunc_getImage pGetImageFunc );

Image FWE_DLLPUBLIC SAL_CALL GetImageFromURL( const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame >& rFrame, const ::rtl::OUString& aURL, sal_Bool bBig, sal_Bool bHiContrast );

}

#endif // __FRAMEWORK_HELPER_IMAGEPRODUCER_HXX_
