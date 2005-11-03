#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.91 $
#
#   last change: $Author: kz $ $Date: 2005-11-03 14:28:17 $
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

PRJ=..
PRJPCH=

PRJNAME=offapi
TARGET=offapi_db


# --- Settings -----------------------------------------------------

.INCLUDE :  makefile.pmk

# ------------------------------------------------------------------

#regcheck .PHONY:
#	$(COMSPEC) /c checkrdb.btm o:\comptest\swt614.rdb $(PRJ)$/$(OUTPATH)$/bin$/$(PRJNAME).rdb

# ------------------------------------------------------------------

#UNOIDLDBREGS= \
#	$(SOLARBINDIR)$/udkapi.rdb

#i20156 - add xmlsecurity module related
UNOIDLDBFILES= \
    $(UCR)$/cssauth.db \
    $(UCR)$/cssawt.db \
    $(UCR)$/csschart.db \
    $(UCR)$/csschart2.db \
    $(UCR)$/cssconfiguration.db \
    $(UCR)$/csscbootstrap.db\
    $(UCR)$/csscbackend.db\
    $(UCR)$/csscbxml.db\
    $(UCR)$/cssdatatransfer.db \
    $(UCR)$/cssdeployment.db \
    $(UCR)$/cssdeploymentui.db \
    $(UCR)$/cssdclipboard.db \
    $(UCR)$/cssddnd.db \
    $(UCR)$/cssdocument.db \
    $(UCR)$/cssdrawing.db \
    $(UCR)$/cssgeometry.db \
    $(UCR)$/cssrendering.db \
    $(UCR)$/cssembed.db \
    $(UCR)$/cssfcontrol.db \
    $(UCR)$/cssform.db \
    $(UCR)$/cssxforms.db \
    $(UCR)$/cssfcomp.db \
    $(UCR)$/cssfbinding.db \
    $(UCR)$/cssfvalidation.db \
    $(UCR)$/cssfsubmission.db \
    $(UCR)$/cssformula.db \
    $(UCR)$/cssgraphic.db \
    $(UCR)$/cssinstallation.db \
    $(UCR)$/cssi18n.db \
    $(UCR)$/cssldap.db \
    $(UCR)$/csslinguistic2.db \
    $(UCR)$/cssmail.db \
    $(UCR)$/cssmedia.db \
    $(UCR)$/cssmozilla.db \
    $(UCR)$/csspackages.db \
    $(UCR)$/cssmanifest.db \
    $(UCR)$/csszippackage.db \
    $(UCR)$/cssplugin.db \
    $(UCR)$/csspresentation.db \
    $(UCR)$/cssanimations.db \
    $(UCR)$/cssresource.db \
    $(UCR)$/csssax.db \
    $(UCR)$/cssdom.db \
    $(UCR)$/cssxpath.db \
    $(UCR)$/cssdomviews.db \
    $(UCR)$/cssdomevents.db \
    $(UCR)$/cssscanner.db \
    $(UCR)$/csssdb.db \
    $(UCR)$/csssdbc.db \
    $(UCR)$/csssdbcx.db \
    $(UCR)$/csssheet.db \
    $(UCR)$/cssstyle.db \
    $(UCR)$/csssystem.db\
    $(UCR)$/csstable.db \
    $(UCR)$/csstask.db \
    $(UCR)$/csstext.db \
    $(UCR)$/csstextfield.db \
    $(UCR)$/csstextfielddocinfo.db \
    $(UCR)$/cssfieldmaster.db \
    $(UCR)$/cssucb.db \
    $(UCR)$/cssutil.db \
    $(UCR)$/cssview.db \
    $(UCR)$/csssvg.db \
    $(UCR)$/csssync.db \
    $(UCR)$/csssync2.db \
    $(UCR)$/cssxml.db \
    $(UCR)$/cssxmlinput.db \
    $(UCR)$/csssetup.db \
    $(UCR)$/cssimage.db\
    $(UCR)$/cssuidialogs.db \
    $(UCR)$/cssui.db \
    $(UCR)$/cssframe.db \
    $(UCR)$/cssfstatus.db \
    $(UCR)$/cssgraphic.db \
    $(UCR)$/cssaccessibility.db\
    $(UCR)$/csssprovider.db \
    $(UCR)$/csssbrowse.db \
    $(UCR)$/cssgallery.db \
    $(UCR)$/cssxsd.db \
    $(UCR)$/dcssform.db \
    $(UCR)$/xsec-security.db \
    $(UCR)$/xsec-crypto.db \
    $(UCR)$/xsec-csax.db \
    $(UCR)$/xsec-sax.db \
    $(UCR)$/xsec-wrapper.db
#i20156 - end

REFERENCE_RDB=$(PRJ)$/type_reference$/OO_20$/types.rdb
REFERENCE_DOC_RDB=$(PRJ)$/type_reference$/OO_20$/types_doc.rdb

REGISTRYCHECKFLAG=$(MISC)$/registrycheck.flag

# --- Targets ------------------------------------------------------

ALLTAR : $(UCR)$/types.db \
       $(OUT)$/ucrdoc$/types_doc.db \
       $(REGISTRYCHECKFLAG)

$(UCR)$/types.db : $(UCR)$/offapi.db $(SOLARBINDIR)$/udkapi.rdb
    +-$(RM) $(REGISTRYCHECKFLAG)
    +$(GNUCOPY) -f $(UCR)$/offapi.db $@
    +$(REGMERGE) $@ / $(SOLARBINDIR)$/udkapi.rdb

$(OUT)$/ucrdoc$/types_doc.db : $(OUT)$/ucrdoc$/offapi_doc.db $(SOLARBINDIR)$/udkapi_doc.rdb
    +-$(RM) $(REGISTRYCHECKFLAG)
    +$(GNUCOPY) -f $(OUT)$/ucrdoc$/offapi_doc.db $@
    +$(REGMERGE) $@ / $(SOLARBINDIR)$/udkapi_doc.rdb

#JSC: The type library has changed, all temporary not checked types are removed
#     and will be check from now on.
# ATTENTION: no special handling for types is allowed.
#
#SB: Once the reference rdbs are new-style registries with explicit published
# types, the "-u" should be removed again from the calls to regcompare:
$(REGISTRYCHECKFLAG) : $(UCR)$/types.db $(OUT)$/ucrdoc$/types_doc.db
    +$(REGCOMPARE) -f -t -r1 $(REFERENCE_RDB) -r2 $(UCR)$/types.db
    +$(REGCOMPARE) -f -t -r1 $(REFERENCE_DOC_RDB) -r2 $(OUT)$/ucrdoc$/types_doc.db \
        && echo > $(REGISTRYCHECKFLAG)

.INCLUDE :  target.mk
