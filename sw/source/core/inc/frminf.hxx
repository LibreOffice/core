/*************************************************************************
 *
 *  $RCSfile: frminf.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-19 00:08:20 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/
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
    inline SwTxtFrmInfo( const SwTxtFrm *pFrm ) : pFrm(pFrm) { }

    // Passt der Absatz in eine Zeile?
    BOOL IsOneLine() const;

    // Ist die Zeile zu X% gefuellt?
    BOOL IsFilled( const BYTE nPercent ) const;

    // Wo beginnt der Text (ohne whitespaces)? (rel. im Frame !!)
    SwTwips GetLineStart() const;

    //returne die mittel Position des n. Charakters
    SwTwips GetCharPos( xub_StrLen nChar, BOOL bCenter = TRUE ) const;

    // Sammelt die whitespaces am Zeilenbeginn und -ende im Pam
    void GetSpaces( SwPaM &rPam, BOOL bWithLineBreak ) const;

    // Ist an der ersten Textposition ein Bullet/Symbol etc?
    BOOL IsBullet( xub_StrLen nTxtPos ) const;

    // Ermittelt Erstzeileneinzug
    SwTwips GetFirstIndent() const;

    // setze und erfrage den Frame;
    const SwTxtFrm* GetFrm() const { return pFrm; }
    SwTxtFrmInfo& SetFrm( const SwTxtFrm* pNew )
        { pFrm = pNew; return *this; }

    // liegt eine Gegenueberstellung vor? (returnt Pos im Frame)
    USHORT GetBigIndent( xub_StrLen& rFndPos,
                        const SwTxtFrm *pNextFrm = 0 ) const;
};



#endif

