#**************************************************************
#  
#  Licensed to the Apache Software Foundation (ASF) under one
#  or more contributor license agreements.  See the NOTICE file
#  distributed with this work for additional information
#  regarding copyright ownership.  The ASF licenses this file
#  to you under the Apache License, Version 2.0 (the
#  "License"); you may not use this file except in compliance
#  with the License.  You may obtain a copy of the License at
#  
#    http://www.apache.org/licenses/LICENSE-2.0
#  
#  Unless required by applicable law or agreed to in writing,
#  software distributed under the License is distributed on an
#  "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
#  KIND, either express or implied.  See the License for the
#  specific language governing permissions and limitations
#  under the License.
#  
#**************************************************************



PRJ=..$/..$/..

PRJNAME=vcl
TARGET=salsrc

# --- Settings -----------------------------------------------------

.INCLUDE :	settings.mk
.INCLUDE :  $(PRJ)$/util$/makefile2.pmk

# --- Files --------------------------------------------------------

RCDEPN= 	nullptr.ptr 		\
            help.ptr			\
            cross.ptr			\
            move.ptr			\
            hsplit.ptr			\
            vsplit.ptr			\
            hsizebar.ptr		\
            vsizebar.ptr		\
            hand.ptr			\
            refhand.ptr 		\
            pen.ptr 			\
            magnify.ptr 		\
            fill.ptr			\
            rotate.ptr			\
            hshear.ptr			\
            vshear.ptr			\
            mirror.ptr			\
            crook.ptr			\
            crop.ptr			\
            movept.ptr			\
            movebw.ptr			\
            movedata.ptr		\
            copydata.ptr		\
            linkdata.ptr		\
            movedlnk.ptr		\
            copydlnk.ptr		\
            movef.ptr			\
            copyf.ptr			\
            linkf.ptr			\
            moveflnk.ptr		\
            copyflnk.ptr		\
            movef2.ptr			\
            copyf2.ptr			\
            dline.ptr			\
            drect.ptr			\
            dpolygon.ptr		\
            dbezier.ptr 		\
            darc.ptr			\
            dpie.ptr			\
            dcirccut.ptr		\
            dellipse.ptr		\
            dfree.ptr			\
            dconnect.ptr		\
            dtext.ptr			\
            dcapt.ptr			\
            chart.ptr			\
            detectiv.ptr		\
            pivotcol.ptr		\
            pivotrow.ptr		\
            pivotfld.ptr		\
            chain.ptr			\
            chainnot.ptr		\
            timemove.ptr		\
            timesize.ptr		\
            asn.ptr 			\
            ass.ptr 			\
            asw.ptr 			\
            ase.ptr 			\
            asnw.ptr			\
            asne.ptr			\
            assw.ptr			\
            asse.ptr			\
            asns.ptr			\
            aswe.ptr			\
            asnswe.ptr			\
            sd.ico

RCFILES=	salsrc.rc

# --- Targets ------------------------------------------------------

.INCLUDE :	target.mk
