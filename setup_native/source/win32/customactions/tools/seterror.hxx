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



#ifndef _SETERROR_HXX_
#define _SETERROR_HXX_

//----------------------------------------------------------
// list of own error codes

#define MSI_ERROR_INVALIDDIRECTORY          9001
#define MSI_ERROR_ISWRONGPRODUCT            9002
#define MSI_ERROR_PATCHISOLDER              9003

#define MSI_ERROR_NEW_VERSION_FOUND         9010
#define MSI_ERROR_SAME_VERSION_FOUND        9011
#define MSI_ERROR_OLD_VERSION_FOUND         9012
#define MSI_ERROR_NEW_PATCH_FOUND           9013
#define MSI_ERROR_SAME_PATCH_FOUND          9014
#define MSI_ERROR_OLD_PATCH_FOUND           9015

#define MSI_ERROR_OFFICE_IS_RUNNING         9020

#define MSI_ERROR_DIRECTORY_NOT_EMPTY       9030

//----------------------------------------------------------

void SetMsiErrorCode( int nErrorCode );

#endif

