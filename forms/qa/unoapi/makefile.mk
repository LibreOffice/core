#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.6 $
#
#   last change: $Author: obo $ $Date: 2007-01-25 12:26:34 $
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

PRJNAME=forms
TARGET=qa_unoapi

ALLTAR:
    $(SOLARENV)$/bin$/checkapi -sce forms.sce -xcl knownissues.xcl -DOCPTH $(PWD)$/testdocuments -THRCNT 1 -tdoc $(PWD)$/testdocuments
    @echo =======================================================================
    @echo Problems with controls might me focus problems during the run of XView
    @echo these seems to appear by chance ... just re-run those testcases with
    @echo $(SOLARENV)$/bin$/checkapi -o componentName -xcl knownissues.xcl -DOCPTH $(PWD)$/testdocuments -THRCNT 1 -tdoc $(PWD)$/testdocuments
    @echo =======================================================================			

run_%:
    $(SOLARENV)$/bin$/checkapi -o $(PRJNAME).$(@:s/run_//) -xcl knownissues.xcl -DOCPTH $(PWD)$/testdocuments -THRCNT 1 -tdoc $(PWD)$/testdocuments
