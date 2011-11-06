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
#include "precompiled_cui.hxx"

#include <string> // HACK: prevent conflict between STLPORT and Workshop headers
#include <tools/ref.hxx>
#include <tools/shl.hxx>
#include <svl/intitem.hxx>
#include <svl/eitem.hxx>
#include <svl/stritem.hxx>
#include <sfx2/dispatch.hxx>
#include <vcl/toolbox.hxx>
#include <vcl/fixed.hxx>
#include <vcl/sound.hxx>
#include <dialmgr.hxx>
#include <cuires.hrc>
#include "cuitbxform.hxx"
#include <sfx2/viewfrm.hxx>
#include <sfx2/viewsh.hxx>
#include "fmsearch.hrc"

//========================================================================
// class FmInputRecordNoDialog
//========================================================================

FmInputRecordNoDialog::FmInputRecordNoDialog(Window * pParent)
    :ModalDialog( pParent, CUI_RES(RID_SVX_DLG_INPUTRECORDNO))
    ,m_aLabel(this, CUI_RES(1))
    ,m_aRecordNo(this, CUI_RES(1))
    ,m_aOk(this, CUI_RES(1))
    ,m_aCancel(this, CUI_RES(1))
{
    m_aRecordNo.SetMin(1);
    m_aRecordNo.SetMax(0x7FFFFFFF);
    m_aRecordNo.SetStrictFormat(sal_True);
    m_aRecordNo.SetDecimalDigits(0);

    FreeResource();
}
