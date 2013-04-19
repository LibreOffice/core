# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Package_Package,formula_inc,$(SRCDIR)/formula/inc/formula))

$(eval $(call gb_Package_add_file,formula_inc,inc/formula/compiler.hrc,compiler.hrc))
$(eval $(call gb_Package_add_file,formula_inc,inc/formula/errorcodes.hxx,errorcodes.hxx))
$(eval $(call gb_Package_add_file,formula_inc,inc/formula/ExternalReferenceHelper.hxx,ExternalReferenceHelper.hxx))
$(eval $(call gb_Package_add_file,formula_inc,inc/formula/formdata.hxx,formdata.hxx))
$(eval $(call gb_Package_add_file,formula_inc,inc/formula/FormulaCompiler.hxx,FormulaCompiler.hxx))
$(eval $(call gb_Package_add_file,formula_inc,inc/formula/formuladllapi.h,formuladllapi.h))
$(eval $(call gb_Package_add_file,formula_inc,inc/formula/formulahelper.hxx,formulahelper.hxx))
$(eval $(call gb_Package_add_file,formula_inc,inc/formula/formula.hxx,formula.hxx))
$(eval $(call gb_Package_add_file,formula_inc,inc/formula/FormulaOpCodeMapperObj.hxx,FormulaOpCodeMapperObj.hxx))
$(eval $(call gb_Package_add_file,formula_inc,inc/formula/funcutl.hxx,funcutl.hxx))
$(eval $(call gb_Package_add_file,formula_inc,inc/formula/grammar.hxx,grammar.hxx))
$(eval $(call gb_Package_add_file,formula_inc,inc/formula/IControlReferenceHandler.hxx,IControlReferenceHandler.hxx))
$(eval $(call gb_Package_add_file,formula_inc,inc/formula/IFunctionDescription.hxx,IFunctionDescription.hxx))
$(eval $(call gb_Package_add_file,formula_inc,inc/formula/omoduleclient.hxx,omoduleclient.hxx))
$(eval $(call gb_Package_add_file,formula_inc,inc/formula/opcode.hxx,opcode.hxx))
$(eval $(call gb_Package_add_file,formula_inc,inc/formula/tokenarray.hxx,tokenarray.hxx))
$(eval $(call gb_Package_add_file,formula_inc,inc/formula/token.hxx,token.hxx))

# vim: set noet sw=4 ts=4:
