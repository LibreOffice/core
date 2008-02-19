/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: fmtfld.hxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: rt $ $Date: 2008-02-19 13:34:16 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/
#ifndef _FMTFLD_HXX
#define _FMTFLD_HXX

#ifndef _SFXPOOLITEM_HXX //autogen
#include <svtools/poolitem.hxx>
#endif

#include <svtools/smplhint.hxx>

#ifndef INCLUDED_SWDLLAPI_H
#include "swdllapi.h"
#endif
#ifndef _CALBCK_HXX //autogen
#include <calbck.hxx>
#endif

#include <svtools/brdcst.hxx>

class SwField;
class SwTxtFld;
// ATT_FLD ***********************************

class SW_DLLPUBLIC SwFmtFld : public SfxPoolItem, public SwClient, public SfxBroadcaster
{
    friend class SwTxtFld;
    friend void _InitCore();

    SwField *pField;
    SwTxtFld* pTxtAttr;     // mein TextAttribut

    SwFmtFld();             // das default-Attibut

    // geschuetzter CopyCtor
    // @@@ copy construction allowed, but copy assignment is not? @@@
    SwFmtFld& operator=(const SwFmtFld& rFld);

public:
    TYPEINFO();

    // single argument constructors shall be explicit.
    explicit SwFmtFld( const SwField &rFld );

    // @@@ copy construction allowed, but copy assignment is not? @@@
    SwFmtFld( const SwFmtFld& rAttr );

    virtual ~SwFmtFld();

    // "pure virtual Methoden" vom SfxPoolItem
    virtual int             operator==( const SfxPoolItem& ) const;
    virtual SfxPoolItem*    Clone( SfxItemPool* pPool = 0 ) const;

    virtual void Modify( SfxPoolItem* pOld, SfxPoolItem* pNew );
    virtual BOOL GetInfo( SfxPoolItem& rInfo ) const;

    const SwField *GetFld() const   { return pField; }
    SwField *GetFld()               { return pField; }

    // #111840#
    /**
       Sets current field.

       @param pField          new field

       @attention The current field will be destroyed before setting the new field.
     */
    void SetFld(SwField * pField);

    const SwTxtFld *GetTxtFld() const   { return pTxtAttr; }
    SwTxtFld *GetTxtFld()               { return pTxtAttr; }

    BOOL IsFldInDoc() const;
    BOOL IsProtect() const;
};

class SW_DLLPUBLIC SwFmtFldHint : public SfxHint
{
#define SWFMTFLD_INSERTED   1
#define SWFMTFLD_REMOVED    2
#define SWFMTFLD_FOCUS      3
#define SWFMTFLD_CHANGED    4

    const SwFmtFld* pFld;
    sal_Int16 nWhich;

public:
    SwFmtFldHint( const SwFmtFld* p, sal_Int16 n )
        : pFld(p)
        , nWhich(n)
    {}

    TYPEINFO();
    const SwFmtFld* GetField() const { return pFld; }
    sal_Int16 Which() const { return nWhich; }
};

#endif

