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



#ifndef _CONNECTIVITY_MAB_NS_INIT_HXX_
#define _CONNECTIVITY_MAB_NS_INIT_HXX_ 1
#include "MNSDeclares.hxx"

#include <sal/types.h>

sal_Bool MNS_Init(sal_Bool& aProfileExists);
sal_Bool MNS_Term(sal_Bool aForce=sal_False);

sal_Bool MNS_InitXPCOM(sal_Bool* aProfileExists);
typedef struct UI_Thread_ARGS
{
    sal_Bool* bProfileExists;
}UI_Thread_ARGS;
void MNS_XPCOM_EventLoop();

#endif // _CONNECTIVITY_MAB_NS_INIT_HXX_ 1

