#*************************************************************************
#
#	$RCSfile: makefile.mk,v $
#
#	$Revision: 1.15 $
#
#	last change: $Author: obo $ $Date: 2004-08-12 10:46:50 $
#
#	The Contents of this file are made available subject to the terms of
#	either of the following licenses
#
#		   - GNU Lesser General Public License Version 2.1
#		   - Sun Industry Standards Source License Version 1.1
#
#	Sun Microsystems Inc., October, 2000
#
#	GNU Lesser General Public License Version 2.1
#	=============================================
#	Copyright 2000 by Sun Microsystems, Inc.
#	901 San Antonio Road, Palo Alto, CA 94303, USA
#
#	This library is free software; you can redistribute it and/or
#	modify it under the terms of the GNU Lesser General Public
#	License version 2.1, as published by the Free Software Foundation.
#
#	This library is distributed in the hope that it will be useful,
#	but WITHOUT ANY WARRANTY; without even the implied warranty of
#	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
#	Lesser General Public License for more details.
#
#	You should have received a copy of the GNU Lesser General Public
#	License along with this library; if not, write to the Free Software
#	Foundation, Inc., 59 Temple Place, Suite 330, Boston,
#	MA	02111-1307	USA
#
#
#	Sun Industry Standards Source License Version 1.1
#	=================================================
#	The contents of this file are subject to the Sun Industry Standards
#	Source License Version 1.1 (the "License"); You may not use this file
#	except in compliance with the License. You may obtain a copy of the
#	License at http://www.openoffice.org/license.html.
#
#	Software provided under this License is provided on an "AS IS" basis,
#	WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
#	WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
#	MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
#	See the License for the specific provisions governing your rights and
#	obligations concerning the Software.
#
#	The Initial Developer of the Original Code is: Sun Microsystems, Inc.
#
#	Copyright: 2000 by Sun Microsystems, Inc.
#
#	All Rights Reserved.
#
#	Contributor(s): _______________________________________
#
#
#
#*************************************************************************

PRJ=..$/..

PRJNAME=vcl
TARGET=win

.INCLUDE :	$(PRJ)$/util$/makefile.pmk

# --- Settings -----------------------------------------------------

.INCLUDE :	settings.mk

# --- Files --------------------------------------------------------

SLOFILES= \
            $(SLO)$/abstdlg.obj		\
            $(SLO)$/accel.obj		\
            $(SLO)$/accmgr.obj		\
            $(SLO)$/brdwin.obj		\
            $(SLO)$/btndlg.obj		\
            $(SLO)$/cmdevt.obj		\
            $(SLO)$/cursor.obj		\
            $(SLO)$/decoview.obj	\
            $(SLO)$/dialog.obj		\
            $(SLO)$/dlgctrl.obj 	\
            $(SLO)$/dndevdis.obj	\
            $(SLO)$/dndlcon.obj     \
            $(SLO)$/dockmgr.obj 	\
            $(SLO)$/dockwin.obj 	\
            $(SLO)$/floatwin.obj	\
            $(SLO)$/introwin.obj	\
            $(SLO)$/keycod.obj		\
            $(SLO)$/keyevent.obj	\
            $(SLO)$/menu.obj		\
            $(SLO)$/mnemonic.obj	\
            $(SLO)$/msgbox.obj		\
            $(SLO)$/scrwnd.obj		\
            $(SLO)$/seleng.obj		\
            $(SLO)$/split.obj		\
            $(SLO)$/splitwin.obj	\
            $(SLO)$/status.obj		\
            $(SLO)$/syschild.obj	\
            $(SLO)$/javachild.obj	\
            $(SLO)$/syswin.obj		\
            $(SLO)$/tabdlg.obj		\
            $(SLO)$/tabpage.obj 	\
            $(SLO)$/taskpanelist.obj	\
            $(SLO)$/toolbox.obj 	\
            $(SLO)$/toolbox2.obj	\
            $(SLO)$/window.obj		\
            $(SLO)$/window2.obj 	\
            $(SLO)$/window3.obj		\
            $(SLO)$/winproc.obj 	\
            $(SLO)$/wrkwin.obj

EXCEPTIONSFILES=					\
            $(SLO)$/window.obj		\
            $(SLO)$/winproc.obj		\
            $(SLO)$/syswin.obj		\
            $(SLO)$/dndevdis.obj	\
            $(SLO)$/dndlcon.obj		\
            $(SLO)$/javachild.obj	
            
# --- Targets ------------------------------------------------------

.INCLUDE :	target.mk

.INCLUDE :	$(PRJ)$/util$/target.pmk

$(INCCOM)$/cuilib.hxx: makefile.mk
.IF "$(GUI)"=="UNX"
    $(RM) $@
    +echo \#define DLL_NAME \"libcui$(UPD)$(DLLPOSTFIX)$(DLLPOST)\" >$@
.ELSE
.IF "$(USE_SHELL)"!="4nt"
    +echo \#define DLL_NAME \"cui$(UPD)$(DLLPOSTFIX)$(DLLPOST)\" >$@
.ELSE          # "$(USE_SHELL)"!="4nt"
    +echo #define DLL_NAME "cui$(UPD)$(DLLPOSTFIX)$(DLLPOST)" >$@
.ENDIF          # "$(USE_SHELL)"!="4nt"
.ENDIF

$(SLO)$/abstdlg.obj : $(INCCOM)$/cuilib.hxx
