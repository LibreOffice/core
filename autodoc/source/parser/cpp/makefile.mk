#*************************************************************************
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.2 $
#
#   last change: $Author: obo $ $Date: 2005-01-27 11:22:53 $
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
#   WITHOUT WARRUNTY OF ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING,
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

PRJNAME=autodoc
TARGET=parser_cpp


# --- Settings -----------------------------------------------------

ENABLE_EXCEPTIONS=true
PRJINC=$(PRJ)$/source


.INCLUDE :  settings.mk
.INCLUDE : $(PRJ)$/source$/mkinc$/fullcpp.mk



# --- Files --------------------------------------------------------


OBJFILES= \
    $(OBJ)$/all_toks.obj	\
    $(OBJ)$/c_dealer.obj	\
    $(OBJ)$/c_rcode.obj		\
    $(OBJ)$/cpp_pe.obj		\
    $(OBJ)$/cx_base.obj	    \
    $(OBJ)$/cx_c_pp.obj	    \
    $(OBJ)$/cx_c_std.obj	\
    $(OBJ)$/cx_c_sub.obj	\
    $(OBJ)$/cxt2ary.obj	    \
    $(OBJ)$/defdescr.obj	\
    $(OBJ)$/icprivow.obj	\
    $(OBJ)$/pe_base.obj		\
    $(OBJ)$/pe_class.obj	\
    $(OBJ)$/pe_defs.obj	    \
    $(OBJ)$/pe_expr.obj	    \
    $(OBJ)$/pe_enum.obj	    \
    $(OBJ)$/pe_enval.obj    \
    $(OBJ)$/pe_file.obj		\
    $(OBJ)$/pe_funct.obj	\
    $(OBJ)$/pe_ignor.obj	\
    $(OBJ)$/pe_namsp.obj	\
    $(OBJ)$/pe_param.obj	\
    $(OBJ)$/pe_tpltp.obj    \
    $(OBJ)$/pe_type.obj		\
    $(OBJ)$/pe_tydef.obj	\
    $(OBJ)$/pe_vafu.obj		\
    $(OBJ)$/pe_vari.obj		\
    $(OBJ)$/preproc.obj		\
    $(OBJ)$/prs_cpp.obj		\
    $(OBJ)$/tkp_cpp.obj



# --- Targets ------------------------------------------------------

.INCLUDE :  target.mk



