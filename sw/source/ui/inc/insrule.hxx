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


#ifndef _INSRULE_HXX
#define _INSRULE_HXX
#include "num.hxx"

class SwRulerValueSet;
class ValueSet;
/*-----------------14.02.97 12.30-------------------

--------------------------------------------------*/
class SwInsertGrfRulerDlg  : public SfxModalDialog
{
    FixedLine       aSelectionFL;
    OKButton        aOkPB;
    CancelButton    aCancelPB;
    HelpButton      aHelpPB;

    List            aGrfNames;
    String          sSimple;
    String          sRulers;
    sal_uInt16          nSelPos;

    SwRulerValueSet* pExampleVS;

protected:
    DECL_LINK(SelectHdl, ValueSet*);
    DECL_LINK(DoubleClickHdl, ValueSet*);

public:
    SwInsertGrfRulerDlg( Window* pParent );
    ~SwInsertGrfRulerDlg();

    String          GetGraphicName();
    sal_Bool            IsSimpleLine() {return nSelPos == 1;}
    sal_Bool            HasImages() const {return 0 != aGrfNames.Count();}
};

#endif



