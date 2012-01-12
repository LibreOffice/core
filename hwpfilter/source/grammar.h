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



typedef union {
    char *dval;
    char *str;
    Node *ptr;
} YYSTYPE;
#define ACCENT  257
#define SMALL_GREEK 258
#define CAPITAL_GREEK   259
#define BINARY_OPERATOR 260
#define RELATION_OPERATOR   261
#define ARROW   262
#define GENERAL_IDEN    263
#define GENERAL_OPER    264
#define BIG_SYMBOL  265
#define FUNCTION    266
#define ROOT    267
#define FRACTION    268
#define SUBSUP  269
#define EQOVER  270
#define DELIMETER   271
#define LARGE_DELIM 272
#define DECORATION  273
#define SPACE_SYMBOL    274
#define CHARACTER   275
#define STRING  276
#define OPERATOR    277
#define EQBEGIN 278
#define EQEND   279
#define EQLEFT  280
#define EQRIGHT 281
#define NEWLINE 282
#define LEFT_DELIM  283
#define RIGHT_DELIM 284
#define DIGIT   285

#ifndef _WIN32
extern YYSTYPE yylval;
#endif
