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



#ifndef ACCESSIBILITY_FLOATINGWINDOWACCESSIBLE_HXX
#define ACCESSIBILITY_FLOATINGWINDOWACCESSIBLE_HXX

#ifndef _TOOLKIT_AWT_VCLXACCESSIBLECOMPONENT_HXX_
#include <toolkit/awt/vclxaccessiblecomponent.hxx>
#endif

class FloatingWindowAccessible : public VCLXAccessibleComponent
{
public:
    FloatingWindowAccessible( VCLXWindow* pWindow );
    virtual ~FloatingWindowAccessible();

    virtual void FillAccessibleRelationSet( utl::AccessibleRelationSetHelper& rRelationSet );
};

#endif // ACCESSIBILITY_FLOATINGWINDOWACCESSIBLE_HXX

