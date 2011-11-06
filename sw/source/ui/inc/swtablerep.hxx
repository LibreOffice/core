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


#ifndef _SWTABLEREP_HXX
#define _SWTABLEREP_HXX


#include <sfx2/tabdlg.hxx>
#include "swdllapi.h"
#include "swtypes.hxx"

class SwTabCols;
struct TColumn;
class SW_DLLPUBLIC SwTableRep
{
    TColumn*    pTColumns;

    SwTwips     nTblWidth;
    SwTwips     nSpace;
    SwTwips     nLeftSpace;
    SwTwips     nRightSpace;
    sal_uInt16      nAlign;
    sal_uInt16      nColCount;
    sal_uInt16      nAllCols;
    sal_uInt16      nWidthPercent;
    sal_Bool        bComplex : 1;
    sal_Bool        bLineSelected : 1;
    sal_Bool        bWidthChanged : 1;
    sal_Bool        bColsChanged : 1;

public:
    SwTableRep( const SwTabCols& rTabCol, sal_Bool bComplex );
    ~SwTableRep();

    sal_Bool        FillTabCols( SwTabCols& rTabCol ) const;

    SwTwips     GetLeftSpace() const            {return nLeftSpace;}
    void        SetLeftSpace(SwTwips nSet)      {nLeftSpace = nSet;}

    SwTwips     GetRightSpace() const           {return nRightSpace;}
    void        SetRightSpace(SwTwips nSet)     {nRightSpace = nSet;}

    SwTwips     GetWidth() const                {return nTblWidth;}
    void        SetWidth(SwTwips nSet)          {nTblWidth = nSet;}

    sal_uInt16      GetWidthPercent() const         {return nWidthPercent;}
    void        SetWidthPercent(sal_uInt16 nSet)    {nWidthPercent = nSet;}

    sal_uInt16      GetAlign() const                {return nAlign;}
    void        SetAlign(sal_uInt16 nSet)           {nAlign = nSet;}

    sal_Bool        IsComplex() const               {return bComplex;}
    sal_uInt16      GetColCount() const             {return nColCount;}
    sal_uInt16      GetAllColCount() const          {return nAllCols;}

    sal_Bool        HasColsChanged() const          {return bColsChanged;}
    void        SetColsChanged()                {bColsChanged = sal_True;}

    sal_Bool        HasWidthChanged() const         {return bWidthChanged;}
    void        SetWidthChanged()               {bWidthChanged  = sal_True;}

    sal_Bool        IsLineSelected() const          {return bLineSelected;}
    void        SetLineSelected(sal_Bool bSet)      {bLineSelected = bSet;}

    SwTwips     GetSpace() const                { return nSpace;}
    void        SetSpace(SwTwips nSet)          {nSpace = nSet;}

    TColumn*    GetColumns() const              {return pTColumns;}
};
#endif

