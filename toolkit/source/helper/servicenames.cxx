/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include <sal/types.h>
#include <tools/solar.h>

#include <toolkit/helper/servicenames.hxx>

const sal_Char szServiceName_Toolkit[] = "stardiv.vcl.VclToolkit", szServiceName2_Toolkit[] = "com.sun.star.awt.Toolkit";
const sal_Char szServiceName_PopupMenu[] = "stardiv.vcl.PopupMenu", szServiceName2_PopupMenu[] = "com.sun.star.awt.PopupMenu";
const sal_Char szServiceName_MenuBar[] = "stardiv.vcl.MenuBar", szServiceName2_MenuBar[] = "com.sun.star.awt.MenuBar";
const sal_Char szServiceName_Pointer[] = "stardiv.vcl.Pointer", szServiceName2_Pointer[] = "com.sun.star.awt.Pointer";
const sal_Char szServiceName_UnoControlContainer[] = "stardiv.vcl.control.ControlContainer", szServiceName2_UnoControlContainer[] = "com.sun.star.awt.UnoControlContainer";


const sal_Char szServiceName_UnoControlContainerModel[] = "stardiv.vcl.controlmodel.ControlContainer", szServiceName2_UnoControlContainerModel[] = "com.sun.star.awt.UnoControlContainerModel";
const sal_Char szServiceName_TabController[] = "stardiv.vcl.control.TabController", szServiceName2_TabController[] = "com.sun.star.awt.TabController";
const sal_Char szServiceName_TabControllerModel[] = "stardiv.vcl.controlmodel.TabController", szServiceName2_TabControllerModel[] = "com.sun.star.awt.TabControllerModel";
const sal_Char szServiceName_UnoControlDialog[] = "stardiv.vcl.control.Dialog", szServiceName2_UnoControlDialog[] = "com.sun.star.awt.UnoControlDialog";
const sal_Char szServiceName_UnoControlDialogModel[] = "stardiv.vcl.controlmodel.Dialog", szServiceName2_UnoControlDialogModel[] = "com.sun.star.awt.UnoControlDialogModel";
const sal_Char szServiceName_UnoControlEdit[] = "stardiv.vcl.control.Edit", szServiceName2_UnoControlEdit[] = "com.sun.star.awt.UnoControlEdit";
const sal_Char szServiceName_UnoControlEditModel[] = "stardiv.vcl.controlmodel.Edit", szServiceName2_UnoControlEditModel[] = "com.sun.star.awt.UnoControlEditModel";
const sal_Char szServiceName_UnoControlFileControl[] = "stardiv.vcl.control.FileControl", szServiceName2_UnoControlFileControl[] = "com.sun.star.awt.UnoControlFileControl";
const sal_Char szServiceName_UnoControlFileControlModel[] = "stardiv.vcl.controlmodel.FileControl", szServiceName2_UnoControlFileControlModel[] = "com.sun.star.awt.UnoControlFileControlModel";
const sal_Char szServiceName_UnoControlButton[] = "stardiv.vcl.control.Button", szServiceName2_UnoControlButton[] = "com.sun.star.awt.UnoControlButton";
const sal_Char szServiceName_UnoControlButtonModel[] = "stardiv.vcl.controlmodel.Button", szServiceName2_UnoControlButtonModel[] = "com.sun.star.awt.UnoControlButtonModel";
const sal_Char szServiceName_UnoControlImageButton[] = "stardiv.vcl.control.ImageButton", szServiceName2_UnoControlImageButton[] = "com.sun.star.awt.UnoControlImageButton";
const sal_Char szServiceName_UnoControlImageButtonModel[] = "stardiv.vcl.controlmodel.ImageButton", szServiceName2_UnoControlImageButtonModel[] = "com.sun.star.awt.UnoControlImageButtonModel";
const sal_Char szServiceName_UnoControlImageControl[] = "stardiv.vcl.control.ImageControl", szServiceName2_UnoControlImageControl[] = "com.sun.star.awt.UnoControlImageControl";
const sal_Char szServiceName_UnoControlImageControlModel[] = "stardiv.vcl.controlmodel.ImageControl", szServiceName2_UnoControlImageControlModel[] = "com.sun.star.awt.UnoControlImageControlModel";
const sal_Char szServiceName_UnoControlRadioButton[] = "stardiv.vcl.control.RadioButton", szServiceName2_UnoControlRadioButton[] = "com.sun.star.awt.UnoControlRadioButton";
const sal_Char szServiceName_UnoControlRadioButtonModel[] = "stardiv.vcl.controlmodel.RadioButton", szServiceName2_UnoControlRadioButtonModel[] = "com.sun.star.awt.UnoControlRadioButtonModel";
const sal_Char szServiceName_UnoControlCheckBox[] = "stardiv.vcl.control.CheckBox", szServiceName2_UnoControlCheckBox[] = "com.sun.star.awt.UnoControlCheckBox";
const sal_Char szServiceName_UnoControlCheckBoxModel[] = "stardiv.vcl.controlmodel.CheckBox", szServiceName2_UnoControlCheckBoxModel[] = "com.sun.star.awt.UnoControlCheckBoxModel";
const sal_Char szServiceName_UnoControlListBox[] = "stardiv.vcl.control.ListBox", szServiceName2_UnoControlListBox[] = "com.sun.star.awt.UnoControlListBox";
const sal_Char szServiceName_UnoControlListBoxModel[] = "stardiv.vcl.controlmodel.ListBox", szServiceName2_UnoControlListBoxModel[] = "com.sun.star.awt.UnoControlListBoxModel";
const sal_Char szServiceName_UnoControlComboBox[] = "stardiv.vcl.control.ComboBox", szServiceName2_UnoControlComboBox[] = "com.sun.star.awt.UnoControlComboBox";
const sal_Char szServiceName_UnoControlComboBoxModel[] = "stardiv.vcl.controlmodel.ComboBox", szServiceName2_UnoControlComboBoxModel[] = "com.sun.star.awt.UnoControlComboBoxModel";
const sal_Char szServiceName_UnoControlFixedText[] = "stardiv.vcl.control.FixedText", szServiceName2_UnoControlFixedText[] = "com.sun.star.awt.UnoControlFixedText";
const sal_Char szServiceName_UnoControlFixedTextModel[] = "stardiv.vcl.controlmodel.FixedText", szServiceName2_UnoControlFixedTextModel[] = "com.sun.star.awt.UnoControlFixedTextModel";
const sal_Char szServiceName_UnoControlGroupBox[] = "stardiv.vcl.control.GroupBox", szServiceName2_UnoControlGroupBox[] = "com.sun.star.awt.UnoControlGroupBox";
const sal_Char szServiceName_UnoControlGroupBoxModel[] = "stardiv.vcl.controlmodel.GroupBox", szServiceName2_UnoControlGroupBoxModel[] = "com.sun.star.awt.UnoControlGroupBoxModel";
const sal_Char szServiceName_UnoControlDateField[] = "stardiv.vcl.control.DateField", szServiceName2_UnoControlDateField[] = "com.sun.star.awt.UnoControlDateField";
const sal_Char szServiceName_UnoControlDateFieldModel[] = "stardiv.vcl.controlmodel.DateField", szServiceName2_UnoControlDateFieldModel[] = "com.sun.star.awt.UnoControlDateFieldModel";
const sal_Char szServiceName_UnoControlTimeField[] = "stardiv.vcl.control.TimeField", szServiceName2_UnoControlTimeField[] = "com.sun.star.awt.UnoControlTimeField";
const sal_Char szServiceName_UnoControlTimeFieldModel[] = "stardiv.vcl.controlmodel.TimeField", szServiceName2_UnoControlTimeFieldModel[] = "com.sun.star.awt.UnoControlTimeFieldModel";
const sal_Char szServiceName_UnoControlNumericField[] = "stardiv.vcl.control.NumericField", szServiceName2_UnoControlNumericField[] = "com.sun.star.awt.UnoControlNumericField";
const sal_Char szServiceName_UnoControlNumericFieldModel[] = "stardiv.vcl.controlmodel.NumericField", szServiceName2_UnoControlNumericFieldModel[] = "com.sun.star.awt.UnoControlNumericFieldModel";
const sal_Char szServiceName_UnoControlCurrencyField[] = "stardiv.vcl.control.CurrencyField", szServiceName2_UnoControlCurrencyField[] = "com.sun.star.awt.UnoControlCurrencyField";
const sal_Char szServiceName_UnoControlCurrencyFieldModel[] = "stardiv.vcl.controlmodel.CurrencyField", szServiceName2_UnoControlCurrencyFieldModel[] = "com.sun.star.awt.UnoControlCurrencyFieldModel";
const sal_Char szServiceName_UnoControlPatternField[] = "stardiv.vcl.control.PatternField", szServiceName2_UnoControlPatternField[] = "com.sun.star.awt.UnoControlPatternField";
const sal_Char szServiceName_UnoControlPatternFieldModel[] = "stardiv.vcl.controlmodel.PatternField", szServiceName2_UnoControlPatternFieldModel[] = "com.sun.star.awt.UnoControlPatternFieldModel";
const sal_Char szServiceName_UnoControlFormattedField[] = "stardiv.vcl.control.FormattedField", szServiceName2_UnoControlFormattedField[] = "com.sun.star.awt.UnoControlFormattedField";
const sal_Char szServiceName_UnoControlFormattedFieldModel[] = "stardiv.vcl.controlmodel.FormattedField", szServiceName2_UnoControlFormattedFieldModel[] = "com.sun.star.awt.UnoControlFormattedFieldModel";
const sal_Char szServiceName_MVCIntrospection[] = "stardiv.vcl.MVCIntrospection", szServiceName2_MVCIntrospection[] = "com.sun.star.awt.MVCIntrospection";
const sal_Char szServiceName_PrinterServer[] = "stardiv.vcl.PrinterServer", szServiceName2_PrinterServer[] = "com.sun.star.awt.PrinterServer";
const sal_Char szServiceName_UnoControlProgressBar[] = "stardiv.vcl.control.ProgressBar", szServiceName2_UnoControlProgressBar[] = "com.sun.star.awt.UnoControlProgressBar";
const sal_Char szServiceName_UnoControlProgressBarModel[] = "stardiv.vcl.controlmodel.ProgressBar", szServiceName2_UnoControlProgressBarModel[] = "com.sun.star.awt.UnoControlProgressBarModel";
const sal_Char szServiceName_UnoControlScrollBar[] = "stardiv.vcl.control.ScrollBar", szServiceName2_UnoControlScrollBar[] = "com.sun.star.awt.UnoControlScrollBar";
const sal_Char szServiceName_UnoControlScrollBarModel[] = "stardiv.vcl.controlmodel.ScrollBar", szServiceName2_UnoControlScrollBarModel[] = "com.sun.star.awt.UnoControlScrollBarModel";
const sal_Char szServiceName_UnoControlFixedLine[] = "stardiv.vcl.control.FixedLine", szServiceName2_UnoControlFixedLine[] = "com.sun.star.awt.UnoControlFixedLine";
const sal_Char szServiceName_UnoControlFixedLineModel[] = "stardiv.vcl.controlmodel.FixedLine", szServiceName2_UnoControlFixedLineModel[] = "com.sun.star.awt.UnoControlFixedLineModel";
const sal_Char szServiceName_UnoControlRoadmap[] = "stardiv.vcl.control.Roadmap", szServiceName2_UnoControlRoadmap[] = "com.sun.star.awt.UnoControlRoadmap";
const sal_Char szServiceName_UnoControlRoadmapModel[] = "stardiv.vcl.controlmodel.Roadmap", szServiceName2_UnoControlRoadmapModel[] = "com.sun.star.awt.UnoControlRoadmapModel";
const sal_Char szServiceName_UnoSpinButtonControl[] = "com.sun.star.awt.UnoControlSpinButton";
const sal_Char szServiceName_UnoSpinButtonModel[] = "com.sun.star.awt.UnoControlSpinButtonModel";
const sal_Char szServiceName_UnoMultiPageControl[] = "com.sun.star.awt.UnoControlMultiPage";
const sal_Char szServiceName_UnoMultiPageModel[] = "com.sun.star.awt.UnoMultiPageModel";
const sal_Char szServiceName_UnoPageControl[] = "com.sun.star.awt.UnoControlPage";
const sal_Char szServiceName_UnoPageModel[] = "com.sun.star.awt.UnoPageModel";
const sal_Char szServiceName_UnoFrameControl[] = "com.sun.star.awt.UnoControlFrame";
const sal_Char szServiceName_AnimatedImagesControl[] = "com.sun.star.awt.AnimatedImagesControl";
const sal_Char szServiceName_AnimatedImagesControlModel[] = "com.sun.star.awt.AnimatedImagesControlModel";
const sal_Char szServiceName_SpinningProgressControlModel[] = "com.sun.star.awt.SpinningProgressControlModel";
const sal_Char szServiceName_UnoFrameModel[] = "com.sun.star.awt.UnoFrameModel";
const sal_Char szServiceName_TreeControl[] = "com.sun.star.awt.tree.TreeControl";
const sal_Char szServiceName_TreeControlModel[] = "com.sun.star.awt.tree.TreeControlModel";
const sal_Char szServiceName_MutableTreeDataModel[] = "com.sun.star.awt.tree.MutableTreeDataModel";
const sal_Char szServiceName_UnoSimpleAnimationControlModel[] = "com.sun.star.awt.UnoSimpleAnimationControlModel", szServiceName2_UnoSimpleAnimationControlModel[] = "com.sun.star.awt.UnoControlSimpleAnimationModel";
const sal_Char szServiceName_UnoSimpleAnimationControl[] = "com.sun.star.awt.UnoSimpleAnimationControl", szServiceName2_UnoSimpleAnimationControl[] = "com.sun.star.awt.UnoControlSimpleAnimation";
const sal_Char szServiceName_UnoThrobberControlModel[] = "com.sun.star.awt.UnoThrobberControlModel", szServiceName2_UnoThrobberControlModel[] = "com.sun.star.awt.UnoControlThrobberModel";
const sal_Char szServiceName_UnoThrobberControl[] = "com.sun.star.awt.UnoThrobberControl", szServiceName2_UnoThrobberControl[] = "com.sun.star.awt.UnoControlThrobber";
const sal_Char szServiceName_UnoControlFixedHyperlink[] = "com.sun.star.awt.UnoControlFixedHyperlink";
const sal_Char szServiceName_UnoControlFixedHyperlinkModel[] = "com.sun.star.awt.UnoControlFixedHyperlinkModel";
const sal_Char szServiceName_GridControl[] = "com.sun.star.awt.grid.UnoControlGrid";
const sal_Char szServiceName_GridControlModel[] = "com.sun.star.awt.grid.UnoControlGridModel";
const sal_Char szServiceName_DefaultGridDataModel[] = "com.sun.star.awt.grid.DefaultGridDataModel";
const sal_Char szServiceName_DefaultGridColumnModel[] = "com.sun.star.awt.grid.DefaultGridColumnModel";
const sal_Char szServiceName_GridColumn[] = "com.sun.star.awt.grid.GridColumn";
const sal_Char szServiceName_UnoControlTabPage[] = "com.sun.star.awt.tab.UnoControlTabPage";
const sal_Char szServiceName_UnoControlTabPageModel[] = "com.sun.star.awt.tab.UnoControlTabPageModel";
const sal_Char szServiceName_UnoControlTabPageContainerModel[] = "com.sun.star.awt.tab.UnoControlTabPageContainerModel";
const sal_Char szServiceName_UnoControlTabPageContainer[] = "com.sun.star.awt.tab.UnoControlTabPageContainer";
const sal_Char szServiceName_SortableGridDataModel[] = "com.sun.star.awt.grid.SortableGridDataModel";

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
