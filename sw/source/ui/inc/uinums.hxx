/*************************************************************************
 *
 *  $RCSfile: uinums.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:14:43 $
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
#ifndef _UINUMS_HXX
#define _UINUMS_HXX

#ifndef _SVARRAY_HXX //autogen
#include <svtools/svarray.hxx>
#endif

#ifndef _NUMRULE_HXX
#include <numrule.hxx>
#endif
#ifndef _CFGID_H
#include <cfgid.h>
#endif

class SfxPoolItem;
class SwWrtShell;
class SvStream;

#define MAX_NUM_RULES 9

typedef SfxPoolItem* SfxPoolItemPtr;
SV_DECL_PTRARR_DEL( _SwNumFmtsAttrs, SfxPoolItemPtr, 5,0 )

//------------------------------------------------------------------------
class SwNumRulesWithName
{
    String aName;
    // die Formate der NumRule muessen! unabhaengig von einem Document sein
    // (Sie sollen immer vorhanden sein!)
    class _SwNumFmtGlobal
    {
        SwNumFmt aFmt;
        String sCharFmtName;
        USHORT nCharPoolId;
        _SwNumFmtsAttrs aItems;

        _SwNumFmtGlobal& operator=( const _SwNumFmtGlobal& );

    public:
        _SwNumFmtGlobal( const SwNumFmt& rFmt );
        _SwNumFmtGlobal( const _SwNumFmtGlobal& );
        _SwNumFmtGlobal( SvStream&, USHORT nVersion );
        ~_SwNumFmtGlobal();

        void Store( SvStream& );
        void ChgNumFmt( SwWrtShell& rSh, SwNumFmt& rChg ) const;
    };

    _SwNumFmtGlobal* aFmts[ MAXLEVEL ];
protected:
    void SetName(const String& rSet) {aName = rSet;}

public:
    SwNumRulesWithName(const SwNumRule &, const String &);
    SwNumRulesWithName( const SwNumRulesWithName & );
    SwNumRulesWithName(SvStream &, USHORT nVersion);
    ~SwNumRulesWithName();

    const SwNumRulesWithName &operator=(const SwNumRulesWithName &);

    const String& GetName() const               { return aName; }
    void MakeNumRule( SwWrtShell& rSh, SwNumRule& rChg ) const;

    void Store( SvStream& );
};
/********************************************************************

********************************************************************/
class SwBaseNumRules
{
public:
    enum { nMaxRules = MAX_NUM_RULES };         // zur Zeit 9 definierte Forms
protected:
    SwNumRulesWithName  *pNumRules[ MAX_NUM_RULES ];
    String              sFileName;
    USHORT              nVersion;
     BOOL               bModified;

    virtual int         Load(SvStream&);
    virtual BOOL        Store(SvStream&);

    void                Init();

public:
    SwBaseNumRules(const String& rFileName);
    virtual ~SwBaseNumRules();

    inline const SwNumRulesWithName*    GetRules(USHORT nIdx) const;
    const SwNumRulesWithName*           GetRules(const String &rName) const;
    virtual void                        ApplyNumRules(
                                                const SwNumRulesWithName &rCopy,
                                                USHORT nIdx);

};

/********************************************************************

********************************************************************/
class SwChapterNumRules : public SwBaseNumRules
{

public:
    SwChapterNumRules();
    virtual ~SwChapterNumRules();

    virtual String      GetName() const;
    virtual void        ApplyNumRules(  const SwNumRulesWithName &rCopy,
                                            USHORT nIdx);
};

// INLINE METHODE --------------------------------------------------------
inline const SwNumRulesWithName *SwBaseNumRules::GetRules(USHORT nIdx) const
{
    ASSERT(nIdx < nMaxRules, Array der NumRules ueberindiziert.);
    return pNumRules[nIdx];
}

#endif
