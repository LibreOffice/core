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



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_svx.hxx"
#include <editeng/eeitem.hxx>

#include <editeng/measfld.hxx>
#include <svx/svdotext.hxx>
#include <svx/svdfield.hxx>

static bool bInit = false;

// Do not remove this, it is still used in src536a! TTTT: is this used?
void SdrRegisterFieldClasses()
{
    if ( !bInit )
    {
        SvxFieldItem::GetClassManager().SV_CLASS_REGISTER(SdrMeasureField);
        SvxFieldItem::GetClassManager().SV_CLASS_REGISTER(SvxHeaderField);
        SvxFieldItem::GetClassManager().SV_CLASS_REGISTER(SvxFooterField);
        SvxFieldItem::GetClassManager().SV_CLASS_REGISTER(SvxDateTimeField);
        bInit = true;
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////// */

bool SdrTextObj::CalcFieldValue(const SvxFieldItem& /*rField*/, sal_uInt16 /*nPara*/, sal_uInt16 /*nPos*/,
    bool /*bEdit*/, Color*& /*rpTxtColor*/, Color*& /*rpFldColor*/, XubString& /*rRet*/) const
{
    return false;
}

