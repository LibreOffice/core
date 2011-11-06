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



#ifndef _DISAS_HXX
#define _DISAS_HXX

#include "image.hxx"
#include "opcodes.hxx"
// find a place for this limit ( also used in
class SvStream;
#define MAX_LABELS 0x2000L
class SbiDisas {
    const SbiImage& rImg;
    SbModule* pMod;
    char     cLabels[ MAX_LABELS ];     // Bitvektor fuer Labels
    sal_uInt32   nOff;                  // aktuelle Position
    sal_uInt32   nPC;                   // Position des Opcodes
    SbiOpcode eOp;                  // Opcode
    sal_uInt32   nOp1, nOp2;            // Operanden
    sal_uInt32   nParts;                // 1, 2 oder 3
    sal_uInt32   nLine;                 // aktuelle Zeile
    sal_Bool     DisasLine( String& );
    sal_Bool     Fetch();               // naechster Opcode
public:
    SbiDisas( SbModule*, const SbiImage* );
    void Disas( SvStream& );
    void Disas( String& );
                                    // NICHT AUFRUFEN
    void     StrOp( String& );
    void     Str2Op( String& );
    void     ImmOp( String& );
    void     OnOp( String& );
    void     LblOp( String& );
    void     ReturnOp( String& );
    void     ResumeOp( String& );
    void     PromptOp( String& );
    void     CloseOp( String& );
    void     CharOp( String& );
    void     VarOp( String& );
    void     VarDefOp( String& );
    void     OffOp( String& );
    void     TypeOp( String& );
    void     CaseOp( String& );
    void     StmntOp( String& );
    void     StrmOp( String& );
};

#endif
