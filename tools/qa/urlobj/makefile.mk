 #*************************************************************************
 #
 # Version: MPL 1.1 / GPLv3+ / LGPLv3+
 #
 # The contents of this file are subject to the Mozilla Public License Version
 # 1.1 (the "License"); you may not use this file except in compliance with
 # the License. You may obtain a copy of the License at
 # http://www.mozilla.org/MPL/
 #
 # Software distributed under the License is distributed on an "AS IS" basis,
 # WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 # for the specific language governing rights and limitations under the
 # License.
 #
 # The Initial Developer of the Original Code is
 #       [ insert your name / company etc. here eg. Jim Bob <jim@bob.org> ]
 # Portions created by the Initial Developer are Copyright (C) 2010 the
 # Initial Developer. All Rights Reserved.
 #
 # Contributor(s): Florian Reuter <freuter@novell.com>
 #
 # Alternatively, the contents of this file may be used under the terms of
 # either the GNU General Public License Version 3 or later (the "GPLv3+"), or
 # the GNU Lesser General Public License Version 3 or later (the "LGPLv3+"),
 # in which case the provisions of the GPLv3+ or the LGPLv3+ are applicable
 # instead of those above.
 #*************************************************************************

 PRJ=..$/..

 PRJNAME=tools
 TARGET=qa_tools_urlobj_test
 # this is removed at the moment because we need some enhancements
 # TESTDIR=TRUE

 ENABLE_EXCEPTIONS=TRUE

 # --- Settings -----------------------------------------------------

 .INCLUDE :  settings.mk

 # BEGIN ----------------------------------------------------------------
 # auto generated Target:job by codegen.pl
 SHL1OBJS=  \
    $(SLO)$/tools_urlobj_test.obj

 SHL1TARGET= tools_urlobj
 SHL1STDLIBS=\
    $(SALLIB) \
    $(CPPUNITLIB) \
    $(TOOLSLIB)

 SHL1IMPLIB= i$(SHL1TARGET)
 DEF1NAME    =$(SHL1TARGET)
 SHL1VERSIONMAP= export.map
 # auto generated Target:job
 # END ------------------------------------------------------------------

 #------------------------------- All object files -------------------------------
 # do this here, so we get right dependencies
 # SLOFILES=$(SHL1OBJS)

 # --- Targets ------------------------------------------------------

 .INCLUDE :  target.mk
 .INCLUDE : _cppunit.mk
