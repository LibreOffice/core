#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.4 $
#
#   last change: $Author: vg $ $Date: 2007-09-28 10:51:57 $
#
#   The Contents of this file are made available subject to
#   the terms of GNU Lesser General Public License Version 2.1.
#
#
#     GNU Lesser General Public License Version 2.1
#     =============================================
#     Copyright 2005 by Sun Microsystems, Inc.
#     901 San Antonio Road, Palo Alto, CA 94303, USA
#
#     This library is free software; you can redistribute it and/or
#     modify it under the terms of the GNU Lesser General Public
#     License version 2.1, as published by the Free Software Foundation.
#
#     This library is distributed in the hope that it will be useful,
#     but WITHOUT ANY WARRANTY; without even the implied warranty of
#     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
#     Lesser General Public License for more details.
#
#     You should have received a copy of the GNU Lesser General Public
#     License along with this library; if not, write to the Free Software
#     Foundation, Inc., 59 Temple Place, Suite 330, Boston,
#     MA  02111-1307  USA
#
#*************************************************************************

PRJ=..$/..

PRJNAME=sysui
TARGET=icons

# --- Settings -----------------------------------------------------------

.INCLUDE :  settings.mk

# --- Targets -------------------------------------------------------------

all: \
    $(MISC)$/ooo-base-app.ico \
    $(MISC)$/ooo-base-doc.ico \
    $(MISC)$/ooo-calc-app.ico \
    $(MISC)$/ooo-calc-doc.ico \
    $(MISC)$/ooo-calc-tem.ico \
    $(MISC)$/ooo-chart-doc.ico \
    $(MISC)$/ooo-configuration.ico \
    $(MISC)$/ooo-draw-app.ico \
    $(MISC)$/ooo-draw-doc.ico \
    $(MISC)$/ooo-draw-tem.ico \
    $(MISC)$/ooo-empty-doc.ico \
    $(MISC)$/ooo-empty-tem.ico \
    $(MISC)$/ooo-image-doc.ico \
    $(MISC)$/ooo-impress-app.ico \
    $(MISC)$/ooo-impress-doc.ico \
    $(MISC)$/ooo-impress-tem.ico \
    $(MISC)$/ooo-macro-doc.ico \
    $(MISC)$/ooo-main-app.ico \
    $(MISC)$/ooo-master-doc.ico \
    $(MISC)$/ooo-math-app.ico \
    $(MISC)$/ooo-math-doc.ico \
    $(MISC)$/ooo-open.ico \
    $(MISC)$/ooo-printer.ico \
    $(MISC)$/ooo-web-doc.ico \
    $(MISC)$/ooo-writer-app.ico \
    $(MISC)$/ooo-writer-doc.ico \
    $(MISC)$/ooo-writer-tem.ico \
    $(MISC)$/ooo11-base-doc.ico \
    $(MISC)$/ooo11-calc-doc.ico \
    $(MISC)$/ooo11-calc-tem.ico \
    $(MISC)$/ooo11-chart-doc.ico \
    $(MISC)$/ooo11-draw-doc.ico \
    $(MISC)$/ooo11-draw-tem.ico \
    $(MISC)$/ooo11-impress-doc.ico \
    $(MISC)$/ooo11-impress-tem.ico \
    $(MISC)$/ooo11-master-doc.ico \
    $(MISC)$/ooo11-math-doc.ico \
    $(MISC)$/ooo11-writer-doc.ico \
    $(MISC)$/ooo11-writer-tem.ico \
    $(MISC)$/so8-base-app.ico \
    $(MISC)$/so8-base-doc.ico \
    $(MISC)$/so8-calc-app.ico \
    $(MISC)$/so8-calc-doc.ico \
    $(MISC)$/so8-calc-tem.ico \
    $(MISC)$/so8-chart-doc.ico \
    $(MISC)$/so8-configuration.ico \
    $(MISC)$/so8-draw-app.ico \
    $(MISC)$/so8-draw-doc.ico \
    $(MISC)$/so8-draw-tem.ico \
    $(MISC)$/so8-empty-doc.ico \
    $(MISC)$/so8-empty-tem.ico \
    $(MISC)$/so8-image-doc.ico \
    $(MISC)$/so8-impress-app.ico \
    $(MISC)$/so8-impress-doc.ico \
    $(MISC)$/so8-impress-tem.ico \
    $(MISC)$/so8-macro-doc.ico \
    $(MISC)$/so8-main-app.ico \
    $(MISC)$/so8-master-doc.ico \
    $(MISC)$/so8-math-app.ico \
    $(MISC)$/so8-math-doc.ico \
    $(MISC)$/so8-open.ico \
    $(MISC)$/so8-printer.ico \
    $(MISC)$/so8-web-doc.ico \
    $(MISC)$/so8-writer-app.ico \
    $(MISC)$/so8-writer-doc.ico \
    $(MISC)$/so8-writer-tem.ico \
    $(MISC)$/so7-base-doc.ico \
    $(MISC)$/so7-calc-doc.ico \
    $(MISC)$/so7-calc-tem.ico \
    $(MISC)$/so7-chart-doc.ico \
    $(MISC)$/so7-draw-doc.ico \
    $(MISC)$/so7-draw-tem.ico \
    $(MISC)$/so7-impress-doc.ico \
    $(MISC)$/so7-impress-tem.ico \
    $(MISC)$/so7-master-doc.ico \
    $(MISC)$/so7-math-doc.ico \
    $(MISC)$/so7-writer-doc.ico \
    $(MISC)$/so7-writer-tem.ico

$(MISC)$/%.ico: %.ico
    +$(COPY) $< $@

.INCLUDE :  target.mk

