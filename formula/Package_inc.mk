#**************************************************************
#  
#  Licensed to the Apache Software Foundation (ASF) under one
#  or more contributor license agreements.  See the NOTICE file
#  distributed with this work for additional information
#  regarding copyright ownership.  The ASF licenses this file
#  to you under the Apache License, Version 2.0 (the
#  "License"); you may not use this file except in compliance
#  with the License.  You may obtain a copy of the License at
#  
#    http://www.apache.org/licenses/LICENSE-2.0
#  
#  Unless required by applicable law or agreed to in writing,
#  software distributed under the License is distributed on an
#  "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
#  KIND, either express or implied.  See the License for the
#  specific language governing permissions and limitations
#  under the License.
#  
#**************************************************************



$(eval $(call gb_Package_Package,formula_inc,$(SRCDIR)/formula/inc))
$(eval $(call gb_Package_add_file,formula_inc,inc/formula/ExternalReferenceHelper.hxx,formula/ExternalReferenceHelper.hxx))
$(eval $(call gb_Package_add_file,formula_inc,inc/formula/FormulaCompiler.hxx,formula/FormulaCompiler.hxx))
$(eval $(call gb_Package_add_file,formula_inc,inc/formula/FormulaOpCodeMapperObj.hxx,formula/FormulaOpCodeMapperObj.hxx))
$(eval $(call gb_Package_add_file,formula_inc,inc/formula/IControlReferenceHandler.hxx,formula/IControlReferenceHandler.hxx))
$(eval $(call gb_Package_add_file,formula_inc,inc/formula/IFunctionDescription.hxx,formula/IFunctionDescription.hxx))
$(eval $(call gb_Package_add_file,formula_inc,inc/formula/compiler.hrc,formula/compiler.hrc))
$(eval $(call gb_Package_add_file,formula_inc,inc/formula/errorcodes.hxx,formula/errorcodes.hxx))
$(eval $(call gb_Package_add_file,formula_inc,inc/formula/formdata.hxx,formula/formdata.hxx))
$(eval $(call gb_Package_add_file,formula_inc,inc/formula/formula.hxx,formula/formula.hxx))
$(eval $(call gb_Package_add_file,formula_inc,inc/formula/formuladllapi.h,formula/formuladllapi.h))
$(eval $(call gb_Package_add_file,formula_inc,inc/formula/formulahelper.hxx,formula/formulahelper.hxx))
$(eval $(call gb_Package_add_file,formula_inc,inc/formula/funcutl.hxx,formula/funcutl.hxx))
$(eval $(call gb_Package_add_file,formula_inc,inc/formula/grammar.hxx,formula/grammar.hxx))
$(eval $(call gb_Package_add_file,formula_inc,inc/formula/intruref.hxx,formula/intruref.hxx))
$(eval $(call gb_Package_add_file,formula_inc,inc/formula/opcode.hxx,formula/opcode.hxx))
$(eval $(call gb_Package_add_file,formula_inc,inc/formula/token.hxx,formula/token.hxx))
$(eval $(call gb_Package_add_file,formula_inc,inc/formula/tokenarray.hxx,formula/tokenarray.hxx))
