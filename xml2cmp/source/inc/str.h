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



#ifndef CPV_STR_H
#define CPV_STR_H


#ifdef UNX
#define stricmp(str1,str2)      strcasecmp(str1, str2)
#define strnicmp(str1,str2,n)   strncasecmp(str1, str2, n)
#endif


typedef struct Cstring
{
    char *              dpText;
    intt                nLength;

} Cstring;

#define Cstring_THIS        Cstring * pThis


void                Cstring_CTOR( Cstring_THIS,
                        char *              pText );
void                Cstring_DTOR( Cstring * pThis );

void                Cs_Assign( Cstring_THIS,
                        char *              i_pNewText );
void                Cs_AssignPart( Cstring_THIS,
                        char *              i_pNewText,
                        intt                i_nLength );

void                Cs_AddCs( Cstring_THIS,
                        Cstring *           i_pAddedText );
void                Cs_Add( Cstring_THIS,
                        char *              i_pAddedText );

char *              Cs_Str( Cstring_THIS );
intt                Cs_Length( Cstring_THIS );

void                Cs_ToUpper( Cstring_THIS );



#endif

