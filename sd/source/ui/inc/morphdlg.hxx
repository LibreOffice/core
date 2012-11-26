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



#ifndef SD_MORPH_DLG_HXX
#define SD_MORPH_DLG_HXX

#include <vcl/button.hxx>
#include <vcl/group.hxx>
#include <vcl/fixed.hxx>
#include <vcl/field.hxx>
#include <vcl/dialog.hxx>

class SdrObject;

namespace sd {

/******************************************************************************
|*
|*
|*
\******************************************************************************/

class MorphDlg
    : public ModalDialog
{
public:
    MorphDlg (
        ::Window* pParent,
        const SdrObject* pObj1,
        const SdrObject* pObj2);
    virtual ~MorphDlg (void);

    void            SaveSettings() const;
    sal_uInt16          GetFadeSteps() const { return (sal_uInt16) aMtfSteps.GetValue(); }
    bool            IsAttributeFade() const { return aCbxAttributes.IsChecked(); }
    bool            IsOrientationFade() const { return aCbxOrientation.IsChecked(); }

private:
    FixedLine       aGrpPreset;
    FixedText       aFtSteps;
    MetricField     aMtfSteps;
    CheckBox        aCbxAttributes;
    CheckBox        aCbxOrientation;
    OKButton        aBtnOK;
    CancelButton    aBtnCancel;
    HelpButton      aBtnHelp;
    SdrObject*      pSdrObj1;
    SdrObject*      pSdrObj2;

    void            LoadSettings();
};


#endif


} // end of namespace sd
