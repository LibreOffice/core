/*************************************************************************
 *
 *  $RCSfile: doctxm.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: jp $ $Date: 2000-10-20 10:53:11 $
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
#ifndef _DOCTXM_HXX
#define _DOCTXM_HXX


#ifndef _SVARRAY_HXX //autogen
#include <svtools/svarray.hxx>
#endif

#ifndef _TOX_HXX
#include <tox.hxx>
#endif
#ifndef _SECTION_HXX
#include <section.hxx>
#endif

class  SwTOXInternational;
class  SvUShorts;
class  SvStringsDtor;
class  SvPtrarr;
class  SwTxtNode;
class  SwTxtFmtColl;
struct SwPosition;
struct SwTOXSortTabBase;

typedef SwTOXSortTabBase* SwTOXSortTabBasePtr;

SV_DECL_PTRARR(SwTOXSortTabBases, SwTOXSortTabBasePtr, 0, 5 );

/*--------------------------------------------------------------------
     Beschreibung: Ring der speziellen Verzeichnisse
 --------------------------------------------------------------------*/

class SwTOXBaseSection : public SwTOXBase, public SwSection
{
    SwTOXSortTabBases aSortArr;

    void    UpdateMarks( const SwTOXInternational& rIntl,
                            const SwTxtNode* pOwnChapterNode );
    void    UpdateOutline( const SwTxtNode* pOwnChapterNode );
    void    UpdateTemplate( const SwTxtNode* pOwnChapterNode );
    void    UpdateCntnt( SwTOXElement eType,
                            const SwTxtNode* pOwnChapterNode );
    void    UpdateTable( const SwTxtNode* pOwnChapterNode );
    void    UpdateSequence( const SwTxtNode* pOwnChapterNode );
    void    UpdateAuthorities( const SwTxtNode* pOwnChapterNode,
                                        const SwTOXInternational& rIntl );
    void    UpdateAll();

    // Sortiert einfuegen ins Array fuer die Generierung
    void    InsertSorted(SwTOXSortTabBase* pBase);

    // Alpha-Trennzeichen bei der Generierung einfuegen
    void    InsertAlphaDelimitter( const SwTOXInternational& rIntl );

    // Textrumpf generieren
    void GenerateText( USHORT nArrayIdx, USHORT nCount, SvStringsDtor& );

    // Seitennummerplatzhalter gegen aktuelle Nummern austauschen
    void    _UpdatePageNum( SwTxtNode* pNd,
                            const SvUShorts& rNums,
                            const SvPtrarr &rDescs,
                            const SvUShorts* pMainEntryNums,
                            const SwTOXInternational& rIntl );

    // Bereich fuer Stichwort einfuegen suchen
    Range GetKeyRange(const String& rStr, USHORT nLevel, const Range& rRange,
                        const SwTOXInternational& rIntl );

    // returne die TextCollection ueber den Namen / aus Format-Pool
    SwTxtFmtColl* GetTxtFmtColl( USHORT nLevel );

public:
    SwTOXBaseSection( const SwTOXBase& rBase );
    virtual ~SwTOXBaseSection();

    void Update(const SfxItemSet* pAttr = 0); // Formatieren
    void UpdatePageNum();               // Seitennummern einfuegen
    TYPEINFO();                         // fuers rtti

    BOOL SetPosAtStartEnd( SwPosition& rPos, BOOL bAtStart = TRUE ) const;
};
/* -----------------02.09.99 07:52-------------------

 --------------------------------------------------*/
struct SwDefTOXBase_Impl
{
    SwTOXBase* pContBase;
    SwTOXBase* pIdxBase;
    SwTOXBase* pUserBase;
    SwTOXBase* pTblBase;
    SwTOXBase* pObjBase;
    SwTOXBase* pIllBase;
    SwTOXBase* pAuthBase;

    SwDefTOXBase_Impl() :
    pContBase(0),
    pIdxBase(0),
    pUserBase(0),
    pTblBase(0),
    pObjBase(0),
    pIllBase(0),
    pAuthBase(0)
    {}
    ~SwDefTOXBase_Impl()
    {
        delete pContBase;
        delete pIdxBase;
        delete pUserBase;
        delete pTblBase;
        delete pObjBase;
        delete pIllBase;
        delete pAuthBase;
    }

};

#endif  // _DOCTXM_HXX
