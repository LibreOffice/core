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



PRJ=..$/..$/..$/..

PRJNAME=offapi

TARGET=cssgeometry
PACKAGE=com$/sun$/star$/geometry

# --- Settings -----------------------------------------------------
.INCLUDE :  $(PRJ)$/util$/makefile.pmk

# ------------------------------------------------------------------------

IDLFILES=\
    AffineMatrix2D.idl			\
    AffineMatrix3D.idl			\
    EllipticalArc.idl			\
    Matrix2D.idl				\
    XMapping2D.idl				\
    IntegerBezierSegment2D.idl 	\
    RealBezierSegment2D.idl 	\
    RealPoint2D.idl				\
    RealRectangle2D.idl			\
    RealRectangle3D.idl			\
    RealSize2D.idl				\
    IntegerPoint2D.idl			\
    IntegerRectangle2D.idl		\
    IntegerSize2D.idl

# ------------------------------------------------------------------

.INCLUDE :  target.mk
.INCLUDE :  $(PRJ)$/util$/target.pmk
