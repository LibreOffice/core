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


#ifndef _CHART2_RES_TITLES_HXX
#define _CHART2_RES_TITLES_HXX

#include "TitleDialogData.hxx"
#include <vcl/edit.hxx>
#include <vcl/fixed.hxx>

//.............................................................................
namespace chart
{
//.............................................................................

class TitleResources
{
public:
    TitleResources( Window* pParent, bool bShowSecondaryAxesTitle );
    virtual ~TitleResources();

    void writeToResources( const TitleDialogData& rInput );
    void readFromResources( TitleDialogData& rOutput );

    void SetUpdateDataHdl( const Link& rLink );
    bool IsModified();
    void ClearModifyFlag();

private:
    FixedText           m_aFT_Main;
    Edit                m_aEd_Main;
    FixedText           m_aFT_Sub;
    Edit                m_aEd_Sub;

    FixedLine           m_aFL_Axes;
    FixedText           m_aFT_XAxis;
    Edit                m_aEd_XAxis;
    FixedText           m_aFT_YAxis;
    Edit                m_aEd_YAxis;
    FixedText           m_aFT_ZAxis;
    Edit                m_aEd_ZAxis;

    FixedLine           m_aFL_SecondaryAxes;
    FixedText           m_aFT_SecondaryXAxis;
    Edit                m_aEd_SecondaryXAxis;
    FixedText           m_aFT_SecondaryYAxis;
    Edit                m_aEd_SecondaryYAxis;
};

//.............................................................................
} //namespace chart
//.............................................................................

#endif
