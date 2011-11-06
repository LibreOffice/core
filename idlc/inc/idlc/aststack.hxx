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


#ifndef _IDLC_ASTSTACK_HXX_
#define _IDLC_ASTSTACK_HXX_

#include <sal/types.h>

class AstScope;

class AstStack
{
public:
    AstStack();
    virtual ~AstStack();

    sal_uInt32 depth();
    AstScope* top();
    AstScope* bottom();
    AstScope* nextToTop();
    AstScope* topNonNull();
    AstStack* push(AstScope* pScope);
    void pop();
    void clear();

private:
    AstScope**  m_stack;
    sal_uInt32  m_size;
    sal_uInt32  m_top;
};

#endif // _IDLC_ASTSTACK_HXX_

