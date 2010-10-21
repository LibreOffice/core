PRJ=..$/..

PRJNAME=oox
TARGET=export
AUTOSEG=true

ENABLE_EXCEPTIONS=TRUE

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk
.INCLUDE: $(PRJ)$/util$/makefile.pmk

# --- Files --------------------------------------------------------

SLOFILES =	\
        $(SLO)$/drawingml.obj \
        $(SLO)$/shapes.obj \
        $(SLO)$/chartexport.obj \
        $(SLO)$/SchXMLSeriesHelper.obj \
        $(SLO)$/ColorPropertySet.obj \
        $(SLO)$/vmlexport.obj \
        $(SLO)$/vmlexport-shape-types.obj

# --- Targets -------------------------------------------------------

.INCLUDE :  target.mk

$(MISC)$/vmlexport-shape-types.cxx : preset-definitions-to-shape-types.pl presetShapeDefinitions.xml presetTextWarpDefinitions.xml
    $(PERL) $< > $@.in_progress 2> $(MISC)$/vmlexport-shape-types.log && mv $@.in_progress $@

$(SLO)$/vmlexport-shape-types.obj : $(MISC)$/vmlexport-shape-types.cxx

