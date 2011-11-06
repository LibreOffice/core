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



#ifndef SC_TPSTAT_HXX
#define SC_TPSTAT_HXX


#include <sfx2/tabdlg.hxx>
#include <vcl/fixed.hxx>
#include <svtools/stdctrl.hxx>



//========================================================================

class ScDocStatPage: public SfxTabPage
{
public:
    static SfxTabPage* Create( Window* pParent, const SfxItemSet& rSet );

private:
            ScDocStatPage( Window *pParent, const SfxItemSet& rSet );
            ~ScDocStatPage();

protected:
    virtual sal_Bool    FillItemSet( SfxItemSet& rSet );
    virtual void    Reset      ( const SfxItemSet& rSet );

private:
    FixedLine       aFlInfo;
    FixedText       aFtTablesLbl;
    FixedInfo       aFtTables;
    FixedText       aFtCellsLbl;
    FixedInfo       aFtCells;
    FixedText       aFtPagesLbl;
    FixedInfo       aFtPages;
};



#endif
