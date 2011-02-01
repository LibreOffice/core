#*************************************************************************
#
# DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
# 
# Copyright 2000, 2010 Oracle and/or its affiliates.
#
# OpenOffice.org - a multi-platform office productivity suite
#
# This file is part of OpenOffice.org.
#
# OpenOffice.org is free software: you can redistribute it and/or modify
# it under the terms of the GNU Lesser General Public License version 3
# only, as published by the Free Software Foundation.
#
# OpenOffice.org is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU Lesser General Public License version 3 for more details
# (a copy is included in the LICENSE file that accompanied this code).
#
# You should have received a copy of the GNU Lesser General Public License
# version 3 along with OpenOffice.org.  If not, see
# <http://www.openoffice.org/license.html>
# for a copy of the LGPLv3 License.
#
#*************************************************************************

PRJ=..$/..$/..$/..

PRJNAME=offapi

TARGET=cssawt
PACKAGE=com$/sun$/star$/awt

# --- Settings -----------------------------------------------------
.INCLUDE :  $(PRJ)$/util$/makefile.pmk

# ------------------------------------------------------------------------

IDLFILES=\
    AccessibleButton.idl\
    AccessibleCheckBox.idl\
    AccessibleComboBox.idl\
    AccessibleDropDownComboBox.idl\
    AccessibleDropDownListBox.idl\
    AccessibleEdit.idl\
    AccessibleFixedText.idl\
    AccessibleIconChoiceControl.idl\
    AccessibleIconChoiceControlEntry.idl\
    AccessibleList.idl\
    AccessibleListBox.idl\
    AccessibleListBoxList.idl\
    AccessibleListItem.idl\
    AccessibleMenu.idl\
    AccessibleMenuBar.idl\
    AccessibleMenuItem.idl\
    AccessibleMenuSeparator.idl\
    AccessiblePopupMenu.idl\
    AccessibleRadioButton.idl\
    AccessibleScrollBar.idl\
    AccessibleStatusBar.idl\
    AccessibleStatusBarItem.idl\
    AccessibleTabBar.idl\
    AccessibleTabBarPage.idl\
    AccessibleTabBarPageList.idl\
    AccessibleTabControl.idl\
    AccessibleTabPage.idl\
    AccessibleTextField.idl\
    AccessibleToolBox.idl\
    AccessibleToolBoxItem.idl\
    AccessibleTreeListBox.idl\
    AccessibleTreeListBoxEntry.idl\
    AccessibleWindow.idl\
    ActionEvent.idl\
    AdjustmentEvent.idl\
    AdjustmentType.idl\
    AsyncCallback.idl\
    CharSet.idl\
    Command.idl\
    DeviceCapability.idl\
    DeviceInfo.idl\
    DialogProvider.idl\
    DialogProvider2.idl\
    ContainerWindowProvider.idl\
    DockingData.idl\
    DockingEvent.idl\
    EndDockingEvent.idl\
    EndPopupModeEvent.idl\
    EnhancedMouseEvent.idl\
    FieldUnit.idl\
    FocusChangeReason.idl\
    FocusEvent.idl\
    FontDescriptor.idl\
    FontEmphasisMark.idl\
    FontFamily.idl\
    FontPitch.idl\
    FontRelief.idl\
    FontSlant.idl\
    FontStrikeout.idl\
    FontType.idl\
    FontUnderline.idl\
    FontWeight.idl\
    FontWidth.idl\
    Gradient.idl\
    GradientStyle.idl\
    ImageAlign.idl\
    ImagePosition.idl\
    ImageScaleMode.idl\
    ImageStatus.idl\
    InputEvent.idl\
    InvalidateStyle.idl\
    ItemEvent.idl\
    Key.idl\
    KeyEvent.idl\
    KeyFunction.idl\
    KeyGroup.idl\
    KeyModifier.idl\
    KeyStroke.idl\
    LineEndFormat.idl\
    MenuEvent.idl\
    MenuItemStyle.idl\
    MessageBoxButtons.idl\
    MessageBoxCommand.idl\
    MouseButton.idl\
    MouseEvent.idl\
    MouseWheelBehavior.idl\
    PaintEvent.idl\
    Point.idl\
    PopupMenuDirection.idl\
    PosSize.idl\
    PrinterException.idl\
    PushButtonType.idl\
    RasterOperation.idl\
    Rectangle.idl\
    RoadmapItem.idl\
    ScrollBarOrientation.idl\
    Selection.idl\
    SimpleFontMetric.idl\
    Size.idl\
    SpinEvent.idl\
    Style.idl\
    SystemPointer.idl\
    TabController.idl\
    TabControllerModel.idl\
    TextAlign.idl\
    TextEvent.idl\
    Toolkit.idl\
    UnoControl.idl\
    UnoControlButton.idl\
    UnoControlButtonModel.idl\
    UnoControlCheckBox.idl\
    UnoControlCheckBoxModel.idl\
    UnoControlComboBox.idl\
    UnoControlComboBoxModel.idl\
    UnoControlContainer.idl\
    UnoControlContainerModel.idl\
    UnoControlCurrencyField.idl\
    UnoControlCurrencyFieldModel.idl\
    UnoControlDateField.idl\
    UnoControlDateFieldModel.idl\
    UnoControlDialog.idl\
    UnoControlDialogElement.idl\
    UnoControlDialogModel.idl\
    UnoControlDialogModelProvider.idl\
    UnoControlEdit.idl\
    UnoControlEditModel.idl\
    UnoControlFileControl.idl\
    UnoControlFileControlModel.idl\
    UnoControlFixedHyperlink.idl\
    UnoControlFixedHyperlinkModel.idl\
    UnoControlFixedLine.idl\
    UnoControlFixedLineModel.idl\
    UnoControlFixedText.idl\
    UnoControlFixedTextModel.idl\
    UnoControlFormattedField.idl\
    UnoControlFormattedFieldModel.idl\
    UnoControlGroupBox.idl\
    UnoControlGroupBoxModel.idl\
    UnoControlImageControl.idl\
    UnoControlImageControlModel.idl\
    UnoControlListBox.idl\
    UnoControlListBoxModel.idl\
    UnoControlModel.idl\
    UnoControlNumericField.idl\
    UnoControlNumericFieldModel.idl\
    UnoControlPatternField.idl\
    UnoControlPatternFieldModel.idl\
    UnoControlProgressBar.idl\
    UnoControlProgressBarModel.idl\
    UnoControlRadioButton.idl\
    UnoControlRadioButtonModel.idl\
    UnoControlRoadmap.idl\
    UnoControlRoadmapModel.idl\
    UnoControlScrollBar.idl\
    UnoControlScrollBarModel.idl\
    UnoControlSimpleAnimation.idl\
    UnoControlSimpleAnimationModel.idl\
    UnoControlSpinButton.idl\
    UnoControlSpinButtonModel.idl\
    UnoControlThrobber.idl\
    UnoControlThrobberModel.idl\
    UnoControlTimeField.idl\
    UnoControlTimeFieldModel.idl\
    VclContainerEvent.idl\
    VclWindowPeerAttribute.idl\
    VisualEffect.idl\
    WindowAttribute.idl\
    WindowClass.idl\
    WindowDescriptor.idl\
    WindowEvent.idl\
    XActionListener.idl\
    XActivateListener.idl\
    XAdjustmentListener.idl\
    XBitmap.idl\
    XButton.idl\
    XCallback.idl\
    XCheckBox.idl\
    XComboBox.idl\
    XControl.idl\
    XControlContainer.idl\
    XControlModel.idl\
    XCurrencyField.idl\
    XDataTransferProviderAccess.idl\
    XDateField.idl\
    XDevice.idl\
    XDialog.idl\
    XDialogProvider.idl\
    XDialogProvider2.idl\
    XDialogEventHandler.idl\
    ContainerWindowProvider.idl\
    XContainerWindowProvider.idl\
    XContainerWindowEventHandler.idl\
    XDisplayBitmap.idl\
    XDisplayConnection.idl\
    XDockableWindow.idl\
    XDockableWindowListener.idl\
    XEnhancedMouseClickHandler.idl\
    XEventHandler.idl\
    XExtendedToolkit.idl\
    XFileDialog.idl\
    XFixedHyperlink.idl\
    XFixedText.idl\
    XFocusListener.idl\
    XFont.idl\
    XFont2.idl\
    XGraphics.idl\
    XImageButton.idl\
    XImageConsumer.idl\
    XImageProducer.idl\
    XInfoPrinter.idl\
    XItemEventBroadcaster.idl\
    XItemListener.idl\
    XKeyHandler.idl\
    XKeyListener.idl\
    XLayoutConstrains.idl\
    XListBox.idl\
    XMenu.idl\
    XMenuBar.idl\
    XMenuListener.idl\
    XMessageBox.idl\
    XMessageBoxFactory.idl\
    XMetricField.idl\
    XMouseClickHandler.idl\
    XMouseListener.idl\
    XMouseMotionListener.idl\
    XMouseMotionHandler.idl\
    XNumericField.idl\
    XPaintListener.idl\
    XPatternField.idl\
    XPointer.idl\
    XPopupMenu.idl\
    XPrinter.idl\
    XPrinterPropertySet.idl\
    XPrinterServer.idl\
    XProgressBar.idl\
    XProgressMonitor.idl\
    XRadioButton.idl\
    XRegion.idl\
    XRequestCallback.idl\
    XReschedule.idl\
    XScrollBar.idl\
    XSimpleTabController.idl\
    XSpinField.idl\
    XSpinListener.idl\
    XSpinValue.idl\
    XStyleChangeListener.idl\
    XStyleSettings.idl\
    XStyleSettingsSupplier.idl\
    XSystemChildFactory.idl\
    XSystemDependentMenuPeer.idl\
    XSystemDependentWindowPeer.idl\
    SystemDependentXWindow.idl\
    XSimpleAnimation.idl\
    XSimpleTabController.idl\
    XThrobber.idl\
    XTabController.idl\
    XTabControllerModel.idl\
        XTabListener.idl\
    XTextArea.idl\
    XTextComponent.idl\
    XTextEditField.idl\
    XTextLayoutConstrains.idl\
    XTextListener.idl\
    XTimeField.idl\
        XToggleButton.idl\
    XToolkit.idl\
    XTopWindow.idl\
    XTopWindow2.idl\
    XTopWindowListener.idl\
    XUnitConversion.idl\
    XUnoControlContainer.idl\
    XUserInputInterception.idl\
    XVclContainer.idl\
    XVclContainerListener.idl\
    XVclContainerPeer.idl\
    XVclWindowPeer.idl\
    XView.idl\
    XWindow.idl\
    XWindow2.idl\
    XWindowListener.idl\
    XWindowListener2.idl\
    XWindowPeer.idl\
    XMenuExtended.idl\
    MaxChildrenException.idl\
    XDialog2.idl\
    XLayoutContainer.idl\
    XLayoutFlow.idl\
    XLayoutFlowContainer.idl\
    XLayoutRoot.idl\
    XLayoutUnit.idl \
    MenuItemType.idl \
    MenuLogo.idl \
    XMenuExtended2.idl \
    XMenuBarExtended.idl \
    XPopupMenuExtended.idl \
    XItemList.idl \
    XItemListListener.idl \
    ItemListEvent.idl \
    AnimatedImagesControl.idl \
    AnimatedImagesControlModel.idl \
    XAnimatedImages.idl \
    XAnimation.idl \
    SpinningProgressControlModel.idl

# ------------------------------------------------------------------

.INCLUDE :  target.mk
.INCLUDE :  $(PRJ)$/util$/target.pmk
