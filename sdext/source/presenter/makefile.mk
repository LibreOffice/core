#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.3 $
#
#   last change: $Author: kz $ $Date: 2008-04-03 16:24:42 $
#
#   The Contents of this file are made available subject to
#   the terms of GNU Lesser General Public License Version 2.1.
#
#
#     GNU Lesser General Public License Version 2.1
#     =============================================
#     Copyright 2005 by Sun Microsystems, Inc.
#     901 San Antonio Road, Palo Alto, CA 94303, USA
#
#     This library is free software; you can redistribute it and/or
#     modify it under the terms of the GNU Lesser General Public
#     License version 2.1, as published by the Free Software Foundation.
#
#     This library is distributed in the hope that it will be useful,
#     but WITHOUT ANY WARRANTY; without even the implied warranty of
#     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
#     Lesser General Public License for more details.
#
#     You should have received a copy of the GNU Lesser General Public
#     License along with this library; if not, write to the Free Software
#     Foundation, Inc., 59 Temple Place, Suite 330, Boston,
#     MA  02111-1307  USA
#
#*************************************************************************

PRJ=..$/..
PRJNAME=presenter
TARGET=PresenterScreen
GEN_HID=FALSE
EXTNAME=presenter

ENABLE_EXCEPTIONS=TRUE

# --- Settings ----------------------------------

.INCLUDE : settings.mk
.INCLUDE :  $(PRJ)$/util$/makefile.pmk

.IF "$(ENABLE_PRESENTER_SCREEN)" != "YES"
@all:
    @echo "Presenter Screen build disabled."
.ELSE
DLLPRE=
common_build_zip=

PRIVATERDB = slideshow.rdb
UNOUCRRDB = $(SOLARBINDIR)$/$(PRIVATERDB)
UNOUCRDEP = $(UNOUCRRDB)
UNOUCROUT = $(OUT)$/inc

CPPUMAKERFLAGS += -C -X$(SOLARBINDIR)$/types.rdb

# --- Files -------------------------------------

SLOFILES=										\
    $(SLO)$/PresenterAnimation.obj				\
    $(SLO)$/PresenterAnimator.obj				\
    $(SLO)$/PresenterBitmapContainer.obj		\
    $(SLO)$/PresenterClock.obj					\
    $(SLO)$/PresenterConfigurationAccess.obj	\
    $(SLO)$/PresenterControlCreator.obj			\
    $(SLO)$/PresenterController.obj				\
    $(SLO)$/PresenterCurrentSlideObserver.obj	\
    $(SLO)$/PresenterFrameworkObserver.obj		\
    $(SLO)$/PresenterGeometryHelper.obj			\
    $(SLO)$/PresenterHelper.obj					\
    $(SLO)$/PresenterHelpView.obj				\
    $(SLO)$/PresenterNotesView.obj				\
    $(SLO)$/PresenterPane.obj					\
    $(SLO)$/PresenterPaneAnimator.obj			\
    $(SLO)$/PresenterPaneBase.obj				\
    $(SLO)$/PresenterPaneBorderManager.obj		\
    $(SLO)$/PresenterPaneBorderPainter.obj		\
    $(SLO)$/PresenterPaneContainer.obj			\
    $(SLO)$/PresenterPaneFactory.obj			\
    $(SLO)$/PresenterProtocolHandler.obj		\
    $(SLO)$/PresenterScreen.obj					\
    $(SLO)$/PresenterScrollBar.obj				\
    $(SLO)$/PresenterSlidePreview.obj			\
    $(SLO)$/PresenterSlideShowView.obj			\
    $(SLO)$/PresenterSlideSorter.obj			\
    $(SLO)$/PresenterSprite.obj					\
    $(SLO)$/PresenterSpritePane.obj				\
    $(SLO)$/PresenterTheme.obj					\
    $(SLO)$/PresenterToolBar.obj				\
    $(SLO)$/PresenterViewFactory.obj			\
    $(SLO)$/PresenterWindowManager.obj			\
    $(SLO)$/PresenterComponent.obj

#$(SLO)$/PresenterAboutDialog.obj			\


DESCRIPTION:=$(MISC)$/$(TARGET)$/description.xml

