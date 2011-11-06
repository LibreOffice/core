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



#ifndef _W1PAR_HXX
#define _W1PAR_HXX

#include <fltshell.hxx>
#ifndef _W1CLASS_HXX
#include <w1class.hxx>
#endif

////////////////////////////////////////////////////////////// Ww1Shell
class Ww1Shell : public SwFltShell
{
public:
    Ww1Shell(SwDoc&, SwPaM&, const String& rBaseURL, sal_Bool bNew, sal_uLong nFieldFlags);
};


#endif

