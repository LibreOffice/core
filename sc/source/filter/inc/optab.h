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



#ifndef SC_OPTAB_H
#define SC_OPTAB_H

typedef void ( *OPCODE_FKT )( SvStream &aStream, sal_uInt16 nLaenge );

#define FKT_LIMIT   101

#define FKT_LIMIT123    101

#define LOTUS_EOF   0x01

#define LOTUS_FILEPASSWD 0x4b

#define LOTUS_PATTERN   0x284

#define LOTUS_FORMAT_INDEX 0x800

#define LOTUS_FORMAT_INFO 0x801

#define ROW_FORMAT_MARKER 0x106

#define COL_FORMAT_MARKER 0x107

#endif

