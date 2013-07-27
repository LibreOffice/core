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



#ifndef _SVX_XPROPERTYTABLE_HXX
#define _SVX_XPROPERTYTABLE_HXX

#include <com/sun/star/uno/XInterface.hpp>
#include "svx/svxdllapi.h"
#include <svx/xtable.hxx>

SVX_DLLPUBLIC ::com::sun::star::uno::Reference< com::sun::star::uno::XInterface > SAL_CALL SvxUnoXColorTable_createInstance( XPropertyList* pTable ) throw();
SVX_DLLPUBLIC ::com::sun::star::uno::Reference< com::sun::star::uno::XInterface > SAL_CALL SvxUnoXLineEndTable_createInstance( XPropertyList* pList ) throw();
SVX_DLLPUBLIC ::com::sun::star::uno::Reference< com::sun::star::uno::XInterface > SAL_CALL SvxUnoXDashTable_createInstance( XPropertyList* pList ) throw();
SVX_DLLPUBLIC ::com::sun::star::uno::Reference< com::sun::star::uno::XInterface > SAL_CALL SvxUnoXHatchTable_createInstance( XPropertyList* pList ) throw();
SVX_DLLPUBLIC ::com::sun::star::uno::Reference< com::sun::star::uno::XInterface > SAL_CALL SvxUnoXGradientTable_createInstance( XPropertyList* pList ) throw();
SVX_DLLPUBLIC ::com::sun::star::uno::Reference< com::sun::star::uno::XInterface > SAL_CALL SvxUnoXBitmapTable_createInstance( XPropertyList* pList ) throw();

#endif


