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


#include <wrtsh.hxx>
#include <splittbl.hxx>
#include <splittbl.hrc>
#include <table.hrc>
#include <tblenum.hxx>
/*-----------------17.03.98 10:56-------------------

--------------------------------------------------*/
SwSplitTblDlg::SwSplitTblDlg( Window *pParent, SwWrtShell &rSh ) :
    SvxStandardDialog(pParent, SW_RES(DLG_SPLIT_TABLE)),
    aOKPB(                  this, SW_RES(PB_OK      )),
    aCancelPB(              this, SW_RES(PB_CANCEL  )),
    aHelpPB(                this, SW_RES(PB_HELP        )),
    aSplitFL(               this, SW_RES(FL_SPLIT    )),
    aCntntCopyRB(           this, SW_RES(RB_CNTNT   )),
    aBoxAttrCopyWithParaRB( this, SW_RES(RB_BOX_PARA    )),
    aBoxAttrCopyNoParaRB(   this, SW_RES(RB_BOX_NOPARA)),
    aBorderCopyRB(          this, SW_RES(RB_BORDER  )),
    rShell(rSh),
    m_nSplit( HEADLINE_CNTNTCOPY )
{
    FreeResource();
    aCntntCopyRB.Check();
}

/*-----------------17.03.98 10:56-------------------

--------------------------------------------------*/
void SwSplitTblDlg::Apply()
{
    m_nSplit = HEADLINE_CNTNTCOPY;
    if(aBoxAttrCopyWithParaRB.IsChecked())
        m_nSplit = HEADLINE_BOXATRCOLLCOPY;
    if(aBoxAttrCopyNoParaRB.IsChecked())
        m_nSplit = HEADLINE_BOXATTRCOPY;
    else if(aBorderCopyRB.IsChecked())
        m_nSplit = HEADLINE_BORDERCOPY;

    rShell.SplitTable( m_nSplit );

}