.IF "$(GUI)" == "WIN" || "$(GUI)" == "WNT"
PACKLICS:=$(foreach,i,$(alllangiso) $(MISC)$/$(TARGET)$/registry$/license_$i)
.ELSE
PACKLICS:=$(foreach,i,$(alllangiso) $(MISC)$/$(TARGET)$/registry$/LICENSE_$i)
.ENDIF


# --- Library -----------------------------------

SHL1TARGET=		$(TARGET).uno

SHL1STDLIBS=	$(COMPHELPERLIB)	\
                $(CPPUHELPERLIB)	\
                $(CPPULIB)			\
                $(SALLIB)			\
                $(VOSLIB)
SHL1DEPN=
SHL1IMPLIB=		i$(SHL1TARGET)
SHL1LIBS=		$(SLB)$/$(TARGET).lib
SHL1DEF=		$(MISC)$/$(SHL1TARGET).def
SHL1VERSIONMAP=	exports.map
DEF1NAME=		$(SHL1TARGET)

COMPONENT_FILES=											\
    $(MISC)$/$(TARGET)$/configuration$/Jobs.xcu				\
    $(MISC)$/$(TARGET)$/configuration$/Addons.xcu			\
    $(MISC)$/$(TARGET)$/configuration$/ProtocolHandler.xcu	\
    $(MISC)$/$(TARGET)$/configuration$/PresenterScreen.xcs	\
    $(MISC)$/$(TARGET)$/configuration$/PresenterScreen.xcu

COMPONENT_BITMAPS=												\
    $(MISC)$/$(TARGET)$/bitmaps$/TitleTopPaneBorder.png			\
    $(MISC)$/$(TARGET)$/bitmaps$/TitleTopLeftPaneBorder.png		\
    $(MISC)$/$(TARGET)$/bitmaps$/TitleTopRightPaneBorder.png	\
    $(MISC)$/$(TARGET)$/bitmaps$/TopPaneBorder.png				\
    $(MISC)$/$(TARGET)$/bitmaps$/TopLeftPaneBorder.png			\
    $(MISC)$/$(TARGET)$/bitmaps$/TopRightPaneBorder.png			\
    $(MISC)$/$(TARGET)$/bitmaps$/LeftPaneBorder.png				\
    $(MISC)$/$(TARGET)$/bitmaps$/RightPaneBorder.png			\
    $(MISC)$/$(TARGET)$/bitmaps$/BottomLeftPaneBorder.png		\
    $(MISC)$/$(TARGET)$/bitmaps$/BottomRightPaneBorder.png		\
    $(MISC)$/$(TARGET)$/bitmaps$/BottomPaneBorder.png			\
                                                                \
    $(MISC)$/$(TARGET)$/bitmaps$/NextEffectButton.png			\
    $(MISC)$/$(TARGET)$/bitmaps$/NextEffectButtonGlow.png		\
    $(MISC)$/$(TARGET)$/bitmaps$/NextEffectButtonPressed.png	\
    $(MISC)$/$(TARGET)$/bitmaps$/NextSlideButton.png			\
    $(MISC)$/$(TARGET)$/bitmaps$/NextSlideButtonGlow.png		\
    $(MISC)$/$(TARGET)$/bitmaps$/NextSlideButtonPressed.png		\
    $(MISC)$/$(TARGET)$/bitmaps$/PrevSlideButton.png			\
    $(MISC)$/$(TARGET)$/bitmaps$/PrevSlideButtonGlow.png		\
    $(MISC)$/$(TARGET)$/bitmaps$/PrevSlideButtonPressed.png		\
                                                                \
    $(MISC)$/$(TARGET)$/bitmaps$/Left.png						\
    $(MISC)$/$(TARGET)$/bitmaps$/Right.png						\
    $(MISC)$/$(TARGET)$/bitmaps$/Up.png							\
    $(MISC)$/$(TARGET)$/bitmaps$/Down.png						\
                                                                \
    $(MISC)$/$(TARGET)$/bitmaps$/PagerHorizontal.png			\
    $(MISC)$/$(TARGET)$/bitmaps$/PagerVertical.png				\
                                                                \
    $(MISC)$/$(TARGET)$/bitmaps$/ThumbLeft.png					\
    $(MISC)$/$(TARGET)$/bitmaps$/ThumbRight.png					\
    $(MISC)$/$(TARGET)$/bitmaps$/ThumbTop.png					\
    $(MISC)$/$(TARGET)$/bitmaps$/ThumbBottom.png				\
    $(MISC)$/$(TARGET)$/bitmaps$/ThumbHorizontal.png			\
    $(MISC)$/$(TARGET)$/bitmaps$/ThumbVertical.png


