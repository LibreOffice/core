#*************************************************************************
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.7 $
#
#   last change: $Author: pluby $ $Date: 2001-03-02 07:21:11 $
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

PRJ=..

PRJNAME=vos
TARGET=cppvos

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk

# --- Files --------------------------------------------------------

SLOFILES=       $(SLO)$/acceptor.obj    \
                $(SLO)$/bytarray.obj    \
                $(SLO)$/bytestrm.obj    \
                $(SLO)$/connectr.obj    \
                $(SLO)$/conditn.obj     \
                $(SLO)$/fibersrv.obj    \
                $(SLO)$/mutex.obj       \
                $(SLO)$/object.obj      \
                $(SLO)$/reactor.obj     \
                $(SLO)$/refernce.obj    \
                $(SLO)$/runthrd.obj     \
                $(SLO)$/socket.obj      \
                $(SLO)$/semaphor.obj    \
                $(SLO)$/thread.obj      \
                $(SLO)$/thrdsrv.obj     \
                $(SLO)$/stream.obj      \
                $(SLO)$/module.obj      \
                $(SLO)$/profile.obj     \
                $(SLO)$/timer.obj       \
                $(SLO)$/process.obj     \
                $(SLO)$/security.obj    \
                $(SLO)$/signal.obj      \
                $(SLO)$/pipe.obj        \
                $(SLO)$/dynload.obj		\
                $(SLO)$/xception.obj	\
                $(SLO)$/evtque.obj

# $(SLO)$/evtque.obj

.IF "$(GUI)"=="WIN"
SLOFILES+=$(SLO)$/dllentry.obj
.ENDIF

.IF "$(UPDATER)"=="YES"
OBJFILES=       $(OBJ)$/acceptor.obj    \
                $(OBJ)$/bytarray.obj    \
                $(OBJ)$/bytestrm.obj    \
                $(OBJ)$/connectr.obj    \
                $(OBJ)$/conditn.obj     \
                $(OBJ)$/fibersrv.obj    \
                $(OBJ)$/mutex.obj       \
                $(OBJ)$/object.obj      \
                $(OBJ)$/reactor.obj     \
                $(OBJ)$/refernce.obj    \
                $(OBJ)$/runthrd.obj     \
                $(OBJ)$/socket.obj      \
                $(OBJ)$/semaphor.obj    \
                $(OBJ)$/thread.obj      \
                $(OBJ)$/thrdsrv.obj     \
                $(OBJ)$/stream.obj      \
                $(OBJ)$/module.obj      \
                $(OBJ)$/profile.obj     \
                $(OBJ)$/timer.obj       \
                $(OBJ)$/process.obj     \
                $(OBJ)$/security.obj    \
                $(OBJ)$/signal.obj      \
                $(OBJ)$/pipe.obj        \
                $(OBJ)$/dynload.obj		\
                $(OBJ)$/xception.obj	\
                $(OBJ)$/evtque.obj

# $(SLO)$/evtque.obj
.ENDIF

# --- Targets ------------------------------------------------------

.INCLUDE :  target.mk



