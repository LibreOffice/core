#*************************************************************************
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.2 $
#
#   last change: $Author: hjs $ $Date: 2000-11-02 12:20:09 $
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
PRJ=..$/..$/..

PRJNAME=plugin
TARGET=plbase
ENABLE_EXCEPTIONS=TRUE

.INCLUDE :  ..$/util$/makefile.pmk

# --- Types -------------------------------------

UNOUCRDEP=$(SOLARBINDIR)$/applicat.rdb makefile.mk
UNOUCRRDB=$(SOLARBINDIR)$/applicat.rdb

UNOUCROUT=$(OUT)$/inc$/$(PRJNAME)
INCPRE+=$(UNOUCROUT)

# --- Types -------------------------------------

UNOTYPES=\
    com.sun.star.awt.PosSize							\
    com.sun.star.awt.MouseButton						\
    com.sun.star.awt.FocusChangeReason					\
    com.sun.star.awt.KeyFunction						\
    com.sun.star.awt.Key								\
    com.sun.star.awt.KeyModifier						\
    com.sun.star.awt.KeyGroup							\
    com.sun.star.awt.XAdjustmentListener				\
    com.sun.star.awt.XActionListener					\
    com.sun.star.awt.XTextListener						\
    com.sun.star.awt.XSpinListener						\
    com.sun.star.awt.XItemListener						\
    com.sun.star.awt.XVclContainerListener				\
    com.sun.star.awt.XVclContainerPeer					\
    com.sun.star.awt.XVclContainer						\
    com.sun.star.awt.XVclWindowPeer						\
    com.sun.star.awt.VclWindowPeerAttribute				\
    com.sun.star.awt.XActivateListener					\
    com.sun.star.awt.XTopWindowListener					\
    com.sun.star.awt.XKeyListener						\
    com.sun.star.awt.XTopWindow							\
    com.sun.star.awt.XControl							\
    com.sun.star.awt.XControlModel						\
    com.sun.star.awt.XUnoControlContainer				\
    com.sun.star.beans.PropertyValues					\
    com.sun.star.beans.PropertyAttribute				\
    com.sun.star.beans.XPropertySet						\
    com.sun.star.beans.XMultiPropertySet				\
    com.sun.star.beans.XFastPropertySet					\
    com.sun.star.beans.XPropertyState					\
    com.sun.star.beans.XPropertyAccess					\
    com.sun.star.beans.XPropertyContainer				\
    com.sun.star.beans.XPropertyChangeListener			\
    com.sun.star.beans.XPropertyStateChangeListener		\
    com.sun.star.plugin.PluginMode						\
    com.sun.star.plugin.XPluginManager					\
    com.sun.star.io.XConnectable						\
    com.sun.star.io.XActiveDataSource					\
    com.sun.star.io.XActiveDataSink						\
    com.sun.star.io.XActiveDataControl					\
    com.sun.star.io.XDataOutputStream					\
    com.sun.star.io.XOutputStream						\
    com.sun.star.io.XDataInputStream					\
    com.sun.star.io.XObjectInputStream					\
    com.sun.star.io.XMarkableStream						\
    com.sun.star.lang.XServiceName						\
    com.sun.star.lang.XServiceInfo						\
    com.sun.star.lang.XTypeProvider						\
    com.sun.star.lang.XMultiServiceFactory				\
    com.sun.star.lang.XSingleServiceFactory				\
    com.sun.star.registry.XRegistryKey					\
    com.sun.star.loader.XImplementationLoader			\
    com.sun.star.container.XSet							\
    com.sun.star.container.XNameAccess					\
    com.sun.star.container.XIndexAccess					\
    com.sun.star.frame.XComponentLoader					\
    com.sun.star.frame.FrameSearchFlag					\
    com.sun.star.uno.TypeClass							\
    com.sun.star.uno.XAggregation						\
    com.sun.star.uno.XWeak

# --- Files --------------------------------------------------------

SLOFILES=		\
                $(SLO)$/plctrl.obj		\
                $(SLO)$/service.obj		\
                $(SLO)$/xplugin.obj		\
                $(SLO)$/nfuncs.obj		\
                $(SLO)$/manager.obj		\
                $(SLO)$/context.obj		\
                $(SLO)$/evtlstnr.obj	\
                $(SLO)$/plcom.obj		\
                $(SLO)$/multiplx.obj    \
                $(SLO)$/plmodel.obj

# --- Targets ------------------------------------------------------

.INCLUDE :  target.mk

.INCLUDE :  ..$/util$/target.pmk
