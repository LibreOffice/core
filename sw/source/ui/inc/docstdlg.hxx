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


#ifndef _DOCSTDLG_HXX
#define _DOCSTDLG_HXX

#include <sfx2/tabdlg.hxx>

#ifndef _FIXED_HXX //autogen
#include <vcl/fixed.hxx>
#endif
#include <svtools/stdctrl.hxx>

#include "docstat.hxx"

/*--------------------------------------------------------------------
    Beschreibung:   DocInfo jetzt als Page
 --------------------------------------------------------------------*/

class SwDocStatPage: public SfxTabPage
{
public:
    SwDocStatPage(Window *pParent, const SfxItemSet &rSet);
    ~SwDocStatPage();

    static SfxTabPage *Create(Window *pParent, const SfxItemSet &rSet);

protected:
    virtual sal_Bool    FillItemSet(      SfxItemSet &rSet);
    virtual void    Reset      (const SfxItemSet &rSet);

    DECL_LINK( UpdateHdl, PushButton*);

private:
    FixedText       aPageLbl;
    FixedInfo       aPageNo;
    FixedText       aTableLbl;
    FixedInfo       aTableNo;
    FixedText       aGrfLbl;
    FixedInfo       aGrfNo;
    FixedText       aOLELbl;
    FixedInfo       aOLENo;
    FixedText       aParaLbl;
    FixedInfo       aParaNo;
    FixedText       aWordLbl;
    FixedInfo       aWordNo;
    FixedText       aCharLbl;
    FixedInfo       aCharNo;
    FixedText       aLineLbl;
    FixedInfo       aLineNo;

    PushButton      aUpdatePB;
    SwDocStat       aDocStat;

    void            Update();

    using Window::SetData;
    void            SetData(const SwDocStat &rStat);
};

#endif
