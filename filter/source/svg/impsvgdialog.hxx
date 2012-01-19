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

#ifndef IMPSVGDIALOG_HXX
#define IMPSVGDIALOG_HXX

#ifndef _COM_SUN_STAR_UNO_SEQUENCE_H_
#include <com/sun/star/uno/Sequence.h>
#endif

#ifndef _COM_SUN_STAR_BEANS_PROPERTYVALUE_HPP_
#include <com/sun/star/beans/PropertyValue.hpp>
#endif

#include <vcl/dialog.hxx>
#include <vcl/button.hxx>
#include <vcl/fixed.hxx>
#include <vcl/field.hxx>
#include <svtools/stdctrl.hxx>
#include <svtools/FilterConfigItem.hxx>

#define SVG_EXPORTFILTER_CONFIGPATH         "Office.Common/Filter/SVG/Export/"
#define SVG_PROP_TINYPROFILE                "TinyMode"
#define SVG_PROP_EMBEDFONTS                 "EmbedFonts"
#define SVG_PROP_NATIVEDECORATION           "UseNativeTextDecoration"
#define SVG_PROP_GLYPHPLACEMENT             "GlyphPlacement"
#define SVG_PROP_OPACITY                    "Opacity"
#define SVG_PROP_GRADIENT                   "Gradient"

// ----------------
// - ImpSVGDialog -
// ----------------

class ResMgr;
class Window;

class ImpSVGDialog : public ModalDialog
{
private:

    FixedLine           maFI;
    CheckBox            maCBTinyProfile;
    CheckBox            maCBEmbedFonts;
    CheckBox            maCBUseNativeDecoration;

    OKButton            maBTOK;
    CancelButton        maBTCancel;
    HelpButton          maBTHelp;

    FilterConfigItem    maConfigItem;
    sal_Bool            mbOldNativeDecoration;

    DECL_LINK( OnToggleCheckbox, CheckBox* );

public:

    ImpSVGDialog( Window* pParent, /*ResMgr& rResMgr,*/
                  com::sun::star::uno::Sequence< com::sun::star::beans::PropertyValue >& rFilterData );
    ~ImpSVGDialog();

    com::sun::star::uno::Sequence< com::sun::star::beans::PropertyValue > GetFilterData();
};

#endif // IMPSVGDIALOG_HXX
