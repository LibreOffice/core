#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.5 $
#
#   last change: $Author: rt $ $Date: 2005-09-08 22:53:09 $
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
PRJINC=$(PRJ)$/source$/inc
PRJNAME=forms
TARGET=fm_cppugen

# --- Settings ----------------------------------

.INCLUDE : settings.mk

# --- Types -------------------------------------



# --- Types -------------------------------------

UNOTYPES+=	\
        com.sun.star.form.binding.XBindableValue \
        com.sun.star.form.binding.XValueBinding \
        com.sun.star.form.binding.XListEntrySink \
        com.sun.star.form.binding.XListEntrySource \
        com.sun.star.form.binding.XListEntryListener \
        com.sun.star.form.validation.XValidator \
        com.sun.star.form.validation.XValidatable \
        com.sun.star.form.validation.XValidityConstraintListener \
        com.sun.star.form.validation.XValidatableFormComponent \
        com.sun.star.form.submission.XSubmissionSupplier \
        com.sun.star.xforms.XModel \
        com.sun.star.xforms.XFormsSupplier \
        com.sun.star.xforms.XSubmission \
        com.sun.star.xsd.WhiteSpaceTreatment \
        com.sun.star.xsd.XDataType \

# --- Targets ----------------------------------

.INCLUDE : target.mk

