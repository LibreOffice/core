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



#ifndef _TEXTCONTROLCOMBO_HXX
#define _TEXTCONTROLCOMBO_HXX

#ifndef _FIXED_HXX
#include <vcl/fixed.hxx>
#endif

#include <vcl/field.hxx>
#include "swdllapi.h"

class SW_DLLPUBLIC TextControlCombo : public Window
{
protected:

    Control&    mrCtrl;
    FixedText&  mrFTbefore;
    FixedText&  mrFTafter;

public:

    using Window::Enable;
    using Window::Disable;

                TextControlCombo( Window* _pParent, const ResId& _rResId,
                            Control& _rCtrl, FixedText& _rFTbefore, FixedText& _rFTafter );
    virtual     ~TextControlCombo();

    void        Arrange( FixedText& _rOrg, sal_Bool bShow = true );

    // identical to window functionality
    void        Show( sal_Bool bVisible = sal_True, sal_uInt16 nFlags = 0 );
    void        Hide( sal_uInt16 nFlags = 0 ) { Show( sal_False, nFlags ); }

    void        Enable( sal_Bool bEnable = sal_True, sal_Bool bChild = sal_True );
    void        Disable( sal_Bool bChild = sal_True ) { Enable( sal_False, bChild ); }
};


#endif
