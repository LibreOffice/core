#*************************************************************************
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.2 $
#
#   last change: $Author: jbu $ $Date: 2000-10-09 07:31:48 $
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

PRJ		= ..$/..$/..$/..$/..
PRJNAME = jurt
PACKAGE = com$/sun$/star$/comp$/bridgefactory
TARGET  = com_sun_star_comp_bridgefactory

# --- Settings -----------------------------------------------------

.INCLUDE : $(PRJ)$/util$/makefile.pmk

# --- Files --------------------------------------------------------

GENJAVAFILES = 																\
    $(MISC)$/java$/com$/sun$/star$/uno$/XInterface.java						\
    $(MISC)$/java$/com$/sun$/star$/bridge$/BridgeExistsException.java		\
    $(MISC)$/java$/com$/sun$/star$/bridge$/XBridgeFactory.java				\
    $(MISC)$/java$/com$/sun$/star$/bridge$/XBridge.java						\
    $(MISC)$/java$/com$/sun$/star$/bridge$/XInstanceProvider.java			\
    $(MISC)$/java$/com$/sun$/star$/lang$/XEventListener.java			\
    $(MISC)$/java$/com$/sun$/star$/lang$/XComponent.java			\
    $(MISC)$/java$/com$/sun$/star$/container$/NoSuchElementException.java	\
    $(MISC)$/java$/com$/sun$/star$/uno$/Exception.java						\
    $(MISC)$/java$/com$/sun$/star$/io$/IOException.java						\
    $(MISC)$/java$/com$/sun$/star$/connection$/XConnection.java

JAVACLASSFILES= \
    $(CLASSDIR)$/$(PACKAGE)$/BridgeFactory.class

# --- Targets ------------------------------------------------------

.INCLUDE :  target.mk
