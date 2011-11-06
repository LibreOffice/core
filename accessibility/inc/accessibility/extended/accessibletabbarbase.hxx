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



#ifndef ACCESSIBILITY_EXT_ACCESSIBLETABBARBASE_HXX
#define ACCESSIBILITY_EXT_ACCESSIBLETABBARBASE_HXX

#include <comphelper/accessiblecomponenthelper.hxx>
#include <tools/link.hxx>

class TabBar;
class VCLExternalSolarLock;
class VclSimpleEvent;
class VclWindowEvent;

//.........................................................................
namespace accessibility
{
//.........................................................................

// ============================================================================

typedef ::comphelper::OAccessibleExtendedComponentHelper AccessibleExtendedComponentHelper_BASE;

class AccessibleTabBarBase : public AccessibleExtendedComponentHelper_BASE
{
public:
    explicit            AccessibleTabBarBase( TabBar* pTabBar );
    virtual             ~AccessibleTabBarBase();

protected:
    DECL_LINK( WindowEventListener, VclSimpleEvent* );

    virtual void        ProcessWindowEvent( const VclWindowEvent& rVclWindowEvent );

    // XComponent
    virtual void SAL_CALL disposing();

private:
    void                SetTabBarPointer( TabBar* pTabBar );
    void                ClearTabBarPointer();

protected:
    VCLExternalSolarLock* m_pExternalLock;
    TabBar*             m_pTabBar;
};

// ============================================================================

//.........................................................................
}   // namespace accessibility
//.........................................................................

#endif