#$(MISC)$/$(TARGET)$/bitmaps$/Clock-Face.png					
#$(MISC)$/$(TARGET)$/bitmaps$/Clock-Hour-Hand.png
#$(MISC)$/$(TARGET)$/bitmaps$/Clock-Minute-Hand.png
#	$(MISC)$/$(TARGET)$/bitmaps$/PagerLeft.png					\
#	$(MISC)$/$(TARGET)$/bitmaps$/PagerRight.png					\
#	$(MISC)$/$(TARGET)$/bitmaps$/PagerTop.png					\
#	$(MISC)$/$(TARGET)$/bitmaps$/PagerBottom.png				\

COMPONENT_MANIFEST= 							\
    $(MISC)$/$(TARGET)$/META-INF$/manifest.xml

COMPONENT_LIBRARY= 								\
    $(MISC)$/$(TARGET)$/$(TARGET).uno$(DLLPOST)

ZIP1DEPS=					\
    $(PACKLICS) 			\
    $(DESCRIPTION)			\
    $(COMPONENT_MANIFEST)	\
    $(COMPONENT_FILES)		\
    $(COMPONENT_BITMAPS)	\
    $(COMPONENT_LIBRARY)

ZIP1TARGET=		presenter-screen
ZIP1DIR=		$(MISC)$/$(TARGET)
ZIP1EXT=		.oxt
ZIP1FLAGS=-r
ZIP1LIST=		*

# --- Targets ----------------------------------

.INCLUDE : target.mk

$(COMPONENT_MANIFEST) : $$(@:f)
    @-$(MKDIRHIER) $(@:d)
    +$(TYPE) $< | $(SED) "s/SHARED_EXTENSION/$(DLLPOST)/" > $@

$(COMPONENT_FILES) : $$(@:f)
    @-$(MKDIRHIER) $(@:d)
    +$(COPY) $< $@

$(COMPONENT_BITMAPS) : bitmaps$/$$(@:f)
    @-$(MKDIRHIER) $(@:d)
    +$(COPY) $< $@

#$(COMPONENT_HELP) : help$/$$(@:f)
#	@-$(MKDIRHIER) $(@:d)
#    +$(COPY) $< $@

$(COMPONENT_LIBRARY) : $(DLLDEST)$/$$(@:f)
    @-$(MKDIRHIER) $(@:d)
    +$(COPY) $< $@

.IF "$(GUI)" == "WIN" || "$(GUI)" == "WNT"
$(PACKLICS) : $(SOLARBINDIR)$/osl$/license$$(@:b:s/_/./:e:s/./_/)$$(@:e).txt
    @@-$(MKDIRHIER) $(@:d)
    $(GNUCOPY) $< $@
.ELSE
$(PACKLICS) : $(SOLARBINDIR)$/osl$/LICENSE$$(@:b:s/_/./:e:s/./_/)$$(@:e)
    @@-$(MKDIRHIER) $(@:d)
    $(GNUCOPY) $< $@
.ENDIF

.INCLUDE .IGNORE : $(MISC)$/$(TARGET)_lang_track.mk
.IF "$(LAST_WITH_LANG)"!="$(WITH_LANG)"
PHONYDESC=.PHONY
.ENDIF			# "$(LAST_WITH_LANG)"!="$(WITH_LANG)"
$(DESCRIPTION) $(PHONYDESC) : $$(@:f)
    @@-$(MKDIRHIER) $(@:d)
    $(PERL) $(SOLARENV)$/bin$/licinserter.pl description.xml registry/LICENSE_xxx $@
    @echo LAST_WITH_LANG=$(WITH_LANG) > $(MISC)$/$(TARGET)_lang_track.mk

.ENDIF # "$(ENABLE_PRESENTER_SCREEN)" != "YES"
