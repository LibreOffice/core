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



DIR_FILTERCFGOUT := $(MISC)$/filters
.IF "$(WITH_LANG)"!=""
DIR_LOCFRAG      := $(DIR_FILTERCFGOUT)$/ui$/fragments
.ELSE			# "$(WITH_LANG)"!=""
DIR_LOCFRAG      := $(PRJ)$/source$/config$/fragments
.ENDIF			# "$(WITH_LANG)"!=""
DIR_LOCMERGE     := $(DIR_FILTERCFGOUT)$/ui$/merge
DIR_LANGPACK     := $(DIR_FILTERCFGOUT)$/ui$/langpacks
DIR_MODPACKS     := $(DIR_FILTERCFGOUT)$/modulepacks

INCLUDE_FRAGMENTS = \
     $(PRJ)$/source$/config$/fragments$/fcfg_base.mk \
     $(PRJ)$/source$/config$/fragments$/fcfg_database.mk \
     $(PRJ)$/source$/config$/fragments$/fcfg_writer.mk \
     $(PRJ)$/source$/config$/fragments$/fcfg_web.mk \
     $(PRJ)$/source$/config$/fragments$/fcfg_global.mk \
     $(PRJ)$/source$/config$/fragments$/fcfg_w4w.mk \
     $(PRJ)$/source$/config$/fragments$/fcfg_calc.mk \
     $(PRJ)$/source$/config$/fragments$/fcfg_draw.mk \
     $(PRJ)$/source$/config$/fragments$/fcfg_impress.mk \
     $(PRJ)$/source$/config$/fragments$/fcfg_database.mk \
     $(PRJ)$/source$/config$/fragments$/fcfg_chart.mk \
     $(PRJ)$/source$/config$/fragments$/fcfg_math.mk \
     $(PRJ)$/source$/config$/fragments$/fcfg_xslt.mk \
     $(PRJ)$/source$/config$/fragments$/fcfg_palm.mk \
     $(PRJ)$/source$/config$/fragments$/fcfg_pocketword.mk \
     $(PRJ)$/source$/config$/fragments$/fcfg_pocketexcel.mk \
     $(PRJ)$/source$/config$/fragments$/fcfg_drawgraphics.mk \
     $(PRJ)$/source$/config$/fragments$/fcfg_impressgraphics.mk \
     $(PRJ)$/source$/config$/fragments$/fcfg_internalgraphics.mk

.INCLUDE: $(INCLUDE_FRAGMENTS)
