#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.5 $
#
#   last change: $Author: obo $ $Date: 2007-01-25 13:50:08 $
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

PRJNAME=dbaccess
TARGET=qa_unoapi

ALLTAR:
    $(SOLARENV)$/bin$/checkapi -sce dbaccess.sce -ini dbaccess.props -xcl knownissues.xcl -tdoc $(PWD)$/testdocuments
    @echo =======================================================================
    @echo In case of problems make sure that you put valid values in the dbaccess.props and added an appropriate jdbc-driver to your office installation
    @echo =======================================================================
    @echo In case of problems with TableWindowAccessibility or JoinViewAccessibility this might be because of connection problems, just re-run the testcases 
    @echo =======================================================================		

run_%:
    $(SOLARENV)$/bin$/checkapi -o dbaccess.$(@:s/run_//) -ini dbaccess.props -xcl knownissues.xcl -tdoc $(PWD)$/testdocuments
