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



#ifndef _SVX_UNOFILL_HXX_
#define _SVX_UNOFILL_HXX_

#include <com/sun/star/uno/XInterface.hpp>
#include "svx/svxdllapi.h"

class SdrModel;

SVX_DLLPUBLIC ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > SAL_CALL SvxUnoGradientTable_createInstance( SdrModel* pModel );
SVX_DLLPUBLIC ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > SAL_CALL SvxUnoHatchTable_createInstance( SdrModel* pModel );
SVX_DLLPUBLIC ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > SAL_CALL SvxUnoBitmapTable_createInstance( SdrModel* pModel );
SVX_DLLPUBLIC ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > SAL_CALL SvxUnoTransGradientTable_createInstance( SdrModel* pModel );
SVX_DLLPUBLIC ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > SAL_CALL SvxUnoMarkerTable_createInstance( SdrModel* pModel );
SVX_DLLPUBLIC ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > SAL_CALL SvxUnoDashTable_createInstance( SdrModel* pModel );

#endif // _SVX_UNOFILL_HXX_

