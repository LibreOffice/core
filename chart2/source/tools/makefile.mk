#*************************************************************************
#
# DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
# 
# Copyright 2000, 2010 Oracle and/or its affiliates.
#
# OpenOffice.org - a multi-platform office productivity suite
#
# This file is part of OpenOffice.org.
#
# OpenOffice.org is free software: you can redistribute it and/or modify
# it under the terms of the GNU Lesser General Public License version 3
# only, as published by the Free Software Foundation.
#
# OpenOffice.org is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU Lesser General Public License version 3 for more details
# (a copy is included in the LICENSE file that accompanied this code).
#
# You should have received a copy of the GNU Lesser General Public License
# version 3 along with OpenOffice.org.  If not, see
# <http://www.openoffice.org/license.html>
# for a copy of the LGPLv3 License.
#
#*************************************************************************

PRJ=				..$/..
PRJNAME=			chart2
TARGET=				charttools

PRJINC=				$(PRJ)$/source

USE_DEFFILE=		TRUE
ENABLE_EXCEPTIONS=	TRUE
VISIBILITY_HIDDEN=  TRUE

# --- Settings -----------------------------------------------------

.INCLUDE: $(PRJ)$/util$/makefile.pmk

CDEFS += -DOOO_DLLIMPLEMENTATION_CHARTTOOLS

# --- export library -------------------------------------------------

#Specifies object files to bind into linked libraries.
SLOFILES=	\
    $(SLO)$/ErrorBar.obj \
    $(SLO)$/TrueGuard.obj \
    $(SLO)$/LifeTime.obj \
    $(SLO)$/MediaDescriptorHelper.obj \
    $(SLO)$/ConfigColorScheme.obj \
    $(SLO)$/ColorPerPointHelper.obj \
    $(SLO)$/ObjectIdentifier.obj \
    $(SLO)$/CachedDataSequence.obj \
    $(SLO)$/CommonConverters.obj \
    $(SLO)$/DataSeriesHelper.obj \
    $(SLO)$/DataSource.obj \
    $(SLO)$/DataSourceHelper.obj \
    $(SLO)$/DiagramHelper.obj \
    $(SLO)$/ExponentialRegressionCurveCalculator.obj \
    $(SLO)$/ImplOPropertySet.obj \
    $(SLO)$/InternalData.obj \
    $(SLO)$/InternalDataProvider.obj \
    $(SLO)$/LabeledDataSequence.obj \
    $(SLO)$/LinearRegressionCurveCalculator.obj \
    $(SLO)$/LogarithmicRegressionCurveCalculator.obj \
    $(SLO)$/MeanValueRegressionCurveCalculator.obj \
    $(SLO)$/NumberFormatterWrapper.obj \
    $(SLO)$/OPropertySet.obj \
    $(SLO)$/WrappedPropertySet.obj \
    $(SLO)$/WrappedProperty.obj \
    $(SLO)$/WrappedIgnoreProperty.obj \
    $(SLO)$/WrappedDefaultProperty.obj \
    $(SLO)$/WrappedDirectStateProperty.obj \
    $(SLO)$/PotentialRegressionCurveCalculator.obj \
    $(SLO)$/RegressionCurveHelper.obj \
    $(SLO)$/RegressionCurveModel.obj \
    $(SLO)$/RelativeSizeHelper.obj \
    $(SLO)$/RelativePositionHelper.obj \
    $(SLO)$/Scaling.obj \
    $(SLO)$/SceneProperties.obj \
    $(SLO)$/ThreeDHelper.obj \
    $(SLO)$/StatisticsHelper.obj \
    $(SLO)$/ChartModelHelper.obj \
    $(SLO)$/ChartViewHelper.obj \
    $(SLO)$/ChartTypeHelper.obj \
    $(SLO)$/AxisHelper.obj \
    $(SLO)$/MutexContainer.obj \
    $(SLO)$/PropertyHelper.obj \
    $(SLO)$/FormattedStringHelper.obj \
    $(SLO)$/TitleHelper.obj \
    $(SLO)$/LegendHelper.obj \
    $(SLO)$/CharacterProperties.obj \
    $(SLO)$/LineProperties.obj \
    $(SLO)$/FillProperties.obj \
    $(SLO)$/UserDefinedProperties.obj \
    $(SLO)$/NameContainer.obj \
    $(SLO)$/ChartDebugTrace.obj \
    $(SLO)$/_serviceregistration_tools.obj \
    $(SLO)$/UncachedDataSequence.obj \
    $(SLO)$/XMLRangeHelper.obj \
    $(SLO)$/ModifyListenerHelper.obj \
    $(SLO)$/ModifyListenerCallBack.obj \
    $(SLO)$/BaseGFXHelper.obj \
    $(SLO)$/ControllerLockGuard.obj \
    $(SLO)$/WeakListenerAdapter.obj \
    $(SLO)$/ResId.obj \
    $(SLO)$/RessourceManager.obj \
    $(SLO)$/RangeHighlighter.obj \
    $(SLO)$/ReferenceSizeProvider.obj \
    $(SLO)$/ExplicitCategoriesProvider.obj \
    $(SLO)$/RegressionCurveCalculator.obj \
    $(SLO)$/RegressionEquation.obj

DISABLED_SLOFILES=	\
    $(SLO)$/NamedFillProperties.obj \
    $(SLO)$/NamedLineProperties.obj \
    $(SLO)$/NamedProperties.obj

#--------

#Indicates the filename of the shared library.
SHL1TARGET=		$(TARGET)$(DLLPOSTFIX)

#indicates dependencies:
SHL1DEPN=
#Specifies an import library to create. For Win32 only.
SHL1IMPLIB=		i$(TARGET)

#Specifies libraries from the same module to put into the shared library.
#was created above
SHL1LIBS= 		$(SLB)$/$(TARGET).lib

#Links import libraries.

SHL1STDLIBS=    \
                $(CPPULIB)			\
                $(CPPUHELPERLIB)	\
                $(COMPHELPERLIB)	\
                $(SALLIB)			\
                $(TOOLSLIB)			\
                $(SVLLIB)		    \
                $(VCLLIB)           \
                $(I18NISOLANGLIB)   \
                $(BASEGFXLIB) 		\
                $(UNOTOOLSLIB)

#--------exports

#specifies the exported symbols for Windows only:
SHL1DEF=		$(MISC)$/$(SHL1TARGET).def

#Specifies the library name to parse for symbols. For Win32 only.
DEFLIB1NAME=	$(TARGET)

#--------definition file

#name of the definition file:
DEF1NAME=		$(SHL1TARGET)

# indicates definition file dependencies
DEF1DEPN=		$(MISC)$/$(SHL1TARGET).flt

#A comment on the definition file.
DEF1DES=		Chart Tools

# --- Targets -----------------------------------------------------------------

.INCLUDE: target.mk

# --- Filter -----------------------------------------------------------

$(MISC)$/$(SHL1TARGET).flt: makefile.mk \
                            exports.flt
    $(TYPE) exports.flt > $@


ALLTAR : $(MISC)/charttools.component

$(MISC)/charttools.component .ERRREMOVE : $(SOLARENV)/bin/createcomponent.xslt \
        charttools.component
    $(XSLTPROC) --nonet --stringparam uri \
        '$(COMPONENTPREFIX_BASIS_NATIVE)$(SHL1TARGETN:f)' -o $@ \
        $(SOLARENV)/bin/createcomponent.xslt charttools.component
