#*************************************************************************
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.10 $
#
#   last change: $Author: mt $ $Date: 2001-03-09 10:19:07 $
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

PRJ=..$/..

PRJNAME=SVTOOLS
TARGET=unoiface
LIBTARGET=NO
ENABLE_EXCEPTIONS=TRUE


# --- Settings -----------------------------------------------------

.INCLUDE : settings.mk

# --- Files --------------------------------------------------------

.IF "$(header)" == ""

UNOUCRDEP=$(SOLARBINDIR)$/applicat.rdb
UNOUCRRDB=$(SOLARBINDIR)$/applicat.rdb
UNOUCROUT=$(OUT)$/inc

#not all needed...

UNOTYPES= \
    com.sun.star.awt.CharSet \
    com.sun.star.awt.DeviceCapability \
    com.sun.star.awt.FontDescriptor \
    com.sun.star.awt.FontPitch \
    com.sun.star.awt.FontSlant \
    com.sun.star.awt.FontStrikeout \
    com.sun.star.awt.FontStrikeout \
    com.sun.star.awt.FontType \
    com.sun.star.awt.FontUnderline \
    com.sun.star.awt.FontWeight \
    com.sun.star.awt.FontWidth \
    com.sun.star.awt.KeyModifier \
    com.sun.star.awt.MenuItemStyle \
    com.sun.star.awt.MessageBoxCommand \
    com.sun.star.awt.MouseButton \
    com.sun.star.awt.PopupMenuDirection \
    com.sun.star.awt.ScrollBarOrientation \
    com.sun.star.awt.Style \
    com.sun.star.awt.TextAlign \
    com.sun.star.awt.VclWindowPeerAttribute \
    com.sun.star.awt.WindowAttribute \
    com.sun.star.awt.XActionListener \
    com.sun.star.awt.XAdjustmentListener \
    com.sun.star.awt.XBitmap \
    com.sun.star.awt.XButton \
    com.sun.star.awt.XCheckBox \
    com.sun.star.awt.XComboBox \
    com.sun.star.awt.XControl \
    com.sun.star.awt.XControlContainer \
    com.sun.star.awt.XControlModel \
    com.sun.star.awt.XCurrencyField \
    com.sun.star.awt.XDateField \
    com.sun.star.awt.XDevice \
    com.sun.star.awt.XDialog \
    com.sun.star.awt.XFileDialog \
    com.sun.star.awt.XFixedText \
    com.sun.star.awt.XFocusListener \
    com.sun.star.awt.XFont \
    com.sun.star.awt.XGraphics \
    com.sun.star.awt.XImageButton \
    com.sun.star.awt.XImageConsumer \
    com.sun.star.awt.XImageProducer \
    com.sun.star.awt.ImageStatus \
    com.sun.star.awt.XInfoPrinter \
    com.sun.star.awt.XItemListener \
    com.sun.star.awt.XKeyListener \
    com.sun.star.awt.XLayoutConstrains \
    com.sun.star.awt.XListBox \
    com.sun.star.awt.XMenuListener \
    com.sun.star.awt.XMessageBox \
    com.sun.star.awt.XMouseListener \
    com.sun.star.awt.XMouseMotionListener \
    com.sun.star.awt.XNumericField \
    com.sun.star.util.XNumberFormatsSupplier \
    com.sun.star.awt.XPaintListener \
    com.sun.star.awt.XPatternField \
    com.sun.star.awt.XPointer \
    com.sun.star.awt.XPrinter \
    com.sun.star.awt.XPrinterPropertySet \
    com.sun.star.awt.XPrinterServer \
    com.sun.star.awt.XProgressBar \
    com.sun.star.awt.XProgressMonitor \
    com.sun.star.awt.XRadioButton \
    com.sun.star.awt.XRegion \
    com.sun.star.awt.XScrollBar \
    com.sun.star.awt.XSpinField \
    com.sun.star.awt.XSpinListener \
    com.sun.star.awt.XSystemDependentWindowPeer \
    com.sun.star.awt.XTabController \
    com.sun.star.awt.XTabControllerModel \
    com.sun.star.awt.XTextArea \
    com.sun.star.awt.XTextComponent \
    com.sun.star.awt.XTextEditField \
    com.sun.star.awt.XTextLayoutConstrains \
    com.sun.star.awt.XTextListener \
    com.sun.star.awt.XTimeField \
    com.sun.star.awt.XToolkit \
    com.sun.star.awt.XTopWindow \
    com.sun.star.awt.XTopWindowListener \
    com.sun.star.awt.XUnoControlContainer \
    com.sun.star.awt.XVclContainer \
    com.sun.star.awt.XVclContainerListener \
    com.sun.star.awt.XVclContainerPeer \
    com.sun.star.awt.XVclWindowPeer \
    com.sun.star.awt.XWindow \
    com.sun.star.awt.XWindowListener \
    com.sun.star.beans.PropertyAttribute \
    com.sun.star.beans.PropertyChangeEvent \
    com.sun.star.beans.PropertyState \
    com.sun.star.beans.PropertyStateChangeEvent \
    com.sun.star.beans.PropertyValue \
    com.sun.star.beans.PropertyValues \
    com.sun.star.beans.XFastPropertySet \
    com.sun.star.beans.XMultiPropertySet \
    com.sun.star.beans.XPropertiesChangeListener \
    com.sun.star.beans.XPropertyAccess \
    com.sun.star.beans.XPropertyChangeListener \
    com.sun.star.beans.XPropertyContainer \
    com.sun.star.beans.XPropertySet \
    com.sun.star.beans.XPropertySetInfo \
    com.sun.star.beans.XPropertyState \
    com.sun.star.beans.XVetoableChangeListener \
    com.sun.star.container.XContainer \
    com.sun.star.container.XContainerListener \
    com.sun.star.container.XNameContainer \
    com.sun.star.frame.XModel \
    com.sun.star.frame.XFrameLoader \
    com.sun.star.frame.XLoadEventListener \
    com.sun.star.frame.XController \
    com.sun.star.frame.XDispatch \
    com.sun.star.frame.XDispatchProvider \
    com.sun.star.frame.XStorable \
    com.sun.star.frame.XFrame \
    com.sun.star.frame.FeatureStateEvent \
    com.sun.star.frame.DispatchDescriptor \
    com.sun.star.io.XInputStream \
    com.sun.star.io.XMarkableStream \
    com.sun.star.io.XObjectInputStream \
    com.sun.star.io.XConnectable \
    com.sun.star.io.XActiveDataControl \
    com.sun.star.io.XActiveDataSink \
    com.sun.star.io.XActiveDataSource \
    com.sun.star.lang.IllegalArgumentException \
    com.sun.star.lang.SystemDependent \
    com.sun.star.lang.XEventListener \
    com.sun.star.lang.XMultiServiceFactory \
    com.sun.star.lang.XServiceInfo \
    com.sun.star.lang.XServiceName \
    com.sun.star.lang.XSingleServiceFactory \
    com.sun.star.lang.XTypeProvider \
    com.sun.star.lang.XUnoTunnel \
    com.sun.star.registry.XRegistryKey \
    com.sun.star.uno.TypeClass \
    com.sun.star.util.XSearchable \
    com.sun.star.util.XSearchDescriptor \
    com.sun.star.util.XURLTransformer \
    com.sun.star.view.XScreenCursor \
    com.sun.star.text.XTextCursor \
    com.sun.star.text.XText \
    com.sun.star.text.XTextRange \
    com.sun.star.text.XTextViewCursor \
    com.sun.star.text.XTextViewCursorSupplier \
    com.sun.star.uno.XAggregation \
    com.sun.star.uno.XWeak


.IF "$(GUI)"!="DOS"
SRS1NAME=	uno
SRC1FILES=  unoifac2.src
.ENDIF

SLOFILES= \
            $(SLO)$/addrtempuno.obj	\
            $(SLO)$/genericunodialog.obj \
            $(SLO)$/registerservices.obj \
                        $(SLO)$/registerlight.obj \
            $(SLO)$/unoiface.obj	\
            $(SLO)$/imgprod.obj

LIB1TARGET= $(SLB)$/$(TARGET).lib
LIB1OBJFILES=  \
            $(SLO)$/addrtempuno.obj	\
            $(SLO)$/genericunodialog.obj \
            $(SLO)$/registerservices.obj \
            $(SLO)$/unoiface.obj	\
            $(SLO)$/imgprod.obj

.ENDIF

# --- Targets ------------------------------------------------------

.INCLUDE : target.mk
