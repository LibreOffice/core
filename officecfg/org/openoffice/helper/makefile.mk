#*************************************************************************
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.1 $
#
#   last change: $Author: dg $ $Date: 2000-09-20 16:14:58 $
#
#   Copyright according the GNU Public License.
#
#*************************************************************************

PRJ=..$/..$/..

PRJNAME=officecfg
TARGET =schema
PACKAGE=org$/openoffice$/helper

# --- Settings -----------------------------------------------------

.INCLUDE :  svpre.mk
.INCLUDE :  settings.mk
.INCLUDE :  sv.mk

EXTRAJARFILES = jaxp.jar parser.jar

JAVACLASSFILES=	\
    $(CLASSDIR)$/$(PACKAGE)$/Validator.class \
    $(CLASSDIR)$/$(PACKAGE)$/PrettyPrinter.class

JAVAFILES= $(subst,$(CLASSDIR)$/$(PACKAGE)$/, $(subst,.class,.java $(JAVACLASSFILES)))

RC_SUBDIRSDEPS=$(JAVATARGET)

JARCLASSDIRS	= $(PACKAGE)
JARTARGET		= $(TARGET).jar
JARCOMPRESS		= TRUE


# --- Targets ------------------------------------------------------


.INCLUDE :  target.mk

