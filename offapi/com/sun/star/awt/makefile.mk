#*************************************************************************
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.7 $
#
#   last change: $Author: obr $ $Date: 2001-03-12 12:36:17 $
#
#   The Contents of this file are made available subject to the terms of
#   either of the following licenses
#
#          - GNU Lesser General Public License Version 2.1
#          - Sun Industry Standards Source License Version 1.1
#
#   Sun Microsystems Inc., October, 2000
#
#   GNU Lesser General Public License Version 2.1
#   =============================================
#   Copyright 2000 by Sun Microsystems, Inc.
#   901 San Antonio Road, Palo Alto, CA 94303, USA
#
#   This library is free software; you can redistribute it and/or
#   modify it under the terms of the GNU Lesser General Public
#   License version 2.1, as published by the Free Software Foundation.
#
#   This library is distributed in the hope that it will be useful,
#   but WITHOUT ANY WARRANTY; without even the implied warranty of
#   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
#   Lesser General Public License for more details.
#
#   You should have received a copy of the GNU Lesser General Public
#   License along with this library; if not, write to the Free Software
#   Foundation, Inc., 59 Temple Place, Suite 330, Boston,
#   MA  02111-1307  USA
#
#
#   Sun Industry Standards Source License Version 1.1
#   =================================================
#   The contents of this file are subject to the Sun Industry Standards
#   Source License Version 1.1 (the "License"); You may not use this file
#   except in compliance with the License. You may obtain a copy of the
#   License at http://www.openoffice.org/license.html.
#
#   Software provided under this License is provided on an "AS IS" basis,
#   WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
#   WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
#   MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
#   See the License for the specific provisions governing your rights and
#   obligations concerning the Software.
#
#   The Initial Developer of the Original Code is: Sun Microsystems, Inc.
#
#   Copyright: 2000 by Sun Microsystems, Inc.
#
#   All Rights Reserved.
#
#   Contributor(s): _______________________________________
#
#
#
#*************************************************************************

PRJ=..$/..$/..$/..

PRJNAME=api

TARGET=cssawt
PACKAGE=com$/sun$/star$/awt

# --- Settings -----------------------------------------------------
.INCLUDE :  $(PRJ)$/util$/makefile.pmk

# ------------------------------------------------------------------------

IDLFILES=\
    ActionEvent.idl\
    AdjustmentEvent.idl\
    AdjustmentType.idl\
    CharSet.idl\
    DeviceCapability.idl\
    DeviceInfo.idl\
    FocusChangeReason.idl\
    FocusEvent.idl\
    FontDescriptor.idl\
    FontFamily.idl\
    FontPitch.idl\
    FontSlant.idl\
    FontStrikeout.idl\
    FontType.idl\
    FontUnderline.idl\
    FontWeight.idl\
    FontWidth.idl\
    Gradient.idl\
    GradientStyle.idl\
    ImageStatus.idl\
    InputEvent.idl\
    InvalidateStyle.idl\
    ItemEvent.idl\
    Key.idl\
    KeyEvent.idl\
    KeyFunction.idl\
    KeyGroup.idl\
    KeyModifier.idl\
    MenuEvent.idl\
    MenuItemStyle.idl\
    MessageBoxCommand.idl\
    MouseButton.idl\
    MouseEvent.idl\
    PaintEvent.idl\
    Point.idl\
    PopupMenuDirection.idl\
    PosSize.idl\
    PrinterException.idl\
    RasterOperation.idl\
    Rectangle.idl\
    ScrollBarOrientation.idl\
    Selection.idl\
    SimpleFontMetric.idl\
    Size.idl\
    SpinEvent.idl\
    Style.idl\
    SystemPointer.idl\
    TextAlign.idl\
    TextEvent.idl\
    Toolkit.idl\
    UnoButtonControl.idl\
    UnoCheckBoxControl.idl\
    UnoComboBoxControl.idl\
    UnoControl.idl\
    UnoControlButtonModel.idl\
    UnoControlCheckBoxModel.idl\
    UnoControlComboBoxModel.idl\
    UnoControlContainer.idl\
    UnoControlContainerModel.idl\
    UnoControlCurrencyFieldModel.idl\
    UnoControlDateFieldModel.idl\
    UnoControlEditModel.idl\
    UnoControlFileControlModel.idl\
    UnoControlFixedTextModel.idl\
    UnoControlFormattedFieldModel.idl\
    UnoControlGroupBoxModel.idl\
    UnoControlImageControlModel.idl\
    UnoControlListBoxModel.idl\
    UnoControlModel.idl\
    UnoControlNumericFieldModel.idl\
    UnoControlPatternFieldModel.idl\
    UnoControlRadioButtonModel.idl\
    UnoControlTimeFieldModel.idl\
    UnoCurrencyFieldControl.idl\
    UnoEditControl.idl\
    UnoFileControl.idl\
    UnoFixedTextControl.idl\
    UnoGroupBoxControl.idl\
    UnoImageControl.idl\
    UnoListBoxControl.idl\
    UnoNumericFieldControl.idl\
    UnoPatternFieldControl.idl\
    UnoRadioButtonControl.idl\
    UnoTimeFieldControl.idl\
    VclContainerEvent.idl\
    VclWindowPeerAttribute.idl\
    WindowAttribute.idl\
    WindowClass.idl\
    WindowDescriptor.idl\
    WindowEvent.idl\
    XActionListener.idl\
    XActivateListener.idl\
    XAdjustmentListener.idl\
    XBitmap.idl\
    XButton.idl\
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
    XDisplayBitmap.idl\
    XDisplayConnection.idl\
    XEventHandler.idl\
    XFileDialog.idl\
    XFixedText.idl\
    XFocusListener.idl\
    XFont.idl\
    XGraphics.idl\
    XImageButton.idl\
    XImageConsumer.idl\
    XImageProducer.idl\
    XInfoPrinter.idl\
    XItemListener.idl\
    XKeyListener.idl\
    XLayoutConstrains.idl\
    XListBox.idl\
    XMenu.idl\
    XMenuBar.idl\
    XMenuListener.idl\
    XMessageBox.idl\
    XMouseListener.idl\
    XMouseMotionListener.idl\
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
    XScrollBar.idl\
    XSpinField.idl\
    XSpinListener.idl\
    XSystemDependentWindowPeer.idl\
    SystemDependentXWindow.idl\
    XTabController.idl\
    XTabControllerModel.idl\
    XTextArea.idl\
    XTextComponent.idl\
    XTextEditField.idl\
    XTextLayoutConstrains.idl\
    XTextListener.idl\
    XTimeField.idl\
    XToolkit.idl\
    XTopWindow.idl\
    XTopWindowListener.idl\
    XUnoControlContainer.idl\
    XVclContainer.idl\
    XVclContainerListener.idl\
    XVclContainerPeer.idl\
    XVclWindowPeer.idl\
    XView.idl\
    XWindow.idl\
    XWindowListener.idl\
    XWindowPeer.idl

# ------------------------------------------------------------------

.INCLUDE :  target.mk
.INCLUDE :  $(PRJ)$/util$/target.pmk
