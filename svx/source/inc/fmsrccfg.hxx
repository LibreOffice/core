/*************************************************************************
 *
 *  $RCSfile: fmsrccfg.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:01:19 $
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

#ifndef _FMSRCCF_HXX_
#define _FMSRCCF_HXX_

#ifndef _STRING_HXX //autogen
#include <tools/string.hxx>
#endif

#ifndef _SFXCFGITEM_HXX //autogen
#include <sfx2/cfgitem.hxx>
#endif

// ===================================================================================================

#define MATCHING_ANYWHERE       0
#define MATCHING_BEGINNING      1
#define MATCHING_END            2
#define MATCHING_WHOLETEXT      3

// diese Konstanten duerfen NICHT einfach geaendert werden, da der SearchDialog aus den Einstellungen seiner ListBox
// keine Umrechnungen vornimmt, sondern die einfach in die entsprechenden Methoden der SearchEngine steckt
// (wenn man also die Konstanten oder die Reihenfolge der Listbox-Eintraege im Dialog aendert, muss das jeweils andere
// angepasst werden)


// ===================================================================================================
// = struct FmSearchParams - Parameter einer Suche
// ===================================================================================================

struct FmSearchParams
{
public:
    String  strHistory;
        // der Dialog haelt maximal MAX_HISTORY_ENTRIES Eintraege
    String  strSingleSearchField;
    INT16   nSearchForType : 2;
    UINT16  nPosition : 2;
    INT16   nLevOther;
    INT16   nLevShorter;
    INT16   nLevLonger;

    BOOL    bAllFields : 1;
    BOOL    bUseFormatter : 1;
    BOOL    bCaseSensitive : 1;
    BOOL    bBackwards : 1;
    BOOL    bWildcard : 1;
    BOOL    bRegular : 1;
    BOOL    bApproxSearch : 1;
        // die letzten 3 schliessen sich gegenseitig aus !
    // fuer Levenshtein-Suche :
    BOOL    bLevRelaxed : 1;

public:
    // Vergleich nur unter Beachtung der persistenten Eigenschaften !
    BOOL operator ==(const FmSearchParams& rComp) const;
    BOOL operator !=(const FmSearchParams& rComp) const { return !(*this == rComp); }
};

// ===================================================================================================
// = class FmSearchConfigItem - ein ConfigItem, dass sich Suchparameter merkt
// ===================================================================================================

class FmSearchConfigItem : public SfxConfigItem
{
    friend class FmSearchConfigAdmin;

private:
    FmSearchParams      m_aParams;

private:
    FmSearchConfigItem();

public:
    virtual int         Load(SvStream&);
    virtual BOOL        Store(SvStream&);
    virtual void        UseDefault();

protected:
    INT32 CalcCheckSum(const FmSearchParams&, short);
};

// ===================================================================================================
// = class FmSearchConfigAdmin - verwaltet den Zugriff auf das einzige FmSearchConfigItem, das es gibt
// ===================================================================================================

class FmSearchConfigAdmin
{
private:
    static  FmSearchConfigItem*     s_pItem;
    static  INT32                   s_nUsageCounter;

public:
    FmSearchConfigAdmin();
    ~FmSearchConfigAdmin();

public:
    FmSearchParams      GetParams() const { return GetItem()->m_aParams; }
    void                PutParams(const FmSearchParams& rParams);

protected:
    FmSearchConfigItem* GetItem() const;
};

// ===================================================================================================

#endif // _FMSRCCF_HXX_

