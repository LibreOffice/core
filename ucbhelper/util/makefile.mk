#*************************************************************************
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.5 $
#
#   last change: $Author: obo $ $Date: 2001-12-21 09:36:30 $
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

PRJNAME=	ucbhelper
TARGET=		ucbhelper
NO_BSYMBOLIC=	TRUE
ENABLE_EXCEPTIONS=TRUE

# --- Settings -----------------------------------------------------

.INCLUDE :	svpre.mk
.INCLUDE :	settings.mk
.INCLUDE :	sv.mk

.INCLUDE :	..$/version.mk

# --- Files --------------------------------------------------------

LIB1TARGET=	$(SLB)$/$(TARGET).lib
LIB1FILES=	$(SLB)$/client.lib \
        $(SLB)$/provider.lib

SHL1TARGET=	$(TARGET)$(UCBHELPER_MAJOR)$(COMID)
SHL1STDLIBS = \
    $(CPPUHELPERLIB) \
    $(CPPULIB) \
    $(SALHELPERLIB) \
    $(SALLIB) \
    $(VOSLIB)
SHL1DEPN=
SHL1IMPLIB=	i$(TARGET)
SHL1LIBS=	$(LIB1TARGET)
SHL1DEF=	$(MISC)$/$(SHL1TARGET).def

DEF1NAME=	$(SHL1TARGET)
DEF1DEPN=	$(MISC)$/$(SHL1TARGET).flt
DEFLIB1NAME=	$(TARGET)
DEF1DES=	Universal Content Broker - Helpers
#DEF1EXPORT1=	createComponentFactory
#DEF1EXPORT2=	writeComponentInfo

# --- Targets ------------------------------------------------------

.INCLUDE :	target.mk

# --- Filter -----------------------------------------------------------

$(MISC)$/$(SHL1TARGET).flt : ucbhelper.flt
    @echo ------------------------------
    @echo Making: $@
    @+$(TYPE) ucbhelper.flt > $@
