#*************************************************************************
#
#   $RCSfile: postset.mk,v $
#
#   $Revision: 1.21 $
#
#   last change: $Author: hjs $ $Date: 2004-06-25 16:11:46 $
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



.IF "$(RES_GER)"!="" || "$(give_me_all_languages)"!=""
alllangiso+=de
.ENDIF
completelangiso+=de

# keep this one!!! #################################################################################
#
#
####################################################################################################
#

.IF "$(RES_ENUS)"!="" || "$(give_me_all_languages)"!=""
alllangiso+=en-US
.ENDIF
completelangiso+=en-US

# keep this one!!! #################################################################################
#
defaultlangiso=en-US
#
####################################################################################################
#

.IF "$(RES_ITAL)"!="" || "$(give_me_all_languages)"!=""
alllangiso+=it
.ENDIF
completelangiso+=it

.IF "$(RES_SPAN)"!="" || "$(give_me_all_languages)"!=""
alllangiso+=es
.ENDIF
completelangiso+=es

.IF "$(RES_FREN)"!="" || "$(give_me_all_languages)"!=""
alllangiso+=fr
.ENDIF
completelangiso+=fr

.IF "$(RES_DTCH)"!="" || "$(give_me_all_languages)"!=""
alllangiso+=nl
.ENDIF
completelangiso+=nl

.IF "$(RES_DAN)"!="" || "$(give_me_all_languages)"!=""
alllangiso+=da
.ENDIF
completelangiso+=da

.IF "$(RES_SWED)"!="" || "$(give_me_all_languages)"!=""
alllangiso+=sv
.ENDIF
completelangiso+=sv

.IF "$(RES_PORT)"!="" || "$(give_me_all_languages)"!=""
alllangiso+=pt
.ENDIF 
completelangiso+=pt

.IF "$(RES_JAPN)"!="" || "$(give_me_all_languages)"!=""
alllangiso+=ja
.ENDIF
completelangiso+=ja

.IF "$(RES_CHINSIM)"!="" || "$(give_me_all_languages)"!=""
alllangiso+=zh-CN
.ENDIF 
completelangiso+=zh-CN

.IF "$(RES_CHINTRAD)"!="" || "$(give_me_all_languages)"!=""
alllangiso+=zh-TW
.ENDIF 
completelangiso+=zh-TW

.IF "$(RES_RUSS)"!="" || "$(give_me_all_languages)"!=""
alllangiso+=ru
.ENDIF
completelangiso+=ru

.IF "$(RES_POL)"!="" || "$(give_me_all_languages)"!=""
alllangiso+=pl
.ENDIF 
completelangiso+=pl

.IF "$(RES_ARAB)"!="" || "$(give_me_all_languages)"!=""
alllangiso+=ar
.ENDIF 
completelangiso+=ar

.IF "$(RES_GREEK)"!="" || "$(give_me_all_languages)"!=""
alllangiso+=el
.ENDIF 
completelangiso+=el

.IF "$(RES_TURK)"!="" || "$(give_me_all_languages)"!=""
alllangiso+=tr
.ENDIF 
completelangiso+=tr

.IF "$(RES_KOREAN)"!="" || "$(give_me_all_languages)"!=""
alllangiso+=ko
.ENDIF 
completelangiso+=ko

.IF "$(RES_CAT)"!="" || "$(give_me_all_languages)"!=""
alllangiso+=ca
.ENDIF 
completelangiso+=ca

.IF "$(RES_FINN)"!="" || "$(give_me_all_languages)"!=""
alllangiso+=fi
.ENDIF          # "$(RES_FINN)"!="" || "$(give_me_all_languages)"!=""
completelangiso+=fi

.IF "$(RES_HUNG)"!="" || "$(give_me_all_languages)"!=""
alllangiso+=hu
.ENDIF
completelangiso+=hu

.IF "$(RES_THAI)"!="" || "$(give_me_all_languages)"!=""
alllangiso+=th
.ENDIF          # "$(RES_THAI)"!="" || "$(give_me_all_languages)"!=""
completelangiso+=th

.IF "$(RES_HINDI)"!="" || "$(give_me_all_languages)"!=""
alllangiso+=hi-IN
.ENDIF          # "$(RES_HINDI)"!="" || "$(give_me_all_languages)"!=""
completelangiso+=hi-IN

.IF "$(RES_PORTBR)"!="" || "$(give_me_all_languages)"!=""
alllangiso+=pt-BR
.ENDIF 
completelangiso+=pt-BR

.IF "$(RES_CZECH)"!="" || "$(give_me_all_languages)"!=""
alllangiso+=cs
.ENDIF 
completelangiso+=cs

.IF "$(RES_SLOVAK)"!="" || "$(give_me_all_languages)"!=""
alllangiso+=sk
.ENDIF 
completelangiso+=sk

.IF "$(RES_SLOVENIAN)"!="" || "$(give_me_all_languages)"!=""
alllangiso+=sl
.ENDIF 
completelangiso+=sl

.IF "$(RES_ESTONIAN)"!="" || "$(give_me_all_languages)"!=""
alllangiso+=et
.ENDIF 
completelangiso+=et

.IF "$(RES_HEBREW)"!="" || "$(give_me_all_languages)"!=""
alllangiso+=he
.ENDIF 
completelangiso+=he

