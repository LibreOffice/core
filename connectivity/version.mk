#*************************************************************************
#
#   $RCSfile: version.mk,v $
#
#   $Revision: 1.2 $
#
#   last change: $Author: oj $ $Date: 2000-10-06 06:19:39 $
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

# ----------------------------ADO settings------------------------------------#
# target
ADO_TARGET=ado

# the major 
ADO_MAJOR=2
# the minor 
ADO_MINOR=0
# the micro 
ADO_MICRO=0

# this is a c++ compatible library 
ADO_CPP=1

ADO=$(ADO_TARGET_TARGET)_$(CMPEXT)

# ----------------------------ODBC settings-----------------------------------#
# target
ODBC_TARGET=odbc

# the major 
ODBC_MAJOR=2
# the minor 
ODBC_MINOR=0
# the micro 
ODBC_MICRO=0

# this is a c++ compatible library 
ODBC_CPP=1

ODBC=$(ODBC_TARGET_TARGET)_$(CMPEXT)

# ----------------------------JDBC settings-----------------------------------#
# target
JDBC_TARGET=jdbc

# the major 
JDBC_MAJOR=2
# the minor 
JDBC_MINOR=0
# the micro 
JDBC_MICRO=0

# this is a c++ compatible library 
JDBC_CPP=1

JDBC=$(JDBC_TARGET_TARGET)_$(CMPEXT)

# ----------------------------ADABAS settings-----------------------------------#
# target
ADABAS_TARGET=adabas

# the major 
ADABAS_MAJOR=2
# the minor 
ADABAS_MINOR=0
# the micro 
ADABAS_MICRO=0

# this is a c++ compatible library 
ADABAS_CPP=1

ADABAS=$(ADABAS_TARGET_TARGET)_$(CMPEXT)

# ----------------------------flat settings-----------------------------------#
# target
FLAT_TARGET=flat

# the major 
FLAT_MAJOR=2
# the minor 
FLAT_MINOR=0
# the micro 
FLAT_MICRO=0

# this is a c++ compatible library 
FLAT_CPP=1

FLAT=$(FLAT_TARGET_TARGET)_$(CMPEXT)

# -----------------------DRIVER MANAGER settings--------------------------------#
# target
SDBC_TARGET=sdbc

# the major 
SDBC_MAJOR=2
# the minor 
SDBC_MINOR=0
# the micro 
SDBC_MICRO=0

# this is a c++ compatible library 
SDBC_CPP=1

SDBC=$(SDBC_TARGET_TARGET)_$(CMPEXT)

# ----------------------------DBASE settings-----------------------------------#
# target
DBASE_TARGET=dbase

# the major 
DBASE_MAJOR=2
# the minor 
DBASE_MINOR=0
# the micro 
DBASE_MICRO=0

# this is a c++ compatible library 
DBASE_CPP=1

DBASE=$(DBASE_TARGET_TARGET)_$(CMPEXT)


