PRJ=..$/..
PRJNAME=writerfilter
TARGET=dmapper
GEN_HID=TRUE

ENABLE_EXCEPTIONS=TRUE

# --- Settings ----------------------------------

.INCLUDE : settings.mk
CDEFS+=-DWRITERFILTER_DLLIMPLEMENTATION

# --- Files -------------------------------------

SLOFILES=           $(SLO)$/ConversionHelper.obj \
                    $(SLO)$/DomainMapper.obj \
                    $(SLO)$/DomainMapper_Impl.obj \
                    $(SLO)$/DomainMapperTableHandler.obj \
                    $(SLO)$/PropertyMap.obj  \
                    $(SLO)$/PropertyIds.obj  \
                    $(SLO)$/FontTable.obj   \
                    $(SLO)$/StyleSheetTable.obj \
                    $(SLO)$/ListTable.obj  \
                    $(SLO)$/LFOTable.obj  

# --- Targets ----------------------------------

.INCLUDE : target.mk



