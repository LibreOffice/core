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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cpp.h"

/*
    Nlist *             pMacro;
    Token *             pTokenWhereMacroBecomesValid;
    struct macroValidator *
                        pNext;
*/

void
mvl_init(MacroValidatorList * out_pValidators)
{
    out_pValidators->pFirst = 0;
    out_pValidators->nextFreeIdentifier = 1;
}

void
mvl_destruct(MacroValidatorList * out_pValidators)
{
    MacroValidator * pV = out_pValidators->pFirst;
    MacroValidator * pDel;
    for ( pDel = out_pValidators->pFirst;
          pDel != 0;
          pDel = pV )
    {
        pV = pV->pNext;

        pDel->pMacro->flag &= (~ISACTIVE);
        dofree(pDel);
    }
}


#define INVALID_TILL_ENDOFROW 32000

/*  If in_pTokenWhereMacroBecomesValid == 0, the macro is at row end
    and therefore there does not exist any token, where the macro becomes
    valid again. It is revalidated, when the row was processed complete.
*/
void
mvl_add( MacroValidatorList *   inout_pValidators,
         Nlist *                in_pMacro,
         Token *                in_pTokenWhereMacroBecomesValid )
{

    MacroValidator * pNew = new(MacroValidator);
    pNew->pMacro = in_pMacro;

    if (in_pTokenWhereMacroBecomesValid == 0)
    {
        pNew->nTokenWhereMacroBecomesValid = INVALID_TILL_ENDOFROW;
    }
    else if (in_pTokenWhereMacroBecomesValid->identifier > 0)
    {
        pNew->nTokenWhereMacroBecomesValid = in_pTokenWhereMacroBecomesValid->identifier;
    }
    else
    {
        pNew->nTokenWhereMacroBecomesValid = inout_pValidators->nextFreeIdentifier;
        in_pTokenWhereMacroBecomesValid->identifier = inout_pValidators->nextFreeIdentifier;
        inout_pValidators->nextFreeIdentifier++;
    }

    pNew->pNext = inout_pValidators->pFirst;
    inout_pValidators->pFirst = pNew;
}

/*
void
mvl_move( MacroValidatorList * inout_pValidators,
          int                  in_nSpace )
{
    MacroValidator * pV;
    for ( pV = inout_pValidators->pFirst;
          pV != 0;
          pV = pV->pNext )
    {
        pV->pTokenWhereMacroBecomesValid += in_nSpace;
    }
}
*/

void
mvl_check(  MacroValidatorList * inout_pValidators,
            Token *              inout_pTokenToCheck)
{
    MacroValidator * pV;            /* Running pointer */
    MacroValidator * pCheckedOnes;  /* Here new list is built.  */
    pCheckedOnes = 0;

    for ( pV = inout_pValidators->pFirst;
          pV != 0;
          pV = inout_pValidators->pFirst )
    {
        inout_pValidators->pFirst = pV->pNext;

        if (pV->nTokenWhereMacroBecomesValid == inout_pTokenToCheck->identifier)
        {
            pV->pMacro->flag &= (~ISACTIVE);
            dofree(pV);
        }
        else
        {
            pV->pNext = pCheckedOnes;
            pCheckedOnes = pV;
        }
    }   /* end for  */

    /* Assign new built list (too old ones were removed) to
       original list:
    */
    inout_pValidators->pFirst = pCheckedOnes;
}


void
tokenrow_zeroTokenIdentifiers(Tokenrow* trp)
{
    Token * tp;
    for (tp = trp->bp; tp < trp->lp; tp++)
    {
        tp->identifier = 0;
    }
}

