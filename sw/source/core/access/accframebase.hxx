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


#ifndef _ACCFRAMEBASE_HXX
#define _ACCFRAMEBASE_HXX

//IAccessibility2 Implementation 2009-----
#ifndef _PAM_HXX
#include <pam.hxx>
#endif
//-----IAccessibility2 Implementation 2009

#include <acccontext.hxx>

#include <calbck.hxx>

class SwFlyFrm;

class SwAccessibleFrameBase : public SwAccessibleContext,
                              public SwClient
{
    sal_Bool    bIsSelected;    // protected by base class mutex

    sal_Bool    IsSelected();

protected:

    // Set states for getAccessibleStateSet.
    // This drived class additionaly sets SELECTABLE(1), SELECTED(+),
    // FOCUSABLE(1) and FOCUSED(+)
    virtual void GetStates( ::utl::AccessibleStateSetHelper& rStateSet );
    //IAccessibility2 Implementation 2009-----
    SwFlyFrm* getFlyFrm() const;
    sal_Bool GetSelectedState( );
    SwPaM* GetCrsr();
    //-----IAccessibility2 Implementation 2009

    virtual void _InvalidateCursorPos();
    virtual void _InvalidateFocus();

    virtual ~SwAccessibleFrameBase();
    virtual void Modify( const SfxPoolItem* pOld, const SfxPoolItem *pNew);

public:

    SwAccessibleFrameBase( SwAccessibleMap* pInitMap,
                           sal_Int16 nInitRole,
                           const SwFlyFrm *pFlyFrm );

    virtual sal_Bool HasCursor();   // required by map to remember that object

    static sal_uInt8 GetNodeType( const SwFlyFrm *pFlyFrm );

    // The object is not visible an longer and should be destroyed
    virtual void Dispose( sal_Bool bRecursive = sal_False );
    virtual sal_Bool SetSelectedState( sal_Bool bSeleted );
    //-----IAccessibility2 Implementation 2009
};


#endif
