# Version: MPL 1.1 / GPLv3+ / LGPLv3+
#
# The contents of this file are subject to the Mozilla Public License Version
# 1.1 (the "License"); you may not use this file except in compliance with
# the License or as specified alternatively below. You may obtain a copy of
# the License at http://www.mozilla.org/MPL/
#
# Software distributed under the License is distributed on an "AS IS" basis,
# WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
# for the specific language governing rights and limitations under the
# License.
#
# The Initial Developer of the Original Code is
#       David Tardon, Red Hat Inc. <dtardon@redhat.com>
# Portions created by the Initial Developer are Copyright (C) 2010 the
# Initial Developer. All Rights Reserved.
#
# Major Contributor(s):
#
# For minor contributions see the git repository.
#
# Alternatively, the contents of this file may be used under the terms of
# either the GNU General Public License Version 3 or later (the "GPLv3+"), or
# the GNU Lesser General Public License Version 3 or later (the "LGPLv3+"),
# in which case the provisions of the GPLv3+ or the LGPLv3+ are applicable
# instead of those above.

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
$(eval $(call gb_Package_add_file,formula_inc,inc/formula/opcode.hxx,opcode.hxx))
$(eval $(call gb_Package_add_file,formula_inc,inc/formula/tokenarray.hxx,tokenarray.hxx))
$(eval $(call gb_Package_add_file,formula_inc,inc/formula/token.hxx,token.hxx))

# vim: set noet ts=4 sw=4:
