###############################################################
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
###############################################################



$(eval $(call gb_Zip_Zip,ridl_src,$(SRCDIR)/ridljar/java/ridl/src/main/java))

$(eval $(call gb_Zip_add_files,ridl_src,\
	com/sun/star/lib/uno/typedesc/MethodDescription.java \
	com/sun/star/lib/uno/typedesc/MemberDescriptionHelper.java \
	com/sun/star/lib/uno/typedesc/FieldDescription.java \
	com/sun/star/lib/uno/typeinfo/MemberTypeInfo.java \
	com/sun/star/lib/uno/typeinfo/ParameterTypeInfo.java \
	com/sun/star/lib/uno/typeinfo/TypeInfo.java \
	com/sun/star/lib/uno/typeinfo/AttributeTypeInfo.java \
	com/sun/star/lib/uno/typeinfo/ConstantTypeInfo.java \
	com/sun/star/lib/uno/typeinfo/MethodTypeInfo.java \
	com/sun/star/lib/util/WeakMap.java \
	com/sun/star/lib/util/DisposeNotifier.java \
	com/sun/star/lib/util/DisposeListener.java \
	com/sun/star/uno/Type.java \
	com/sun/star/uno/IMemberDescription.java \
	com/sun/star/uno/UnoRuntime.java \
	com/sun/star/uno/IMapping.java \
	com/sun/star/uno/Union.java \
	com/sun/star/uno/IEnvironment.java \
	com/sun/star/uno/IQueryInterface.java \
	com/sun/star/uno/Any.java \
	com/sun/star/uno/IBridge.java \
	com/sun/star/uno/IMethodDescription.java \
	com/sun/star/uno/ITypeDescription.java \
	com/sun/star/uno/IFieldDescription.java \
	com/sun/star/uno/Enum.java \
))

# vim: set noet sw=4 ts=4:
