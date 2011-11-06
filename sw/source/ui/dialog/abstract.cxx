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
#include "precompiled_sw.hxx"

#ifdef SW_DLLIMPLEMENTATION
#undef SW_DLLIMPLEMENTATION
#endif


#include "swtypes.hxx"
#include "hintids.hxx"

#include "dialog.hrc"
#include "abstract.hxx"
#include "abstract.hrc"


/*-----------------22.02.97 15.32-------------------

--------------------------------------------------*/

SwInsertAbstractDlg::SwInsertAbstractDlg( Window* pParent ) :
    SfxModalDialog(pParent, SW_RES(DLG_INSERT_ABSTRACT)),
    aFL     (this, SW_RES(FL_1       )),
    aLevelFT(this, SW_RES(FT_LEVEL   )),
    aLevelNF(this, SW_RES(NF_LEVEL   )),
    aParaFT (this, SW_RES(FT_PARA   )),
    aParaNF (this, SW_RES(NF_PARA   )),
    aDescFT (this, SW_RES(FT_DESC   )),
    aOkPB   (this, SW_RES(PB_OK     )),
    aCancelPB (this, SW_RES(PB_CANCEL   )),
    aHelpPB (this, SW_RES(PB_HELP   ))
{
    FreeResource();
}

/*-----------------22.02.97 15.32-------------------

--------------------------------------------------*/

SwInsertAbstractDlg::~SwInsertAbstractDlg()
{
}

/*-----------------22.02.97 15.34-------------------

--------------------------------------------------*/

sal_uInt8 SwInsertAbstractDlg::GetLevel() const
{
    return static_cast< sal_uInt8 >(aLevelNF.GetValue() - 1);
}
/*-----------------22.02.97 15.34-------------------

--------------------------------------------------*/

sal_uInt8 SwInsertAbstractDlg::GetPara() const
{
    return (sal_uInt8) aParaNF.GetValue();
}






