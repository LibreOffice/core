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



PRJ=..$/..

PRJNAME=sysui
TARGET=icons

# --- Settings -----------------------------------------------------------

.INCLUDE :  settings.mk

# --- Targets -------------------------------------------------------------

all: \
    $(MISC)$/ooo-base-app.ico \
    $(MISC)$/ooo-base-doc.ico \
    $(MISC)$/ooo-calc-app.ico \
    $(MISC)$/ooo-calc-doc.ico \
    $(MISC)$/ooo-calc-tem.ico \
    $(MISC)$/ooo-chart-doc.ico \
    $(MISC)$/ooo-configuration.ico \
    $(MISC)$/ooo-draw-app.ico \
    $(MISC)$/ooo-draw-doc.ico \
    $(MISC)$/ooo-draw-tem.ico \
    $(MISC)$/ooo-empty-doc.ico \
    $(MISC)$/ooo-empty-tem.ico \
    $(MISC)$/ooo-image-doc.ico \
    $(MISC)$/ooo-impress-app.ico \
    $(MISC)$/ooo-impress-doc.ico \
    $(MISC)$/ooo-impress-tem.ico \
    $(MISC)$/ooo-macro-doc.ico \
    $(MISC)$/ooo-main-app.ico \
    $(MISC)$/ooo-master-doc.ico \
    $(MISC)$/ooo-math-app.ico \
    $(MISC)$/ooo-math-doc.ico \
    $(MISC)$/ooo-open.ico \
    $(MISC)$/ooo-printer.ico \
    $(MISC)$/ooo-web-doc.ico \
    $(MISC)$/ooo-writer-app.ico \
    $(MISC)$/ooo-writer-doc.ico \
    $(MISC)$/ooo-writer-tem.ico \
    $(MISC)$/ooo11-base-doc.ico \
    $(MISC)$/ooo11-calc-doc.ico \
    $(MISC)$/ooo11-calc-tem.ico \
    $(MISC)$/ooo11-chart-doc.ico \
    $(MISC)$/ooo11-draw-doc.ico \
    $(MISC)$/ooo11-draw-tem.ico \
    $(MISC)$/ooo11-impress-doc.ico \
    $(MISC)$/ooo11-impress-tem.ico \
    $(MISC)$/ooo11-master-doc.ico \
    $(MISC)$/ooo11-math-doc.ico \
    $(MISC)$/ooo11-writer-doc.ico \
    $(MISC)$/ooo11-writer-tem.ico \
    $(MISC)$/so8-base-app.ico \
    $(MISC)$/so8-base-doc.ico \
    $(MISC)$/so8-calc-app.ico \
    $(MISC)$/so8-calc-doc.ico \
    $(MISC)$/so8-calc-tem.ico \
    $(MISC)$/so8-chart-doc.ico \
    $(MISC)$/so8-configuration.ico \
    $(MISC)$/so8-draw-app.ico \
    $(MISC)$/so8-draw-doc.ico \
    $(MISC)$/so8-draw-tem.ico \
    $(MISC)$/so8-empty-doc.ico \
    $(MISC)$/so8-empty-tem.ico \
    $(MISC)$/so8-image-doc.ico \
    $(MISC)$/so8-impress-app.ico \
    $(MISC)$/so8-impress-doc.ico \
    $(MISC)$/so8-impress-tem.ico \
    $(MISC)$/so8-macro-doc.ico \
    $(MISC)$/so8-main-app.ico \
    $(MISC)$/so8-master-doc.ico \
    $(MISC)$/so8-math-app.ico \
    $(MISC)$/so8-math-doc.ico \
    $(MISC)$/so8-open.ico \
    $(MISC)$/so8-printer.ico \
    $(MISC)$/so8-web-doc.ico \
    $(MISC)$/so8-writer-app.ico \
    $(MISC)$/so8-writer-doc.ico \
    $(MISC)$/so8-writer-tem.ico \
    $(MISC)$/so7-base-doc.ico \
    $(MISC)$/so7-calc-doc.ico \
    $(MISC)$/so7-calc-tem.ico \
    $(MISC)$/so7-chart-doc.ico \
    $(MISC)$/so7-draw-doc.ico \
    $(MISC)$/so7-draw-tem.ico \
    $(MISC)$/so7-impress-doc.ico \
    $(MISC)$/so7-impress-tem.ico \
    $(MISC)$/so7-master-doc.ico \
    $(MISC)$/so7-math-doc.ico \
    $(MISC)$/so7-writer-doc.ico \
    $(MISC)$/so7-writer-tem.ico

$(MISC)$/%.ico: %.ico
    +$(COPY) $< $@

.INCLUDE :  target.mk

