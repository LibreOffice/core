# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_AutoInstallLibs_AutoInstallLibs))

$(eval $(call gb_AutoInstallLibs_add_category,base,BASE,LIBO_LIB_FILE,auto_Base_Lib))
$(eval $(call gb_AutoInstallLibs_add_category,gnome,GNOME,LIBO_LIB_FILE,auto_Gnome_Lib))
$(eval $(call gb_AutoInstallLibs_add_category,graphicfilter,GRAPHICFILTER,LIBO_LIB_FILE,auto_Graphicfilter_Lib))
$(eval $(call gb_AutoInstallLibs_add_category,impress,IMPRESS,LIBO_LIB_FILE,auto_Impress_Lib))
$(eval $(call gb_AutoInstallLibs_add_category,kde,KDE,LIBO_LIB_FILE,auto_Kde_Lib))
$(eval $(call gb_AutoInstallLibs_add_category,math,MATH,LIBO_LIB_FILE,auto_Math_Lib))
$(eval $(call gb_AutoInstallLibs_add_category,onlineupdate,ONLINEUPDATE,LIBO_LIB_FILE,auto_Onlineupdate_Lib))
$(eval $(call gb_AutoInstallLibs_add_category,ooo,OOO,LIBO_LIB_FILE,auto_File_Lib))
$(eval $(call gb_AutoInstallLibs_add_category,tde,TDE,LIBO_LIB_FILE,auto_Tde_Lib))
$(eval $(call gb_AutoInstallLibs_add_category,ure,URE,URE_PRIVATE_LIB,auto_Ure_Private_Lib))
$(eval $(call gb_AutoInstallLibs_add_category,writer,WRITER,LIBO_LIB_FILE,auto_Writer_Lib))

# vim: set noet sw=4 ts=4:
