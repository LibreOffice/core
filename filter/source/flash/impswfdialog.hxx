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



#ifndef IMPSWFDIALOG_HXX
#define IMPSWFDIALOG_HXX

#include <com/sun/star/uno/Sequence.h>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <svtools/stdctrl.hxx>

#include <vcl/dialog.hxx>
#include <vcl/button.hxx>
#include <vcl/fixed.hxx>
#include <vcl/field.hxx>

#include <svtools/FilterConfigItem.hxx>

// ----------------
// - ImpSWFDialog -
// ----------------

class ResMgr;
class Window;

class ImpSWFDialog : public ModalDialog
{
private:
    FixedInfo           maFiDescr;
    NumericField        maNumFldQuality;
    FixedInfo           maFiExportAllDescr;
    CheckBox            maCheckExportAll;
    FixedInfo           maFiExportBackgroundsDescr;
    CheckBox            maCheckExportBackgrounds;
    FixedInfo           maFiExportBackgroundObjectsDescr;
    CheckBox            maCheckExportBackgroundObjects;
    FixedInfo           maFiExportSlideContentsDescr;
    CheckBox            maCheckExportSlideContents;
    FixedInfo           maFiExportSoundDescr;
    CheckBox            maCheckExportSound;
    FixedInfo           maFiExportOLEAsJPEGDescr;
    CheckBox            maCheckExportOLEAsJPEG;
    FixedInfo           maFiExportMultipleFilesDescr;
    CheckBox            maCheckExportMultipleFiles;

    OKButton            maBtnOK;
    CancelButton        maBtnCancel;
    HelpButton          maBtnHelp;

    FilterConfigItem    maConfigItem;

    DECL_LINK( OnToggleCheckbox, CheckBox* );

public:
    ImpSWFDialog( Window* pParent, ResMgr& rResMgr,
                  com::sun::star::uno::Sequence< com::sun::star::beans::PropertyValue >& rFilterData );
    ~ImpSWFDialog();

    com::sun::star::uno::Sequence< com::sun::star::beans::PropertyValue > GetFilterData();
};

#endif // IMPDIALOG_HXX
