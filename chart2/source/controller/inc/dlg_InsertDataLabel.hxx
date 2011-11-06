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


#ifndef CHART2_DLG_INSERT_DATALABELS_GRID_HXX
#define CHART2_DLG_INSERT_DATALABELS_GRID_HXX

// header for class ModalDialog
#include <vcl/dialog.hxx>
// header for class CheckBox
#ifndef _SV_BUTTON_HXX
#include <vcl/button.hxx>
#endif
// header for class SfxItemSet
#include <svl/itemset.hxx>
//for auto_ptr
#include <memory>

class SvNumberFormatter;

//.............................................................................
namespace chart
{
//.............................................................................

class DataLabelResources;
class DataLabelsDialog : public ModalDialog
{
private:
    OKButton            m_aBtnOK;
    CancelButton        m_aBtnCancel;
    HelpButton          m_aBtnHelp;
    ::std::auto_ptr< DataLabelResources >    m_apDataLabelResources;

    const SfxItemSet&   m_rInAttrs;

    void Reset();

public:
    DataLabelsDialog(Window* pParent, const SfxItemSet& rInAttrs, SvNumberFormatter* pFormatter);
    virtual ~DataLabelsDialog();

    void FillItemSet(SfxItemSet& rOutAttrs);
};

//.............................................................................
} //namespace chart
//.............................................................................

#endif
