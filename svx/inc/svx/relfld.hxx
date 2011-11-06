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


#ifndef _SVX_RELFLD_HXX
#define _SVX_RELFLD_HXX

// include ---------------------------------------------------------------

#ifndef _FIELD_HXX //autogen
#include <vcl/field.hxx>
#endif
#include "svx/svxdllapi.h"


// class SvxRelativeField ------------------------------------------------
/*
    [Beschreibung]

    "Ahnlich der Klasse FontSizeBox. Abgeleitet von der Klasse MetricField.
    Zus"atzliche Funktionalit"at: relative Angaben.
*/

class SVX_DLLPUBLIC SvxRelativeField : public MetricField
{
private:
    sal_uInt16          nRelMin;
    sal_uInt16          nRelMax;
    sal_uInt16          nRelStep;
    sal_Bool            bRelativeMode;
    sal_Bool            bRelative;
    sal_Bool            bNegativeEnabled;

protected:
    void            Modify();

public:
    SvxRelativeField( Window* pParent, WinBits nWinStyle = 0 );
    SvxRelativeField( Window* pParent, const ResId& rResId );

    void            EnableRelativeMode( sal_uInt16 nMin = 50, sal_uInt16 nMax = 150,
                                        sal_uInt16 nStep = 5 );
    sal_Bool            IsRelativeMode() const { return bRelativeMode; }
    void            SetRelative( sal_Bool bRelative = sal_False );
    sal_Bool            IsRelative() const { return bRelative; }
    void            EnableNegativeMode() {bNegativeEnabled = sal_True;}
};


#endif