.IF "$(COM)"=="MSC"
    @echo __CT>>$@
    @echo ??_5>>$@
    @echo ??_7>>$@
    @echo ??_8>>$@
    @echo ??_9>>$@
    @echo ??_C>>$@
    @echo ??_E>>$@
    @echo ??_F>>$@
    @echo ??_G>>$@
    @echo ??_H>>$@
    @echo ??_I>>$@
    @echo .cxx>>$@
        @echo ??_R0?AV>>$@
        @echo ??3@YAXPAX0@Z>>$@
        @echo ?_M_allocate_and_copy@?$$vector@>>$@
        @echo ?_M_insert_overflow@?$$vector@>>$@
        @echo ?_M_fill_insert@?$$vector@>>$@
        @echo ?_M_create_node@?$$list@>>$@
        @echo ?_M_list@?$$_Stl_prime@_N@_STL@@>>$@
        @echo ?__lower_bound@_STL@@>>$@
        @echo ?__uninitialized_copy_aux@_STL@@>>$@
        @echo ?__uninitialized_fill_n_aux@_STL@@>>$@
        @echo ??1?$$_Vector_base@>>$@
        @echo ??0?$$_Vector_base@>>$@
        @echo ??1?$$_STL_alloc_proxy@>>$@
        @echo ??0?$$_STL_alloc_proxy@>>$@
        @echo ??1?$$allocator@>>$@
        @echo ??1?$$pair@>>$@
        @echo ??0?$$pair@>>$@
        @echo ??1?$$vector@>>$@
        @echo ??1?$$hash_map@>>$@
        @echo ??A?$$hash_map@>>$@
        @echo ?get_allocator@>>$@
        @echo ?copy@_STL@@>>$@
        @echo ??1?$$map@>>$@
        @echo ?erase@?$$map@>>$@
        @echo ?make_pair@_STL@@>>$@
        @echo ??0?$$_List_iterator@>>$@
        @echo ?__copy@_STL@@>>$@
        @echo ?__copy_backward@_STL@@>>$@
        @echo ?end@?$$list@>>$@
        @echo ?insert@?$$list@>>$@
        @echo ?deallocate@?$$allocator@>>$@
        @echo ??0?$$_Rb_tree_iterator@>>$@
        @echo ??1?$$_Rb_tree_base@>>$@
        @echo ?_M_create_node@?$$_Rb_tree@>>$@
        @echo ?_M_decrement@?$$_Rb_global@>>$@
        @echo ?_M_empty_initialize@?$$_Rb_tree@>>$@
        @echo ?_M_erase@?$$_Rb_tree@>>$@
        @echo ?_Rebalance@?$$_Rb_global@>>$@
        @echo ?_Rebalance_for_erase@?$$_Rb_global@>>$@
        @echo ?destroy_node@?$$_Rb_tree@>>$@
        @echo ?erase@?$$_Rb_tree@>>$@
        @echo ?find@?$$_Rb_tree@>>$@
        @echo ?insert_unique@?$$_Rb_tree@>>$@
        @echo ?reserve@?$$vector@>>$@
        @echo ??1?$$Guard@VMutex@osl@@@osl@@>>$@
        @echo ??1OGuard@vos@@>>$@
        @echo ??1?$$ClearableGuard@VMutex@osl@@@osl@@>>$@
        @echo ??1Mutex@osl@@>>$@
        @echo ??1OUString@rtl@@>>$@
        @echo ??0OUString@rtl@@>>$@
        @echo ??8@YAEABVOUString@rtl@@>>$@
        @echo ?createFromAscii@OUString@rtl@@>>$@
        @echo ?copy@OUString@rtl@@>>$@
        @echo ?replaceAt@OUString@rtl@@>>$@
        @echo ?compareTo@OUString@rtl@@>>$@
        @echo ?toAsciiLowerCase@OUString@rtl@@>>$@
        @echo ??1OUStringBuffer@rtl@@>>$@
        @echo ?makeStringAndClear@OUStringBuffer@rtl@@>>$@
        @echo ?appendAscii@OUStringBuffer@rtl@@>>$@
        @echo ??1?$$ORef@>>$@
        @echo ?s_pType@?$$Sequence@>>$@
        @echo ??0?$$Reference@>>$@
        @echo ??1?$$Reference@>>$@
        @echo ??C?$$Reference@>>$@
        @echo ?__query@?$$Reference@>>$@
        @echo ?set@?$$Reference@>>$@
        @echo ??0?$$Sequence@>>$@
        @echo ??1?$$Sequence@>>$@
        @echo ??A?$$Sequence@V?$$Reference@>>$@
        @echo ??1OTypeCollection@cppu@@>>$@
        @echo ?getTypes@OTypeCollection@cppu@@>>$@
        @echo ?queryInterface@cppu@@>>$@
        @echo ??0OWeakObject@cppu@@>>$@
        @echo ??3OWeakObject@cppu@@>>$@
        @echo ?makeAny@uno@star@sun@com@@>>$@
        @echo ??0Any@uno@star@sun@com@@>>$@
        @echo ??1Any@uno@star@sun@com@@>>$@
        @echo ??4Any@uno@star@sun@com@@>>$@
        @echo ??0Type@uno@star@sun@com@@>>$@
        @echo ??0Exception@uno@star@sun@com@@>>$@
        @echo ??1Exception@uno@star@sun@com@@>>$@
        @echo ??0RuntimeException@uno@star@sun@com@@>>$@
        @echo ??1RuntimeException@uno@star@sun@com@@>>$@
        @echo ??8BaseReference@uno@star@sun@com@@>>$@
        @echo ??1?$$WeakImplHelper1@>>$@
        @echo ?acquire@?$$WeakImplHelper1@>>$@
        @echo ?release@?$$WeakImplHelper1@>>$@
        @echo ?getImplementationId@?$$WeakImplHelper1@>>$@
        @echo ?getTypes@?$$WeakImplHelper1@>>$@
        @echo ?queryInterface@?$$WeakImplHelper1@>>$@
        @echo ?s_cd@?$$WeakImplHelper1@>>$@
        @echo ??0OpenCommandArgument@ucb@star@sun@com@@>>$@
        @echo ??1Command@ucb@star@sun@com@@>>$@
        @echo ??1CommandInfo@ucb@star@sun@com@@>>$@
        @echo ??1ContentInfo@ucb@star@sun@com@@>>$@
        @echo ??1InsertCommandArgument@ucb@star@sun@com@@>>$@
        @echo ??1OpenCommandArgument2@ucb@star@sun@com@@>>$@
        @echo ??1OpenCommandArgument@ucb@star@sun@com@@>>$@
        @echo ??0EventObject@lang@star@sun@com@@>>$@
        @echo ??1EventObject@lang@star@sun@com@@>>$@
        @echo ??0IllegalArgumentException@lang@star@sun@com@@>>$@
        @echo ??1IllegalArgumentException@lang@star@sun@com@@>>$@
        @echo ??0IllegalTypeException@beans@star@sun@com@@>>$@
        @echo ??1IllegalTypeException@beans@star@sun@com@@>>$@
        @echo ??0NoSupportException@lang@star@sun@com@@>>$@
        @echo ??1NoSupportException@lang@star@sun@com@@>>$@
        @echo ??0NotRemoveableException@beans@star@sun@com@@>>$@
        @echo ??1NotRemoveableException@beans@star@sun@com@@>>$@
        @echo ??0PropertyExistException@beans@star@sun@com@@>>$@
        @echo ??1PropertyExistException@beans@star@sun@com@@>>$@
        @echo ??0UnknownPropertyException@beans@star@sun@com@@>>$@
        @echo ??1UnknownPropertyException@beans@star@sun@com@@>>$@
        @echo ??0UnsupportedCommandException@ucb@star@sun@com@@>>$@
        @echo ??1UnsupportedCommandException@ucb@star@sun@com@@>>$@
        @echo ??0ListenerAlreadySetException@ucb@star@sun@com@@>>$@
        @echo ??1ListenerAlreadySetException@ucb@star@sun@com@@>>$@
        @echo ??0ContentCreationException@ucb@star@sun@com@@>>$@
        @echo ??1ContentCreationException@ucb@star@sun@com@@>>$@
        @echo ??0ServiceNotFoundException@ucb@star@sun@com@@>>$@
        @echo ??1ServiceNotFoundException@ucb@star@sun@com@@>>$@
        @echo ??0SQLException@sdbc@star@sun@com@@>>$@
        @echo ??1SQLException@sdbc@star@sun@com@@>>$@
        @echo ??1InteractiveAugmentedIOException@ucb@star@sun@com@@>>$@
        @echo ??0CommandFailedException@ucb@star@sun@com@@>>$@
        @echo ??1CommandFailedException@ucb@star@sun@com@@>>$@
        @echo ??1ContentEvent@ucb@star@sun@com@@>>$@
        @echo ??1Property@beans@star@sun@com@@>>$@
        @echo ??0Property@beans@star@sun@com@@>>$@
        @echo ??1PropertyValue@beans@star@sun@com@@>>$@
        @echo ??1PropertySetInfoChangeEvent@beans@star@sun@com@@>>$@
        @echo ??1PropertyChangeEvent@beans@star@sun@com@@>>$@
        @echo ??4PropertyChangeEvent@beans@star@sun@com@@>>$@
        @echo ??1ListEvent@ucb@star@sun@com@@>>$@
        @echo ??1GlobalTransferCommandArgument@ucb@star@sun@com@@>>$@
        @echo ?m_pTheBroker@ContentBroker@ucb@@>>$@
        @echo ?getArray@?$$Sequence@UPropertyChangeEvent@beans@star@sun@com@@>>$@
        @echo ??0Date@util@star@sun@com@@>>$@
        @echo ??0Time@util@star@sun@com@@>>$@
        @echo ??0DateTime@util@star@sun@com@@>>$@
        @echo ??0ClassifiedInteractionRequest@task@star@sun@com@@>>$@
        @echo ??1AuthenticationRequest@ucb@star@sun@com@@>>$@
        @echo ??1NameClashResolveRequest@ucb@star@sun@com@@>>$@
        @echo _TI2?AVIllegalArgumentException@lang@star@sun@com@@>>$@
        @echo _TI2?AVIllegalTypeException@beans@star@sun@com@@>>$@
        @echo _TI2?AVNoSupportException@lang@star@sun@com@@>>$@
        @echo _TI2?AVNotRemoveableException@beans@star@sun@com@@>>$@
        @echo _TI2?AVPropertyExistException@beans@star@sun@com@@>>$@
        @echo _TI2?AVUnknownPropertyException@beans@star@sun@com@@>>$@
        @echo _TI2?AVUnsupportedCommandException@ucb@star@sun@com@@>>$@
        @echo _TI2?AVSQLException@sdbc@star@sun@com@@>>$@
        @echo _TI2?AVContentCreationException@ucb@star@sun@com@@>>$@
        @echo _TI2?AVListenerAlreadySetException@ucb@star@sun@com@@>>$@
        @echo _TI2?AVServiceNotFoundException@ucb@star@sun@com@@>>$@
        @echo _TI2?AVCommandFailedException@ucb@star@sun@com@@>>$@
        @echo _TI2?AVRuntimeException@uno@star@sun@com@@>>$@
        @echo _real@4@00000000000000000000>>$@
        @echo _real@8@00000000000000000000>>$@
        @echo ?makeAndAppendXMLName@ucb@@>>$@
.ENDIF # MSC
.IF "$(GUI)"=="WNT"
    @echo ?CreateType@>>$@
.ENDIF # WNT

