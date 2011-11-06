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


#ifndef _FRMINF_HXX
#define _FRMINF_HXX

#include "swtypes.hxx"

class SwTxtFrm;     // SwTxtFrmInfo
class SwPaM;        // SwTxtFrmInfo
class SwTxtCursor;  // SwTxtFrmInfo


/*************************************************************************
 *                      class SwTxtFrmInfo
 *************************************************************************/

class SwTxtFrmInfo
{
    const SwTxtFrm *pFrm;

    // Wo beginnt der Text (ohne whitespaces)? (Dokument global !!)
    SwTwips GetLineStart( const SwTxtCursor &rLine ) const;

public:
    inline SwTxtFrmInfo( const SwTxtFrm *pTxtFrm ) : pFrm(pTxtFrm) { }

    // Passt der Absatz in eine Zeile?
    sal_Bool IsOneLine() const;

    // Ist die Zeile zu X% gefuellt?
    sal_Bool IsFilled( const sal_uInt8 nPercent ) const;

    // Wo beginnt der Text (ohne whitespaces)? (rel. im Frame !!)
    SwTwips GetLineStart() const;

    //returne die mittel Position des n. Charakters
    SwTwips GetCharPos( xub_StrLen nChar, sal_Bool bCenter = sal_True ) const;

    // Sammelt die whitespaces am Zeilenbeginn und -ende im Pam
    void GetSpaces( SwPaM &rPam, sal_Bool bWithLineBreak ) const;

    // Ist an der ersten Textposition ein Bullet/Symbol etc?
    sal_Bool IsBullet( xub_StrLen nTxtPos ) const;

    // Ermittelt Erstzeileneinzug
    SwTwips GetFirstIndent() const;

    // setze und erfrage den Frame;
    const SwTxtFrm* GetFrm() const { return pFrm; }
    SwTxtFrmInfo& SetFrm( const SwTxtFrm* pNew )
        { pFrm = pNew; return *this; }

    // liegt eine Gegenueberstellung vor? (returnt Pos im Frame)
    sal_uInt16 GetBigIndent( xub_StrLen& rFndPos,
                        const SwTxtFrm *pNextFrm = 0 ) const;
};



#endif

