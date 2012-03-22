/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/

#ifndef MYSQLC_TYPES_HXX
#define MYSQLC_TYPES_HXX

struct TypeInfoDef {
    const char *typeName;
    sal_Int32 dataType;
    sal_Int32 precision;
    const char *literalPrefix;
    const char *literalSuffix;
    const char *createParams;
    sal_Int16 nullable;
    sal_Bool caseSensitive;
    sal_Int16 searchable;
    sal_Bool isUnsigned;
    sal_Bool fixedPrecScale;
    sal_Bool autoIncrement;
    const char *localTypeName;
    sal_Int32 minScale;
    sal_Int32 maxScale;
    sal_Int32 sqlDataType;
    sal_Int32 sqlDateTimeSub;
    sal_Int32 numPrecRadix;
};

extern TypeInfoDef mysqlc_types[];

#endif /* MYSQLC_TYPES_HXX */
